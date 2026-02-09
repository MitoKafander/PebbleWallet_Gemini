#include "common.h"
#include <stddef.h> 

// Keys:
// PERSIST_KEY_COUNT: card count
// PERSIST_KEY_BASE-1: Global Invert Setting
// BASE + (i*12): Info
// BASE + (i*12) + 1..11: Data

#define KEYS_PER_CARD 12
#define STORAGE_CHUNK_SIZE 100 
#define KEY_SETTING_INVERT (PERSIST_KEY_BASE - 1)

void storage_load_settings(void) {
    // 1. Load Invert Setting
    g_invert_colors = persist_exists(KEY_SETTING_INVERT) ? persist_read_bool(KEY_SETTING_INVERT) : false;

    // 2. Load Card Infos
    if (!persist_exists(PERSIST_KEY_COUNT)) {
        g_card_count = 0;
        return;
    }
    g_card_count = persist_read_int(PERSIST_KEY_COUNT);
    if (g_card_count > MAX_CARDS) g_card_count = MAX_CARDS;

    for (int i = 0; i < g_card_count; i++) {
        persist_read_data(PERSIST_KEY_BASE + (i * KEYS_PER_CARD), &g_card_infos[i], sizeof(WalletCardInfo));
    }
}

void storage_save_settings(void) {
    persist_write_bool(KEY_SETTING_INVERT, g_invert_colors);
}

void storage_load_card_data(int index, uint8_t *buffer, int max_len) {
    if (!buffer || index < 0 || index >= g_card_count) return;
    int base_key = PERSIST_KEY_BASE + (index * KEYS_PER_CARD);
    
    int total_read = 0;
    for (int k=1; k < KEYS_PER_CARD; k++) {
         int chunk_key = base_key + k;
         if (persist_exists(chunk_key)) {
             int read = persist_read_data(chunk_key, buffer + total_read, STORAGE_CHUNK_SIZE);
             total_read += read;
         } else break;
    }
}

void storage_save_card(int index, WalletCardInfo *info, const uint8_t *bits, int bits_len) {
    if (index < 0 || index >= MAX_CARDS) return;
    int base_key = PERSIST_KEY_BASE + (index * KEYS_PER_CARD);
    persist_write_data(base_key, info, sizeof(WalletCardInfo));

    int offset = 0;
    for (int k=1; k < KEYS_PER_CARD; k++) {
        int chunk_key = base_key + k;
        if (offset < bits_len) {
            int remaining = bits_len - offset;
            int write_len = (remaining > STORAGE_CHUNK_SIZE) ? STORAGE_CHUNK_SIZE : remaining;
            persist_write_data(chunk_key, bits + offset, write_len);
            offset += write_len;
        } else if (persist_exists(chunk_key)) persist_delete(chunk_key);
    }
}

void storage_save_count(int count) {
    if (count > MAX_CARDS) count = MAX_CARDS;
    g_card_count = count;
    persist_write_int(PERSIST_KEY_COUNT, count);
}

void storage_clear_all(void) {
    storage_save_count(0);
}