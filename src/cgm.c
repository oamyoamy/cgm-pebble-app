#include "pebble.h"
#include "chart.h"
#include "private.h"

static Window *window;

static TextLayer *bg_layer;
static TextLayer *readtime_layer;
static TextLayer *datetime_layer;
static TextLayer *message_layer;
static TextLayer *clock_layer;
static TextLayer *status_layer;
static TextLayer *divider_layer;
static BitmapLayer *icon_layer;
static GBitmap *icon_bitmap = NULL;

static GFont s_time_font;
static GFont s_bg_font;
static GFont s_msg_font;
static GFont s_sts_font;

static AppSync sync;

static int last_read = -1;
static int last_icon = 0;

static int last_readtime = 0;
static int last_uploadtime = 0;

static int alert_active = 0;

static uint8_t sync_buffer[256];
static char new_time[124];
static char last_bg[124];
static char check_string[124] = "Checking for new data...";
static char time_text[124] = "Loading...";
static char status_text[124] = "--";
static char str[124] = "--";
static char str_read[124] = "--";
static char str_up[124] = "--";

static AppTimer *timer;

static const uint32_t const high[] = { 100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100};
static const uint32_t const low[] = { 1000,100,2000};

static const uint32_t const hypo[] = { 3200,200,3200 };
static const uint32_t const hyper[] = { 50,150,50,150,50,150,50,150,50,150,50,150,50,150,50,150,50,150,50,150,50,150,50,150,50,150,50,150 };

static const uint32_t const trend_high[] = { 200,200,1000,200,200,200,1000 };
static const uint32_t const trend_low[] = { 2000,200,1000 };

static const uint32_t const error[] = { 100,100,100,100,100,100,100,100,100,100};


static const uint32_t const alert[] = { 500,200,1000 };
static int bg_array[12];

enum CgmKey {
    CGM_ICON_KEY = 0x0,         // TUPLE_INT
    CGM_BG_KEY = 0x1,           // TUPLE_CSTRING
    CGM_READTIME_KEY = 0x2,     // TUPLE_CSTRING
    CGM_ALERT_KEY = 0x3,        // TUPLE_INT
    CGM_TIME_NOW = 0x4,         // TUPLE_CSTRING
    CGM_DELTA_KEY = 0x5,
    CGM_BG_ONE = 0x6,
    CGM_BG_TWO = 0x7,
    CGM_BG_THREE = 0x8,
    CGM_BG_FOUR = 0x9,
    CGM_BG_FIVE = 0xA,
    CGM_BG_SIX = 0xB,
    CGM_BG_SEVEN = 0xC,
    CGM_BG_EIGHT = 0xD,
    CGM_BG_NINE = 0xE,
    CGM_BG_TEN = 0xF,
    CGM_BG_ELEVEN = 0x10,
    CGM_BG_TWELVE = 0x11,
// JWS 3/27/15
//     CGM_BG_LASTREAD = 0x12,
//     CGM_BG_LASTUPLOAD = 0x13,
};

static const uint32_t CGM_ICONS[] = {
    RESOURCE_ID_IMAGE_UPUP,     //0
    RESOURCE_ID_IMAGE_UP,       //1
    RESOURCE_ID_IMAGE_UP45,     //2
    RESOURCE_ID_IMAGE_FLAT,     //3
    RESOURCE_ID_IMAGE_NONE,     //4
    RESOURCE_ID_IMAGE_DOWN45,   //5
    RESOURCE_ID_IMAGE_DOWN,     //6
    RESOURCE_ID_IMAGE_DOWNDOWN,  //7
    RESOURCE_ID_IMAGE_LOADER_WHITE, //8
    RESOURCE_ID_IMAGE_DISCONNECTED,  //9
//    RESOURCE_ID_IMAGE_CLOUDX_WHITE,
    RESOURCE_ID_IMAGE_ALERT_WHITE
};


static int max_repeats = 12;
static int counter = 0;

static void timer_callback(void *data) {
    
    if (counter < max_repeats)
    {
        icon_bitmap = gbitmap_create_with_resource(CGM_ICONS[11]);
        bitmap_layer_set_bitmap(icon_layer, icon_bitmap);
        vibes_long_pulse();
        if (alert_active == 0) {
            timer = app_timer_register(5000, timer_callback, NULL);
            alert_active = 1;
        }
        counter++;
    } else {
        alert_active = 0;
        app_timer_cancel(timer);
        counter = 0;
    }
    
}
static void sync_error_callback(DictionaryResult dict_error, AppMessageResult app_message_error, void *context) {
    
    icon_bitmap = gbitmap_create_with_resource(CGM_ICONS[9]);
    bitmap_layer_set_bitmap(icon_layer, icon_bitmap);
    
    VibePattern pat = {
        .durations = alert,
        .num_segments = ARRAY_LENGTH(alert),
    };
    
    vibes_enqueue_custom_pattern(pat);
}

static void alert_handler(uint8_t alertValue)
{
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "Alert code: %d", alertValue);
    
    switch(alertValue){
            //No alert
        case 0:
            break;
            
            //Normal (new data, in range, trend okay)
        case 1:
        case 2:
        case 3:
            last_read = 0;
            vibes_double_pulse();
            last_read = 0;
            break;
            
        case 4: ;
            timer = app_timer_register(100, timer_callback, NULL);
            
            last_read = 0;
            break;
            
        case 5: ;
            timer = app_timer_register(100, timer_callback, NULL);
            last_read = 0;
            break;
            
            //out of range
        case 6: ;
            VibePattern errorpat = {
                .durations = error,
                .num_segments = ARRAY_LENGTH(error),
            };
            vibes_enqueue_custom_pattern(errorpat);
            last_read = 0;
            break;
            
            //uploader not uploading
        case 7: ;
            VibePattern uperrorpat = {
                .durations = error,
                .num_segments = ARRAY_LENGTH(error),
            };
            vibes_enqueue_custom_pattern(uperrorpat);
            last_read = 0;
            break;
            
            //Hypo
            
            //Hyper
            
            //Trend Low
            
            //Trend High
            
            //Data Alert
            
    }
    
    if (last_read < 0) {
        last_read = 0;
        
    } else {
        switch(last_read) {
            case(0):
                snprintf(str, 15, "get: %i", last_read);
                snprintf(str_up, 15, "up: %i", last_uploadtime);
                snprintf(str_read, 15, "rx: %i", last_readtime);
                strncpy(status_text, ", ", 32);
                strcat(str, status_text);
                strcat(str, str_read);
                strcat(str, status_text);
                strcat(str, str_up);
                //APP_LOG(APP_LOG_LEVEL_DEBUG, "str: %s", str_up);
                break;
                
            case(1):
                snprintf(str, 15, "get: %i", last_read);
                snprintf(str_up, 15, "up: %i", last_uploadtime);
                snprintf(str_read, 15, "rx: %i", last_readtime);
                strncpy(status_text, ", ", 32);
                strcat(str, status_text);
                strcat(str, str_read);
                strcat(str, status_text);
                strcat(str, str_up);
                //APP_LOG(APP_LOG_LEVEL_DEBUG, "str: %s", str_up);
                break;
                
            default:
                snprintf(str, 15, "get: %i", last_read);
                snprintf(str_up, 15, "up: %i", last_uploadtime);
                snprintf(str_read, 15, "rx: %i", last_readtime);
                strncpy(status_text, ", ", 32);
                strcat(str, status_text);
                strcat(str, str_read);
                strcat(str, status_text);
                strcat(str, str_up);
                
                if(alertValue == 6) {
                    strncpy(str, "Out of range!", 64);
                    icon_bitmap = gbitmap_create_with_resource(CGM_ICONS[10]);
                    bitmap_layer_set_bitmap(icon_layer, icon_bitmap);
                }
                
                if(alertValue == 7) {
                    strncpy(str, "Not uploading!", 64);
                    icon_bitmap = gbitmap_create_with_resource(CGM_ICONS[10]);
                    bitmap_layer_set_bitmap(icon_layer, icon_bitmap);
                }
                
                break;
                
        }
    }
    if (last_read > 15)
    {
        strncpy(str, "Get fail!", 64);
        icon_bitmap = gbitmap_create_with_resource(CGM_ICONS[10]);
        bitmap_layer_set_bitmap(icon_layer, icon_bitmap);
        VibePattern cloud_error = {
            .durations = high,
            .num_segments = ARRAY_LENGTH(high),
        };
        vibes_enqueue_custom_pattern(cloud_error);
    }
    text_layer_set_text(status_layer, str);
}

static void sync_tuple_changed_callback(const uint32_t key, const Tuple* new_tuple, const Tuple* old_tuple, void* context) {
    switch (key) {
            
        case CGM_ICON_KEY:
            if (icon_bitmap) {
                gbitmap_destroy(icon_bitmap);
            }
            icon_bitmap = gbitmap_create_with_resource(CGM_ICONS[new_tuple->value->uint8]);
            bitmap_layer_set_bitmap(icon_layer, icon_bitmap);
            last_icon = new_tuple->value->uint8;
            break;
            
        case CGM_BG_KEY:
            text_layer_set_text(bg_layer, new_tuple->value->cstring);
            strncpy(last_bg, new_tuple->value->cstring, 124);
            break;
            
        case CGM_READTIME_KEY:
            strncpy(new_time, new_tuple->value->cstring, 124);
            text_layer_set_text(readtime_layer, new_tuple->value->cstring);
            break;
            
        case CGM_TIME_NOW:
            text_layer_set_text(datetime_layer, new_tuple->value->cstring);
            break;
            
        case CGM_ALERT_KEY:
            
            alert_handler(new_tuple->value->uint8);
            
            break;
            
        case CGM_DELTA_KEY:
            text_layer_set_text(message_layer, new_tuple->value->cstring);
            break;
            
        case CGM_BG_ONE:
            bg_array[0] = (new_tuple->value->uint8);
            break;
            
        case CGM_BG_TWO:
            bg_array[1] = (new_tuple->value->uint8);
            break;
            
        case CGM_BG_THREE:
            bg_array[2] = (new_tuple->value->uint8);
            break;
            
        case CGM_BG_FOUR:
            bg_array[3] = (new_tuple->value->uint8);
            break;
            
        case CGM_BG_FIVE:
            bg_array[4] = (new_tuple->value->uint8);
            break;
            
        case CGM_BG_SIX:
            bg_array[5] = (new_tuple->value->uint8);
            break;
            
        case CGM_BG_SEVEN:
            bg_array[6] = (new_tuple->value->uint8);
            break;
            
        case CGM_BG_EIGHT:
            bg_array[7] = (new_tuple->value->uint8);
            break;
            
        case CGM_BG_NINE:
            bg_array[8] = (new_tuple->value->uint8);
            break;
            
        case CGM_BG_TEN:
            bg_array[9] = (new_tuple->value->uint8);
            break;
            
        case CGM_BG_ELEVEN:
            bg_array[10] = (new_tuple->value->uint8);
            break;
            
        case CGM_BG_TWELVE:
            bg_array[11] = (new_tuple->value->uint8);
            break;
            
//         case CGM_BG_LASTREAD:
//             last_readtime = (new_tuple->value->uint8);
//             break;
            
//         case CGM_BG_LASTUPLOAD:
//             last_uploadtime = (new_tuple->value->uint8);
//             break;
            
            
    }
    
}

static void send_cmd(void) {
    
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
    
    if (iter == NULL) {
        return;
    }
    static char *bgptr = last_bg;
    static char *timeptr = new_time;
    static char *clockptr= time_text;
    
    Tuplet alertval = TupletInteger(3, 0);
    Tuplet iconval = TupletInteger(0, 8);
    Tuplet bgVal = TupletCString(1, bgptr);
    Tuplet lastTimeVal = TupletCString(2, timeptr);
    
    dict_write_tuplet(iter, &alertval);
    dict_write_tuplet(iter, &iconval);
    dict_write_tuplet(iter, &bgVal);
    dict_write_tuplet(iter, &lastTimeVal);
    
    dict_write_end(iter);
    
    app_message_outbox_send();
    
    last_read = last_read + 1;
}


static void window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    
    //Create GFont
    s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_CALIBRI_BOLD_25));
    s_bg_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_CALIBRI_BOLD_44));
    s_msg_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_CALIBRI_BOLD_16));
    s_sts_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_CALIBRI_BOLD_14));
    
    datetime_layer = text_layer_create(GRect(0, 90, 144, 20));
    text_layer_set_text_color(datetime_layer, GColorBlack);
    text_layer_set_background_color(datetime_layer, GColorWhite);
    text_layer_set_font(datetime_layer, s_msg_font);
    text_layer_set_text_alignment(datetime_layer, GTextAlignmentCenter);
    layer_add_child(window_layer, text_layer_get_layer(datetime_layer));
    
    icon_layer = bitmap_layer_create(GRect(84, 0, 60, 60));
    layer_add_child(window_layer, bitmap_layer_get_layer(icon_layer));
    
    bg_layer = text_layer_create(GRect(0, 2, 90, 68));
    text_layer_set_text_color(bg_layer, GColorWhite);
    text_layer_set_background_color(bg_layer, GColorClear);
    text_layer_set_font(bg_layer, s_bg_font);
    text_layer_set_text_alignment(bg_layer, GTextAlignmentCenter);
    layer_add_child(window_layer, text_layer_get_layer(bg_layer));
    
    readtime_layer = text_layer_create(GRect(0, 113, 72, 31));
    text_layer_set_text_color(readtime_layer, GColorBlack);
    text_layer_set_background_color(readtime_layer, GColorWhite);
    text_layer_set_font(readtime_layer, s_time_font);
    text_layer_set_text_alignment(readtime_layer, GTextAlignmentCenter);
    layer_add_child(window_layer, text_layer_get_layer(readtime_layer));
    
    clock_layer = text_layer_create(GRect(72, 113, 72, 30));
    text_layer_set_text_color(clock_layer, GColorWhite);
    text_layer_set_background_color(clock_layer, GColorBlack);
    text_layer_set_font(clock_layer, s_time_font);
    text_layer_set_text_alignment(clock_layer, GTextAlignmentCenter);
    layer_add_child(window_layer, text_layer_get_layer(clock_layer));
    
    message_layer = text_layer_create(GRect(0, 52, 144, 115));
    text_layer_set_text_color(message_layer, GColorWhite);
    text_layer_set_background_color(message_layer, GColorClear);
    text_layer_set_font(message_layer, s_msg_font);
    text_layer_set_text_alignment(message_layer, GTextAlignmentCenter);
    layer_add_child(window_layer, text_layer_get_layer(message_layer));
    
    status_layer = text_layer_create(GRect(0, 148, 144, 22));
    text_layer_set_text_color(status_layer, GColorWhite);
    text_layer_set_background_color(status_layer, GColorBlack);
    text_layer_set_font(status_layer, s_sts_font);
    text_layer_set_text_alignment(status_layer, GTextAlignmentCenter);
    layer_add_child(window_layer, text_layer_get_layer(status_layer));
    
    divider_layer = text_layer_create(GRect(0, 144, 144, 3));
    text_layer_set_background_color(divider_layer, GColorWhite);
    layer_add_child(window_layer, text_layer_get_layer(divider_layer));
    
}

void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
    
    char *time_format;
    
    if (!tick_time) {
        time_t now = time(NULL);
        tick_time = localtime(&now);
    }
    
    if (clock_is_24h_style()) {
        time_format = "%I:%M";
    } else {
        time_format = "%I:%M";
    }
    
    strftime(time_text, sizeof(time_text), time_format, tick_time);
    
    if (time_text[0] == '0') {
        memmove(time_text, &time_text[1], sizeof(time_text) - 1);
    }
    
    text_layer_set_text(clock_layer, time_text);
    send_cmd();
}

static void window_unload(Window *window) {
    app_sync_deinit(&sync);
    
    if (icon_bitmap) {
        gbitmap_destroy(icon_bitmap);
    }
    text_layer_destroy(datetime_layer);
    text_layer_destroy(readtime_layer);
    text_layer_destroy(bg_layer);
    text_layer_destroy(message_layer);
    text_layer_destroy(status_layer);
    bitmap_layer_destroy(icon_layer);
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
    last_read = last_read - 1;
    send_cmd();
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
    show_chart(last_bg, bg_array);
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
    if (alert_active == 1) {
		vibes_cancel();
    	app_timer_cancel(timer);
    	alert_active = 0;
    	counter = 0;
    	icon_bitmap = gbitmap_create_with_resource(CGM_ICONS[last_icon]);
    	bitmap_layer_set_bitmap(icon_layer, icon_bitmap);
	}
}

static void click_config_provider(void *context) {
    window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
    window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
    window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}


static void init(void) {
    window = window_create();
    window_set_background_color(window, GColorBlack);
    window_set_fullscreen(window, true);
    window_set_click_config_provider(window, click_config_provider);
    window_set_window_handlers(window, (WindowHandlers) {
        .load = window_load,
        .unload = window_unload
    });
    Tuplet initial_values[] = {
        TupletInteger(CGM_ICON_KEY, (uint8_t) 8),
        TupletCString(CGM_BG_KEY, ""),
        TupletCString(CGM_READTIME_KEY, ""),
        TupletInteger(CGM_ALERT_KEY, 0),
        TupletCString(CGM_TIME_NOW, "Loading..."),
        TupletCString(CGM_DELTA_KEY, ""),
        TupletInteger(CGM_BG_ONE, (uint8_t) 200),
        TupletInteger(CGM_BG_TWO, (uint8_t) 200),
        TupletInteger(CGM_BG_THREE, (uint8_t) 200),
        TupletInteger(CGM_BG_FOUR, (uint8_t) 200),
        TupletInteger(CGM_BG_FIVE, (uint8_t) 200),
        TupletInteger(CGM_BG_SIX, (uint8_t) 200),
        TupletInteger(CGM_BG_SEVEN, (uint8_t) 200),
        TupletInteger(CGM_BG_EIGHT, (uint8_t) 200),
        TupletInteger(CGM_BG_NINE, (uint8_t) 200),
        TupletInteger(CGM_BG_TEN, (uint8_t) 200),
        TupletInteger(CGM_BG_ELEVEN, (uint8_t) 200),
        TupletInteger(CGM_BG_TWELVE, (uint8_t) 200),
        TupletInteger(CGM_BG_LASTREAD, (uint8_t) 0),
        TupletInteger(CGM_BG_LASTUPLOAD, (uint8_t) 0)
    };
    app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
    app_sync_init(&sync, sync_buffer, sizeof(sync_buffer), initial_values, ARRAY_LENGTH(initial_values),sync_tuple_changed_callback, sync_error_callback, NULL);
    tick_timer_service_subscribe(MINUTE_UNIT, &handle_minute_tick);
    
    const bool animated = true;
    window_stack_push(window, animated);
}

static void deinit(void) {
    window_destroy(window);
}

int main(void) {
    init();   
    app_event_loop();
    deinit();
}




