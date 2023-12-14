#ifndef __FSMC_H__
#define __FSMC_H__

#include <stdint.h>
#include "lvgl/lvgl.h"
#include "autoconf.h"

typedef struct
{
  volatile uint16_t REG;
  volatile uint16_t RAM;
} LCD_TypeDef;

void fsmc_write_reg(uint16_t reg);
void fsmc_write_data(uint16_t value);
uint16_t fsmc_read_data(uint16_t reg);
void fsmc_dma_transfer(uint16_t *buff, uint32_t size);

#endif
