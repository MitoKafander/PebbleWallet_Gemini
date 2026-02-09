#include "common.h"

// Helper: Draw matrix from raw bit-packed array
// Logic: If the code is WIDER than it is TALL, rotate it 90 degrees 
// to utilize the 168px vertical axis as the "width".
static void draw_bits_matrix(GContext *ctx, GRect bounds, uint16_t w, uint16_t h, const uint8_t *bits) {
    if (w == 0 || h == 0 || !bits) return;

    bool rotate = (w > h);
    int screen_w = bounds.size.w;
    int screen_h = bounds.size.h;

    // Determine available space
    int avail_w = rotate ? screen_h : screen_w;
    int avail_h = rotate ? screen_w : screen_h;
    avail_w -= 4; // Margin
    avail_h -= 4; // Margin

    // Integer scaling
    int scale_w = avail_w / w;
    int scale_h = avail_h / h;
    
    // For 2D codes (Aztec/QR), use uniform scale
    int scale = (scale_w < scale_h) ? scale_w : scale_h;
    if (scale < 1) scale = 1;

    // For 1D codes (where h is very small), allow height stretching
    int h_scale = scale;
    if (w > h * 2) {
        h_scale = avail_h / h;
        if (h_scale < scale) h_scale = scale;
    }

    int pix_w = w * scale;
    int pix_h = h * h_scale;

    // Center offsets
    int ox = (avail_w - pix_w) / 2 + 2;
    int oy = (avail_h - pix_h) / 2 + 2;

    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_rect(ctx, bounds, 0, GCornerNone);
    graphics_context_set_fill_color(ctx, GColorBlack);

    for (int r = 0; r < h; r++) {
        int run_start = -1;
        for (int c = 0; c < w; c++) {
            int bit_idx = r * w + c;
            bool is_black = (bits[bit_idx / 8] & (1 << (7 - (bit_idx % 8))));
            
            if (is_black) {
                if (run_start == -1) run_start = c;
            } else {
                if (run_start != -1) {
                    int run_len = c - run_start;
                    if (rotate) {
                        // In rotated mode: 
                        // original row (r) becomes X
                        // original col (c) becomes Y (inverted)
                        graphics_fill_rect(ctx, GRect(oy + r*h_scale, screen_h - (ox + c*scale), h_scale, run_len*scale), 0, GCornerNone);
                    } else {
                        graphics_fill_rect(ctx, GRect(ox + run_start*scale, oy + r*h_scale, run_len*scale, h_scale), 0, GCornerNone);
                    }
                    run_start = -1;
                }
            }
        }
        if (run_start != -1) {
            int run_len = w - run_start;
            if (rotate) {
                graphics_fill_rect(ctx, GRect(oy + r*h_scale, screen_h - (ox + w*scale), h_scale, run_len*scale), 0, GCornerNone);
            } else {
                graphics_fill_rect(ctx, GRect(ox + run_start*scale, oy + r*h_scale, run_len*scale, h_scale), 0, GCornerNone);
            }
        }
    }
}

void barcode_draw(GContext *ctx, GRect bounds, BarcodeFormat format, uint16_t w, uint16_t h, const uint8_t *bits) {
    if (w > 0 && h > 0) {
        draw_bits_matrix(ctx, bounds, w, h, bits);
    } else {
        graphics_context_set_fill_color(ctx, GColorWhite);
        graphics_fill_rect(ctx, bounds, 0, GCornerNone);
        graphics_context_set_text_color(ctx, GColorBlack);
        graphics_draw_text(ctx, (char*)bits, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21), bounds, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
    }
}
