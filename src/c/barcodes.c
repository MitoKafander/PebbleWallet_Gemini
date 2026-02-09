#include "common.h"

// Helper: Draw matrix from raw bit-packed array
static void draw_bits_matrix(GContext *ctx, GRect bounds, uint16_t width, uint16_t height, const uint8_t *bits) {
    if (width == 0 || height == 0 || !bits) return;

    int avail_w = bounds.size.w - 10;
    int avail_h = bounds.size.h - 10;
    int scale_w = avail_w / width;
    int scale_h = avail_h / height;
    int scale = (scale_w < scale_h) ? scale_w : scale_h;
    if (scale < 1) scale = 1;
    
    int pix_w = width * scale;
    int pix_h = height * scale;
    int ox = (bounds.size.w - pix_w) / 2;
    int oy = (bounds.size.h - pix_h) / 2;
    
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_rect(ctx, GRect(ox-4, oy-4, pix_w+8, pix_h+8), 0, GCornerNone);
    graphics_context_set_fill_color(ctx, GColorBlack);

    for (int r = 0; r < height; r++) {
        int run_start = -1;
        for (int c = 0; r < height && c < width; c++) {
            int bit_idx = r * width + c;
            bool is_black = (bits[bit_idx / 8] & (1 << (7 - (bit_idx % 8))));
            
            if (is_black) {
                if (run_start == -1) run_start = c;
            } else {
                if (run_start != -1) {
                    graphics_fill_rect(ctx, GRect(ox + run_start*scale, oy + r*scale, (c - run_start)*scale, scale), 0, GCornerNone);
                    run_start = -1;
                }
            }
        }
        if (run_start != -1) {
            graphics_fill_rect(ctx, GRect(ox + run_start*scale, oy + r*scale, (width - run_start)*scale, scale), 0, GCornerNone);
        }
    }
}

void barcode_draw(GContext *ctx, GRect bounds, BarcodeFormat format, uint16_t w, uint16_t h, const uint8_t *bits) {
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_rect(ctx, bounds, 0, GCornerNone);

    if (w > 0 && h > 0) {
        draw_bits_matrix(ctx, bounds, w, h, bits);
    } else {
        // 1D Barcode (Text fallback)
        graphics_context_set_text_color(ctx, GColorBlack);
        graphics_draw_text(ctx, (char*)bits, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21), bounds, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
        graphics_draw_text(ctx, "(1D Barcode)", fonts_get_system_font(FONT_KEY_GOTHIC_14), GRect(0, bounds.size.h-30, bounds.size.w, 30), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
    }
}
