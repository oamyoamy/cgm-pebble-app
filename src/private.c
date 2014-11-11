#include "private.h"
#include "chart.h"
#include "cgm.h"
#include <pebble.h>

// BEGIN AUTO-GENERATED UI CODE; DO NOT MODIFY
static Window *s_window;
static GBitmap *s_res_image_private_white;
static GFont s_res_calibri_bold_44;
static BitmapLayer *private;
static TextLayer *clock_layer;
static char time_text[124] = "";
static AppTimer *timer = NULL;

static void timer_callback(void *data) {
  
 show_private();
  
}

void handle_minute_tick_private(struct tm *tick_time, TimeUnits units_changed) {
  // Need to be static because they're used by the system later.
  //static char time_text = "00:00";
  //static char date_text[] = "Xxxxxxxxx 00";

  char *time_format;

  if (!tick_time) {
    time_t now = time(NULL);
    tick_time = localtime(&now);
  }

  if (clock_is_24h_style()) {
    time_format = "%R";
  } else {
    time_format = "%I:%M";
  }

  strftime(time_text, sizeof(time_text), time_format, tick_time);

  if (time_text[0] == '0') {
    memmove(time_text, &time_text[1], sizeof(time_text) - 1);
  }
  
  refresh_bground();
  text_layer_set_text(clock_layer, time_text);
  

}

void accel_tap_handler(AccelAxisType axis, int32_t direction) {  
  hide_private();
  timer = app_timer_register((5*1000), timer_callback, NULL);
}


static void initialise_ui(void) {
  s_window = window_create();
  window_set_background_color(s_window, GColorBlack);
  window_set_fullscreen(s_window, true);
  
  accel_tap_service_subscribe(accel_tap_handler);
  
  s_res_image_private_white = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_PRIVATE_WHITE);
  s_res_calibri_bold_44 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_CALIBRI_BOLD_44));
  // private
  private = bitmap_layer_create(GRect(22, 57, 100, 100));
  bitmap_layer_set_bitmap(private, s_res_image_private_white);
  layer_add_child(window_get_root_layer(s_window), (Layer *)private);
  
  // clock_layer
  clock_layer = text_layer_create(GRect(0, 4, 144, 44));
  text_layer_set_background_color(clock_layer, GColorClear);
  text_layer_set_text_color(clock_layer, GColorWhite);
  //text_layer_set_text(clock_layer, "00:00");
  text_layer_set_text_alignment(clock_layer, GTextAlignmentCenter);
  text_layer_set_font(clock_layer, s_res_calibri_bold_44);
  layer_add_child(window_get_root_layer(s_window), (Layer *)clock_layer);
}

static void destroy_ui(void) {
  window_destroy(s_window);
  bitmap_layer_destroy(private);
  text_layer_destroy(clock_layer);
  gbitmap_destroy(s_res_image_private_white);
  fonts_unload_custom_font(s_res_calibri_bold_44);
  accel_tap_service_unsubscribe();
  
  refresh_cgm();
  //tick_timer_service_unsubscribe();
}
// END AUTO-GENERATED UI CODE

static void handle_window_unload(Window* window) {
  destroy_ui();
}

void show_private(void) {
  initialise_ui();
  tick_timer_service_subscribe(MINUTE_UNIT, &handle_minute_tick_private);
  window_set_window_handlers(s_window, (WindowHandlers) {
    .unload = handle_window_unload,
  });
  window_stack_push(s_window, true);
}

void hide_private(void) {
  //tick_timer_service_unsubscribe();
  refresh_cgm();
  window_stack_remove(s_window, true);
}
