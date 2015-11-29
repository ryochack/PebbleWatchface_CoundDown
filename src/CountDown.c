#include <pebble.h>

#define RECT_COUNTDOWN_Y 10
#define RECT_COUNTDOWN_H 80
#define RECT_TIME_Y ((RECT_COUNTDOWN_Y) + (RECT_COUNTDOWN_H))
#define RECT_TIME_H 80

typedef struct time_layer {
  TextLayer *text_layer;
  time_t time;
  char text[16];
} time_layer_t;

static struct {
  Window  *window;
  time_layer_t countdown;
  time_layer_t clock;
} cd;

#if defined(PBL_BW)
#elif defined(PBL_COLOR)
#endif

static void update_time(void) {
  cd.clock.time = time(NULL);
  struct tm *tick_time = localtime(&cd.clock.time);
  strftime(cd.clock.text, sizeof(cd.clock.text),
      clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);

  snprintf(cd.countdown.text, sizeof(cd.countdown.text), "%d",
      (int)difftime(cd.countdown.time, cd.clock.time)/60);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  window_set_background_color(window, (GColor)GColorOrangeARGB8);
  update_time();

  cd.countdown.text_layer = text_layer_create(
      GRect(0, RECT_COUNTDOWN_Y, bounds.size.w, RECT_COUNTDOWN_H)
      );
  text_layer_set_background_color(cd.countdown.text_layer, GColorBlack);
  text_layer_set_text_color(cd.countdown.text_layer, (GColor)GColorGreenARGB8);
  text_layer_set_text_alignment(cd.countdown.text_layer, GTextAlignmentCenter);
  text_layer_set_font(cd.countdown.text_layer, fonts_get_system_font(FONT_KEY_BITHAM_34_MEDIUM_NUMBERS));
  text_layer_set_text(cd.countdown.text_layer, cd.countdown.text);
  layer_add_child(window_layer, text_layer_get_layer(cd.countdown.text_layer));

  cd.clock.text_layer = text_layer_create(
      GRect(0, RECT_TIME_Y, bounds.size.w, RECT_TIME_H)
      );
  text_layer_set_background_color(cd.clock.text_layer, GColorWhite);
  text_layer_set_text_color(cd.clock.text_layer, (GColor)GColorDarkCandyAppleRedARGB8);
  text_layer_set_text_alignment(cd.clock.text_layer, GTextAlignmentCenter);
  text_layer_set_font(cd.clock.text_layer, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));
  text_layer_set_text(cd.clock.text_layer, cd.clock.text);
  layer_add_child(window_layer, text_layer_get_layer(cd.clock.text_layer));
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
  text_layer_set_text(cd.countdown.text_layer, cd.countdown.text);
  text_layer_set_text(cd.clock.text_layer, cd.clock.text);
}

static void window_unload(Window *window) {
  text_layer_destroy(cd.countdown.text_layer);
  text_layer_destroy(cd.clock.text_layer);
}

static void init(void) {
  /**
   * TODO:
   * - local time limit.
   * - save limit time to ROM.
   */
  cd.countdown.time = mktime( &(struct tm){
      .tm_sec  = 0,
      .tm_min  = 0,
      .tm_hour = 0,
      .tm_mday = 30,
      .tm_mon  = 10,
      .tm_year = 115,
      .tm_isdst = -1,
      } );

  cd.window = window_create();
  window_set_window_handlers(cd.window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
      });
  const bool animated = true;
  window_stack_push(cd.window, animated);

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void deinit(void) {
  window_destroy(cd.window);
}

int main(void) {
  init();
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", cd.window);
  app_event_loop();
  deinit();
}
