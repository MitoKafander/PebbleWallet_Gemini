# GeminiWallet Project Summary (v1.1)
*Final Stable Build - Feb 8, 2026*

This version represents the fully polished, multi-format wallet with verified boarding pass support and persistent storage.

## Core Features
- **Broad Format Support**:
    - **Aztec Code**: Full boarding pass support (pre-rendered on phone).
    - **QR Code**: Supports all characters (lowercase, symbols) via phone-side pre-rendering.
    - **PDF417**: Supported via phone-side pre-rendering.
    - **Code 128/39/EAN**: Basic 1D support.
- **Offline First**: Cards are stored in the watch's **Persistent Storage** (Persist API). They load instantly even without a phone connection.
- **High-Performance Rendering**: Uses **Run-Length Encoding (RLE)** drawing logic to render dense matrices (like 62x62 Aztec) in milliseconds without watch lag.
- **UI Polish**: 
    - **Description Field**: Custom subtitles for each card (e.g., "Seat 4A").
    - **Clean Menu**: Hides messy hex strings; shows "Boarding Pass Data" or "QR Code" fallbacks.
    - **Card Reordering**: Drag-and-drop style reordering on the phone settings page.

## Technical Architecture
- **Watch (C)**: Modularized into `main.c`, `storage.c`, `qr.c`, and `barcodes.c`.
- **Phone (JS)**: Uses the classic `pebble-js-app.js` path for maximum sideloading compatibility.
- **Web (HTML)**: Integrates `bwip-js` for heavy-duty barcode encoding in the browser.
- **Memory**: Optimized for **Aplite** (Original Pebble) with a 2048-byte AppMessage buffer and 1024-byte data segments.

## Sideloading Compatibility
- **UUID**: `e1f2a3b4-c5d6-4e7f-8a9b-0c1d2e3f4a5b`
- **Version**: `1.0.0`
- **Target Platforms**: `aplite`, `basalt`, `chalk`.

---

## How to Revert to this Version
If you ever make changes that break the app, you can restore this exact state by running these commands in the **Rebble Cloud terminal**:

```bash
git fetch --tags
git reset --hard v1.1
pebble clean && pebble build
```

This version is also tagged as **v1.1** on GitHub.
