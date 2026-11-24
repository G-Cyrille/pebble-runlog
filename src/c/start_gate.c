/* Full-screen warning shown when Start is pressed but the phone, which owns
 * GPS, cannot record yet. Starting anyway is a deliberate hold, so a route is
 * never lost without the runner noticing. */
#include "start_gate.h"

static Window *s_window;
static TextLayer *s_title, *s_body, *s_hint;
static StartGateHandler s_start_without_gps;
static bool s_phone_connected;

static void prv_update_body(void) {
  if (!s_body) return;
  text_layer_set_text(s_body, s_phone_connected
      ? "Téléphone connecté, en attente de RunLog sur le téléphone."
      : "Téléphone non connecté. Le parcours GPS ne sera pas enregistré.");
}

static void prv_select_long(ClickRecognizerRef rec, void *ctx) {
  (void)rec; (void)ctx;
  StartGateHandler start = s_start_without_gps;
  window_stack_remove(s_window, false);
  if (start) start();
}

static void prv_click_config(void *ctx) {
  (void)ctx;
  window_long_click_subscribe(BUTTON_ID_SELECT, 1000, prv_select_long, NULL);
}

static TextLayer *prv_text(Layer *root, GRect frame, const char *font, GTextAlignment align) {
  TextLayer *layer = text_layer_create(frame);
  text_layer_set_background_color(layer, GColorClear);
  text_layer_set_text_color(layer, GColorBlack);
  text_layer_set_font(layer, fonts_get_system_font(font));
  text_layer_set_text_alignment(layer, align);
  text_layer_set_overflow_mode(layer, GTextOverflowModeWordWrap);
  layer_add_child(root, text_layer_get_layer(layer));
  return layer;
}

static void prv_load(Window *window) {
  Layer *root = window_get_root_layer(window);
  GRect b = layer_get_bounds(root);
  const int pad = 10, w = b.size.w - 2 * pad;
  s_title = prv_text(root, GRect(pad, 8, w, 36), FONT_KEY_GOTHIC_28_BOLD, GTextAlignmentCenter);
  text_layer_set_text(s_title, "PAS DE GPS");
  s_body = prv_text(root, GRect(pad, 48, w, 110), FONT_KEY_GOTHIC_24_BOLD, GTextAlignmentCenter);
  prv_update_body();
  s_hint = prv_text(root, GRect(pad, b.size.h - 50, w, 46), FONT_KEY_GOTHIC_18, GTextAlignmentCenter);
  text_layer_set_text(s_hint, "Retour : annuler\nMaintenir SELECT : sans GPS");
}

static void prv_unload(Window *window) {
  (void)window;
  text_layer_destroy(s_title);
  text_layer_destroy(s_body);
  text_layer_destroy(s_hint);
  s_title = s_body = s_hint = NULL;
}

void start_gate_show(bool phone_connected, StartGateHandler start_without_gps) {
  s_phone_connected = phone_connected;
  s_start_without_gps = start_without_gps;
  if (!s_window) {
    s_window = window_create();
    if (!s_window) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "start gate window allocation failed");
      return;
    }
    window_set_background_color(s_window, GColorWhite);
    window_set_click_config_provider(s_window, prv_click_config);
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = prv_load,
      .unload = prv_unload,
    });
  }
  if (!start_gate_visible()) {
    APP_LOG(APP_LOG_LEVEL_INFO, "start gate shown (phone %d)", phone_connected);
    window_stack_push(s_window, true);
  }
  prv_update_body();
}

void start_gate_set_phone_connected(bool phone_connected) {
  s_phone_connected = phone_connected;
  prv_update_body();
}

bool start_gate_visible(void) {
  return s_window && window_stack_contains_window(s_window);
}

void start_gate_hide(void) {
  if (!start_gate_visible()) return;
  APP_LOG(APP_LOG_LEVEL_INFO, "start gate closed: phone app ready");
  window_stack_remove(s_window, true);
}

void start_gate_deinit(void) {
  if (!s_window) return;
  window_destroy(s_window);
  s_window = NULL;
}
