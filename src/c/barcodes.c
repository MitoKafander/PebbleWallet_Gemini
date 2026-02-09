#include "common.h"

static void draw_bits_matrix(GContext *ctx, GRect bounds, uint16_t w, uint16_t h, const uint8_t *bits) {
    if (w == 0 || h == 0 || !bits) return;

    int sw = bounds.size.w;
    int sh = bounds.size.h;
    
    // 1. Determine Rotation & Base Dimensions
    bool rotate = (w > h * 2);
    int p_axis_max = rotate ? sh : sw;
    int b_axis_max = rotate ? sw : sh;

    // 2. Force Large Quiet Zones for 1D scannability
    // We want at least 30px gap at the start and end of the pattern
    int quiet_zone = 30;
    int avail_p = p_axis_max - (quiet_zone * 2);
    int avail_b = b_axis_max - 10; // Slight side margin

    // 3. Calculate Scaling
    int scale = avail_p / w;
    if (scale < 1) scale = 1;

    // 4. Calculate Bar Length (Thickness of the barcode block)
    int bar_len = rotate ? (sw - 20) : (h * scale);
    if (!rotate && h < 20) bar_len = 100; // Portrait 1D stretching

    // 5. Centering
    int pattern_px = w * scale;
    int p_offset = (p_axis_max - pattern_px) / 2;
    int b_offset = (b_axis_max - bar_len) / 2;

    // 6. Colors
    GColor bg_color = g_invert_colors ? GColorBlack : GColorWhite;
    GColor fg_color = g_invert_colors ? GColorWhite : GColorBlack;

    // Clear background
    graphics_context_set_fill_color(ctx, bg_color);
    graphics_fill_rect(ctx, bounds, 0, GCornerNone);
    
    // Draw a White Quiet Zone background for the barcode if in Invert mode
    // (Most laser scanners CANNOT read white-on-black 1D barcodes)
    if (g_invert_colors && (w > h * 2)) {
        // For 1D barcodes, we force a white background plate for scannability
        graphics_context_set_fill_color(ctx, GColorWhite);
        if (rotate) {
            graphics_fill_rect(ctx, GRect(b_offset - 4, p_offset - 10, bar_len + 8, pattern_px + 20), 0, GCornerNone);
        } else {
            graphics_fill_rect(ctx, GRect(p_offset - 10, b_offset - 4, pattern_px + 20, bar_len + 8), 0, GCornerNone);
        }
        graphics_context_set_fill_color(ctx, GColorBlack); // Use black bars on the white plate
    } else {
        graphics_context_set_fill_color(ctx, fg_color);
    }

    // 7. Drawing Loop
    for (int r = 0; r < h; r++) {
        int run_start = -1;
        for (int c = 0; c < w; c++) {
            int bit_idx = r * w + c;
            bool is_active = (bits[bit_idx / 8] & (1 << (7 - (bit_idx % 8))));
            if (is_active) {
                if (run_start == -1) run_start = c;
            } else {
                if (run_start != -1) {
                    int run_w_px = (c - run_start) * scale;
                    int p_pos = p_offset + run_start * scale;
                    if (rotate) graphics_fill_rect(ctx, GRect(b_offset, p_pos, bar_len, run_w_px), 0, GCornerNone);
                    else graphics_fill_rect(ctx, GRect(p_pos, b_offset + r * scale, run_w_px, scale), 0, GCornerNone);
                    run_start = -1;
                }
            }
        }
        if (run_start != -1) {
            int run_w_px = (w - run_start) * scale;
            int p_pos = p_offset + run_start * scale;
            if (rotate) graphics_fill_rect(ctx, GRect(b_offset, p_pos, bar_len, run_w_px), 0, GCornerNone);
            else graphics_fill_rect(ctx, GRect(p_pos, b_offset + r * scale, run_w_px, scale), 0, GCornerNone);
        }
    }
}

void barcode_draw(GContext *ctx, GRect bounds, BarcodeFormat format, uint16_t w, uint16_t h, const uint8_t *bits) {
    draw_bits_matrix(ctx, bounds, w, h, bits);
}
