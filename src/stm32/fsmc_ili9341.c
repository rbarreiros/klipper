
#include "lvgl/lvgl.h"              // display_flush
#include "generic/armcm_timer.h"    // udelay
#include "fsmc.h"                   // fsmc_*
#include "board/misc.h"             // timer_*
#include "sched.h"                  // DECL_TASK

/**
  * @brief  ILI9341 Registers
  */

#define ILI9341_SWRESET            0x01   /* Software Reset */
#define ILI9341_LCD_ID             0xD3
#define ILI9341_SLEEP_IN           0x10
#define ILI9341_SLEEP_OUT          0x11
#define ILI9341_PARTIAL_DISPLAY    0x12
#define ILI9341_NORON              0x13
#define ILI9341_DINVOFF            0x20
#define ILI9341_DISPLAY_INVERSION  0x21
#define ILI9341_GAMMA              0x26
#define ILI9341_DISPLAY_OFF        0x28
#define ILI9341_DISPLAY_ON         0x29
#define ILI9341_WRITE_RAM          0x2C
#define ILI9341_READ_RAM           0x2E
#define ILI9341_CASET              0x2A
#define ILI9341_RASET              0x2B
#define ILI9341_VSCRDEF            0x33 /* Vertical Scroll Definition */
#define ILI9341_VSCSAD             0x37 /* Vertical Scroll Start Address of RAM */
#define ILI9341_TEARING_EFFECT     0x35
#define ILI9341_NORMAL_DISPLAY     0x36
#define ILI9341_IDLE_MODE_OFF      0x38
#define ILI9341_IDLE_MODE_ON       0x39
#define ILI9341_COLOR_MODE         0x3A
#define ILI9341_ID4                0xD3  // real ID register for 9341
#define ILI9341_BLKING_PORCH_CTRL  0xB5
#define ILI9341_DFC                0xb6
#define ILI9341_VCOM_CTRL1         0xC5
#define ILI9341_VCOM_CTRL2         0xC7
#define ILI9341_FR_CTRL            0xB1
#define ILI9341_POWER_CTRL1        0xC0
#define ILI9341_POWER_CTRL2        0xC1
#define ILI9341_POWERA             0xcb
#define ILI9341_POWERB             0xcf
#define ILI9341_PGAMMA             0xe0
#define ILI9341_NGAMMA             0xe1
#define ILI9341_DTCA               0xe8
#define ILI9341_DTCB               0xea
#define ILI9341_POWER_SEQ          0xed
#define ILI9341_3GAMMA_EN          0xf2
#define ILI9341_PRC                0xf7

#define ILI9341_ID                 0x9341

#define ILI9341_WIDTH               320
#define ILI9341_HEIGHT              240

extern LCD_TypeDef *LCD;

#define DISPLAY_BUFF_SIZE           ILI9341_WIDTH * ILI9341_HEIGHT / 10U

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf1[DISPLAY_BUFF_SIZE];
static lv_disp_drv_t disp_drv;

void lvgl_display_init(void)
{
  fsmc_write_reg(ILI9341_SWRESET);
  /* Wait for 200ms */
  udelay(200000);

  /* Sleep In Command */
  fsmc_write_reg(ILI9341_SLEEP_OUT);
  /* Wait for 20ms */
  udelay(20000);

  fsmc_write_reg(ILI9341_POWERA);
  fsmc_write_data(0x39);
  fsmc_write_data(0x2C);
  fsmc_write_data(0x00);
  fsmc_write_data(0x34);
  fsmc_write_data(0x02);

  fsmc_write_reg(ILI9341_POWERB);
  fsmc_write_data(0x00); 
  fsmc_write_data(0xC1);
  fsmc_write_data(0x30);

  fsmc_write_reg(ILI9341_DTCA);
  fsmc_write_data(0x85);
  fsmc_write_data(0x00);
  fsmc_write_data(0x78);

  fsmc_write_reg(ILI9341_DTCB);
  fsmc_write_data(0x00);
  fsmc_write_data(0x00);

  fsmc_write_reg(ILI9341_POWER_SEQ);
  fsmc_write_data(0x64);
  fsmc_write_data(0x03);
  fsmc_write_data(0x12);
  fsmc_write_data(0x81);

  fsmc_write_reg(ILI9341_DFC);
  fsmc_write_data(0x08);
  fsmc_write_data(0x82);
  fsmc_write_data(0x27); // Source Output Scan Direction: 0, Gate Output Scan Direction: 0

  fsmc_write_reg(ILI9341_DINVOFF);
  fsmc_write_reg(ILI9341_PRC);
  fsmc_write_data(0x20);

  /* VCOM setting */
  fsmc_write_reg(ILI9341_VCOM_CTRL1);
  fsmc_write_data(0x3E);
  fsmc_write_data(0x28);
  fsmc_write_reg(ILI9341_VCOM_CTRL2);
  fsmc_write_data(0x86);

  /* Frame Rate Control in normal mode */
  fsmc_write_reg(ILI9341_FR_CTRL);
  fsmc_write_data(0x00);
  fsmc_write_data(0x18);

  /* Power Control */
  fsmc_write_reg(ILI9341_POWER_CTRL1);
  fsmc_write_data(0x23);
  fsmc_write_reg(ILI9341_POWER_CTRL2);
  fsmc_write_data(0x10);

  /* Normal display for Driver Down side */
  fsmc_write_reg(ILI9341_NORMAL_DISPLAY); 
  fsmc_write_data(0xE8); // MY and ML flipped +  bit 3 RGB and BGR changed.

  /* Color mode 16bits/pixel */
  fsmc_write_reg(ILI9341_COLOR_MODE);
  fsmc_write_data(0x55);

/* Gamma Correction */
  fsmc_write_reg(ILI9341_3GAMMA_EN);
  fsmc_write_data(0x00);                 // 3Gamma Function Disable
  fsmc_write_reg(ILI9341_GAMMA);
  fsmc_write_data(0x01);               // Gamma curve selected

  fsmc_write_reg(ILI9341_PGAMMA);
  fsmc_write_data(0x0F);
  fsmc_write_data(0x31);
  fsmc_write_data(0x2B);
  fsmc_write_data(0x0C);
  fsmc_write_data(0x0E);
  fsmc_write_data(0x08);
  fsmc_write_data(0x4E);
  fsmc_write_data(0xF1);
  fsmc_write_data(0x37);
  fsmc_write_data(0x07);
  fsmc_write_data(0x10);
  fsmc_write_data(0x03);
  fsmc_write_data(0x0E);
  fsmc_write_data(0x09);
  fsmc_write_data(0x00);

  fsmc_write_reg(ILI9341_NGAMMA);
  fsmc_write_data(0x00);
  fsmc_write_data(0x0E);
  fsmc_write_data(0x14);
  fsmc_write_data(0x03);
  fsmc_write_data(0x11);
  fsmc_write_data(0x07);
  fsmc_write_data(0x31);
  fsmc_write_data(0xC1);
  fsmc_write_data(0x48);
  fsmc_write_data(0x08);
  fsmc_write_data(0x0F);
  fsmc_write_data(0x0C);
  fsmc_write_data(0x31);
  fsmc_write_data(0x36);
  fsmc_write_data(0x0F);

  fsmc_write_reg(ILI9341_NORON);
  fsmc_write_reg(ILI9341_DISPLAY_ON);

  // Initialize LVGL probably better to have this for each display driver
  udelay(100000);

  lv_init();
  lv_disp_draw_buf_init(&draw_buf, buf1, NULL, DISPLAY_BUFF_SIZE);
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = ILI9341_WIDTH;
  disp_drv.ver_res = ILI9341_HEIGHT;
  disp_drv.flush_cb = lvgl_display_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  
  
  
  
  lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x003a57), LV_PART_MAIN);
  lv_obj_t * btn = lv_btn_create(lv_scr_act());     /*Add a button the current screen*/
  lv_obj_set_pos(btn, 10, 10);                            /*Set its position*/
  lv_obj_set_size(btn, 120, 50);                          /*Set its size*/
  //lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_ALL, NULL);           /*Assign a callback to the button*/

  lv_obj_t * label = lv_label_create(btn);          /*Add a label to the button*/
  lv_label_set_text(label, "Button");                     /*Set the labels text*/
  lv_obj_center(label);
}



void lvgl_display_flush(lv_disp_drv_t * drv, const lv_area_t * area, lv_color_t * color_p)
{
    if(area->x2 < 0 || area->y2 < 0 || area->x1 > (ILI9341_WIDTH - 1) || area->y1 > (ILI9341_HEIGHT - 1)) {
        lv_disp_flush_ready(drv);
        return;
    }
    
    /* Truncate the area to the screen */
    int32_t act_x1 = area->x1 < 0 ? 0 : area->x1;
    int32_t act_y1 = area->y1 < 0 ? 0 : area->y1;
    int32_t act_x2 = area->x2 > ILI9341_WIDTH - 1 ? ILI9341_WIDTH - 1 : area->x2;
    int32_t act_y2 = area->y2 > ILI9341_HEIGHT - 1 ? ILI9341_HEIGHT - 1 : area->y2;

    /* window horizontal */
    fsmc_write_reg(ILI9341_CASET);
    fsmc_write_data(act_x1 >> 8);
    fsmc_write_data(act_x1 & 0xFF);
    fsmc_write_data(act_x2 >> 8);
    fsmc_write_data(act_x2 & 0xFF);

    /* window vertical */
    fsmc_write_reg (ILI9341_RASET);
    fsmc_write_data(act_y1 >> 8);
    fsmc_write_data(act_y1 & 0xFF);
    fsmc_write_data(act_y2 >> 8);
    fsmc_write_data(act_y2 & 0xFF);
    fsmc_write_reg(ILI9341_WRITE_RAM);

    uint32_t size = (area->x2 - area->x1 + 1) * (area->y2 - area->y1 + 1);
    while (size--) { fsmc_write_data(color_p->full); color_p++; }
    lv_disp_flush_ready(drv);
}

// This should be in a lvgl dedicated .c file, not really display driver specific
// for now keep it here, we tidy up everything later.
static uint32_t lastTick;
void lvgl_display_task(void)
{
  uint32_t now = timer_read_time();
  int32_t diff = now - lastTick;

  if(diff > 0 && diff > timer_from_us(1000))
  {
    lv_tick_inc(1);
    lastTick = now;
  }
  else if(diff < 0)
    lastTick = now;

  lv_timer_handler();
}
DECL_TASK(lvgl_display_task);
