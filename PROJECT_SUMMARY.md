# GeminiWallet Project Summary (v1.2.0 Hotfix)
*Storage Optimization Fix - Feb 9, 2026*

This version includes a critical fix for persistent storage corruption that caused barcodes to revert to raw text after app restarts.

## Key Changes (v1.2.0)
- **Struct Reordering**: Moved `BarcodeFormat` to the start of the data structure to prevent it from being lost if storage is truncated.
- **Variable Length Storage**: Now only writes the actual length of data to Persistent Storage, significantly reducing memory usage (essential for Aplite's 4KB limit).

## Core Features
- **Comprehensive Format Support**:
    - **Aztec Code**: Verified boarding pass support (pre-rendered on phone).
    - **QR Code**: Now uses phone-side pre-rendering, allowing **lowercase letters, apostrophes, and symbols**.
    - **PDF417**: Supported via phone-side pre-rendering.
    - **1D Codes**: Logic ready for Code 128/39/EAN.
- **Offline First**: Cards are stored in the watch's **Persistent Storage** (Persist API).
- **Smooth Rendering**: High-speed **Run-Length Encoding (RLE)** drawing loop.
- **Advanced UI**: 
    - **Description Field**: Custom subtitles for each card.
    - **Smart Menu**: Automatically hides complex data strings; shows friendly fallbacks.
    - **Card Reordering**: Live reordering (Up/Down buttons) on the settings page.
- **Emulator Ready**: Support for the `return_to` bridge allows testing settings in Rebble Cloud without a physical phone.

## Technical Snapshot
- **UUID**: `e1f2a3b4-c5d6-4e7f-8a9b-0c1d2e3f4a5b`
- **Version**: `1.2.0`
- **Memory**: Optimized for **Aplite** (2KB buffer, 1KB data segments).
- **Structure**: Modular C (`main.c`, `storage.c`, `qr.c`, `barcodes.c`) and standard JS path (`src/js/pebble-js-app.js`).

## Sideloading Compatibility
This version uses a fresh UUID and classic file paths to ensure the **Core Devices Pebble App** accepts it without "Sideloading Errors."

---

## Maintenance & Recovery
- **Revert to Baseline**: `git reset --hard v1.2`
- **Emergency Sync**: Use `SYNC_COMMANDS.txt` if Rebble Cloud sidebar gets out of sync.
- **Publishing**: Prepared for submission to [dev-portal.rebble.io](https://dev-portal.rebble.io/).

---
**Next Session Goal:** Publish to the Rebble App Store & implement final UI toggles (Invert Colors).