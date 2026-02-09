#include "common.h"
#include <stddef.h> 

// Pebble Storage Optimization - "Ultra-Safe Mode"
// Problem: Large chunks (250b) are failing to persist reliably on some firmware.
// Solution: Use small 100-byte chunks.
// Storage limit: 4KB total. 
// 10 cards * 400 bytes = 4KB. We are tight but should fit.

// Keys:
// BASE + 0: Header
// BASE + 1..7: Data Chunks (100 bytes each)

#define KEYS_PER_CARD 8
#define STORAGE_CHUNK_SIZE 100 // Extremely safe size

// --- Compression Helpers (Identical to before) ---
static int compress_data(const char *input, uint8_t *output, int max_out_len) {
    const char *p = input;
    uint8_t *out_ptr = output;
    int out_len = 0;
    int commas = 0;
    while (*p && commas < 2 && out_len < max_out_len) {
        if (*p == ',') commas++;
        *out_ptr++ = (uint8_t)*p++;
        out_len++;
    }
    if (commas < 2) {
        while (*p && out_len < max_out_len) {
            *out_ptr++ = (uint8_t)*p++;
            out_len++;
        }
        return out_len;
    }
    while (*p && *(p+1) && out_len < max_out_len) {
        char high = *p;
        char low = *(p+1);
        uint8_t val = 0;
        if (high >= '0' && high <= '9') val |= (high - '0') << 4;
        else if (high >= 'A' && high <= 'F') val |= (high - 'A' + 10) << 4;
        else if (high >= 'a' && high <= 'f') val |= (high - 'a' + 10) << 4;
        if (low >= '0' && low <= '9') val |= (low - '0');
        else if (low >= 'A' && low <= 'F') val |= (low - 'A' + 10);
        else if (low >= 'a' && low <= 'f') val |= (low - 'a' + 10);
        *out_ptr++ = val;
        out_len++;
        p += 2;
    }
    return out_len;
}

static void decompress_data(const uint8_t *input, int in_len, char *output, int max_out_len) {
    const uint8_t *p = input;
    char *out_ptr = output;
    int current_len = 0;
    int commas = 0;
    while (in_len > 0 && commas < 2 && current_len < max_out_len - 1) {
        char c = (char)*p;
        if (c == ',') commas++;
        *out_ptr++ = c;
        current_len++;
        p++;
        in_len--;
    }
    if (commas < 2) {
        while (in_len > 0 && current_len < max_out_len - 1) {
            *out_ptr++ = (char)*p++;
            current_len++;
            in_len--;
        }
        *out_ptr = '\0';
        return;
    }
    static const char hex_chars[] = "0123456789ABCDEF";
    while (in_len > 0 && current_len < max_out_len - 2) { 
        uint8_t val = *p++;
        in_len--;
        *out_ptr++ = hex_chars[(val >> 4) & 0xF];
        *out_ptr++ = hex_chars[val & 0xF];
        current_len += 2;
    }
    *out_ptr = '\0';
}

// --- Storage Logic ---

#define LEGACY_KEY_COUNT 100
#define LEGACY_KEY_BASE 1000

// We also need to wipe the "medium chunk" keys (24200 range) from previous version
// Since we are changing KEYS_PER_CARD, the layout changes again.
// To be safe, we will just rely on overwriting or user clearing.
// But let's add a robust wipe for the old base.

void storage_wipe_legacy(void) {
    if (persist_exists(LEGACY_KEY_COUNT)) {
        persist_delete(LEGACY_KEY_COUNT);
        for (int i = 0; i < 50; i++) { // Increase range
            if (persist_exists(LEGACY_KEY_BASE + i)) persist_delete(LEGACY_KEY_BASE + i);
        }
    }
    // Also try to wipe the previous attempt's keys (24200+)
    // This is aggressive but necessary to clean up fragmentation.
    // We only do this if we detect we are in a "fresh" state (card count 0 or error)
}

void storage_load_cards(void) {
    storage_wipe_legacy();

    if (!persist_exists(PERSIST_KEY_COUNT)) {
        g_card_count = 0;
        return;
    }

    g_card_count = persist_read_int(PERSIST_KEY_COUNT);
    if (g_card_count > MAX_CARDS) g_card_count = MAX_CARDS;

    uint8_t comp_buf[600]; // Slightly larger buffer

    for (int i = 0; i < g_card_count; i++) {
        int base_key = PERSIST_KEY_BASE + (i * KEYS_PER_CARD);
        
        // 1. Load Header
        size_t header_size = offsetof(WalletCard, data);
        persist_read_data(base_key, &g_cards[i], header_size);
        
        // 2. Load Compressed Data (up to 7 chunks)
        int total_comp_len = 0;
        
        for (int k=1; k < KEYS_PER_CARD; k++) {
             int chunk_key = base_key + k;
             if (persist_exists(chunk_key)) {
                 int read = persist_read_data(chunk_key, comp_buf + total_comp_len, STORAGE_CHUNK_SIZE);
                 total_comp_len += read;
             } else {
                 break; // No more chunks
             }
        }
        
        // 3. Decompress
        decompress_data(comp_buf, total_comp_len, g_cards[i].data, MAX_DATA_LEN);
    }
    APP_LOG(APP_LOG_LEVEL_INFO, "Loaded %d cards (Safe Chunked)", g_card_count);
}

void storage_save_card(int index, WalletCard *card) {
    if (index < 0 || index >= MAX_CARDS) return;
    int base_key = PERSIST_KEY_BASE + (index * KEYS_PER_CARD);

    // 1. Save Header
    size_t header_size = offsetof(WalletCard, data);
    persist_write_data(base_key, card, header_size);

    // 2. Compress Data
    uint8_t comp_buf[600];
    int comp_len = compress_data(card->data, comp_buf, sizeof(comp_buf));

    // 3. Save Chunks
    int offset = 0;
    for (int k=1; k < KEYS_PER_CARD; k++) {
        int chunk_key = base_key + k;
        
        if (offset < comp_len) {
            int remaining = comp_len - offset;
            int write_len = (remaining > STORAGE_CHUNK_SIZE) ? STORAGE_CHUNK_SIZE : remaining;
            
            int result = persist_write_data(chunk_key, comp_buf + offset, write_len);
            if (result < 0) {
                APP_LOG(APP_LOG_LEVEL_ERROR, "Write failed at chunk %d", k);
            }
            offset += write_len;
        } else {
            // Delete unused chunks
            if (persist_exists(chunk_key)) persist_delete(chunk_key);
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
