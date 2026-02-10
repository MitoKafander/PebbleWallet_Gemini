#include "common.h"

// --- Global State ---
WalletCardInfo g_card_infos[MAX_CARDS];
int g_card_count = 0;
uint8_t g_active_bits[MAX_BITS_LEN]; 
bool g_invert_colors = false;

static Window *s_main_window;
static MenuLayer *s_menu_layer;
static Window *s_detail_window;
static Layer *s_barcode_layer;
static int s_current_index = 0;

// --- AppMessage ---
static void inbox_received_handler(DictionaryIterator *iter, void *context) {
    if (dict_find(iter, MESSAGE_KEY_CMD_SYNC_START)) {
        g_card_count = 0;
        storage_save_count(0);
        
        Tuple *t_inv = dict_find(iter, MESSAGE_KEY_KEY_INVERT);
        if (t_inv) {
            g_invert_colors = (t_inv->value->int32 == 1);
            storage_save_settings();
        }
        menu_layer_reload_data(s_menu_layer);
    }

    Tuple *t_idx = dict_find(iter, MESSAGE_KEY_KEY_INDEX);
    Tuple *t_name = dict_find(iter, MESSAGE_KEY_KEY_NAME);
    Tuple *t_desc = dict_find(iter, MESSAGE_KEY_KEY_DESCRIPTION);
    Tuple *t_data = dict_find(iter, MESSAGE_KEY_KEY_DATA);
    Tuple *t_fmt = dict_find(iter, MESSAGE_KEY_KEY_FORMAT);
    Tuple *t_w = dict_find(iter, MESSAGE_KEY_KEY_WIDTH);
    Tuple *t_h = dict_find(iter, MESSAGE_KEY_KEY_HEIGHT);

    if (t_idx && t_name && t_data && t_fmt) {
        int i = t_idx->value->int32;
        if (i >= 0 && i < MAX_CARDS) {
            strncpy(g_card_infos[i].name, t_name->value->cstring, MAX_NAME_LEN-1);
            strncpy(g_card_infos[i].description, t_desc ? t_desc->value->cstring : "", MAX_NAME_LEN-1);
            g_card_infos[i].format = (BarcodeFormat)t_fmt->value->int32;
            g_card_infos[i].width = t_w ? t_w->value->int32 : 0;
            g_card_infos[i].height = t_h ? t_h->value->int32 : 0;
            
            storage_save_card(i, &g_card_infos[i], t_data->value->data, t_data->length);
            
            if (i >= g_card_count) {
                g_card_count = i + 1;
                storage_save_count(g_card_count);
            }
            menu_layer_reload_data(s_menu_layer);
        }
    }
}

static void barcode_update_proc(Layer *layer, GContext *ctx) {
    if (s_current_index >= 0 && s_current_index < g_card_count) {
        WalletCardInfo *info = &g_card_infos[s_current_index];
        GRect bounds = layer_get_bounds(layer);
        barcode_draw(ctx, bounds, info->format, info->width, info->height, g_active_bits);
    }
}

static void detail_click_handler(ClickRecognizerRef recognizer, void *context) {
    if (g_card_count <= 1) return;
    ButtonId btn = click_recognizer_get_button_id(recognizer);
    if (btn == BUTTON_ID_DOWN) s_current_index = (s_current_index + 1) % g_card_count;
    else if (btn == BUTTON_ID_UP) s_current_index = (s_current_index - 1 + g_card_count) % g_card_count;
    
    storage_load_card_data(s_current_index, g_active_bits, MAX_BITS_LEN);
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
    storage_load_card_data(index, g_active_bits, MAX_BITS_LEN);
    if (!s_detail_window) {
        s_detail_window = window_create();
        window_set_window_handlers(s_detail_window, (WindowHandlers){ .load = detail_window_load, .unload = detail_window_unload });
        window_set_click_config_provider(s_detail_window, detail_config_provider);
    }
    window_stack_push(s_detail_window, true);
    light_enable(true); 
}

static uint16_t menu_get_num_rows(MenuLayer *menu_layer, uint16_t section_index, void *data) {
    return (g_card_count > 0) ? g_card_count : 1;
}

static void menu_draw_row(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
    if (g_card_count == 0) menu_cell_basic_draw(ctx, cell_layer, "No Cards", "Add via Settings", NULL);
    else {
        WalletCardInfo *c = &g_card_infos[cell_index->row];
        menu_cell_basic_draw(ctx, cell_layer, c->name, c->description, NULL);
    }
}

static void menu_select(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
    if (g_card_count > 0) ui_push_card_detail(cell_index->row);
}

static void main_window_load(Window *window) {
    Layer *root = window_get_root_layer(window);
    s_menu_layer = menu_layer_create(layer_get_bounds(root));
    menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks){ .get_num_rows = menu_get_num_rows, .draw_row = menu_draw_row, .select_click = menu_select });
    menu_layer_set_click_config_onto_window(s_menu_layer, window);
    layer_add_child(root, menu_layer_get_layer(s_menu_layer));
}

static void main_window_unload(Window *window) {
    menu_layer_destroy(s_menu_layer);
}

static void init(void) {
    storage_load_settings();
    app_message_register_inbox_received(inbox_received_handler);
    app_message_open(2048, 256);
    s_main_window = window_create();
    window_set_window_handlers(s_main_window, (WindowHandlers){ .load = main_window_load, .unload = main_window_unload });
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