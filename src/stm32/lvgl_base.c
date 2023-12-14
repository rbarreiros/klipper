#include "autoconf.h"       // defines
#include "lvgl_base.h"
#include "sched.h"          // sched_*
#include "board/misc.h"     // timer_*

// Include the driver
#ifdef CONFIG_STM32_LVGL_ILI9341
    #include "lvgl_ili9341.h"
#endif

#define DISPLAY_BUFF_SIZE ((DISPLAY_WIDTH * DISPLAY_HEIGHT) / 10U)
#define TICK_TIME_MS      5U

struct lvgl_info {
    lv_disp_draw_buf_t draw_buf;
    lv_color_t buf1[DISPLAY_BUFF_SIZE];
    lv_disp_drv_t disp_drv;
    struct timer tick_timer;
    struct timer handler_timer;
    uint32_t handler_interval;
    uint32_t last_handler_tick;
};

static struct lvgl_info lvgl_info = {0};
static struct task_wake lvgl_wake;

static uint_fast8_t
lvgl_tick_event(struct timer *t)
{
    lvgl_info.tick_timer.waketime += timer_from_us(TICK_TIME_MS * 1000);
    lv_tick_inc(TICK_TIME_MS);
    return SF_RESCHEDULE;
}

static uint_fast8_t
lvgl_handler_event(struct timer *t)
{
    sched_wake_task(&lvgl_wake);
    lvgl_info.handler_timer.waketime += lvgl_info.handler_interval;
    return SF_RESCHEDULE;
}

void lvgl_start(void)
{
    lvgl_display_init();

    lv_init();
    lv_disp_draw_buf_init(&lvgl_info.draw_buf, lvgl_info.buf1, NULL, DISPLAY_BUFF_SIZE);
    lv_disp_drv_init(&lvgl_info.disp_drv);
    lvgl_info.disp_drv.hor_res = DISPLAY_WIDTH;
    lvgl_info.disp_drv.ver_res = DISPLAY_HEIGHT;
    lvgl_info.disp_drv.flush_cb = lvgl_display_flush;
    lvgl_info.disp_drv.draw_buf = &lvgl_info.draw_buf;
    lv_disp_drv_register(&lvgl_info.disp_drv);

    // tick timer
    lvgl_info.tick_timer.func = lvgl_tick_event;
    lvgl_info.tick_timer.waketime = timer_from_us(TICK_TIME_MS * 1000);

    // handler timer
    lvgl_info.handler_interval = timer_from_us(10); // could be customized in the future
    lvgl_info.handler_timer.waketime = lvgl_info.handler_interval;
    lvgl_info.handler_timer.func = lvgl_handler_event;
    
    sched_add_timer(&lvgl_info.tick_timer);
    sched_add_timer(&lvgl_info.handler_timer);


    // just a test Button
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x003a57), LV_PART_MAIN);
    lv_obj_t *btn = lv_btn_create(lv_scr_act()); /*Add a button the current screen*/
    lv_obj_set_pos(btn, 190, 10);                 /*Set its position*/
    lv_obj_set_size(btn, 120, 50);               /*Set its size*/
    // lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_ALL, NULL);           /*Assign a callback to the button*/

    lv_obj_t *label = lv_label_create(btn); /*Add a label to the button*/
    lv_label_set_text(label, "Home");     /*Set the labels text*/
    lv_obj_center(label);
}

void lvgl_handler_task(void)
{
  if (!sched_check_wake(&lvgl_wake))
        return;

  uint32_t now = timer_read_time();
  int32_t diff = now - lvgl_info.last_handler_tick;

  if(diff > 0 && diff > timer_from_us(1000))
  {
    lv_tick_inc(1); // We could add a timer just for this, of 1ms and leave the task for lv_timer_handler() - TODO
    lvgl_info.last_handler_tick = now;
  }
  else if(diff < 0)
    lvgl_info.last_handler_tick = now;

  lv_timer_handler();
}
DECL_TASK(lvgl_handler_task);

//#if STM32_FSMC_LVGL_DMA_ENABLED
void lvgl_dma_transfer_complete(void)
{
    lv_disp_flush_ready(&lvgl_info.disp_drv);
}
//#endif
