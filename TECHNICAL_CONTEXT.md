# Technical Context & Architecture Guide

## Storage System (The "Ultra-Safe" Pattern)
Pebble persistent storage is fragile. We use a custom sharding system to ensure reliability.

### Key Layout (Base: 24200)
We allocated a high key range to avoid collisions with legacy data or other apps.
*   **Keys Per Card:** 12
*   **Max Cards:** 10
*   **Total Keys Used:** 120 (Range: 24200 - 24320)

### Card Structure
Each card uses `index * 12` as its offset.
*   **Key 0:** `WalletCardInfo` (Struct) - Format, Name (32b), Desc (32b), Width (16b), Height (16b).
*   **Key 1..11:** Raw Data Chunks.
    *   **Chunk Size:** 100 bytes (Strictly enforced. Do not increase > 100 or writes will fail on older firmware).
    *   **Max Capacity:** 11 * 100 = 1100 bytes binary (~2200 bytes hex/text).

## Data Pipeline (The "Skunk Protocol")
We do not send text. We send pre-processed bits.

1.  **Config Page (JS):** 
    *   Uses `bwip-js` to render barcode to Canvas.
    *   Reads pixels: Black = 1, White = 0.
    *   Packs 8 pixels into 1 byte.
2.  **AppMessage (Phone -> Watch):**
    *   Sends `KEY_DATA` as a byte array (Blob).
    *   Max payload ~2KB.
3.  **Watch (C):**
    *   Receives Blob.
    *   Splits Blob into 100-byte chunks.
    *   Writes chunks to Persistent Storage immediately.
    *   **NEVER** stores the full blob in a global RAM buffer (would cause OOM).

## Rendering Logic
*   **Smart Rotation:**
    *   If `width > height * 2` (typical for 1D codes), we flip coordinates.
    *   Screen X becomes Barcode Y. Screen Y becomes Barcode X.
    *   This utilizes the 168px vertical resolution for the barcode's "width".
*   **Scaling:**
    *   Integer scaling only (`avail / width`).
    *   **Aztec/QR:** Zero margins to attempt 4x scale.
    *   **1D (128/39/EAN):** Forced 20px quiet zones at top/bottom (rotated) to help laser scanners.

## Build Notes
*   **SDK:** Pebble SDK 3.x / 4.3.
*   **Icons:** Must be 1-bit or 8-bit palette PNGs. 25x25 (Menu) and 48x48 (App).
*   **Versioning:** Must be `Major.Minor` (e.g., `1.3.0`). `1.3.1` is invalid.
