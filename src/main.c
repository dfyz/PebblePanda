#include <pebble.h>

#define SCREEN_WIDTH 144
#define SCREEN_HEIGHT 168

#define TZ_NAME 0
#define TZ_UTC_OFFSET 1

typedef enum {
    LI_TZ_NAME = 0,
    LI_TZ_UTC_OFFSET,
    LI_LAST,
} LabelId;

#define MAX_LABELS 10
#define LABEL_HEIGHT 30
    
typedef struct {
    TextLayer* labels[MAX_LABELS];
    size_t count;
} TLabels;

void labels_add_new(TLabels* labels, Window* main_window) {
    size_t row_index = labels->count;
    TextLayer* current_label = text_layer_create(GRect(0, LABEL_HEIGHT * row_index, SCREEN_WIDTH, LABEL_HEIGHT));    

    // text_layer_set_font(current_label, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_PT_SANS_24)));
    text_layer_set_font(current_label, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
    text_layer_set_text_alignment(current_label, GTextAlignmentCenter);
    
    labels->labels[labels->count++] = current_label;
    
    layer_add_child(window_get_root_layer(main_window), text_layer_get_layer(current_label));
}

void labels_clean(TLabels* labels) {
    for (size_t i = 0, count = labels->count; i < count; ++i) {
        text_layer_destroy(labels->labels[i]);
    }
}

void labels_set_text(TLabels* labels, LabelId label_id, const char* text) {
    text_layer_set_text(labels->labels[label_id], text);
}

static Window* main_window;
static TLabels labels;

static AppSync sync;
static uint8_t sync_buffer[200];

void redraw() {
    const Tuple* tz_name_tuple = app_sync_get(&sync, TZ_NAME);
    if (tz_name_tuple) {
        static char tz_name_buffer[32];
        snprintf(tz_name_buffer, sizeof(tz_name_buffer), "%s", tz_name_tuple->value->cstring);
        labels_set_text(&labels, LI_TZ_NAME, tz_name_buffer);
    }
    
    const Tuple* tz_utc_offset_tuple = app_sync_get(&sync, TZ_UTC_OFFSET);
    if (tz_utc_offset_tuple) {
        static char tz_utc_offset_buffer[10];
        int tz_utc_offset = tz_utc_offset_tuple->value->int32;
        tz_utc_offset /= 60 * 60;
        const char* tz_utc_offset_sign = tz_utc_offset >= 0 ? "+" : "-";
        snprintf(tz_utc_offset_buffer, sizeof(tz_utc_offset_buffer), "UTC%s%d", tz_utc_offset_sign, tz_utc_offset);
        labels_set_text(&labels, LI_TZ_UTC_OFFSET, tz_utc_offset_buffer);
    }
}

void sync_changed_handler(const uint32_t key, const Tuple *new_tuple, const Tuple *old_tuple, void *context) {
    redraw();
}

void sync_error_handler(DictionaryResult dict_error, AppMessageResult app_message_error, void *context) {
}

void setup_app_sync() {
    app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
    
    Tuplet initial_values[] = {
        TupletCString(TZ_NAME, "???"),
        TupletInteger(TZ_UTC_OFFSET, 0), 
    };
    
    app_sync_init(&sync, sync_buffer, sizeof(sync_buffer), initial_values, ARRAY_LENGTH(initial_values),
                  sync_changed_handler, sync_error_handler, NULL);
}

void handle_init() {
	main_window = window_create();
    
    setup_app_sync();
    
    for (size_t i = 0; i < LI_LAST; i++) {
        labels_add_new(&labels, main_window);
    }
    redraw();
    
	window_stack_push(main_window, true);
}

void handle_deinit() {
    labels_clean(&labels);
	window_destroy(main_window);
    app_sync_deinit(&sync);
}

int main(void) {  
    handle_init();
    app_event_loop();
    handle_deinit();
}
