#include "common.h"

// Helper: Draw matrix from raw bit-packed array
static void draw_bits_matrix(GContext *ctx, GRect bounds, uint16_t w, uint16_t h, const uint8_t *bits) {
    if (w == 0 || h == 0 || !bits) return;

    bool rotate = (w > h);
    int screen_w = bounds.size.w;
    int screen_h = bounds.size.h;

    int avail_w = rotate ? screen_h : screen_w;
    int avail_h = rotate ? screen_w : screen_h;
    // Zero margins to allow max integer scaling
    // avail_w -= 2; avail_h -= 2;

    int scale_w = avail_w / w;
    int scale_h = avail_h / h;
    int scale = (scale_w < scale_h) ? scale_w : scale_h;
    if (scale < 1) scale = 1;

    int h_scale = scale;
    if (w > h * 2) {
        h_scale = avail_h / h;
        if (h_scale < scale) h_scale = scale;
    }

    int pix_w = w * scale;
    int pix_h = h * h_scale;
    int ox = (avail_w - pix_w) / 2 + 1;
    int oy = (avail_h - pix_h) / 2 + 1;

    // Use current invert setting
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
                    if (rotate) graphics_fill_rect(ctx, GRect(oy + r*h_scale, screen_h - (ox + c*scale), h_scale, run_len*scale), 0, GCornerNone);
                    else graphics_fill_rect(ctx, GRect(ox + run_start*scale, oy + r*h_scale, run_len*scale, h_scale), 0, GCornerNone);
                    run_start = -1;
                }
            }
        }
        if (run_start != -1) {
            int run_len = w - run_start;
            if (rotate) graphics_fill_rect(ctx, GRect(oy + r*h_scale, screen_h - (ox + w*scale), h_scale, run_len*scale), 0, GCornerNone);
            else graphics_fill_rect(ctx, GRect(ox + run_start*scale, oy + r*h_scale, run_len*scale, h_scale), 0, GCornerNone);
        }
    }
}

void barcode_draw(GContext *ctx, GRect bounds, BarcodeFormat format, uint16_t w, uint16_t h, const uint8_t *bits) {
    if (w > 0 && h > 0) {
        draw_bits_matrix(ctx, bounds, w, h, bits);
    } else {
        GColor bg_color = g_invert_colors ? GColorBlack : GColorWhite;
        GColor fg_color = g_invert_colors ? GColorWhite : GColorBlack;
        graphics_context_set_fill_color(ctx, bg_color);
        graphics_fill_rect(ctx, bounds, 0, GCornerNone);
        graphics_context_set_text_color(ctx, fg_color);
        graphics_draw_text(ctx, (char*)bits, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21), bounds, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
    }
}