#ifndef __LVGL_BASE_H
#define __LVGL_BASE_H

#include "lvgl/lvgl.h"      // lv_*

// Muse be defined in driver code (lvgl_ili9341.c for example)

void lvgl_display_init(void);
void lvgl_display_flush(lv_disp_drv_t * drv, const lv_area_t * area, lv_color_t * color_p);
void lvgl_display_transfer_complete(void);


void lvgl_start(void);
void lvgl_dma_transfer_complete(void);

#endif
