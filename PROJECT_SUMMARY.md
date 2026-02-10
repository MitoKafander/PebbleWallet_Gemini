# Project Summary

## Overview
**PebbleWallet Gemini** is a digital wallet for Pebble smartwatches. It renders Code 128, Code 39, EAN-13, QR, Aztec, and PDF417 barcodes offline on the watch.

## Current Status
**Functional / Polishing Phase**
*   **Version:** 1.4.0 (Dev)
*   **Last Update:** Session 2026-02-10

## Achievements (This Session)
1.  **Fixed Bit Packing:** Migrated to "Continuous Bit Packing" in JS and C. This solved the "fuzzy/shifted" rendering issues for all formats.
2.  **Fixed Menu UI:** Switched to `menu_cell_basic_draw` to fix the "black rectangle" selection bug.
3.  **Robust Sync:** Implemented proactive card fetching and a "Loading..." state with timeout.
4.  **Optimized 2D Codes:** Aztec and PDF417 now render and scan correctly.
5.  **Fixed EAN-13:** Now renders and scans correctly.

## Outstanding Issue: Code 128 Margins
**Symptom:** Code 128 renders but is "too big" and touches the top/bottom edges of the screen, making it unscannable.
**Cause:** The barcode is too long (too many modules) to fit within the 128px safe area (168px screen - 40px margin) when forced to 1px/module scaling.

## Next Session Plan (Priorities)
1.  **Fix Code 128/39 Scaling (Shrinking Logic):**
    *   **Action:** Implement "Fit to Screen" logic for barcodes that are too long for integer scaling.
    *   **Strategy:** If `scale < 1`, instead of forcing `scale = 1` (which spills off screen), use fractional downsampling to shrink the barcode until it fits within the margins.
    *   *Reference:* EAN-13 (fixed length) works well because it always fits. Long Code 128/39 need to be scaled down to achieve that same look.
2.  **Adjust Margins:** Reduce `margin` in `draw_1d_rotated` from 20 to **10** to reclaim vertical space.
3.  **Final Polish:** Check if any other cosmetic tweaks are needed.

## Reference Commands
*   **Sync:** `git pull origin main && pebble clean && pebble build`