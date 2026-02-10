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
1.  **Fix Code 128 Margins:**
    *   **Action:** Reduce the vertical margin in `draw_1d_rotated`.
    *   *Calculation:* Code 128 quiet zone is 10x module width. At scale=1, that's 10px.
    *   *Proposal:* Change `margin` from 20 to **10** (or even 8). This increases safe space from 128px to **148px**, allowing for ~2 more characters.
    *   *Fallback:* If the code is *still* too long (>148px), we might need to implement a scrolling view or acknowledge the hardware limit.
2.  **Verify Code 39:** Ensure the margin fix doesn't negatively impact Code 39 (which is currently "difficult but working").
3.  **Final Polish:** Check if any other cosmetic tweaks are needed.

## Reference Commands
*   **Sync:** `git pull origin main && pebble clean && pebble build`