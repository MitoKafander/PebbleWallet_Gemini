#pragma once
#include <pebble.h>

// --- Constants ---
#define MAX_CARDS 10
#define MAX_NAME_LEN 32
// 1000 bytes of raw bits = 8000 pixels (enough for 88x88 matrix)
#define MAX_BITS_LEN 1000 

#define PERSIST_KEY_COUNT 500
#define PERSIST_KEY_BASE 24200

// --- Types ---
typedef enum {
    FORMAT_CODE128 = 0,
    FORMAT_CODE39 = 1,
    FORMAT_EAN13 = 2,
    FORMAT_QR = 3,
    FORMAT_AZTEC = 4,   
    FORMAT_PDF417 = 5   
} BarcodeFormat;

typedef struct {
    BarcodeFormat format;
    char name[MAX_NAME_LEN];
    char description[MAX_NAME_LEN]; 
    uint16_t width;
    uint16_t height;
} WalletCardInfo;

// --- Global State ---
extern WalletCardInfo g_card_infos[MAX_CARDS];
extern int g_card_count;
extern uint8_t g_active_bits[MAX_BITS_LEN]; 
extern bool g_invert_colors; 

// --- Modules ---
void storage_load_settings(void);
void storage_save_settings(void);
void storage_load_card_data(int index, uint8_t *buffer, int max_len);
void storage_save_card(int index, WalletCardInfo *info, const uint8_t *bits, int bits_len);
void storage_save_count(int count);
void storage_clear_all(void);

// Barcode Renderer
void barcode_draw(GContext *ctx, GRect bounds, BarcodeFormat format, uint16_t w, uint16_t h, const uint8_t *bits);

// QR Code Generator
bool qr_generate_packed(const char *data, uint8_t *output_buffer, uint8_t *out_size);

// UI
void ui_push_main_menu(void);
void ui_push_card_detail(int index);
void ui_redraw_main_menu(void);