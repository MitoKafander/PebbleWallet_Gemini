#include "common.h"

static void draw_bits_matrix(GContext *ctx, GRect bounds, uint16_t w, uint16_t h, const uint8_t *bits) {
    if (w == 0 || h == 0 || !bits) return;

    int sw = bounds.size.w; // 144
    int sh = bounds.size.h; // 168
    
    // 1. Determine if we should rotate (only for 1D codes)
    bool is_1d = (w > h);
    bool rotate = is_1d; 

    // 2. Setup Dimensions based on rotation
    int p_axis_max = rotate ? sh : sw;
    int b_axis_max = rotate ? sw : sh;

    // 3. Scaling & Margins
    int margin = is_1d ? 15 : 0; // Only 1D needs quiet zones for scannability
    int avail_p = p_axis_max - (margin * 2);
    int avail_b = b_axis_max;

    int scale = avail_p / w;
    if (scale < 1) scale = 1;

    // 4. Bar Length (The vertical height of the barcode bars)
    int bar_len = is_1d ? (b_axis_max - 20) : (h * scale);

    // 5. Centering offsets
    int pattern_px = w * scale;
    int p_offset = (p_axis_max - pattern_px) / 2;
    int b_offset = (b_axis_max - bar_len) / 2;

    // 6. Colors
    GColor bg = g_invert_colors ? GColorBlack : GColorWhite;
    GColor fg = g_invert_colors ? GColorWhite : GColorBlack;

    graphics_context_set_fill_color(ctx, bg);
    graphics_fill_rect(ctx, bounds, 0, GCornerNone);
    
    // For 1D barcodes, force a white background plate for laser scanners
    if (is_1d && g_invert_colors) {
        graphics_context_set_fill_color(ctx, GColorWhite);
        if (rotate) graphics_fill_rect(ctx, GRect(b_offset - 4, p_offset - 10, bar_len + 8, pattern_px + 20), 0, GCornerNone);
        else graphics_fill_rect(ctx, GRect(p_offset - 10, b_offset - 4, pattern_px + 20, bar_len + 8), 0, GCornerNone);
        graphics_context_set_fill_color(ctx, GColorBlack);
    } else {
        graphics_context_set_fill_color(ctx, fg);
    }

    // 7. Render
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
                    if (rotate) {
                        // Standing Bars: X = across watch (b), Y = up/down watch (p)
                        graphics_fill_rect(ctx, GRect(b_offset, p_pos, bar_len, run_w_px), 0, GCornerNone);
                    } else {
                        graphics_fill_rect(ctx, GRect(p_pos, b_offset + r * scale, run_w_px, scale), 0, GCornerNone);
                    }
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
    
    // 8. Enforce Quiet Zones with Masking (if needed)
    if (is_1d) {
        // For 1D barcodes, the quiet zone must always be white for scanners.
        graphics_context_set_fill_color(ctx, GColorWhite);

        if (rotate) {
            // Rotated: quiet zones are top and bottom
            graphics_fill_rect(ctx, GRect(0, 0, b_axis_max, margin), 0, GCornerNone);
            graphics_fill_rect(ctx, GRect(0, p_axis_max - margin, b_axis_max, margin), 0, GCornerNone);
        } else {
            // Not rotated: quiet zones are left and right
            graphics_fill_rect(ctx, GRect(0, 0, margin, b_axis_max), 0, GCornerNone);
            graphics_fill_rect(ctx, GRect(p_axis_max - margin, 0, margin, b_axis_max), 0, GCornerNone);
        }
    }
}

void barcode_draw(GContext *ctx, GRect bounds, BarcodeFormat format, uint16_t w, uint16_t h, const uint8_t *bits) {
    draw_bits_matrix(ctx, bounds, w, h, bits);
}