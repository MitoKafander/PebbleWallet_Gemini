#pragma once
#include <pebble.h>

// --- Constants ---
#define MAX_CARDS 10
#define MAX_NAME_LEN 32
// Increased buffer for complex Aztec codes (80x80 matrix = ~1600 chars hex)
#define MAX_DATA_LEN 2500 

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

// Lightweight struct for the menu list (kept in RAM)
typedef struct {
    BarcodeFormat format;
    char name[MAX_NAME_LEN];
    char description[MAX_NAME_LEN]; 
} WalletCardInfo;

// Heavy struct for the active card (loaded on demand)
typedef struct {
    char data[MAX_DATA_LEN];
} WalletCardData;

// --- Global State ---
extern WalletCardInfo g_card_infos[MAX_CARDS];
extern int g_card_count;
// Shared buffer for the currently active barcode to save RAM
extern char g_active_card_data[MAX_DATA_LEN]; 

// --- Modules ---
void storage_load_infos(void);
void storage_load_card_data(int index, char *buffer, int max_len);
void storage_save_card(int index, WalletCardInfo *info, const char *data);
void storage_save_count(int count);
void storage_clear_all(void);

// QR Generator
bool qr_generate_packed(const char *data, uint8_t *output_buffer, uint8_t *out_size);

// Barcode Renderer
void barcode_draw(GContext *ctx, GRect bounds, const char *data, BarcodeFormat format);

// UI
void ui_push_main_menu(void);
void ui_push_card_detail(int index);
void ui_redraw_main_menu(void);
