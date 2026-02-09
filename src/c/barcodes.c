#include "common.h"

static void draw_bits_matrix(GContext *ctx, GRect bounds, uint16_t w, uint16_t h, const uint8_t *bits) {
    if (w == 0 || h == 0 || !bits) return;

    int sw = bounds.size.w;
    int sh = bounds.size.h;
    
    // Rotate if pattern is much wider than tall (1D codes)
    bool rotate = (w > h * 2);
    
    // Pattern runs along 'avail_w' axis. Bars extend along 'avail_h' axis.
    int avail_w = rotate ? sh : sw;
    int avail_h = rotate ? sw : sh;

    // Force Quiet Zones (at the ends of the pattern)
    avail_w -= 40; // 20px at each end

    int scale = avail_w / w;
    if (scale < 1) scale = 1;

    // Bar length (height of the bars)
    int bar_len = rotate ? 124 : h * scale; 
    if (!rotate && h < 20) bar_len = 100; // Stretch if it's a 1D code in portrait

    // Centering offsets
    int pattern_total_px = w * scale;
    int offset_pattern = (avail_w - pattern_total_px) / 2 + 20;
    int offset_bar = (avail_h - bar_len) / 2;

    // Background
    GColor bg = g_invert_colors ? GColorBlack : GColorWhite;
    GColor fg = g_invert_colors ? GColorWhite : GColorBlack;
    graphics_context_set_fill_color(ctx, bg);
    graphics_fill_rect(ctx, bounds, 0, GCornerNone);
    graphics_context_set_fill_color(ctx, fg);

    for (int r = 0; r < h; r++) {
        int run_start = -1;
        for (int c = 0; c < w; c++) {
            int bit_idx = r * w + c;
            bool is_active = (bits[bit_idx / 8] & (1 << (7 - (bit_idx % 8))));
            if (is_active) {
                if (run_start == -1) run_start = c;
            } else {
                if (run_start != -1) {
                    int run_w = (c - run_start) * scale;
                    int p_pos = offset_pattern + run_start * scale;
                    if (rotate) {
                        // Pattern goes TOP to BOTTOM. Bars go LEFT to RIGHT.
                        graphics_fill_rect(ctx, GRect(offset_bar, p_pos, bar_len, run_w), 0, GCornerNone);
                    } else {
                        // Pattern goes LEFT to RIGHT. Bars go TOP to BOTTOM.
                        graphics_fill_rect(ctx, GRect(p_pos, offset_bar + r * scale, run_w, scale), 0, GCornerNone);
                    }
                    run_start = -1;
                }
            }
        }
        if (run_start != -1) {
            int run_w = (w - run_start) * scale;
            int p_pos = offset_pattern + run_start * scale;
            if (rotate) {
                graphics_fill_rect(ctx, GRect(offset_bar, p_pos, bar_len, run_w), 0, GCornerNone);
            } else {
                graphics_fill_rect(ctx, GRect(p_pos, offset_bar + r * scale, run_w, scale), 0, GCornerNone);
            }
        }
    }
}

void barcode_draw(GContext *ctx, GRect bounds, BarcodeFormat format, uint16_t w, uint16_t h, const uint8_t *bits) {
    draw_bits_matrix(ctx, bounds, w, h, bits);
}