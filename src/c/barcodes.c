#include "common.h"

// NEW: Clean, Simplified, Robust Renderer
static void draw_bits_matrix(GContext *ctx, GRect bounds, uint16_t w, uint16_t h, const uint8_t *bits) {
    if (w == 0 || h == 0 || !bits) return;

    int screen_w = bounds.size.w;
    int screen_h = bounds.size.h;
    
    // DECIDE ROTATION
    // If it's a wide 1D barcode (w >> h), always rotate to use the 168px vertical space
    bool rotate = (w > h * 2);
    
    // DECIDE SCALING
    int avail_w = rotate ? screen_h : screen_w;
    int avail_h = rotate ? screen_w : screen_h;
    
    // For scannability, we MUST leave room for quiet zones on 1D barcodes
    if (rotate) {
        avail_w -= 40; // 20px quiet zone on each end
    }

    int scale_w = avail_w / w;
    int scale_h = avail_h / h;
    int scale = (scale_w < scale_h) ? scale_w : scale_h;
    if (scale < 1) scale = 1;

    // For 1D codes, we stretch the bars to be nice and tall
    int bar_len_px = scale * h;
    if (rotate && h < 20) {
        // Stretch 1D bars to fill roughly half the screen height (70px)
        bar_len_px = 70;
    }

    // CALCULATE OFFSETS (Centered)
    int total_w = w * scale;
    int total_h = bar_len_px;
    
    int ox = (avail_w - total_w) / 2 + (rotate ? 20 : 0);
    int oy = (avail_h - total_h) / 2;

    // DRAW
    GColor bg_color = g_invert_colors ? GColorBlack : GColorWhite;
    GColor fg_color = g_invert_colors ? GColorWhite : GColorBlack;

    graphics_context_set_fill_color(ctx, bg_color);
    graphics_fill_rect(ctx, bounds, 0, GCornerNone);
    graphics_context_set_fill_color(ctx, fg_color);

    for (int r = 0; r < h; r++) {
        int run_start = -1;
        for (int c = 0; c < w; c++) {
            int bit_idx = r * w + c;
            bool is_active = (bits[bit_idx / 8] & (1 << (7 - (bit_idx % 8))));
            
            if (is_active) {
                if (run_start == -1) run_start = c;
            } else {
                if (run_start != -1) {
                    int run_len = c - run_start;
                    if (rotate) {
                        // Vertical Bars (X is vertical position, Y is bar thickness)
                        graphics_fill_rect(ctx, GRect(oy, screen_h - (ox + c*scale), bar_len_px, run_len*scale), 0, GCornerNone);
                    } else {
                        // Standard View
                        graphics_fill_rect(ctx, GRect(ox + run_start*scale, oy + r*scale, run_len*scale, scale), 0, GCornerNone);
                    }
                    run_start = -1;
                }
            }
        }
        if (run_start != -1) {
            int run_len = w - run_start;
            if (rotate) graphics_fill_rect(ctx, GRect(oy, screen_h - (ox + w*scale), bar_len_px, run_len*scale), 0, GCornerNone);
            else graphics_fill_rect(ctx, GRect(ox + run_start*scale, oy + r*scale, run_len*scale, scale), 0, GCornerNone);
        }
    }
}

void barcode_draw(GContext *ctx, GRect bounds, BarcodeFormat format, uint16_t w, uint16_t h, const uint8_t *bits) {
    draw_bits_matrix(ctx, bounds, w, h, bits);
}
