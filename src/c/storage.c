#include "common.h"

// We store cards individually to avoid hitting max Persist blob size
// KEY_COUNT: int
// KEY_BASE + index: WalletCard struct

void storage_load_cards(void) {
    if (persist_exists(PERSIST_KEY_COUNT)) {
        g_card_count = persist_read_int(PERSIST_KEY_COUNT);
        if (g_card_count > MAX_CARDS) g_card_count = MAX_CARDS;

        for (int i = 0; i < g_card_count; i++) {
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
    persist_write_data(PERSIST_KEY_BASE + index, card, sizeof(WalletCard));
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
