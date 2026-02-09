#include "common.h"
#include <stddef.h> 

// Pebble's PERSIST_DATA_MAX_LENGTH is 256 bytes.
// We must split our 1024-byte data across multiple keys.
#define CHUNK_SIZE 240 
#define KEYS_PER_CARD 5 // 1 for header, 4 for data chunks

// Old keys to wipe
#define LEGACY_KEY_COUNT 100
#define LEGACY_KEY_BASE 1000

void storage_wipe_legacy(void) {
    if (persist_exists(LEGACY_KEY_COUNT)) {
        APP_LOG(APP_LOG_LEVEL_INFO, "Migrating: Wiping legacy storage...");
        persist_delete(LEGACY_KEY_COUNT);
        for (int i = 0; i < 15; i++) { // Wipe a few more than MAX_CARDS just in case
            if (persist_exists(LEGACY_KEY_BASE + i)) {
                persist_delete(LEGACY_KEY_BASE + i);
            }
        }
    }
}

void storage_load_cards(void) {
    // 1. Run Cleanup First
    storage_wipe_legacy();

    if (!persist_exists(PERSIST_KEY_COUNT)) {
        g_card_count = 0;
        return;
    }

    g_card_count = persist_read_int(PERSIST_KEY_COUNT);
    if (g_card_count > MAX_CARDS) g_card_count = MAX_CARDS;

    for (int i = 0; i < g_card_count; i++) {
        int base_key = PERSIST_KEY_BASE + (i * KEYS_PER_CARD);
        
        // 1. Load Header (Format, Name, Desc)
        // We read only the beginning of the struct (before the data field)
        size_t header_size = offsetof(WalletCard, data);
        persist_read_data(base_key, &g_cards[i], header_size);
        
        // 2. Load Data Chunks
        char *data_ptr = g_cards[i].data;
        memset(data_ptr, 0, MAX_DATA_LEN);
        
        for (int j = 0; j < 4; j++) {
            int chunk_key = base_key + 1 + j;
            if (persist_exists(chunk_key)) {
                persist_read_data(chunk_key, data_ptr + (j * CHUNK_SIZE), CHUNK_SIZE);
            }
        }
    }
    APP_LOG(APP_LOG_LEVEL_INFO, "Loaded %d cards (Chunked)", g_card_count);
}

void storage_save_card(int index, WalletCard *card) {
    if (index < 0 || index >= MAX_CARDS) return;
    int base_key = PERSIST_KEY_BASE + (index * KEYS_PER_CARD);

    // 1. Save Header
    size_t header_size = offsetof(WalletCard, data);
    persist_write_data(base_key, card, header_size);

    // 2. Save Data in Chunks
    char *data_ptr = card->data;
    size_t total_len = strlen(data_ptr) + 1; // Include null terminator

    for (int j = 0; j < 4; j++) {
        int chunk_key = base_key + 1 + j;
        size_t offset = j * CHUNK_SIZE;
        
        if (offset < total_len) {
            size_t remaining = total_len - offset;
            size_t to_write = (remaining > CHUNK_SIZE) ? CHUNK_SIZE : remaining;
            persist_write_data(chunk_key, data_ptr + offset, to_write);
        } else {
            // Delete old chunk if it's no longer needed
            persist_delete(chunk_key);
        }
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
