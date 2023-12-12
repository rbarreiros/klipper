// FSMC functions on STM32
//
// Copyright (C) 2022  Rui Barreiros <rbarreiros@gmail.com>
//
// This file may be distributed under the terms of the GNU GPLv3 license.

#include "fsmc.h" 
#include "sched.h"                // decl_init
#include "generic/armcm_timer.h"  // udelay
#include "gpio.h"                 // gpio_out*
#include "command.h"              // DECL_*
#include "board/misc.h"           // timer_*
#include "internal.h"             // MACH & gpio
#include "autoconf.h"             // Kconfig

#include <stdlib.h>               // atoi

/*

  TODO

  - setup DMA, use DMA by default, but, provide a setting to be able to disable DMA in config if required
  - setup input devices, do touch and encoder which should be selected in config, if encoder, supply pins
  - setup orientation, portrait/landscape

  - create a default screen informing the user to connect to klipper, and only then it should switch to
    the regular UI, whatever it might be.

*/














/* Only for stm32f103ze, also uses FSMC, and we should eventually add support for it
#if ENABLED(STM32_XL_DENSITY)
  #define FSMC_CS_NE2 PG9
  #define FSMC_CS_NE3 PG10
  #define FSMC_CS_NE4 PG12

  #define FSMC_RS_A0  PF0
  #define FSMC_RS_A1  PF1
  #define FSMC_RS_A2  PF2
  #define FSMC_RS_A3  PF3
  #define FSMC_RS_A4  PF4
  #define FSMC_RS_A5  PF5
  #define FSMC_RS_A6  PF12
  #define FSMC_RS_A7  PF13
  #define FSMC_RS_A8  PF14
  #define FSMC_RS_A9  PF15
  #define FSMC_RS_A10 PG0
  #define FSMC_RS_A11 PG1
  #define FSMC_RS_A12 PG2
  #define FSMC_RS_A13 PG3
  #define FSMC_RS_A14 PG4
  #define FSMC_RS_A15 PG5
#endif
*/

#define FSMC_RS_A16   GPIO('D', 11)
#define FSMC_RS_A17   GPIO('D', 12)
#define FSMC_RS_A18   GPIO('D', 13)
#define FSMC_RS_A19   GPIO('E', 3)
#define FSMC_RS_A20   GPIO('E', 4)
#define FSMC_RS_A21   GPIO('E', 5)
#define FSMC_RS_A22   GPIO('E', 6)
#define FSMC_RS_A23   GPIO('E', 2)

/* for stm32f103ze
#if ENABLED(STM32_XL_DENSITY)
  #define FSMC_RS_A24 PG13
  #define FSMC_RS_A25 PG14
#endif
*/

static struct fsmc_lvgl {
//  struct timer timer;
//  uint32_t rest_ticks;
  struct gpio_out bl;
  struct gpio_out rst;
  struct gpio_out led;
} fsmc_lvgl;

// The board being used to debug is a Longer, which has a
// bootloader, app starts at 0x08010000 (64kb bootloader)

LCD_TypeDef *LCD;

inline void
fsmc_write_reg(uint16_t reg)
{
  LCD->REG = reg;
  __DSB();
}

inline void
fsmc_write_data(uint16_t value)
{
  LCD->RAM = value;
  __DSB();
}

inline uint16_t
fsmc_read_data(uint16_t reg)
{
  LCD->REG = reg;
  __DSB();

  return LCD->RAM;
}

// STM32 pin description format is P<PORT><PIN NR> ex PD7 or PD10
uint32_t
fsmc_pin_to_gpio(char *str)
{
  // quick an easy method of getting the info we need, is there a better way ???   
  int pinNr = 9999;
  char port = str[1];
  char nr[3] = {0};

  nr[0] = str[2];  
  nr[1] = (strlen(str) == 4) ? str[3] : '\0';
  nr[2] = '\0';

  pinNr = atoi(nr);

  output("PIN: %s %s %s %u", str, port, nr, pinNr);

  return GPIO(port, pinNr);
}

void
fsmc_set_address(void)
{
  uint32_t controllerAddress = (uint32_t)FSMC_BANK1;

#define _ORADDR(x) controllerAddress |= ((1 << x) - 2);

  switch (fsmc_pin_to_gpio(CONFIG_STM32_FSMC_RS_PIN)) 
  {
    /* TODO
    #if ENABLED(STM32_XL_DENSITY)
      case FSMC_RS_A0:  _ORADDR( 1); break;
      case FSMC_RS_A1:  _ORADDR( 2); break;
      case FSMC_RS_A2:  _ORADDR( 3); break;
      case FSMC_RS_A3:  _ORADDR( 4); break;
      case FSMC_RS_A4:  _ORADDR( 5); break;
      case FSMC_RS_A5:  _ORADDR( 6); break;
      case FSMC_RS_A6:  _ORADDR( 7); break;
      case FSMC_RS_A7:  _ORADDR( 8); break;
      case FSMC_RS_A8:  _ORADDR( 9); break;
      case FSMC_RS_A9:  _ORADDR(10); break;
      case FSMC_RS_A10: _ORADDR(11); break;
      case FSMC_RS_A11: _ORADDR(12); break;
      case FSMC_RS_A12: _ORADDR(13); break;
      case FSMC_RS_A13: _ORADDR(14); break;
      case FSMC_RS_A14: _ORADDR(15); break;
      case FSMC_RS_A15: _ORADDR(16); break;
    #endif
    */
    case FSMC_RS_A16: _ORADDR(17); break;
    case FSMC_RS_A17: _ORADDR(18); break;
    case FSMC_RS_A18: _ORADDR(19); break;
    case FSMC_RS_A19: _ORADDR(20); break;
    case FSMC_RS_A20: _ORADDR(21); break;
    case FSMC_RS_A21: _ORADDR(22); break;
    case FSMC_RS_A22: _ORADDR(23); break;
    case FSMC_RS_A23: _ORADDR(24); break;
    /* TODO
    #if ENABLED(STM32_XL_DENSITY)
      case FSMC_RS_A24: _ORADDR(25); break;
      case FSMC_RS_A25: _ORADDR(26); break;
    #endif
    */
    default:
      output("UNABLE TO FIND THE RIGHT FSMC RS ADDRESS --- CHECK CONFIG ---");
  }

  LCD = (LCD_TypeDef*)controllerAddress;
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
#define FSMC_DMA_CHANNEL                    DMA_CH5

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

void
fsmc_setup(void)
{
  uint32_t reg, mask;

  fsmc_set_address();

  // Led Pin PC2 
  fsmc_lvgl.led = gpio_out_setup(GPIO('C', 2), 1);
  gpio_out_write(fsmc_lvgl.led, 0);

  // Disable FSMC
  FSMC_Bank1->BTCR[0] &= ~(0x1UL << (0U));


  // Enable clock and setup FSMC pins
  // I'm using some stm32f103vet6 boards as reference
  // one would need to check variations and change accordingly

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
    gpio_peripheral(GPIO('E', 14), GPIO_FUNCTION(0), 0);  // D11
    gpio_peripheral(GPIO('E', 15), GPIO_FUNCTION(0), 0);  // D12

    // 
    //gpio_peripheral(GPIO('D', 7), GPIO_FUNCTION(0), 0);   // NE1 (CS)
    //gpio_peripheral(GPIO('D', 11), GPIO_FUNCTION(0), 0);   // RS (A16)
    gpio_peripheral(fsmc_pin_to_gpio(CONFIG_STM32_FSMC_CS_PIN), GPIO_FUNCTION(0), 0);   // NE1 (CS)
    gpio_peripheral(fsmc_pin_to_gpio(CONFIG_STM32_FSMC_RS_PIN), GPIO_FUNCTION(0), 0);   // RS (A16)

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

    // Enable FSMC
    FSMC_Bank1->BTCR[0] |= (0x1UL << (0U));

    // Disconnect NADV
    ((((AFIO_TypeDef *)((0x40000000UL + 0x00010000UL) + 0x00000000UL))->MAPR2) |= ((0x1UL << (10U))));


  // Reset LCD

  fsmc_lvgl.bl = gpio_out_setup(fsmc_pin_to_gpio(CONFIG_STM32_FSMC_BACKLIGHT_PIN), 0); // backlight
  fsmc_lvgl.rst = gpio_out_setup(fsmc_pin_to_gpio(CONFIG_STM32_FSMC_RESET_PIN), 0);  // reset

  gpio_out_write(fsmc_lvgl.rst, 0);
  udelay(10000);
  gpio_out_write(fsmc_lvgl.rst, 1);
  udelay(10000);
  gpio_out_write(fsmc_lvgl.bl, 1);

  // Start LVGL

  lvgl_display_init();

}
DECL_INIT(fsmc_setup);

void fsmc_check_pin(uint32_t *args)
{
  // we should test this by supplying some doofus non existant pin
  uint32_t pin = fsmc_pin_to_gpio(CONFIG_STM32_FSMC_RS_PIN);
  output("Setup pin %s with integer %u", CONFIG_STM32_FSMC_RS_PIN, pin);
  output("Correct pin is %u", GPIO('D', 11));
}
DECL_COMMAND(fsmc_check_pin, "fsmc_check_pin");

// Simple debug task to watch if mcu crashes, useless otherwise
static uint32_t lastLedTick;
void fsmc_led_blink_task(void)
{
  uint32_t now = timer_read_time();
  int32_t ledDiff = now - lastLedTick;

  if(ledDiff > 0 && ledDiff > timer_from_us(1000000))
  {
    gpio_out_toggle_noirq(fsmc_lvgl.led);
    lastLedTick = now;
  }
  else if(ledDiff < 0)
    lastLedTick = now;
}
DECL_TASK(fsmc_led_blink_task);

