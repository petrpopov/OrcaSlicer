# Bambuddy Print Dialog Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Split Bambuddy upload and print flows in OrcaSlicer's send dialog and add navigation/settings/printer-selection UX.

**Architecture:** Keep Bambuddy HTTP/API behavior in `BambuddyClient`; add URL-building helpers there with unit coverage. Keep send dialog UI orchestration in `SelectMachineDialog`, reusing the existing `BBLStatusBarPrint` sending panel for progress. Avoid full Bambuddy filament mapping until API support is known.

**Tech Stack:** C++17, wxWidgets, OrcaSlicer GUI helpers, existing `BambuddyClient`, Catch2-style slic3rutils tests.

---

## Chunk 1: Client URL helpers

**Files:**
- Modify: `/Users/petrpopov/Developer/projects/main/OrcaSlicer/src/slic3r/Utils/BambuddyClient.hpp`
- Modify: `/Users/petrpopov/Developer/projects/main/OrcaSlicer/src/slic3r/Utils/BambuddyClient.cpp`
- Test: `/Users/petrpopov/Developer/projects/main/OrcaSlicer/tests/slic3rutils/test_bambuddy_client.cpp`

- [ ] Add failing tests for `build_page_url(config, "archives")` and `build_page_url(config, "queue")` preserving Pangolin query token.
- [ ] Run an ad-hoc harness or target test and verify failure before implementation.
- [ ] Implement `BambuddyClient::build_page_url` to join `base_url` with a UI page path and apply Pangolin query-token auth.
- [ ] Verify tests pass.

## Chunk 2: Split actions and progress

**Files:**
- Modify: `/Users/petrpopov/Developer/projects/main/OrcaSlicer/src/slic3r/GUI/SelectMachine.hpp`
- Modify: `/Users/petrpopov/Developer/projects/main/OrcaSlicer/src/slic3r/GUI/SelectMachine.cpp`

- [ ] Add `Print in Bambuddy` button next to `Send to Bambuddy`.
- [ ] Refactor current handler into a helper accepting mode: upload-only vs upload-and-print.
- [ ] Use `m_simplebook` sending page and `m_status_bar` to report upload/queue progress.
- [ ] Open `/archives` after upload-only success and `/queue` after print success.
- [ ] Restore prepare page and buttons on failures.

## Chunk 3: Settings, links, printer choice

**Files:**
- Modify: `/Users/petrpopov/Developer/projects/main/OrcaSlicer/src/slic3r/GUI/SelectMachine.hpp`
- Modify: `/Users/petrpopov/Developer/projects/main/OrcaSlicer/src/slic3r/GUI/SelectMachine.cpp`
- Inspect: `/Users/petrpopov/Developer/projects/main/OrcaSlicer/src/slic3r/GUI/Preferences.cpp`

- [ ] Add footer links `Archives` and `Print Queue` when Bambuddy is enabled and URL is configured.
- [ ] Add small settings button/icon in the print dialog that opens `BambuddySettingsDialog`.
- [ ] Add Bambuddy printer dropdown visible when settings are valid; load printers via API and fall back to configured default if list fails.
- [ ] Ensure `Print in Bambuddy` uses selected printer ID/name.
- [ ] Confirm Peter's Settings contains the same Bambuddy settings dialog entry.

## Chunk 4: Verification and commit

- [ ] Run `git diff --check`.
- [ ] Run focused compile/syntax checks for changed C++ files.
- [ ] Run/add ad-hoc unit harness for Bambuddy URL helpers if full test target is too expensive.
- [ ] Commit the implementation.
