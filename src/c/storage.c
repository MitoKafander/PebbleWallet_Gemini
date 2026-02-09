#include "common.h"
#include <stddef.h> // for offsetof

// We store cards individually to avoid hitting max Persist blob size
// KEY_COUNT: int
// KEY_BASE + index: WalletCard struct

void storage_load_cards(void) {
    if (persist_exists(PERSIST_KEY_COUNT)) {
        g_card_count = persist_read_int(PERSIST_KEY_COUNT);
        if (g_card_count > MAX_CARDS) g_card_count = MAX_CARDS;

        for (int i = 0; i < g_card_count; i++) {
            // Clear memory first (important because we write variable length!)
            memset(&g_cards[i], 0, sizeof(WalletCard));
            // Read what we have. If stored data is shorter than sizeof(WalletCard),
            // the rest remains 0 (which is safe, as we zeroed it).
            persist_read_data(PERSIST_KEY_BASE + i, &g_cards[i], sizeof(WalletCard));
        }
        APP_LOG(APP_LOG_LEVEL_INFO, "Loaded %d cards from storage", g_card_count);
    } else {
        g_card_count = 0;
        APP_LOG(APP_LOG_LEVEL_INFO, "No existing storage found");
    }
}

void storage_save_card(int index, WalletCard *card) {
    if (index < 0 || index >= MAX_CARDS) return;
    
    // Optimization: Only write the actual used size to save persistent storage space.
    // Especially important on Aplite (4KB limit).
    // Write: Format + Name + Desc + Data(up to null terminator)
    size_t data_len = strlen(card->data);
    size_t write_size = offsetof(WalletCard, data) + data_len + 1; 
    
    // Safety check
    if(write_size > sizeof(WalletCard)) write_size = sizeof(WalletCard);
    
    persist_write_data(PERSIST_KEY_BASE + index, card, write_size);
}

void storage_save_count(int count) {
    if (count > MAX_CARDS) count = MAX_CARDS;
    g_card_count = count;
    persist_write_int(PERSIST_KEY_COUNT, count);
}

void storage_clear_all(void) {
    // Optional: actually delete keys
    // For now, just setting count to 0 is enough logic-wise
    storage_save_count(0);
}