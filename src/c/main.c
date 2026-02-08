#include "common.h"

// --- Main UI & Logic ---

WalletCard g_cards[MAX_CARDS];
int g_card_count = 0;

static Window *s_main_window;
static MenuLayer *s_menu_layer;
static Window *s_detail_window;
static Layer *s_barcode_layer;
static int s_current_index = 0;

// --- AppMessage ---
static void inbox_received_handler(DictionaryIterator *iter, void *context) {
    // 1. Check for sync start/clear
    Tuple *t_sync_start = dict_find(iter, MESSAGE_KEY_CMD_SYNC_START);
    if (t_sync_start) {
        g_card_count = 0; // Reset for incoming sync
        storage_save_count(0);
        menu_layer_reload_data(s_menu_layer);
    }

    // 2. Check for card data
    Tuple *t_idx = dict_find(iter, MESSAGE_KEY_KEY_INDEX);
    Tuple *t_name = dict_find(iter, MESSAGE_KEY_KEY_NAME);
    Tuple *t_desc = dict_find(iter, MESSAGE_KEY_KEY_DESCRIPTION);
    Tuple *t_data = dict_find(iter, MESSAGE_KEY_KEY_DATA);
    Tuple *t_fmt = dict_find(iter, MESSAGE_KEY_KEY_FORMAT);

    if (t_idx && t_name && t_data && t_fmt) {
        int i = t_idx->value->int32;
        if (i >= 0 && i < MAX_CARDS) {
            strncpy(g_cards[i].name, t_name->value->cstring, MAX_NAME_LEN-1);
            
            if (t_desc) {
                strncpy(g_cards[i].description, t_desc->value->cstring, MAX_NAME_LEN-1);
            } else {
                g_cards[i].description[0] = '\0';
            }

            strncpy(g_cards[i].data, t_data->value->cstring, MAX_DATA_LEN-1);
            g_cards[i].format = (BarcodeFormat)t_fmt->value->int32;
            
            // Save immediately
            storage_save_card(i, &g_cards[i]);
            
            // Update count if expanding
            if (i >= g_card_count) {
                g_card_count = i + 1;
                storage_save_count(g_card_count);
            }
            
            menu_layer_reload_data(s_menu_layer);
            
            // If viewing detail of this card, redraw
            if (window_stack_contains_window(s_detail_window) && s_current_index == i) {
                layer_mark_dirty(s_barcode_layer);
            }
        }
    }
    
    // 3. Sync Complete
    Tuple *t_complete = dict_find(iter, MESSAGE_KEY_CMD_SYNC_COMPLETE);
    if (t_complete) {
        // Maybe show a toast?
    }
}

// --- Detail Window (Barcode) ---
static void barcode_update_proc(Layer *layer, GContext *ctx) {
    if (s_current_index >= 0 && s_current_index < g_card_count) {
        barcode_draw(ctx, layer_get_bounds(layer), g_cards[s_current_index].data, g_cards[s_current_index].format);
    }
}

static void detail_click_handler(ClickRecognizerRef recognizer, void *context) {
    // Up/Down to cycle cards
    if (g_card_count <= 1) return;
    
    ButtonId btn = click_recognizer_get_button_id(recognizer);
    if (btn == BUTTON_ID_DOWN) {
        s_current_index = (s_current_index + 1) % g_card_count;
    } else if (btn == BUTTON_ID_UP) {
        s_current_index = (s_current_index - 1 + g_card_count) % g_card_count;
    }
    
    // Update Window Title (if we had one)
    // Redraw
    layer_mark_dirty(s_barcode_layer);
}

static void detail_config_provider(void *ctx) {
    window_single_click_subscribe(BUTTON_ID_UP, detail_click_handler);
    window_single_click_subscribe(BUTTON_ID_DOWN, detail_click_handler);
}

static void detail_window_load(Window *window) {
    Layer *root = window_get_root_layer(window);
    s_barcode_layer = layer_create(layer_get_bounds(root));
    layer_set_update_proc(s_barcode_layer, barcode_update_proc);
    layer_add_child(root, s_barcode_layer);
}

static void detail_window_unload(Window *window) {
    layer_destroy(s_barcode_layer);
}

void ui_push_card_detail(int index) {
    s_current_index = index;
    if (!s_detail_window) {
        s_detail_window = window_create();
        window_set_window_handlers(s_detail_window, (WindowHandlers){
            .load = detail_window_load,
            .unload = detail_window_unload
        });
        window_set_click_config_provider(s_detail_window, detail_config_provider);
    }
    window_stack_push(s_detail_window, true);
    
    // High Brightness (Flashlight mode)
    // Note: light_enable_interaction() is usually enough, but we want it sticky?
    light_enable(true); 
}

// --- Main Menu ---
static uint16_t menu_get_num_rows(MenuLayer *menu_layer, uint16_t section_index, void *data) {
    return (g_card_count > 0) ? g_card_count : 1;
}

static void menu_draw_row(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
    if (g_card_count == 0) {
        menu_cell_basic_draw(ctx, cell_layer, "No Cards", "Add via Settings", NULL);
    } else {
        WalletCard *c = &g_cards[cell_index->row];
        
        char subtitle[MAX_NAME_LEN];
        if (strlen(c->description) > 0) {
            strncpy(subtitle, c->description, sizeof(subtitle));
        } else {
            // Fallback if description is empty
            if (c->format == FORMAT_AZTEC || c->format == FORMAT_PDF417) {
                snprintf(subtitle, sizeof(subtitle), "Boarding Pass Data");
            } else if (c->format == FORMAT_QR) {
                snprintf(subtitle, sizeof(subtitle), "QR Code");
            } else {
                strncpy(subtitle, c->data, sizeof(subtitle));
            }
        }
        
        menu_cell_basic_draw(ctx, cell_layer, c->name, subtitle, NULL);
    }
}

static void menu_select(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
    if (g_card_count > 0) {
        ui_push_card_detail(cell_index->row);
    }
}

static void main_window_load(Window *window) {
    Layer *root = window_get_root_layer(window);
    s_menu_layer = menu_layer_create(layer_get_bounds(root));
    menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks){
        .get_num_rows = menu_get_num_rows,
        .draw_row = menu_draw_row,
        .select_click = menu_select
    });
    menu_layer_set_click_config_onto_window(s_menu_layer, window);
    layer_add_child(root, menu_layer_get_layer(s_menu_layer));
}

static void main_window_unload(Window *window) {
    menu_layer_destroy(s_menu_layer);
}

// --- Entry Point ---
static void init(void) {
    storage_load_cards();
    
    app_message_register_inbox_received(inbox_received_handler);
    app_message_open(2048, 256);

    s_main_window = window_create();
    window_set_window_handlers(s_main_window, (WindowHandlers){
        .load = main_window_load,
        .unload = main_window_unload
    });
    window_stack_push(s_main_window, true);
}

static void deinit(void) {
    window_destroy(s_main_window);
    if (s_detail_window) window_destroy(s_detail_window);
}

int main(void) {
    init();
    app_event_loop();
    deinit();
}
