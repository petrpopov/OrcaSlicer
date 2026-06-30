# Send to Bambuddy Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a Bambu-specific **Send to Bambuddy** action that exports `.gcode.3mf`, uploads it to Bambuddy, and queues it for the selected Bambuddy printer with configurable reverse-proxy authentication.

**Architecture:** Keep this independent of the generic `PrintHost` list. Add a focused Bambuddy config/client layer, a settings UI, and a button/handler beside the existing BBL Connect flow in `SelectMachineDialog`. Reuse OrcaSlicer's existing `.gcode.3mf` export path (`Plater::send_gcode`) and send the resulting file to Bambuddy's `/api/v1/library/files` and `/api/v1/queue/` APIs.

**Tech Stack:** C++17, wxWidgets UI, OrcaSlicer `Http` helper, Boost filesystem/property_tree, Catch2 tests under `tests/`, existing OrcaSlicer/Bambu send UI.

---

## Reference

- Spec: `/Users/petrpopov/Developer/projects/main/OrcaSlicer/docs/superpowers/specs/2026-06-30-bambuddy-send-design.md`
- Bambuddy upstream inspected at commit `62f45c8c95f0b44d9fe4c402beb0813415f233e1`.
- Bambuddy API flow:
  - `POST /api/v1/library/files` multipart field `file` → response contains `id`.
  - `POST /api/v1/queue/` JSON with `library_file_id`, `printer_id`, `manual_start=false`, print options.
- OrcaSlicer export flow:
  - `src/slic3r/GUI/SelectMachine.cpp::on_bambu_connect_btn` already calls `m_plater->send_gcode(...)` and reads `PrintPrepareData::_3mf_path`.
  - `src/slic3r/GUI/Plater.cpp::send_gcode` creates the `.gcode.3mf`-style 3MF bundle used by Bambu send flows.

## File Structure

Create:

- `/Users/petrpopov/Developer/projects/main/OrcaSlicer/src/slic3r/Utils/BambuddyClient.hpp` — small structs, config value object, response types, and `BambuddyClient` public API.
- `/Users/petrpopov/Developer/projects/main/OrcaSlicer/src/slic3r/Utils/BambuddyClient.cpp` — URL building, auth injection, JSON parsing, HTTP upload/queue/test/list calls.
- `/Users/petrpopov/Developer/projects/main/OrcaSlicer/src/slic3r/GUI/BambuddySettingsDialog.hpp` — settings dialog class declarations.
- `/Users/petrpopov/Developer/projects/main/OrcaSlicer/src/slic3r/GUI/BambuddySettingsDialog.cpp` — Bambuddy settings UI, printer refresh, connection test.
- `/Users/petrpopov/Developer/projects/main/OrcaSlicer/tests/slic3r/Utils/TestBambuddyClient.cpp` — unit tests for pure helper behavior and response parsing.

Modify:

- `/Users/petrpopov/Developer/projects/main/OrcaSlicer/src/slic3r/GUI/SelectMachine.hpp` — add Bambuddy button member and handler declarations.
- `/Users/petrpopov/Developer/projects/main/OrcaSlicer/src/slic3r/GUI/SelectMachine.cpp` — place **Send to Bambuddy** next to BBL Connect and implement send handler.
- `/Users/petrpopov/Developer/projects/main/OrcaSlicer/src/slic3r/GUI/Preferences.cpp` — add entry point/button for Bambuddy settings, or embed the settings block if the existing preferences layout is straightforward.
- `/Users/petrpopov/Developer/projects/main/OrcaSlicer/src/libslic3r/AppConfig.hpp` / related app config defaults if needed — add stable config keys or section constants.
- CMake files that list GUI/Utils sources and tests. Search existing source lists before editing; likely `/Users/petrpopov/Developer/projects/main/OrcaSlicer/src/slic3r/CMakeLists.txt` and tests CMake under `/Users/petrpopov/Developer/projects/main/OrcaSlicer/tests/`.

Do not modify Bambuddy server code unless live testing proves the API is insufficient.

---

## Chunk 1: Bambuddy config and pure HTTP helpers

### Task 1: Add Bambuddy data types and helper declarations

**Files:**
- Create: `/Users/petrpopov/Developer/projects/main/OrcaSlicer/src/slic3r/Utils/BambuddyClient.hpp`
- Test: `/Users/petrpopov/Developer/projects/main/OrcaSlicer/tests/slic3r/Utils/TestBambuddyClient.cpp`

- [ ] **Step 1: Write failing helper tests**

Add tests covering URL joining and auth injection without making network calls:

```cpp
TEST_CASE("Bambuddy API URL joins base and endpoint", "[BambuddyClient]")
{
    Slic3r::BambuddyConfig cfg;
    cfg.base_url = "https://bambuddy.ezheg.xyz/";
    REQUIRE(Slic3r::BambuddyClient::build_api_url(cfg, "/library/files") ==
            "https://bambuddy.ezheg.xyz/api/v1/library/files");
}

TEST_CASE("Bambuddy Pangolin query token preserves existing query", "[BambuddyClient]")
{
    Slic3r::BambuddyConfig cfg;
    cfg.base_url = "https://bambuddy.ezheg.xyz?foo=bar";
    cfg.proxy_auth.mode = Slic3r::BambuddyProxyAuthMode::PangolinQueryToken;
    cfg.proxy_auth.pangolin_query_token = "secret";

    const std::string url = Slic3r::BambuddyClient::build_api_url(cfg, "/printers/");
    REQUIRE(url.find("p_token=secret") != std::string::npos);
}
```

If there is no existing `tests/slic3r/Utils` target, create the nearest equivalent test file/target following existing test CMake patterns.

- [ ] **Step 2: Run test to verify it fails**

Run:

```bash
cmake --build build --target tests --parallel
ctest --test-dir build --output-on-failure -R Bambuddy
```

Expected: compile fails because `BambuddyClient.hpp` does not exist or symbols are undefined.

- [ ] **Step 3: Add minimal declarations**

Create `BambuddyClient.hpp` with focused types:

```cpp
#pragma once

#include <map>
#include <string>
#include <vector>
#include <boost/filesystem/path.hpp>

namespace Slic3r {

enum class BambuddyProxyAuthMode {
    None,
    PangolinHeaders,
    PangolinQueryToken,
    CustomHeaders,
};

struct BambuddyProxyAuthConfig {
    BambuddyProxyAuthMode mode { BambuddyProxyAuthMode::None };
    std::string pangolin_token_id;
    std::string pangolin_token_secret;
    std::string pangolin_query_token;
    std::map<std::string, std::string> custom_headers;
};

struct BambuddyConfig {
    bool enabled { false };
    std::string base_url;
    std::string api_key;
    int default_printer_id { 0 };
    std::string default_printer_name;
    BambuddyProxyAuthConfig proxy_auth;
};

struct BambuddyPrinter {
    int id { 0 };
    std::string name;
    std::string model;
    bool is_active { true };
};

struct BambuddyUploadResult {
    int library_file_id { 0 };
    std::string filename;
};

struct BambuddyQueueResult {
    int queue_item_id { 0 };
};

struct BambuddyPrintOptions {
    bool insert_at_top { true };
    bool manual_start { false };
    bool bed_levelling { true };
    bool flow_cali { false };
    bool vibration_cali { true };
    bool layer_inspect { false };
    bool timelapse { false };
    bool use_ams { true };
};

class BambuddyClient
{
public:
    explicit BambuddyClient(BambuddyConfig config);

    static std::string build_api_url(const BambuddyConfig& config, const std::string& api_path);
    static std::map<std::string, std::string> build_headers(const BambuddyConfig& config);
    static bool looks_like_html_login(const std::string& body, const std::string& content_type = {});

    bool test_connection(std::string& error) const;
    bool list_printers(std::vector<BambuddyPrinter>& printers, std::string& error) const;
    bool upload_file(const boost::filesystem::path& path, BambuddyUploadResult& result, std::string& error) const;
    bool enqueue_print(int library_file_id, int printer_id, const BambuddyPrintOptions& options,
                       BambuddyQueueResult& result, std::string& error) const;

private:
    BambuddyConfig m_config;
};

} // namespace Slic3r
```

- [ ] **Step 4: Run compile to verify expected missing implementation errors**

Run the same build/test command. Expected: declarations compile, but link fails for helper functions.

- [ ] **Step 5: Commit**

```bash
git add src/slic3r/Utils/BambuddyClient.hpp tests/slic3r/Utils/TestBambuddyClient.cpp
git commit -m "Add Bambuddy client interface"
```

### Task 2: Implement helper functions and tests

**Files:**
- Create/Modify: `/Users/petrpopov/Developer/projects/main/OrcaSlicer/src/slic3r/Utils/BambuddyClient.cpp`
- Modify: CMake source list(s) for new file and tests.
- Test: `/Users/petrpopov/Developer/projects/main/OrcaSlicer/tests/slic3r/Utils/TestBambuddyClient.cpp`

- [ ] **Step 1: Add failing tests for headers and HTML detection**

```cpp
TEST_CASE("Bambuddy headers include API key and Pangolin header token", "[BambuddyClient]")
{
    Slic3r::BambuddyConfig cfg;
    cfg.api_key = "bb_secret";
    cfg.proxy_auth.mode = Slic3r::BambuddyProxyAuthMode::PangolinHeaders;
    cfg.proxy_auth.pangolin_token_id = "id1";
    cfg.proxy_auth.pangolin_token_secret = "token1";

    const auto headers = Slic3r::BambuddyClient::build_headers(cfg);
    REQUIRE(headers.at("X-API-Key") == "bb_secret");
    REQUIRE(headers.at("P-Access-Token-Id") == "id1");
    REQUIRE(headers.at("P-Access-Token") == "token1");
}

TEST_CASE("Bambuddy detects reverse proxy HTML login", "[BambuddyClient]")
{
    REQUIRE(Slic3r::BambuddyClient::looks_like_html_login("<!DOCTYPE html><html><body>login</body></html>", "text/html"));
    REQUIRE_FALSE(Slic3r::BambuddyClient::looks_like_html_login("{\"id\":1}", "application/json"));
}
```

- [ ] **Step 2: Run tests to verify failure**

Run:

```bash
cmake --build build --target tests --parallel
ctest --test-dir build --output-on-failure -R Bambuddy
```

Expected: fails due missing implementations.

- [ ] **Step 3: Implement minimal helpers**

Implement:

- trim trailing `/` from base URL;
- append `/api/v1` unless the user already entered a URL ending in `/api/v1`;
- append endpoint;
- inject `p_token` after existing query params;
- build headers with `X-API-Key`, Pangolin headers, and custom headers;
- detect HTML by content type or leading `<!doctype` / `<html`.

Keep helpers pure and unit-testable. Do not log secrets.

- [ ] **Step 4: Run helper tests**

Run:

```bash
cmake --build build --target tests --parallel
ctest --test-dir build --output-on-failure -R Bambuddy
```

Expected: Bambuddy helper tests pass.

- [ ] **Step 5: Commit**

```bash
git add src/slic3r/Utils/BambuddyClient.cpp tests/slic3r/Utils/TestBambuddyClient.cpp CMakeLists.txt src/slic3r/CMakeLists.txt tests/CMakeLists.txt
git commit -m "Implement Bambuddy client helpers"
```

---

## Chunk 2: Bambuddy HTTP API methods

### Task 3: Implement `test_connection()` and `list_printers()`

**Files:**
- Modify: `/Users/petrpopov/Developer/projects/main/OrcaSlicer/src/slic3r/Utils/BambuddyClient.cpp`
- Modify: `/Users/petrpopov/Developer/projects/main/OrcaSlicer/src/slic3r/Utils/BambuddyClient.hpp` if response structs need adjustment.
- Test: `/Users/petrpopov/Developer/projects/main/OrcaSlicer/tests/slic3r/Utils/TestBambuddyClient.cpp`

- [ ] **Step 1: Write parsing tests**

Add pure parser functions if direct HTTP mocking is too invasive:

```cpp
TEST_CASE("Bambuddy parses printer list", "[BambuddyClient]")
{
    const std::string body = R"([{"id":5,"name":"X1C","model":"X1C","is_active":true}])";
    std::vector<Slic3r::BambuddyPrinter> printers;
    std::string error;
    REQUIRE(Slic3r::BambuddyClient::parse_printers_response(body, printers, error));
    REQUIRE(printers.size() == 1);
    REQUIRE(printers[0].id == 5);
    REQUIRE(printers[0].name == "X1C");
}
```

If adding static parse functions, declare them in the header as testable helpers.

- [ ] **Step 2: Run test to verify it fails**

Expected: parser function does not exist.

- [ ] **Step 3: Implement parsers and HTTP GETs**

Use `Http::get(...)`:

- `test_connection()` should call a lightweight endpoint. Prefer `/api/v1/printers/` because it verifies API auth and returns JSON; if that is too heavy, use `/api/v1/auth/status` only if it works with API keys.
- `list_printers()` calls `/api/v1/printers/`.
- Apply `build_headers()` to the request.
- `tls_verify(true)` for HTTPS requests carrying tokens unless existing Orca conventions require compatibility with self-signed LAN hosts; if disabled for compatibility, document why in code.
- On HTML response, return reverse-proxy auth error.

- [ ] **Step 4: Run tests**

Run Bambuddy tests. Expected: parser tests pass. HTTP methods may be covered manually if no mock harness exists.

- [ ] **Step 5: Commit**

```bash
git add src/slic3r/Utils/BambuddyClient.hpp src/slic3r/Utils/BambuddyClient.cpp tests/slic3r/Utils/TestBambuddyClient.cpp
git commit -m "Add Bambuddy connection and printer APIs"
```

### Task 4: Implement upload and enqueue APIs

**Files:**
- Modify: `/Users/petrpopov/Developer/projects/main/OrcaSlicer/src/slic3r/Utils/BambuddyClient.cpp`
- Modify: `/Users/petrpopov/Developer/projects/main/OrcaSlicer/src/slic3r/Utils/BambuddyClient.hpp`
- Test: `/Users/petrpopov/Developer/projects/main/OrcaSlicer/tests/slic3r/Utils/TestBambuddyClient.cpp`

- [ ] **Step 1: Write JSON body/parser tests**

Add tests for queue payload and upload response parsing:

```cpp
TEST_CASE("Bambuddy parses upload response id", "[BambuddyClient]")
{
    Slic3r::BambuddyUploadResult result;
    std::string error;
    REQUIRE(Slic3r::BambuddyClient::parse_upload_response("{\"id\":123,\"filename\":\"a.gcode.3mf\"}", result, error));
    REQUIRE(result.library_file_id == 123);
}

TEST_CASE("Bambuddy queue payload uses library file and printer", "[BambuddyClient]")
{
    Slic3r::BambuddyPrintOptions options;
    options.timelapse = true;
    const std::string body = Slic3r::BambuddyClient::build_queue_body(123, 5, options);
    REQUIRE(body.find("\"library_file_id\":123") != std::string::npos);
    REQUIRE(body.find("\"printer_id\":5") != std::string::npos);
    REQUIRE(body.find("\"timelapse\":true") != std::string::npos);
}
```

- [ ] **Step 2: Run tests to verify failure**

Expected: missing parser/body functions.

- [ ] **Step 3: Implement upload**

Use `Http::post(build_api_url(config, "/library/files"))` and multipart:

```cpp
http.form_add_file("file", path, path.filename().string());
```

Reject non-existing files and filenames not ending in `.gcode.3mf` before making HTTP calls.

- [ ] **Step 4: Implement enqueue**

Use `Http::post(build_api_url(config, "/queue/"))`, `Content-Type: application/json`, and `set_post_body(body)`.

Payload fields:

- `library_file_id`
- `printer_id`
- `insert_at_top`
- `manual_start`
- `bed_levelling`
- `flow_cali`
- `vibration_cali`
- `layer_inspect`
- `timelapse`
- `use_ams`

Parse the queue response id if present. If Bambuddy's response shape differs, treat any 2xx JSON response as success and keep id optional.

- [ ] **Step 5: Run tests**

Run:

```bash
cmake --build build --target tests --parallel
ctest --test-dir build --output-on-failure -R Bambuddy
```

Expected: tests pass.

- [ ] **Step 6: Commit**

```bash
git add src/slic3r/Utils/BambuddyClient.hpp src/slic3r/Utils/BambuddyClient.cpp tests/slic3r/Utils/TestBambuddyClient.cpp
git commit -m "Add Bambuddy upload and queue APIs"
```

---

## Chunk 3: Settings UI

### Task 5: Persist Bambuddy settings

**Files:**
- Modify: `/Users/petrpopov/Developer/projects/main/OrcaSlicer/src/libslic3r/AppConfig.hpp`
- Modify: `/Users/petrpopov/Developer/projects/main/OrcaSlicer/src/libslic3r/AppConfig.cpp` if key defaults/serialization helpers live there.
- Modify: `/Users/petrpopov/Developer/projects/main/OrcaSlicer/src/slic3r/Utils/BambuddyClient.hpp/.cpp` if adding `load_from_app_config` helpers.

- [ ] **Step 1: Add config key constants**

Prefer a dedicated app config section, e.g. `bambuddy`, so custom headers can be serialized without polluting `[app]`.

Keys:

```text
enabled
base_url
api_key
default_printer_id
default_printer_name
proxy_auth_mode
pangolin_token_id
pangolin_token_secret
pangolin_query_token
custom_headers_json
```

- [ ] **Step 2: Add load/save helper tests if AppConfig tests exist**

If there are existing AppConfig tests, add round-trip coverage. Otherwise, test only serialization of custom headers as a pure helper in `TestBambuddyClient.cpp`.

- [ ] **Step 3: Implement load/save helpers**

Implement functions like:

```cpp
BambuddyConfig load_bambuddy_config(const AppConfig& app_config);
void save_bambuddy_config(AppConfig& app_config, const BambuddyConfig& config);
```

Do not print secrets in logs.

- [ ] **Step 4: Run tests/build**

Run Bambuddy tests and a GUI compile target.

- [ ] **Step 5: Commit**

```bash
git add src/libslic3r/AppConfig.hpp src/libslic3r/AppConfig.cpp src/slic3r/Utils/BambuddyClient.hpp src/slic3r/Utils/BambuddyClient.cpp tests/slic3r/Utils/TestBambuddyClient.cpp
git commit -m "Persist Bambuddy integration settings"
```

### Task 6: Add Bambuddy settings dialog

**Files:**
- Create: `/Users/petrpopov/Developer/projects/main/OrcaSlicer/src/slic3r/GUI/BambuddySettingsDialog.hpp`
- Create: `/Users/petrpopov/Developer/projects/main/OrcaSlicer/src/slic3r/GUI/BambuddySettingsDialog.cpp`
- Modify: `/Users/petrpopov/Developer/projects/main/OrcaSlicer/src/slic3r/GUI/Preferences.cpp`
- Modify: CMake source list for new GUI files.

- [ ] **Step 1: Create dialog skeleton**

Fields:

- enable checkbox;
- URL text input;
- API key password-style text input;
- default printer dropdown;
- refresh printers button;
- test connection button;
- proxy auth mode dropdown;
- Pangolin token id/token fields;
- Pangolin query token field;
- custom headers grid/list.

- [ ] **Step 2: Wire field visibility**

When auth mode changes:

- `None`: hide all proxy credential fields.
- `PangolinHeaders`: show token id + token secret.
- `PangolinQueryToken`: show `p_token`.
- `CustomHeaders`: show custom headers list.

- [ ] **Step 3: Wire Test Connection**

On click:

1. Read current dialog values into `BambuddyConfig`.
2. Call `BambuddyClient::test_connection()`.
3. Show success/failure dialog.
4. For HTML login failures, use the reverse-proxy-specific error string.

- [ ] **Step 4: Wire Refresh Printers**

On click:

1. Call `list_printers()`.
2. Populate dropdown with `name/model/id`.
3. Preserve selection if the id still exists.
4. Allow manual id entry if list fails.

- [ ] **Step 5: Save settings on OK/Apply**

Persist all fields using the helpers from Task 5.

- [ ] **Step 6: Compile**

Run:

```bash
cmake --build build --target OrcaSlicer --parallel
```

If local build infrastructure is unavailable, at least run the smallest compile target available and `git diff --check`.

- [ ] **Step 7: Commit**

```bash
git add src/slic3r/GUI/BambuddySettingsDialog.hpp src/slic3r/GUI/BambuddySettingsDialog.cpp src/slic3r/GUI/Preferences.cpp src/slic3r/CMakeLists.txt
git commit -m "Add Bambuddy settings dialog"
```

---

## Chunk 4: Send to Bambuddy button and flow

### Task 7: Add button next to BBL Connect

**Files:**
- Modify: `/Users/petrpopov/Developer/projects/main/OrcaSlicer/src/slic3r/GUI/SelectMachine.hpp`
- Modify: `/Users/petrpopov/Developer/projects/main/OrcaSlicer/src/slic3r/GUI/SelectMachine.cpp`

- [ ] **Step 1: Locate BBL Connect button creation**

Use the existing `m_button_bambu_connect`, `on_bambu_connect_btn`, and `update_bambu_connect_button_visibility()` patterns. Add `m_button_bambuddy` and a similar visibility update.

- [ ] **Step 2: Add button member and event handler declarations**

Add declarations:

```cpp
Button* m_button_bambuddy { nullptr };
void on_bambuddy_btn(wxCommandEvent& event);
void update_bambuddy_button_visibility();
```

Use the actual button class/style used by `m_button_bambu_connect`.

- [ ] **Step 3: Create and place button**

Place **Send to Bambuddy** adjacent to BBL Connect, with same enable/disable lifecycle. Show it only when Bambuddy integration is enabled and print type is compatible.

- [ ] **Step 4: Compile UI target**

Run:

```bash
cmake --build build --target OrcaSlicer --parallel
```

Expected: compiles; button may not do anything yet.

- [ ] **Step 5: Commit**

```bash
git add src/slic3r/GUI/SelectMachine.hpp src/slic3r/GUI/SelectMachine.cpp
git commit -m "Add Send to Bambuddy button"
```

### Task 8: Implement send handler

**Files:**
- Modify: `/Users/petrpopov/Developer/projects/main/OrcaSlicer/src/slic3r/GUI/SelectMachine.cpp`
- Modify: `/Users/petrpopov/Developer/projects/main/OrcaSlicer/src/slic3r/GUI/SelectMachine.hpp` if helper declarations are needed.
- Modify: `/Users/petrpopov/Developer/projects/main/OrcaSlicer/src/slic3r/GUI/BambuddySettingsDialog.*` if a compact printer/options picker is reused from settings.

- [ ] **Step 1: Write handler skeleton**

`on_bambuddy_btn` should:

1. return early if `m_print_type != PrintFromType::FROM_NORMAL` or `m_plater == nullptr`;
2. disable send and Bambuddy buttons;
3. load Bambuddy config;
4. if missing config/default printer, open settings or compact picker;
5. call `m_plater->send_gcode(m_print_plate_idx, progress_fn)`;
6. read `PrintPrepareData` via `m_plater->get_print_job_data(&print_data)`;
7. validate `_3mf_path` exists;
8. call upload;
9. call queue;
10. show status/success and close dialog only after queue success.

- [ ] **Step 2: Implement export validation**

Mirror BBL Connect error handling:

```cpp
const int result = m_plater->send_gcode(m_print_plate_idx, nullptr);
if (result < 0) {
    show_error(this, _L("Abnormal print file data. Please slice again"), false);
    // re-enable buttons
    return;
}
```

Then verify file path and suffix `.gcode.3mf` or `.3mf` with gcode bundle. Prefer strict `.gcode.3mf` if that is what `send_gcode` writes in this flow.

- [ ] **Step 3: Implement upload and queue calls**

Use `BambuddyClient` synchronously first, matching existing BBL Connect blocking behavior. If upload duration freezes UI too much, move network calls to the existing worker/status infrastructure in a follow-up.

On upload success but queue failure, show:

```text
File uploaded to Bambuddy library, but print queue creation failed: <error>
```

- [ ] **Step 4: Implement compact picker if needed**

If no default printer id exists:

- open settings dialog, or
- show a compact dialog with printer dropdown, print options, and “Remember as default”.

For first implementation, prefer reusing settings dialog to avoid duplicating UI unless the code stays small.

- [ ] **Step 5: Manual UI test**

Run OrcaSlicer, open send dialog, verify:

- BBL Connect still appears/works as before.
- Send to Bambuddy appears when enabled.
- Button disables during export/upload/queue.
- Missing settings produces actionable prompt.

- [ ] **Step 6: Commit**

```bash
git add src/slic3r/GUI/SelectMachine.cpp src/slic3r/GUI/SelectMachine.hpp src/slic3r/GUI/BambuddySettingsDialog.cpp src/slic3r/GUI/BambuddySettingsDialog.hpp
git commit -m "Implement Send to Bambuddy flow"
```

---

## Chunk 5: Polish, verification, and live testing

### Task 9: Polish user-facing strings and logging

**Files:**
- Modify files touched above.
- Modify localization files only if this project requires immediate string extraction for new UI text.

- [ ] **Step 1: Review strings**

Ensure errors distinguish:

- missing Bambuddy URL;
- missing Bambuddy API key;
- reverse proxy auth failed;
- Bambuddy auth failed;
- file uploaded but queue failed;
- selected printer missing.

- [ ] **Step 2: Review logs**

Search new code for token logging. No API key, Pangolin token, or custom header value may appear in logs.

- [ ] **Step 3: Run formatting**

Run clang-format on new/modified C++ files:

```bash
clang-format -i \
  src/slic3r/Utils/BambuddyClient.hpp \
  src/slic3r/Utils/BambuddyClient.cpp \
  src/slic3r/GUI/BambuddySettingsDialog.hpp \
  src/slic3r/GUI/BambuddySettingsDialog.cpp \
  src/slic3r/GUI/SelectMachine.hpp \
  src/slic3r/GUI/SelectMachine.cpp
```

- [ ] **Step 4: Commit**

```bash
git add src/slic3r/Utils/BambuddyClient.* src/slic3r/GUI/BambuddySettingsDialog.* src/slic3r/GUI/SelectMachine.*
git commit -m "Polish Bambuddy send integration"
```

### Task 10: Verify and document manual setup

**Files:**
- Create or modify a short doc if the repository has an appropriate user-doc location, otherwise add notes to PR description only.
- Potential doc: `/Users/petrpopov/Developer/projects/main/OrcaSlicer/doc/Bambuddy.md` if project conventions allow.

- [ ] **Step 1: Run source checks**

```bash
git diff --check
```

Expected: no whitespace errors in files changed by this feature.

- [ ] **Step 2: Run unit tests**

```bash
cmake --build build --target tests --parallel
ctest --test-dir build --output-on-failure -R Bambuddy
```

Expected: Bambuddy tests pass.

- [ ] **Step 3: Run app build if feasible**

```bash
cmake --build build --target OrcaSlicer --parallel
```

If the local build dir is broken, record the exact failure and run the narrowest available compile/source-level checks instead.

- [ ] **Step 4: Live smoke test against Bambuddy**

When Pangolin access/token or SSH access is available:

1. Configure `Bambuddy URL = https://bambuddy.ezheg.xyz`.
2. Configure Bambuddy API key if auth is enabled.
3. Configure reverse-proxy auth:
   - Pangolin headers: `P-Access-Token-Id` + `P-Access-Token`, or
   - Pangolin query: `p_token`, or
   - custom headers.
4. Test connection.
5. Refresh printers.
6. Slice a small model.
7. Send to Bambuddy.
8. Confirm Bambuddy library contains the uploaded `.gcode.3mf`.
9. Confirm queue item is created for the selected printer.
10. Confirm print starts only when expected.

- [ ] **Step 5: Final commit if docs changed**

```bash
git add doc/Bambuddy.md
git commit -m "Document Bambuddy setup"
```

---

## Execution Notes

- Use `superpowers:test-driven-development` for the implementation tasks.
- Use `superpowers:systematic-debugging` for any build/test failures.
- Use `superpowers:verification-before-completion` before claiming the feature works.
- The current environment previously had an Xcode SDK/build-dir issue; if full macOS build fails for unrelated SDK reasons, capture the exact command and failure and continue with targeted checks.
- The codebase-memory MCP server may not have an indexed project in this session; if graph search is unavailable, use source search as fallback and state why.
