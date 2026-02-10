# Technical Context

## Barcode Rendering Architecture (Current State)

### 1. Data Pipeline
*   **Phone (JS):** Uses `bwip-js` to generate bitmaps.
*   **Optimization:** `cropBitmap` uses **Continuous Bit Packing** (no row padding) to remove whitespace and maximize resolution. This matches the C reader's logic.
*   **Protocol:** Sends `KEY_WIDTH`, `KEY_HEIGHT`, and `KEY_DATA` (raw byte stream).

### 2. Watch Rendering (C Side)
The C code (`src/c/barcodes.c`) handles rendering based on format:

#### 2D Codes (QR, Aztec, PDF417)
*   **Function:** `draw_2d_centered`
*   **Logic:** Standard integer scaling (`scale = min(screen_w/w, screen_h/h)`).
*   **Status:** Working well. Aztec and PDF417 are crisp.

#### 1D Codes (Code 128, Code 39, EAN-13)
*   **Function:** `draw_1d_rotated`
*   **Orientation:** Rotated 90 degrees (vertical on screen) to utilize the 168px height.
*   **Scaling:**
    *   **Integer Scaling ONLY:** `scale = available_h / w`.
    *   **Forced Minimum:** `if (scale < 1) scale = 1`. This prevents fractional "fuzzy" rendering.
*   **Margins (The Current Bottleneck):**
    *   **Sides (X-axis):** `bar_len = screen_w - 40` (20px left/right). Good for scanning.
    *   **Top/Bottom (Y-axis):** `margin = 20`. `available_h = 168 - 40 = 128px`.

### 3. The Code 128 "Too Big" Issue
The current configuration fails for long Code 128 strings.

#### Comparative Analysis:
*   **EAN-13:** Always 95 modules. At 1px/module, it uses **95px**. On a 168px screen, this leaves **73px** for margins (36px each side). Perfect scannability.
*   **Code 128:** Variable length. A typical 16-digit card is ~211 modules. Even at 1px/module, it uses **211px**.
*   **The Problem:** 211px is larger than the **168px** screen. 
*   **Current Failure:** The code forces `scale = 1`, draws all 211px, and the ends are "chopped off" by the screen edges.
*   **Solution Strategy:** Implement fractional downsampling (shrinking) for these cases so the 211 modules are squeezed into the available ~148px (168px - 20px margins).

## User Interface
*   **Menu:** Uses `menu_cell_basic_draw` for native look-and-feel (correct selection inversion).
*   **Sync:** Proactive fetch (500ms startup) + 3s loading timeout.

## Known Limitations
1.  **Long Code 128:** Cannot fit on screen with 1px/module scaling AND 20px margins.
2.  **Screen Resolution:** 144x168 is a hard physical limit.
