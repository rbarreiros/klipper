
#include "autoconf.h"               // defines
#include "lvgl_ili9341.h"
#include "generic/armcm_timer.h"    // udelay
#include "fsmc.h"                   // fsmc_*

static lv_disp_drv_t *lastDriverUsed;

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
}

void lvgl_display_flush(lv_disp_drv_t * drv, const lv_area_t * area, lv_color_t * color_p)
{
    if(area->x2 < 0 || area->y2 < 0 || area->x1 > (DISPLAY_WIDTH - 1) || area->y1 > (DISPLAY_HEIGHT - 1)) {
        lv_disp_flush_ready(drv);
        return;
    }

    lastDriverUsed = drv;

    /* Truncate the area to the screen */
    int32_t act_x1 = area->x1 < 0 ? 0 : area->x1;
    int32_t act_y1 = area->y1 < 0 ? 0 : area->y1;
    int32_t act_x2 = area->x2 > DISPLAY_WIDTH - 1 ? DISPLAY_WIDTH - 1 : area->x2;
    int32_t act_y2 = area->y2 > DISPLAY_HEIGHT - 1 ? DISPLAY_HEIGHT - 1 : area->y2;

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

// TODO
//#if FSMC_LVGL_DMA_ENABLED
    fsmc_dma_transfer((uint16_t *)color_p, size);
//#else
    //while (size--) { fsmc_write_data(color_p->full); color_p++; }
    //lv_disp_flush_ready(drv);
//#endif
}
