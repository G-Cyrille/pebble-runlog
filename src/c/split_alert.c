/* Static, high-contrast milestone display. The normal recorder keeps running
 * underneath; one timer removes the overlay without changing its page. */
#include "split_alert.h"
#include "runlog.h"

static Layer *s_layer;
static AppTimer *s_timer;
static char s_heading[20], s_last_label[24], s_average_label[24];
static char s_last[16], s_average[16];
static uint32_t s_vibrations[63];

static void prv_draw_number(GContext *ctx, const char *text, GRect frame) {
  GFont font = fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD);
  GSize size = graphics_text_layout_get_content_size(text, font, GRect(0, 0, 1000, 100),
      GTextOverflowModeFill, GTextAlignmentCenter);
  if (size.w > frame.size.w - 12)
    font = fonts_get_system_font(FONT_KEY_LECO_32_BOLD_NUMBERS);
  graphics_draw_text(ctx, text, font, frame, GTextOverflowModeFill, GTextAlignmentCenter, NULL);
}

static void prv_draw(Layer *layer, GContext *ctx) {
  GRect b = layer_get_bounds(layer);
  const int header = 36, row = (b.size.h - header) / 2;
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, b, 0, GCornerNone);
  graphics_context_set_text_color(ctx, GColorBlack);
  graphics_draw_text(ctx, s_heading, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD),
      GRect(0, 0, b.size.w, header), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
  GFont caption = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
  graphics_draw_text(ctx, s_last_label, caption, GRect(0, header, b.size.w, 24),
      GTextOverflowModeFill, GTextAlignmentCenter, NULL);
  prv_draw_number(ctx, s_last, GRect(0, header + 23, b.size.w, row - 23));
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_draw_line(ctx, GPoint(14, header + row - 2), GPoint(b.size.w - 14, header + row - 2));
  graphics_draw_text(ctx, s_average_label, caption, GRect(0, header + row, b.size.w, 24),
      GTextOverflowModeFill, GTextAlignmentCenter, NULL);
  prv_draw_number(ctx, s_average, GRect(0, header + row + 23, b.size.w, row - 23));
}

static void prv_timeout(void *data) {
  (void)data;
  s_timer = NULL;
  if (s_layer) layer_set_hidden(s_layer, true);
}

static void prv_vibrate(int distance) {
  int long_count = distance / 5, short_count = distance % 5;
  int pulses = long_count + short_count;
  if (pulses <= 0 || pulses * 2 - 1 > (int)ARRAY_LENGTH(s_vibrations)) return;
  uint32_t total = 0;
  int n = 0;
  for (int i = 0; i < pulses; i++) {
    if (i) { s_vibrations[n++] = 120; total += 120; }
    uint32_t duration = i < long_count ? 350 : 120;
    s_vibrations[n++] = duration;
    total += duration;
  }
  // Pebble caps a custom pattern at ten seconds. Never truncate a count into
  // a different kilometre number; the complete number remains on screen.
  if (total > 10000) return;
  vibes_enqueue_custom_pattern((VibePattern){ .durations = s_vibrations, .num_segments = n });
}

void split_alert_init(Layer *root) {
  s_layer = layer_create(layer_get_bounds(root));
  if (!s_layer) return;
  layer_set_update_proc(s_layer, prv_draw);
  layer_add_child(root, s_layer);
  layer_set_hidden(s_layer, true);
}

void split_alert_show(int distance, int last_seconds, int average_pace) {
  const char *unit = g_set.miles ? "mi" : "km";
  snprintf(s_heading, sizeof(s_heading), "%s %d", g_set.miles ? "MI" : "KM", distance);
  if (split_mult() == 1) snprintf(s_last_label, sizeof(s_last_label), "Dernier %s", unit);
  else snprintf(s_last_label, sizeof(s_last_label), "Derniers %d %s", split_mult(), unit);
  snprintf(s_average_label, sizeof(s_average_label), "Moyenne /%s", unit);
  if (last_seconds > 0) fmt_dur(s_last, sizeof(s_last), last_seconds);
  else snprintf(s_last, sizeof(s_last), "--:--");
  fmt_pace(s_average, sizeof(s_average), average_pace);
  prv_vibrate(distance);
  light_enable_interaction();
  if (!s_layer) return;
  layer_set_hidden(s_layer, false);
  layer_mark_dirty(s_layer);
  if (s_timer) app_timer_reschedule(s_timer, 10000);
  else s_timer = app_timer_register(10000, prv_timeout, NULL);
}

void split_alert_dismiss(void) {
  if (s_timer) { app_timer_cancel(s_timer); s_timer = NULL; }
  if (s_layer) layer_set_hidden(s_layer, true);
}

void split_alert_deinit(void) {
  split_alert_dismiss();
  if (s_layer) { layer_destroy(s_layer); s_layer = NULL; }
}
