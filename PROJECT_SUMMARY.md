# Project Summary

## Overview
**PebbleWallet Gemini** is a digital wallet application for Pebble smartwatches (Classic, Steel, Time, Time Steel, Round, 2). It allows users to store and display loyalty cards, membership cards, and tickets (QR, Aztec, PDF417, Code 128, EAN-13) on their wrist.

## Current Status
**Stable / Maintenance**
*   **Version:** 1.3.0
*   **Last Update:** February 10, 2026

## Key Features
*   **Offline Support:** Cards are stored on the watch.
*   **Wide Format Support:** QR, Aztec, PDF417, Code 128, Code 39, EAN-13.
*   **Cloud Config:** Settings and cards are managed via a web-based configuration page.
*   **Optimized Rendering:** Uses integer scaling for 1D barcodes to ensure scannability on low-res screens.

## Recent Changes (Session 2026-02-10)
1.  **Fixed EAN-13 & 1D Scanning:**
    *   Implemented **integer scaling** for rotated 1D barcodes. This prevents "jitter" (irregular bar widths) caused by fractional scaling, ensuring codes are scannable.
    *   Added **fixed 25px margins** (quiet zones) to prevent barcodes from touching screen edges.
    *   Centered barcodes with a standardized width (114px).
2.  **Fixed Fallback Logic:**
    *   Prevented the app from trying to render EAN-13 using the Code 128 renderer when bitmap data is missing.
3.  **Sync Workflow:**
    *   Streamlined `SYNC_COMMANDS.txt` for use with Rebble/Codespaces.

## Next Steps / Todo
*   **Testing:** Verify EAN-13 scanning on physical hardware once charged.
*   **Refinement:** If "huge" barcodes appear (scale < 1), consider a scrolling view or a warning, as downsampling 1D codes usually renders them useless.
