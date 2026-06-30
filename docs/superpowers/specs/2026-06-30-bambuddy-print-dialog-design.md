# Bambuddy Print Dialog UX Design

## Goal
Improve OrcaSlicer's Bambuddy integration so upload-only and upload-and-print are separate explicit actions, with clear progress, navigation, settings access, and printer choice.

## Design
- `Send to Bambuddy` uploads the sliced `.3mf/.gcode.3mf` archive only, then opens the configured Bambuddy Archives page (`base_url + /archives`).
- `Print in Bambuddy` uploads the archive, queues it for the selected Bambuddy printer, then opens the configured Bambuddy Queue page (`base_url + /queue`).
- Both actions reuse the existing send dialog sending page/progress indicator instead of a busy cursor.
- The print dialog shows footer links to Archives and Print Queue only when Bambuddy is enabled and has a URL.
- The print dialog exposes a small settings button that opens the same Bambuddy settings dialog as Peter's Settings.
- A Bambuddy printer dropdown is shown when the integration is configured, so users can choose the target for `Print in Bambuddy` without reopening settings.

## Filament Mapping Scope
If native printer sync times out, this iteration does not block Bambuddy printing. `Print in Bambuddy` opens `/queue` after queueing so the user can handle Bambuddy's filament mapping override there. Full AMS/filament synchronization through Bambuddy is deferred until the Bambuddy API for printer material/filament state is confirmed.
