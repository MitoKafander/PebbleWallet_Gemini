# GeminiWallet Project Summary (v1.3.0)
*Stable "Skunk Protocol" Build - Feb 9, 2026*

This version represents a complete rewrite of the data transport and storage layer. It is now highly stable, crash-resistant, and capable of handling massive boarding pass data.

## 🏗️ Architecture: The "Split & Skunk" Model
To overcome Pebble's RAM (24KB) and AppMessage (2KB) limits, we implemented two major patterns:

1.  **Split Architecture (RAM Fix):**
    *   **Problem:** Storing 10 cards * 1KB data in RAM crashed the watch.
    *   **Solution:** We now only keep lightweight `WalletCardInfo` (Name/Type) in RAM.
    *   **On-Demand:** Heavy barcode data is fetched from persistent storage *only* when the user clicks a card, and is freed immediately after.

2.  **Skunk Protocol (Binary Transfer):**
    *   **Problem:** Sending Hex strings (e.g. "A1F0") wasted 50% of bandwidth and storage.
    *   **Solution:** The phone now converts data to **Raw Binary** before sending.
    *   **Result:** Double the capacity, faster transfers, and no "maximum length" errors.

3.  **Ultra-Safe Storage (Persistence Fix):**
    *   **Problem:** Pebble writes > 256 bytes often fail silently.
    *   **Solution:** Data is sliced into **12 chunks of 100 bytes**.
    *   **Safe Zone:** Keys are stored at index `24200+` to avoid collisions with other apps.

## 🎨 UI & Rendering
-   **Smart Rotation:** Wide codes (EAN-13, Code 128) automatically rotate 90° to use the screen's 168px vertical axis.
-   **Quiet Zones:** 1D barcodes force a white background plate and (attempt to) enforce 20px margins for laser scanner compatibility.
-   **Max Scaling:** Aztec and QR codes use "Zero Margin" logic to hit the highest possible integer scale factor (3x/4x).
-   **Invert Colors:** Global setting for White-on-Black high contrast mode.

## 🐛 Current Known Issues (For Next Session)
-   **1D Margins:** User reported Code 128/39 still touching screen edges despite the quiet zone logic. We may need to enforce a "hard clamp" or draw explicit white masking rectangles over the ends in the next session.
-   **Aztec Size:** Some Aztec codes remain at ~12mm (3x scale) because 4x scale is just *slightly* too wide for the 144px screen. This is likely a physical limit, but we could explore "cropping" non-essential corners to force 4x.

## 🛠️ Commands Reference
**Sync to Web:**
```bash
cd /workspaces/codespaces-pebble/GeminiWallet && 
git fetch origin && 
git reset --hard origin/main && 
pebble clean && 
pebble build
```

**Install:**
```bash
pebble install --phone <IP_ADDRESS>
# OR use 'python3 -m http.server 8080' and scan the QR
```

**Recovery:**
If data looks weird, always: **Phone Settings -> Save & Sync**. This regenerates the binary data and clears old storage keys.