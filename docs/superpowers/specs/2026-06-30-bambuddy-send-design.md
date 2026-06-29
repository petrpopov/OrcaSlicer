# Send to Bambuddy Design

## Goal

Add a Bambu-specific **Send to Bambuddy** action in OrcaSlicer that exports the current sliced job as a `.gcode.3mf`, uploads it to Bambuddy through its HTTP API, and immediately queues it for the selected Bambuddy printer.

## Context

The user has a Bambuddy deployment at `https://bambuddy.ezheg.xyz`. Bambuddy itself can be open, but the public URL is protected by Pangolin. Live probing was limited because the URL redirected to Pangolin auth and `ssh mbp15` was unreachable from this environment (`No route to host`). Code inspection used upstream `maziggy/bambuddy` commit `62f45c8c95f0b44d9fe4c402beb0813415f233e1`.

Bambuddy already supports the required server-side behavior:

1. Upload a ready sliced `.gcode.3mf` file to the library via `POST /api/v1/library/files` with multipart field `file`.
2. Queue it for a printer via `POST /api/v1/queue/` with `library_file_id`, `printer_id`, and print options.
3. Bambuddy's scheduler archives the library file, uploads it to the Bambu printer over FTP, and starts the print.

Bambuddy intentionally rejects raw `.gcode` for Bambu network printing. OrcaSlicer must send the `.gcode.3mf` bundle produced by the existing `use_3mf` / `Plater::send_gcode` path.

## User Experience

### Entry point

Add a dedicated **Send to Bambuddy** button next to the existing BBL Connect action. Do not add Bambuddy as a generic `Host Type` in the OctoPrint/Moonraker-style print-host list.

### Settings

Add a Bambuddy settings section with:

- Enable Bambuddy integration.
- Bambuddy URL, e.g. `https://bambuddy.ezheg.xyz`.
- Bambuddy API key/token, sent as `X-API-Key` or `Authorization: Bearer <token>`.
- Default Bambuddy printer, selected from `GET /api/v1/printers/`.
- Test connection.
- Refresh printers.

Add reverse-proxy authentication settings:

- Mode: `None`, `Pangolin access token headers`, `Pangolin p_token query`, or `Custom headers`.
- Pangolin headers mode:
  - `P-Access-Token-Id`
  - `P-Access-Token`
- Pangolin query mode:
  - `p_token`
- Custom headers mode:
  - repeatable `Header name` / `Header value` rows.

All reverse-proxy auth is applied centrally to every Bambuddy request: test, printer list, upload, and queue creation.

### Send flow

When the user clicks **Send to Bambuddy**:

1. Validate that Bambuddy integration is enabled and configured.
2. If no default printer is set, open a compact dialog with:
   - Bambuddy printer dropdown.
   - Print options.
   - `Remember as default` checkbox.
3. Export the current plate/all-plates using the existing `.gcode.3mf` path.
4. Upload the `.gcode.3mf` to Bambuddy library.
5. Create a Bambuddy queue item for the selected printer.
6. Show success with the Bambuddy printer name and queued job/file name.

## Architecture

### Components

#### `BambuddyConfig`

Small configuration wrapper for loading and saving Bambuddy settings from OrcaSlicer app configuration.

Responsibilities:

- Store URL, API key, selected/default printer id/name.
- Store reverse-proxy auth mode and credentials.
- Normalize URL paths so API calls target `/api/v1/...` consistently.
- Avoid leaking secrets in logs.

#### `BambuddyClient`

Focused HTTP API client independent of the generic `PrintHost` abstraction.

Responsibilities:

- Build requests with Bambuddy API authentication.
- Apply reverse-proxy auth:
  - no-op for `None`;
  - add `P-Access-Token-Id` / `P-Access-Token` headers;
  - append `p_token` query parameter;
  - add custom headers.
- `test_connection()`.
- `list_printers()`.
- `upload_file(path, filename)`.
- `enqueue_print(library_file_id, printer_id, print_options)`.
- Parse JSON responses and return typed/simple result structs.
- Convert HTTP/network/JSON failures into user-facing errors.

#### Send UI controller

Coordinates OrcaSlicer's existing slicing/export path with `BambuddyClient`.

Responsibilities:

- Trigger export of `.gcode.3mf` using the same path currently used for BBL/3MF send.
- Show progress for export/upload/queue phases.
- Open printer/options dialog when needed.
- Display final success or error.

## API Contract

### Upload file

Request:

```http
POST /api/v1/library/files
Content-Type: multipart/form-data
X-API-Key: <bambuddy-api-key>
```

Multipart field:

- `file`: sliced `.gcode.3mf`

Expected response includes a library file id, for example:

```json
{
  "id": 123,
  "filename": "model.gcode.3mf",
  "file_type": "gcode.3mf"
}
```

### Queue print

Request:

```http
POST /api/v1/queue/
Content-Type: application/json
X-API-Key: <bambuddy-api-key>
```

Body:

```json
{
  "library_file_id": 123,
  "printer_id": 5,
  "insert_at_top": true,
  "manual_start": false,
  "bed_levelling": true,
  "flow_cali": false,
  "vibration_cali": true,
  "layer_inspect": false,
  "timelapse": false,
  "use_ams": true
}
```

## Error Handling

- If Pangolin or another proxy returns an HTML login page or redirect instead of JSON, show: reverse-proxy authentication failed; check Pangolin/custom auth settings.
- If Bambuddy returns `401` or `403`, show an API key/permission error.
- If printer list fails but URL is reachable, keep manual printer id entry available as fallback.
- If upload succeeds but queue creation fails, show that the file was uploaded to Bambuddy library but queue creation failed.
- If the selected printer no longer exists, prompt the user to refresh printers and choose again.
- If OrcaSlicer cannot produce `.gcode.3mf`, block sending and ask the user to slice/export again.
- Do not log API keys, Pangolin tokens, or custom header values.

## Security

- Prefer machine-client reverse-proxy auth over trying to automate interactive OAuth in OrcaSlicer.
- Support Pangolin access tokens through headers or query parameter for script/integration access.
- Support custom headers for Pangolin alternatives such as Cloudflare Access, Authentik, Authelia, or Nginx secret headers.
- Do not recommend exposing Bambuddy's write API publicly without proxy auth or Bambuddy API keys.

## Testing

Automated tests should cover:

- Reverse-proxy auth injection for all modes.
- URL joining and query parameter preservation.
- Upload request shape for `.gcode.3mf`.
- Queue request body and print option mapping.
- HTML/redirect response detection.
- Upload-success/queue-failure partial-success error.

Manual smoke test:

1. Configure Bambuddy URL and auth.
2. Test connection.
3. Refresh printers.
4. Slice a model.
5. Click **Send to Bambuddy**.
6. Confirm the file appears in Bambuddy library and a queue item is created for the selected printer.
7. Confirm Bambuddy starts the print when the selected printer is idle.

## Non-goals

- Do not implement Bambuddy as a generic `PrintHost` type in this iteration.
- Do not implement interactive Pangolin OAuth inside OrcaSlicer.
- Do not send raw `.gcode` to Bambuddy for Bambu network printing.
- Do not modify Bambuddy server code for the first iteration unless the current API proves insufficient during live testing.
