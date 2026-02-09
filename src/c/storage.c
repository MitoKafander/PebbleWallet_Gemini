#include "common.h"
#include <stddef.h> 

// Pebble Storage Optimization
// Problem: Barcode matrices (Hex Strings) are huge (e.g., "A1F0..." is 2 bytes per 4 bits of data).
// Solution: Compress Hex Strings to Binary (1 byte per 8 bits) before saving.
// This reduces storage usage by 50%.

// Keys:
// BASE + 0: Header (Format, Name, Desc)
// BASE + 1: Compressed Data Part 1 (max 256 bytes)
// BASE + 2: Compressed Data Part 2 (overflow, if needed)

#define KEYS_PER_CARD 3
#define STORAGE_CHUNK_SIZE 250 // Leave a little safety margin below 256

// --- Compression Helpers ---

// "23,23,A1F0..." -> "23,23," + [0xA1, 0xF0...]
// Returns total size of the compressed output
static int compress_data(const char *input, uint8_t *output, int max_out_len) {
    const char *p = input;
    uint8_t *out_ptr = output;
    int out_len = 0;

    // 1. Copy the "width,height," prefix (ASCII)
    // Find second comma
    int commas = 0;
    while (*p && commas < 2 && out_len < max_out_len) {
        if (*p == ',') commas++;
        *out_ptr++ = (uint8_t)*p++;
        out_len++;
    }

    // If we didn't find 2 commas, it's not a matrix string (maybe 1D?). Just copy the rest.
    if (commas < 2) {
        while (*p && out_len < max_out_len) {
            *out_ptr++ = (uint8_t)*p++;
            out_len++;
        }
        return out_len;
    }

    // 2. Compress the Hex String
    // "A1" -> 0xA1
    while (*p && *(p+1) && out_len < max_out_len) {
        char high = *p;
        char low = *(p+1);
        uint8_t val = 0;

        // High Nibble
        if (high >= '0' && high <= '9') val |= (high - '0') << 4;
        else if (high >= 'A' && high <= 'F') val |= (high - 'A' + 10) << 4;
        else if (high >= 'a' && high <= 'f') val |= (high - 'a' + 10) << 4;

        // Low Nibble
        if (low >= '0' && low <= '9') val |= (low - '0');
        else if (low >= 'A' && low <= 'F') val |= (low - 'A' + 10);
        else if (low >= 'a' && low <= 'f') val |= (low - 'a' + 10);

        *out_ptr++ = val;
        out_len++;
        p += 2;
    }

    return out_len;
}

// "23,23," + [0xA1...] -> "23,23,A1..."
static void decompress_data(const uint8_t *input, int in_len, char *output, int max_out_len) {
    const uint8_t *p = input;
    char *out_ptr = output;
    int current_len = 0;

    // 1. Copy Prefix
    // Scan for second comma in the binary stream (it's ASCII)
    int commas = 0;
    while (in_len > 0 && commas < 2 && current_len < max_out_len - 1) {
        char c = (char)*p;
        if (c == ',') commas++;
        *out_ptr++ = c;
        current_len++;
        p++;
        in_len--;
    }

    // If we didn't find 2 commas, just copy the rest as-is (it was raw text)
    if (commas < 2) {
        while (in_len > 0 && current_len < max_out_len - 1) {
            *out_ptr++ = (char)*p++;
            current_len++;
            in_len--;
        }
        *out_ptr = '\0';
        return;
    }

    // 2. Decompress Binary to Hex
    static const char hex_chars[] = "0123456789ABCDEF";
    while (in_len > 0 && current_len < max_out_len - 2) { // Need 2 chars
        uint8_t val = *p++;
        in_len--;

        *out_ptr++ = hex_chars[(val >> 4) & 0xF];
        *out_ptr++ = hex_chars[val & 0xF];
        current_len += 2;
    }
    *out_ptr = '\0';
}


// --- Storage Logic ---

// Old keys to wipe (Legacy)
#define LEGACY_KEY_COUNT 100
#define LEGACY_KEY_BASE 1000

void storage_wipe_legacy(void) {
    if (persist_exists(LEGACY_KEY_COUNT)) {
        persist_delete(LEGACY_KEY_COUNT);
        for (int i = 0; i < 20; i++) {
            if (persist_exists(LEGACY_KEY_BASE + i)) persist_delete(LEGACY_KEY_BASE + i);
        }
    }
}

void storage_load_cards(void) {
    storage_wipe_legacy();

    if (!persist_exists(PERSIST_KEY_COUNT)) {
        g_card_count = 0;
        return;
    }

    g_card_count = persist_read_int(PERSIST_KEY_COUNT);
    if (g_card_count > MAX_CARDS) g_card_count = MAX_CARDS;

    // Temp buffer for compressed data
    uint8_t comp_buf[512]; 

    for (int i = 0; i < g_card_count; i++) {
        int base_key = PERSIST_KEY_BASE + (i * KEYS_PER_CARD);
        
        // 1. Load Header
        size_t header_size = offsetof(WalletCard, data);
        persist_read_data(base_key, &g_cards[i], header_size);
        
        // 2. Load Compressed Data
        int total_comp_len = 0;
        
        // Chunk 1
        if (persist_exists(base_key + 1)) {
            int read = persist_read_data(base_key + 1, comp_buf, STORAGE_CHUNK_SIZE);
            total_comp_len += read;
        }
        // Chunk 2 (if exists)
        if (persist_exists(base_key + 2)) {
            int read = persist_read_data(base_key + 2, comp_buf + total_comp_len, STORAGE_CHUNK_SIZE);
            total_comp_len += read;
        }
        
        // 3. Decompress
        decompress_data(comp_buf, total_comp_len, g_cards[i].data, MAX_DATA_LEN);
    }
    APP_LOG(APP_LOG_LEVEL_INFO, "Loaded %d cards (Compressed)", g_card_count);
}

void storage_save_card(int index, WalletCard *card) {
    if (index < 0 || index >= MAX_CARDS) return;
    int base_key = PERSIST_KEY_BASE + (index * KEYS_PER_CARD);

    // 1. Save Header
    size_t header_size = offsetof(WalletCard, data);
    persist_write_data(base_key, card, header_size);

    // 2. Compress Data
    uint8_t comp_buf[512];
    int comp_len = compress_data(card->data, comp_buf, sizeof(comp_buf));

    // 3. Save Compressed Data (Split if needed)
    // Chunk 1
    int chunk1_len = (comp_len > STORAGE_CHUNK_SIZE) ? STORAGE_CHUNK_SIZE : comp_len;
    persist_write_data(base_key + 1, comp_buf, chunk1_len);
    
    // Chunk 2
    if (comp_len > STORAGE_CHUNK_SIZE) {
        int chunk2_len = comp_len - STORAGE_CHUNK_SIZE;
        persist_write_data(base_key + 2, comp_buf + STORAGE_CHUNK_SIZE, chunk2_len);
    } else {
        // Clean up overflow key if not needed
        if (persist_exists(base_key + 2)) persist_delete(base_key + 2);
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