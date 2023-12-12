#include "command.h"
#include "lvgl/lvgl.h"


void lvgl_draw_text(uint32_t *args)
{
  (void)args;

  // Create a white label, set its text and align it to the center
  lv_obj_t * label = lv_label_create(lv_scr_act());
  lv_label_set_text(label, "Hello world");
  lv_obj_set_style_text_color(lv_scr_act(), lv_color_hex(0xffffff), LV_PART_MAIN);
  lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
}
DECL_COMMAND(lvgl_draw_text, "lvgl_draw_text");



