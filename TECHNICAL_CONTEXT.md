# Technical Context

## Barcode Rendering Strategy
The app uses a hybrid approach to render barcodes on the Pebble watch.

### 1. Data Source (JavaScript Side)
*   The phone app (`src/pkjs/index.js` or `pebble-js-app.js`) uses `bwip-js` (via a webview or internal logic) to generate the barcode bitmap.
*   The bitmap is **cropped** to remove all whitespace/quiet zones to maximize resolution.
*   It is sent to the watch as a raw bitstream.

### 2. Watch Rendering (C Side)
The C code in `src/c/barcodes.c` is responsible for scaling and drawing this bitmap.

#### 2D Codes (QR, Aztec, PDF417)
*   **Method:** `draw_2d_centered`
*   **Scaling:** Uses nearest-neighbor scaling to fit the screen dimensions (144x168).
*   **Margins:** Minimal (centered).

#### 1D Codes (Code 128, Code 39, EAN-13)
*   **Method:** `draw_1d_rotated`
*   **Orientation:** Rotated 90 degrees to use the full height of the watch (168px) as the "length" of the barcode.
*   **Scaling (CRITICAL):**
    *   **Integer Scaling:** The renderer calculates the maximum *integer* multiplier (`floor(available_h / source_width)`).
    *   **Why?** Fractional scaling (e.g., 1.5x) creates irregular bar widths (jitter) which makes standard 1D barcodes unscannable on low-resolution displays like the Pebble.
    *   **Margins:** Fixed 25px margin at top and bottom (relative to screen) to provide necessary "quiet zones".
    *   **Fallback:** If the barcode is wider than the screen height (scale < 1), it falls back to fractional downsampling.

### 3. Fallback Rendering
If the pre-rendered bitmap data is missing (e.g., demo cards or sync errors), the watch attempts to generate the barcode locally:
*   **Code 128/39:** Native C renderer `draw_code128_barcode`.
*   **QR:** Native C renderer `draw_qr_code_onwatch`.
*   **Others (EAN-13, Aztec, PDF417):** Cannot be rendered locally; displays "Resync from phone".

## Known Limitations
1.  **Screen Resolution:** The Pebble's 144x168 resolution is the hard limit. Very dense barcodes (high capacity QR or long Code 128) may not be scannable if the module width drops below 1-2 pixels.
2.  **Backlight:** Scanners struggle with the Pebble's reflective screen in low light if the backlight is off.
3.  **Inversion:** Some scanners require black-on-white. The app supports an Invert setting which must be toggled if the scanner expects white-on-black.