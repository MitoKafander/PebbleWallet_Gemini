#include "common.h"

// Helper: Parse "w,h,data" where data is raw binary bytes (compact)
// Format: "w,h," followed by raw bytes. 
// Since AppMessage/JS strings can contain nulls if not careful,
// we will use a "Safe Hex" approach or just improve the storage logic.
// For now, let's stick to the current string format but make it more robust.

static void draw_precalc_matrix(GContext *ctx, GRect bounds, const char *data) {
    int width = 0, height = 0;
    const char *p = data;
    
    // Parse Width
    while(*p && *p != ',') {
        if(*p >= '0' && *p <= '9') width = width * 10 + (*p - '0');
        p++;
    }
    if(*p == ',') p++; else return;
    
    // Parse Height
    while(*p && *p != ',') {
        if(*p >= '0' && *p <= '9') height = height * 10 + (*p - '0');
        p++;
    }
    if(*p == ',') p++; else return;
    
    if(width <= 0 || height <= 0) return;

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

    int current_bit = 0;
    int total_pixels = width * height;
    int row = 0, col = 0;
    int run_start_col = -1;

    // Check if data is corrupted/truncated
    size_t actual_data_len = strlen(p);
    // Each hex char is 4 bits. 
    if (actual_data_len * 4 < (size_t)total_pixels) {
        // Data is truncated! Show error.
        graphics_context_set_text_color(ctx, GColorBlack);
        graphics_draw_text(ctx, "Data Truncated\nPlease Resync", fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), bounds, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
        return;
    }

    while(*p && current_bit < total_pixels) {
        char c = *p;
        int val = 0;
        if(c >= '0' && c <= '9') val = c - '0';
        else if(c >= 'A' && c <= 'F') val = c - 'A' + 10;
        else if(c >= 'a' && c <= 'f') val = c - 'a' + 10;
        
        for(int b=3; b>=0; b--) {
            if(current_bit >= total_pixels) break;
            bool is_black = (val & (1 << b));
            if (is_black) {
                if (run_start_col == -1) run_start_col = col;
            } else {
                if (run_start_col != -1) {
                    int run_width = col - run_start_col;
                    graphics_fill_rect(ctx, GRect(ox + run_start_col*scale, oy + row*scale, run_width*scale, scale), 0, GCornerNone);
                    run_start_col = -1;
                }
            }
            col++;
            if (col >= width) {
                if (run_start_col != -1) {
                    int run_width = col - run_start_col;
                    graphics_fill_rect(ctx, GRect(ox + run_start_col*scale, oy + row*scale, run_width*scale, scale), 0, GCornerNone);
                    run_start_col = -1;
                }
                col = 0;
                row++;
            }
            current_bit++;
        }
        p++;
    }
}

void barcode_draw(GContext *ctx, GRect bounds, const char *data, BarcodeFormat format) {
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_rect(ctx, bounds, 0, GCornerNone);

    if (format == FORMAT_AZTEC || format == FORMAT_PDF417 || 
       (format == FORMAT_QR && data[0] >= '0' && data[0] <= '9')) {
        draw_precalc_matrix(ctx, bounds, data);
    } 
    else if (format == FORMAT_QR) {
        uint8_t packed[200];
        uint8_t size = 0;
        if (qr_generate_packed(data, packed, &size)) {
            int avail = (bounds.size.w < bounds.size.h ? bounds.size.w : bounds.size.h) - 10;
            int scale = avail / size;
            if (scale < 1) scale = 1;
            int pix_size = size * scale;
            int ox = (bounds.size.w - pix_size) / 2;
            int oy = (bounds.size.h - pix_size) / 2;
            graphics_context_set_fill_color(ctx, GColorWhite);
            graphics_fill_rect(ctx, GRect(ox-4, oy-4, pix_size+8, pix_size+8), 0, GCornerNone);
            graphics_context_set_fill_color(ctx, GColorBlack);
            for(int r=0; r<size; r++) {
                for(int c=0; c<size; c++) {
                    int idx = r*size + c;
                    if (packed[idx/8] & (1 << (7-(idx%8)))) {
                        graphics_fill_rect(ctx, GRect(ox + c*scale, oy + r*scale, scale, scale), 0, GCornerNone);
                    }
                }
            }
        } else {
            graphics_context_set_text_color(ctx, GColorBlack);
            graphics_draw_text(ctx, "QR Error", fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), bounds, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
        }
    } else {
        graphics_context_set_text_color(ctx, GColorBlack);
        graphics_draw_text(ctx, data, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21), bounds, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
        graphics_draw_text(ctx, "(1D Barcode)", fonts_get_system_font(FONT_KEY_GOTHIC_14), GRect(0, bounds.size.h-30, bounds.size.w, 30), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
    }
}