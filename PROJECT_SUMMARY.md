# GeminiWallet Project Summary (v1.3.0 Split Architecture)
*Memory & Storage Optimization - Feb 9, 2026*

This version includes a complete architectural refactor to support large/complex barcodes without crashing or data truncation.

## Key Changes (v1.3.0)
- **Split Architecture**: Separates lightweight metadata (Menu) from heavy barcode data (Details).
- **On-Demand Loading**: Barcode data is fetched and decompressed only when viewing a card, saving ~20KB of RAM.
- **Ultra-Safe Storage**: Uses small 100-byte chunks to bypass Pebble's 256-byte internal limit.
- **Binary Compression**: Barcode matrices are compressed to 1-bit binary before saving, reducing storage usage by 50%.

## Core Features
- **Comprehensive Format Support**: Aztec, QR (full charset), PDF417, and 1D codes.
- **Offline First**: Full persistent storage on the watch.
- **Smart Menu**: Dynamic subtitles and automatic fallback for complex data.
- **Card Reordering**: Live reordering on the settings page.

## Technical Snapshot
- **UUID**: `e1f2a3b4-c5d6-4e7f-8a9b-0c1d2e3f4a5b`
- **Version**: `1.3.0`
- **Memory**: Optimized for **Aplite** (Metadata list < 1KB, dynamic data buffer 2.5KB).
- **Structure**: Split-storage C modules (`main.c`, `storage.c`, `qr.c`, `barcodes.c`).

---

## Maintenance & Recovery
- **Revert to Baseline**: `git reset --hard v1.2`
- **Emergency Sync**: Use `SYNC_COMMANDS.txt` if Rebble Cloud sidebar gets out of sync.

---
**Next Session Goal:** Publish to the Rebble App Store & implement final UI toggles (Invert Colors).
