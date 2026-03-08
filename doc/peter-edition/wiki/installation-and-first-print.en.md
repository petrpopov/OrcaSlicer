# OrcaSlicer PE: installation and first print

This guide is for a first-time user installing `OrcaSlicer PE` and printing to a Bambu Lab printer through `Bambu Connect`.

## What you need

1. `OrcaSlicer PE` from [Releases](https://github.com/petrpopov/OrcaSlicer/releases).
2. `Bambu Connect` from Bambu Lab.
   You can download it from:
   - the Bambu Lab wiki: [Bambu Connect](https://wiki.bambulab.com/en/software/bambu-connect)
   - the official announcement with workflow details: [Updates and Third-Party Integration with Bambu Connect](https://blog.bambulab.com/updates-and-third-party-integration-with-bambu-connect/)
3. Access to your printer on the same network or through the supported Bambu Connect workflow.

## Installing OrcaSlicer PE

### Windows

1. Download the archive or installer from the release page.
2. Extract it or run the installer.
3. Launch `OrcaSlicerPE`.

### Linux

1. Download the archive or package from the release page.
2. Extract it to a convenient location.
3. Make the binary executable if needed.
4. Launch OrcaSlicer PE.

### macOS

1. Download the `.dmg` from the release page.
2. Open the image and drag `OrcaSlicerPE.app` to `/Applications`.
3. Because the build is not Apple-signed, open `Terminal` and run:

```bash
xattr -dr com.apple.quarantine "/Applications/OrcaSlicerPE.app"
```

4. Launch the app from `/Applications`.

## Installing Bambu Connect

1. Download and install `Bambu Connect` using one of the official links above.
2. Launch `Bambu Connect`.
3. Make sure it is signed in and can see your printer.
4. Keep `Bambu Connect` running before sending from OrcaSlicer PE.

Important: `OrcaSlicer PE` does not replace `Bambu Connect`; it hands the prepared print job off to it.

## Initial setup in OrcaSlicer PE

1. Launch `OrcaSlicer PE`.
2. Open `Preferences`.
3. Enable `Use Bambu Lab Connect`.
4. Select your printer, nozzle profile, filament, and print settings.

The fork also adds several UX improvements around this workflow: AMS prefetch, better filament search, grouped fork-specific settings, and a smoother Bambu Connect handoff.

## How to send a model to print

1. Open or create a project.
2. Add your model.
3. Choose the printer and filament profile.
4. Click `Slice plate`.
5. Open the send/print dialog.
6. Click `BBL Connect`.
7. OrcaSlicer PE will prepare the project and hand it off to `Bambu Connect`.
8. Confirm the send action in `Bambu Connect` if your workflow requires it.

## What OrcaSlicer PE adds

Since the first fork release, OrcaSlicer PE has added and refined:

- direct `Bambu Connect` handoff from the send-to-printer dialog;
- project-name-based export names instead of generic temporary filenames;
- automatic closing of the send dialog after `Bambu Connect` starts successfully;
- searchable filament dropdowns, including fixes for backspace, keyboard layout input, and filter reset;
- AMS list prefetch for faster filament selection;
- a dedicated fork settings group in `Preferences`;
- a smoother custom filament management flow;
- a unified accent color across native UI and embedded web UI;
- restored Bambu-like preview slider sizing;
- Peter's Edition branding in the about dialog and splash;
- additional stability, OpenGL core/ES compatibility, GitHub Releases-based update checks, and upstream OrcaSlicer syncs.

## If sending to print fails

- Verify that `Bambu Connect` is installed and already running.
- Verify that `Use Bambu Lab Connect` is enabled in `Preferences`.
- Make sure the active printer and project are correct.
- On macOS, verify that the quarantine attribute was removed from the app.

## Useful links

- Releases: [GitHub Releases](https://github.com/petrpopov/OrcaSlicer/releases)
- `v2.3.2-pe.6` release notes: [release notes](https://github.com/petrpopov/OrcaSlicer/blob/main/doc/peter-edition/releases/v2.3.2-pe.6.md)
- Upstream project: [OrcaSlicer](https://github.com/OrcaSlicer/OrcaSlicer)
