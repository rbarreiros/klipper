// FSMC functions on STM32
//
// Copyright (C) 2022  Rui Barreiros <rbarreiros@gmail.com>
//
// This file may be distributed under the terms of the GNU GPLv3 license.

#include "autoconf.h" // CONFIG_MACH_STM32F1
#include "command.h"
#include "gpio.h"     // i2c_setup
#include "internal.h" // GPIO
#include "sched.h"  // decl_init
#include "generic/armcm_timer.h"  // udelay

#include "ili9341.h" // temporary

// The board being used to debug is a Longer, which has a
// bootloader and app starts at 0x08010000 (64kb bootloader)

typedef struct
{
  volatile uint16_t REG;
  volatile uint16_t RAM;
} LCD_TypeDef;

LCD_TypeDef *LCD;

void
fsmc_write_reg(uint16_t reg)
{
  LCD->REG = reg;
  __DSB();
}

void
fsmc_write_data(uint16_t value)
{
  LCD->RAM = value;
  __DSB();
}

uint16_t
fsmc_read_data(uint16_t reg)
{
  LCD->REG = reg;
  __DSB();

  return LCD->RAM;
}

//3. Set cursor position
void ili9341_SetCursorPosition(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {

  fsmc_write_reg (ILI9341_CASET);
  fsmc_write_data(x1>>8);
  fsmc_write_data(x1 & 0xFF);
  fsmc_write_data(x2>>8);
  fsmc_write_data(x2 & 0xFF);

  fsmc_write_reg (ILI9341_RASET);
  fsmc_write_data(y1>>8);
  fsmc_write_data(y1 & 0xFF);
  fsmc_write_data(y2>>8);
  fsmc_write_data(y2 & 0xFF);
  fsmc_write_reg(ILI9341_WRITE_RAM);
}

void
ili9341_init(void)
{
    // TOUCH_LCD_IO_Init();

  fsmc_write_reg(ILI9341_SWRESET);
  /* Wait for 200ms */
  udelay(100000);

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
  fsmc_write_data(0x48); // MY and ML flipped +  bit 3 RGB and BGR changed.

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

void ili9341_Fill(uint16_t color) {
	uint32_t n = 240 * 320;
	
	ili9341_SetCursorPosition(0, 0,   240 - 1, 320 - 1);
	
	while (n--) {
    fsmc_write_data(color);
	}
}



/**

  FSMC                pinout GPIO configuration
  
  FSMC_A[25:0]
  FSMC_D[15:0]        Alternate function push-pull

  FSMC_CK             Alternate function push-pull

  FSMC_NOE
  FSMC_NWE            Alternate function push-pull

  FSMC_NE[4:1]
  FSMC_NCE[3:2]
  FSMC_NCE4_1
  FSMC_NCE4_2         Alternate function push-pull

  FSMC_NWAIT
  FSMC_CD             Input floating/ Input pull-up

  FSMC_NIOS16,
  FSMC_INTR
  FSMC_INT[3:2]       Input floating

  FSMC_NL
  FSMC_NBL[1:0]       Alternate function push-pull

  FSMC_NIORD, 
  FSMC_NIOWR
  FSMC_NREG           Alternate function push-pull

-----------------------------------

// Should be on menuconfig ?? or better yet, on fsmc_setup of printer.cfg!!

#define LCD_RESET_PIN                       PC4   // pin 33
#define LCD_BACKLIGHT_PIN                   PD12  // pin 59

#define FSMC_CS_PIN                         PD7   // pin 88 = FSMC_NE1
#define FSMC_RS_PIN                         PD11  // pin 58 A16 Register. Only one address needed

#define LCD_USE_DMA_FSMC                          // Use DMA transfers to send data to the TFT
#define FSMC_DMA_DEV                        DMA2
#define FSMC_DMA_CHANNEL                 DMA_CH5


 Note: Alfawise U20/U30 boards DON'T use SPI2, as the hardware designer
 mixed up MOSI and MISO pins. SPI is managed in SW, and needs pins
 declared below.
 
#if ENABLED(TOUCH_BUTTONS)
  #define TOUCH_CS_PIN                      PB12  // pin 51 SPI2_NSS
  #define TOUCH_SCK_PIN                     PB13  // pin 52
  #define TOUCH_MOSI_PIN                    PB14  // pin 53
  #define TOUCH_MISO_PIN                    PB15  // pin 54
  #define TOUCH_INT_PIN                     PC6   // pin 63 (PenIRQ coming from ADS7843)
#endif
*/

static struct task_wake blink;
struct gpio_out led, rst, bl;

void led_blink(void)
{
  if (!sched_check_wake(&blink))
    return;

  gpio_out_toggle_noirq(led);
}
DECL_TASK(led_blink);

void
fsmc_setup(void)
{
  uint32_t reg, mask;

  uint32_t controllerAddress = (uint32_t)FSMC_BANK1;
  controllerAddress |= ((1 << 17) - 2);
  LCD = (LCD_TypeDef*)controllerAddress;

  // Led Pin PC2 
  led = gpio_out_setup(GPIO('C', 2), 1);

  // Setup DMA --- TODO

  // 

  // Disable FSMC
  FSMC_Bank1->BTCR[0] &= ~(0x1UL << (0U));


  // Enable clock and setup FSMC pins
  // I'm using some stm32f103vet6 boards as reference
  // one would need to check variations and change accordingly

  //if (!is_enabled_pclock((uint32_t)FSMC_BANK1)) {
    //enable_pclock((uint32_t)FSMC_BANK1);
    RCC->AHBENR |= RCC_AHBENR_FSMCEN;
    /* Delay after an RCC peripheral clock enabling */
    volatile uint32_t tmpreg = READ_BIT(RCC->AHBENR, RCC_AHBENR_FSMCEN);
    (void)(tmpreg);

    gpio_peripheral(GPIO('D', 0), GPIO_FUNCTION(0), 0);   // D2
    gpio_peripheral(GPIO('D', 1), GPIO_FUNCTION(0), 0);   // D3
    gpio_peripheral(GPIO('D', 4), GPIO_FUNCTION(0), 0);   // NOE
    gpio_peripheral(GPIO('D', 5), GPIO_FUNCTION(0), 0);   // NWE
    gpio_peripheral(GPIO('D', 8), GPIO_FUNCTION(0), 0);   // D13
    gpio_peripheral(GPIO('D', 9), GPIO_FUNCTION(0), 0);   // D14
    gpio_peripheral(GPIO('D', 10), GPIO_FUNCTION(0), 0);  // D15
    gpio_peripheral(GPIO('D', 14), GPIO_FUNCTION(0), 0);  // D0
    gpio_peripheral(GPIO('D', 15), GPIO_FUNCTION(0), 0);  // D1

    gpio_peripheral(GPIO('E', 7), GPIO_FUNCTION(0), 0);   // D4
    gpio_peripheral(GPIO('E', 8), GPIO_FUNCTION(0), 0);   // D5
    gpio_peripheral(GPIO('E', 9), GPIO_FUNCTION(0), 0);   // D6
    gpio_peripheral(GPIO('E', 10), GPIO_FUNCTION(0), 0);  // D7
    gpio_peripheral(GPIO('E', 11), GPIO_FUNCTION(0), 0);  // D8
    gpio_peripheral(GPIO('E', 12), GPIO_FUNCTION(0), 0);  // D9
    gpio_peripheral(GPIO('E', 13), GPIO_FUNCTION(0), 0);  // D10
    gpio_peripheral(GPIO('E', 14), GPIO_FUNCTION(0), 0);  // D11ls
    
    gpio_peripheral(GPIO('E', 15), GPIO_FUNCTION(0), 0);  // D12

    gpio_peripheral(GPIO('D', 7), GPIO_FUNCTION(0), 0);   // NE1 (CS)
    gpio_peripheral(GPIO('D', 11), GPIO_FUNCTION(0), 0);   // RS (A16)

    /**
    CRL - CNF7/MODE7 | 6 ... | CNF0/MODE0

    CNF :
      Input Mode:
      00 - Analog
      01 - Floating input (reset)
      10 - Input pull up
      11 - Reserved

      Output Mode:
      00 - General Purpose Push Pull
      01 - General Purpose Open Drain
      10 - Alternate Push Pull
      11 - Alternate Open Drain

    MODE : 
      00 - Input mode (Reset)
      01 - Output mode max speed 10 Mhz
      10 - Output mode max speed 2 Mhz
      11 - Output mode max speed 50 Mhz

    Alternate mode Push Pull max speed - 1011 - 0xb
    Output push pull max speed         - 0011 - 0x3
    ---
    
    AF 0, 1, 4, 5, 7, 8, 9, 10, 11, 14, 15
    OU 12

    GPIOD - bb?3bbbb  b?bb??bb
          - bb42bbbb  b4bb44bb

    GPIOD
    CRL - 0xb4bb44bb
    CRH - 0xbb42bbbb
    
    GPIOE
    CRL - 0xb4444444
    CRH - 0xbbbbbbbb
    
    */




    // Setup FSMC
    // STM32 headers group together BCR and BTR register in BTCR[8]
    // BCR0 -> BTCR[0]
    // BTR0 -> BTCR[1]

    /**
      BCR
      Bits 31: 20 Reserved, must be kept at reset value.
      Bit  19: CBURSTRW: Write burst enable.                        - 0: Write operations are always performed in asynchronous mode
      Bits 18: 16 CPSIZE[2:0]: CRAM page size                       - 000: No burst split when crossing page boundary (default after reset)
      Bit  15: ASYNCWAIT: Wait signal during asynchronous transfers - 0: NWAIT signal is not taken into account when running an asynchronous protocol (default after reset)
      Bit  14: EXTMOD: Extended mode enable.                        - 0: values inside FSMC_BWTR register are not taken into account (default after reset)
      Bit  13: WAITEN: Wait enable bit                              - 1: NWAIT signal is enabled (its level is taken into account after the programmed Flash latency period to insert wait states if asserted) (default after reset)
      Bit  12 WREN: Write enable bit.                               - 1: Write operations are enabled for the bank by the FSMC (default after reset).
      Bit  11 WAITCFG: Wait timing configuration.                   - 0: NWAIT signal is active one data cycle before wait state (default after reset)
      Bit  10 WRAPMOD: Wrapped burst mode support.                  - 0: Direct wrapped burst is not enabled (default after reset)
      Bit   9 WAITPOL: Wait signal polarity bit.                    - 0: NWAIT active low (default after reset)
      Bit   8 BURSTEN: Burst enable bit.                            - 0: Burst mode disabled (default after reset).
      Bit   7 Reserved, must be kept at reset value.
      Bit   6 FACCEN: Flash access enable                           - 1: Corresponding NOR Flash memory access is enabled (default after reset)
      Bits  5:4 MWID[1:0]: Memory databus width.                    - 01: 16 bits (default after reset)
      Bits  3:2 MTYP[1:0]: Memory type                              - 00: SRAM (default after reset for Bank 2...4)
      Bit   1 MUXEN: Address/data multiplexing enable bit           - 1: Address/Data multiplexed on databus (default after reset)
      Bit   0 MBKEN: Memory bank enable bit                         - 1: Corresponding memory bank is enabled
    */


    reg = 0x1010;
    mask = 0xfff7f;

    /*
    mask = (FSMC_BCRx_MBKEN                |
          FSMC_BCRx_MUXEN                |
          FSMC_BCRx_MTYP                 |
          FSMC_BCRx_MWID                 |
          FSMC_BCRx_FACCEN               |
          FSMC_BCRx_BURSTEN              |
          FSMC_BCRx_WAITPOL              |
          FSMC_BCRx_WAITCFG              |
          FSMC_BCRx_WREN                 |
          FSMC_BCRx_WAITEN               |
          FSMC_BCRx_EXTMOD               |
          FSMC_BCRx_ASYNCWAIT            |
          FSMC_BCRx_CBURSTRW);

    mask |= FSMC_BCRx_WRAPMOD;
    mask |= 0x00070000U; // CPSIZE to be defined in CMSIS file 
    */

    MODIFY_REG(FSMC_Bank1->BTCR[0], mask, reg);

    // Timings
    //FSMC_Bank1->BTCR[1] = (15 << 8) | 15;  // (FSMC_DATA_SETUP_TIME << 8) | FSMC_ADDRESS_SETUP_TIME
    FSMC_Bank1->BTCR[1] = 0xfff0fff;

    FSMC_Bank1E->BWTR[0] = 0x0FFFFFFFU;
//#if ENABLED(STM32_XL_DENSITY)
    //FSMC_NOR_PSRAM4_BASE->BCR = FSMC_BCR_WREN | FSMC_BCR_MTYP_SRAM | FSMC_BCR_MWID_16BITS | FSMC_BCR_MBKEN;
    //FSMC_NOR_PSRAM4_BASE->BTR = (FSMC_DATA_SETUP_TIME << 8) | FSMC_ADDRESS_SETUP_TIME;
//#else // PSRAM1 for STM32F103V (high density)
    //FSMC_NOR_PSRAM1_BASE->BCR = FSMC_BCR_WREN | FSMC_BCR_MTYP_SRAM | FSMC_BCR_MWID_16BITS | FSMC_BCR_MBKEN;
    //FSMC_NOR_PSRAM1_BASE->BTR = (FSMC_DATA_SETUP_TIME << 8) | FSMC_ADDRESS_SETUP_TIME;
//#endif

    // Enable FSMC
    FSMC_Bank1->BTCR[0] |= (0x1UL << (0U));

    // Disconnect NADV
    ((((AFIO_TypeDef *)((0x40000000UL + 0x00010000UL) + 0x00000000UL))->MAPR2) |= ((0x1UL << (10U))));

  //}
  
  // Reset LCD

  bl = gpio_out_setup(GPIO('D', 12), 0);  // backlight
  rst = gpio_out_setup(GPIO('C', 4), 0);  // reset

  gpio_out_write(rst, 0);
  udelay(10000);
  gpio_out_write(rst, 1);
  udelay(10000);
  gpio_out_write(bl, 1);

  // Get lcd ID

  ili9341_init();
  ili9341_Fill(0xF800);

  gpio_out_write(led, 0);
}

DECL_INIT(fsmc_setup);
