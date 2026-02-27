<div align="center">
  <img alt="OrcaSlicer logo" src="resources/images/OrcaSlicer.png" width="120" />

# OrcaSlicer (petrpopov fork)

[![RU](https://img.shields.io/badge/README-Russian-lightgrey?style=for-the-badge&logo=readme&logoColor=white)](README.md)
[![EN](https://img.shields.io/badge/README-English-blue?style=for-the-badge&logo=readme&logoColor=white)](README.en.md)

An OrcaSlicer fork for users who want to keep using their favorite slicer with Bambu Lab printers via **Bambu Connect**.

</div>

## Why this fork exists

Short version: **I don't give a shit** about team conflicts and politics. I just want a working setup: favorite slicer + favorite printer. So I built this fork.

## What this project is

This is a fork of [OrcaSlicer](https://github.com/OrcaSlicer/OrcaSlicer) with focused improvements for printing on Bambu Lab via Bambu Connect, plus several UX upgrades.

Important: the core OrcaSlicer functionality was not rewritten or broken; changes are targeted and compatibility-focused.

## Quick start: print via Bambu Connect

1. Install Bambu Connect:
   - official wiki guide: [Bambu Connect](https://wiki.bambulab.com/en/software/bambu-connect)
   - official context post: [Updates and Third-Party Integration with Bambu Connect](https://blog.bambulab.com/updates-and-third-party-integration-with-bambu-connect/)
2. In OrcaSlicer, open `Preferences` and enable `Use Bambu Lab Connect`.
3. Before sending, make sure `Bambu Connect` is already running and can see your current printer.
4. Prepare your model and click `BBL Connect` in the send/print dialog.

## macOS: install and run unsigned `.dmg` build (EN/RU)

### EN

1. Open (mount) the `.dmg`.
2. Drag `OrcaSlicerPE.app` to `/Applications`.
3. Open `Terminal` and run:

```bash
xattr -dr com.apple.quarantine "/Applications/OrcaSlicerPE.app"
```

4. Launch the app from `/Applications`.

### RU

1. Откройте (смонтируйте) `.dmg`.
2. Перетащите `OrcaSlicerPE.app` в `/Applications`.
3. Откройте `Terminal` и выполните:

```bash
xattr -dr com.apple.quarantine "/Applications/OrcaSlicerPE.app"
```

4. Запустите приложение из `/Applications`.

## Main fork features

| Feature | What it gives you | Implemented in |
|---|---|---|
| Bambu Connect handoff | Send print jobs into the Bambu Lab workflow via Bambu Connect directly from OrcaSlicer | [PR #1](https://github.com/petrpopov/OrcaSlicer/pull/1) |
| Filament dropdown search | Fast realtime search in filament presets, including keyboard/layout input fixes | [PR #9](https://github.com/petrpopov/OrcaSlicer/pull/9), [PR #10](https://github.com/petrpopov/OrcaSlicer/pull/10), [PR #11](https://github.com/petrpopov/OrcaSlicer/pull/11), [PR #12](https://github.com/petrpopov/OrcaSlicer/pull/12) |
| Color themes and accent color | Unified accent color across native UI and embedded web UI, plus theme polish | [PR #3](https://github.com/petrpopov/OrcaSlicer/pull/3), [PR #6](https://github.com/petrpopov/OrcaSlicer/pull/6), [PR #8](https://github.com/petrpopov/OrcaSlicer/pull/8) |
| Original preview slider scale (Bambu-like) | Restores the familiar preview slider size/scale behavior | [PR #2](https://github.com/petrpopov/OrcaSlicer/pull/2) |
| Materials/settings UX improvements | AMS prefetch, grouped fork settings, improved custom filament management flow | [PR #4](https://github.com/petrpopov/OrcaSlicer/pull/4), [PR #5](https://github.com/petrpopov/OrcaSlicer/pull/5), [PR #7](https://github.com/petrpopov/OrcaSlicer/pull/7) |

## Build

Build flow is the same as in upstream OrcaSlicer.

- Official build guide: [Orca Wiki: How to build](https://www.orcaslicer.com/wiki/How-to-build)
- Upstream repository: [github.com/OrcaSlicer/OrcaSlicer](https://github.com/OrcaSlicer/OrcaSlicer)

## Download

- Fork releases: [GitHub Releases](https://github.com/petrpopov/OrcaSlicer/releases)

## License and attribution

- This project is licensed under **GNU Affero General Public License v3.0**. See [LICENSE.txt](LICENSE.txt).
- This fork is based on [OrcaSlicer](https://github.com/OrcaSlicer/OrcaSlicer), which itself builds on the Bambu Studio / PrusaSlicer / Slic3r ecosystem.
- All applicable upstream licensing obligations remain in effect.
