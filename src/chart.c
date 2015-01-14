#include "chart.h"
#include "private.h"
#include <pebble.h>

static Window *s_window;
static GFont s_res_calibri_40;
static GBitmap *s_res_image_circle_white;
static TextLayer *bg;
static TextLayer *max;
static TextLayer *min;
static BitmapLayer *circle_1;
static BitmapLayer *circle_2;
static BitmapLayer *circle_3;
static BitmapLayer *circle_4;
static BitmapLayer *circle_5;
static BitmapLayer *circle_6;
static BitmapLayer *circle_7;
static BitmapLayer *circle_8;
static BitmapLayer *circle_9;
static BitmapLayer *circle_10;
static BitmapLayer *circle_11;
static BitmapLayer *circle_12;

static AppTimer *timer = NULL;


void hide_chart(void) {
    window_stack_remove(s_window, true);
}
static void timer_callback(void *data) {
    
    hide_chart();
    
}


static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
    app_timer_cancel(timer);
	hide_chart();
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
   	app_timer_cancel(timer);
	hide_chart();
}

static void click_config_provider(void *context) {
    window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
    window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void initialise_ui(const char* bgVal, int bgArray[]) {
    s_window = window_create();
    window_set_background_color(s_window, GColorBlack);
    window_set_fullscreen(s_window, true);
    window_set_click_config_provider(s_window, click_config_provider);
    
    
    s_res_calibri_40 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_CALIBRI_28));
    s_res_image_circle_white = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CIRCLE_WHITE);
    // bg
    int average = (bgArray[11] + bgArray[10] + bgArray[9] + bgArray[8])/4;
    if(average < 84) {
        bg = text_layer_create(GRect(0, 134, 144, 28));
    } else {
        bg = text_layer_create(GRect(0, 0, 144, 28));
    }
    
    text_layer_set_background_color(bg, GColorBlack);
    text_layer_set_text_color(bg, GColorWhite);
    text_layer_set_text(bg, bgVal);
    text_layer_set_text_alignment(bg, GTextAlignmentLeft);
    text_layer_set_font(bg, s_res_calibri_40);
    layer_add_child(window_get_root_layer(s_window), (Layer *)bg);
    
    // bg
    min = text_layer_create(GRect(110, 148, 34, 20));
    text_layer_set_background_color(min, GColorClear);
    text_layer_set_text_color(min, GColorWhite);
    text_layer_set_text(min, "(60");
    text_layer_set_text_alignment(min, GTextAlignmentRight);
    text_layer_set_font(min, fonts_get_system_font(FONT_KEY_GOTHIC_14));
    layer_add_child(window_get_root_layer(s_window), (Layer *)min);
    
    // max
    max =  text_layer_create(GRect(110, 0, 34, 20));
    text_layer_set_background_color(max, GColorClear);
    text_layer_set_text_color(max, GColorWhite);
    text_layer_set_text(max, "(300");
    text_layer_set_text_alignment(max, GTextAlignmentRight);
    text_layer_set_font(max,  fonts_get_system_font(FONT_KEY_GOTHIC_14));
    layer_add_child(window_get_root_layer(s_window), (Layer *)max);
    
    // circle_1
    circle_1 = bitmap_layer_create(GRect(2, bgArray[11], 20, 20));
    bitmap_layer_set_bitmap(circle_1, s_res_image_circle_white);
    layer_add_child(window_get_root_layer(s_window), (Layer *)circle_1);
    
    // circle_2
    circle_2 = bitmap_layer_create(GRect(12, bgArray[10], 20, 20));
    bitmap_layer_set_bitmap(circle_2, s_res_image_circle_white);
    layer_add_child(window_get_root_layer(s_window), (Layer *)circle_2);
    
    // circle_3
    circle_3 = bitmap_layer_create(GRect(22, bgArray[9], 20, 20));
    bitmap_layer_set_bitmap(circle_3, s_res_image_circle_white);
    layer_add_child(window_get_root_layer(s_window), (Layer *)circle_3);
    
    // circle_4
    circle_4 = bitmap_layer_create(GRect(32, bgArray[8], 20, 20));
    bitmap_layer_set_bitmap(circle_4, s_res_image_circle_white);
    layer_add_child(window_get_root_layer(s_window), (Layer *)circle_4);
    
    // circle_5
    circle_5 = bitmap_layer_create(GRect(42, bgArray[7], 20, 20));
    bitmap_layer_set_bitmap(circle_5, s_res_image_circle_white);
    layer_add_child(window_get_root_layer(s_window), (Layer *)circle_5);
    
    // circle_6
    circle_6 = bitmap_layer_create(GRect(52, bgArray[6], 20, 20));
    bitmap_layer_set_bitmap(circle_6, s_res_image_circle_white);
    layer_add_child(window_get_root_layer(s_window), (Layer *)circle_6);
    
    // circle_7
    circle_1 = bitmap_layer_create(GRect(62, bgArray[5], 20, 20));
    bitmap_layer_set_bitmap(circle_1, s_res_image_circle_white);
    layer_add_child(window_get_root_layer(s_window), (Layer *)circle_1);
    
    // circle_8
    circle_2 = bitmap_layer_create(GRect(72, bgArray[4], 20, 20));
    bitmap_layer_set_bitmap(circle_2, s_res_image_circle_white);
    layer_add_child(window_get_root_layer(s_window), (Layer *)circle_2);
    
    // circle_9
    circle_3 = bitmap_layer_create(GRect(82, bgArray[3], 20, 20));
    bitmap_layer_set_bitmap(circle_3, s_res_image_circle_white);
    layer_add_child(window_get_root_layer(s_window), (Layer *)circle_3);
    
    // circle_10
    circle_4 = bitmap_layer_create(GRect(92, bgArray[2], 20, 20));
    bitmap_layer_set_bitmap(circle_4, s_res_image_circle_white);
    layer_add_child(window_get_root_layer(s_window), (Layer *)circle_4);
    
    // circle_11
    circle_5 = bitmap_layer_create(GRect(102, bgArray[1], 20, 20));
    bitmap_layer_set_bitmap(circle_5, s_res_image_circle_white);
    layer_add_child(window_get_root_layer(s_window), (Layer *)circle_5);
    
    // circle_12
    circle_6 = bitmap_layer_create(GRect(112, bgArray[0], 20, 20));
    bitmap_layer_set_bitmap(circle_6, s_res_image_circle_white);
    layer_add_child(window_get_root_layer(s_window), (Layer *)circle_6);
}

static void destroy_ui(void) {
    window_destroy(s_window);
    text_layer_destroy(bg);
    text_layer_destroy(min);
    text_layer_destroy(max);
    bitmap_layer_destroy(circle_1);
    bitmap_layer_destroy(circle_2);
    bitmap_layer_destroy(circle_3);
    bitmap_layer_destroy(circle_4);
    bitmap_layer_destroy(circle_5);
    bitmap_layer_destroy(circle_6);
    bitmap_layer_destroy(circle_7);
    bitmap_layer_destroy(circle_8);
    bitmap_layer_destroy(circle_9);
    bitmap_layer_destroy(circle_10);
    bitmap_layer_destroy(circle_11);
    bitmap_layer_destroy(circle_12);
    fonts_unload_custom_font(s_res_calibri_40);
    gbitmap_destroy(s_res_image_circle_white);
}

static void handle_window_unload(Window* window) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "handling unload");
    destroy_ui();
}



void show_chart(const char* bgVal, int bgArray[]) {

    initialise_ui(bgVal, bgArray);
    window_set_window_handlers(s_window, (WindowHandlers) {
        .unload = handle_window_unload,
    });
    window_stack_push(s_window, true);
    
    timer = app_timer_register((5*1000), timer_callback, NULL);
}



