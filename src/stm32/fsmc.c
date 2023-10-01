// FSMC functions on STM32
//
// Copyright (C) 2022  Rui Barreiros <rbarreiros@gmail.com>
//
// This file may be distributed under the terms of the GNU GPLv3 license.


#include "autoconf.h" // CONFIG_MACH_STM32F1
#include "gpio.h" // i2c_setup
#include "internal.h" // GPIO


/*
const PinMap pinMap_FSMC[] = {
  {PD_14,  FSMC_NORSRAM_DEVICE, FSMC_PIN_DATA}, // FSMC_D00
  {PD_15,  FSMC_NORSRAM_DEVICE, FSMC_PIN_DATA}, // FSMC_D01
  {PD_0,   FSMC_NORSRAM_DEVICE, FSMC_PIN_DATA}, // FSMC_D02
  {PD_1,   FSMC_NORSRAM_DEVICE, FSMC_PIN_DATA}, // FSMC_D03
  {PE_7,   FSMC_NORSRAM_DEVICE, FSMC_PIN_DATA}, // FSMC_D04
  {PE_8,   FSMC_NORSRAM_DEVICE, FSMC_PIN_DATA}, // FSMC_D05
  {PE_9,   FSMC_NORSRAM_DEVICE, FSMC_PIN_DATA}, // FSMC_D06
  {PE_10,  FSMC_NORSRAM_DEVICE, FSMC_PIN_DATA}, // FSMC_D07
  #if DISABLED(TFT_INTERFACE_FSMC_8BIT)
    {PE_11,  FSMC_NORSRAM_DEVICE, FSMC_PIN_DATA}, // FSMC_D08
    {PE_12,  FSMC_NORSRAM_DEVICE, FSMC_PIN_DATA}, // FSMC_D09
    {PE_13,  FSMC_NORSRAM_DEVICE, FSMC_PIN_DATA}, // FSMC_D10
    {PE_14,  FSMC_NORSRAM_DEVICE, FSMC_PIN_DATA}, // FSMC_D11
    {PE_15,  FSMC_NORSRAM_DEVICE, FSMC_PIN_DATA}, // FSMC_D12
    {PD_8,   FSMC_NORSRAM_DEVICE, FSMC_PIN_DATA}, // FSMC_D13
    {PD_9,   FSMC_NORSRAM_DEVICE, FSMC_PIN_DATA}, // FSMC_D14
    {PD_10,  FSMC_NORSRAM_DEVICE, FSMC_PIN_DATA}, // FSMC_D15
  #endif
  {PD_4,   FSMC_NORSRAM_DEVICE, FSMC_PIN_DATA}, // FSMC_NOE
  {PD_5,   FSMC_NORSRAM_DEVICE, FSMC_PIN_DATA}, // FSMC_NWE
  {NC,    NP,    0}
};

const PinMap pinMap_FSMC_CS[] = {
  {PD_7,  (void *)FSMC_NORSRAM_BANK1, FSMC_PIN_DATA}, // FSMC_NE1
  #ifdef PF0
    {PG_9,  (void *)FSMC_NORSRAM_BANK2, FSMC_PIN_DATA}, // FSMC_NE2
    {PG_10, (void *)FSMC_NORSRAM_BANK3, FSMC_PIN_DATA}, // FSMC_NE3
    {PG_12, (void *)FSMC_NORSRAM_BANK4, FSMC_PIN_DATA}, // FSMC_NE4
  #endif
  {NC,    NP,    0}
};

#if ENABLED(TFT_INTERFACE_FSMC_8BIT)
  #define FSMC_RS(A)  (void *)((2 << (A-1)) - 1)
#else
  #define FSMC_RS(A)  (void *)((2 << A) - 2)
#endif
*/


#define FSMC_NORSRAM_TypeDef            FSMC_Bank1_TypeDef
#define FSMC_NORSRAM_EXTENDED_TypeDef   FSMC_Bank1E_TypeDef

#define FSMC_NORSRAM_BANK1                       (0x00000000U)
#define FSMC_NORSRAM_BANK2                       (0x00000002U)
#define FSMC_NORSRAM_BANK3                       (0x00000004U)
#define FSMC_NORSRAM_BANK4                       (0x00000006U)


#define FSMC_NORSRAM_FLASH_ACCESS_ENABLE         (0x00000040U)
#define FSMC_NORSRAM_FLASH_ACCESS_DISABLE        (0x00000000U)

/**
  * @brief  FSMC NORSRAM Timing parameters structure definition
  */
typedef struct
{
  uint32_t AddressSetupTime;             /*!< Defines the number of HCLK cycles to configure
                                              the duration of the address setup time.
                                              This parameter can be a value between Min_Data = 0 and Max_Data = 15.
                                              @note This parameter is not used with synchronous NOR Flash memories.      */

  uint32_t AddressHoldTime;              /*!< Defines the number of HCLK cycles to configure
                                              the duration of the address hold time.
                                              This parameter can be a value between Min_Data = 1 and Max_Data = 15.
                                              @note This parameter is not used with synchronous NOR Flash memories.      */

  uint32_t DataSetupTime;                /*!< Defines the number of HCLK cycles to configure
                                              the duration of the data setup time.
                                              This parameter can be a value between Min_Data = 1 and Max_Data = 255.
                                              @note This parameter is used for SRAMs, ROMs and asynchronous multiplexed
                                              NOR Flash memories.                                                        */

  uint32_t BusTurnAroundDuration;        /*!< Defines the number of HCLK cycles to configure
                                              the duration of the bus turnaround.
                                              This parameter can be a value between Min_Data = 0 and Max_Data = 15.
                                              @note This parameter is only used for multiplexed NOR Flash memories.      */

  uint32_t CLKDivision;                  /*!< Defines the period of CLK clock output signal, expressed in number of
                                              HCLK cycles. This parameter can be a value between Min_Data = 2 and Max_Data = 16.
                                              @note This parameter is not used for asynchronous NOR Flash, SRAM or ROM
                                              accesses.                                                                  */

  uint32_t DataLatency;                  /*!< Defines the number of memory clock cycles to issue
                                              to the memory before getting the first data.
                                              The parameter value depends on the memory type as shown below:
                                              - It must be set to 0 in case of a CRAM
                                              - It is don't care in asynchronous NOR, SRAM or ROM accesses
                                              - It may assume a value between Min_Data = 2 and Max_Data = 17 in NOR Flash memories
                                                with synchronous burst mode enable                                       */

  uint32_t AccessMode;                   /*!< Specifies the asynchronous access mode.
                                              This parameter can be a value of @ref FSMC_Access_Mode                      */
} FSMC_NORSRAM_TimingTypeDef;

typedef struct
{
  FSMC_NORSRAM_TypeDef           *Instance;  //!< Register base address                        
  FSMC_NORSRAM_EXTENDED_TypeDef  *Extended;  //!< Extended mode register base address          
  //FSMC_NORSRAM_InitTypeDef       Init;       //!< SRAM device control configuration parameters 
  //HAL_LockTypeDef               Lock;       //!< SRAM locking object                          
  //volatile HAL_SRAM_StateTypeDef    State;      //!< SRAM device access state                     
  //DMA_HandleTypeDef             *hdma;      //!< Pointer DMA handler                          
} SRAM_HandleTypeDef;


/*
#define FSMC_BASE             0x60000000UL //!< FSMC base address 
#define FSMC_R_BASE           0xA0000000UL //!< FSMC registers base address 
#define FSMC_BANK1_R_BASE     (FSMC_R_BASE + 0x00000000UL)    //!< FSMC Bank1 registers base address 
#define FSMC_BANK1E_R_BASE    (FSMC_R_BASE + 0x00000104UL)    //!< FSMC Bank1E registers base address 

#define FSMC_Bank1          ((FSMC_Bank1_TypeDef *)FSMC_BANK1_R_BASE)
#define FSMC_Bank1E         ((FSMC_Bank1E_TypeDef *)FSMC_BANK1E_R_BASE)

#define FSMC_NORSRAM_DEVICE             FSMC_Bank1
#define FSMC_NORSRAM_EXTENDED_DEVICE    FSMC_Bank1E


//
#define FSMC_DATA_ADDRESS_MUX_DISABLE            ((uint32_t)0x00000000U)

#define FSMC_MEMORY_TYPE_SRAM                    ((uint32_t)0x00000000U)

#define FSMC_NORSRAM_MEM_BUS_WIDTH_8             ((uint32_t)0x00000000U)
#define FSMC_NORSRAM_MEM_BUS_WIDTH_16            ((uint32_t)0x00000010U)
#define FSMC_NORSRAM_MEM_BUS_WIDTH_32            ((uint32_t)0x00000020U)

#define FSMC_BURST_ACCESS_MODE_DISABLE           ((uint32_t)0x00000000U)
#define FSMC_WAIT_SIGNAL_POLARITY_LOW            ((uint32_t)0x00000000U)
#define FSMC_WRAP_MODE_DISABLE                   ((uint32_t)0x00000000U)
#define FSMC_WAIT_TIMING_BEFORE_WS               ((uint32_t)0x00000000U)
#define FSMC_WRITE_OPERATION_ENABLE              ((uint32_t)0x00001000U)
#define FSMC_WAIT_SIGNAL_DISABLE                 ((uint32_t)0x00000000U)
#define FSMC_EXTENDED_MODE_ENABLE                ((uint32_t)0x00004000U)
#define FSMC_ASYNCHRONOUS_WAIT_DISABLE           ((uint32_t)0x00000000U)
#define FSMC_WRITE_BURST_DISABLE                 ((uint32_t)0x00000000U)

#define FSMC_ACCESS_MODE_A                       ((uint32_t)0x00000000U)

*/
// FSMC_Bank1_TypeDef
// FSMC_Bank1E_TypeDef
// FSMC_Bank2_3_TypeDef
// FSMC_Bank4_TypeDef

/* Enable the FMC/FSMC interface clock */
//  RCC->AHB3ENR         |= 0x00000001;

/**

#define FSMC_CS_PIN                       PD7   // FSMC_NE1
#define FSMC_RS_PIN                       PD11  // A16 Register. Only one address needed


 */

void
fsmc_setup(void)
{
  uint32_t tmpreg, mask;

  // TODO We should setup DMA too

  // Enable FSMC Clock
  SET_BIT(RCC->AHBENR, RCC_AHBENR_FSMCEN);
  tmpreg = READ_BIT(RCC->AHBENR, RCC_AHBENR_FSMCEN);
  (void)tmpreg;

  // Setup FSMC interface

  // Disable FSMC_NORSRAM_BANK1
  FSMC_Bank1->BTCR[0] &= ~FSMC_BCRx_MBKEN;

  /**
   * BTCR reg
   * 
   * FSMC_NORSRAM_FLASH_ACCESS_ENABLE (0x00000040U)
   * FSMC_DATA_ADDRESS_MUX_DISABLE    (0x00000000U)
   * FSMC_MEMORY_TYPE_SRAM            (0x00000000U)
   * FSMC_NORSRAM_MEM_BUS_WIDTH_16    (0x00000010U)
   * FSMC_BURST_ACCESS_MODE_DISABLE   (0x00000000U)
   * FSMC_WAIT_SIGNAL_POLARITY_LOW    (0x00000000U)
   * FSMC_WAIT_TIMING_BEFORE_WS       (0x00000000U)
   * FSMC_WRITE_OPERATION_ENABLE      (0x00001000U)
   * FSMC_WAIT_SIGNAL_DISABLE         (0x00000000U)
   * FSMC_EXTENDED_MODE_DISABLE       (0x00000000U)
   * FSMC_ASYNCHRONOUS_WAIT_DISABLE   (0x00000000U)
   * FSMC_WRITE_BURST_DISABLE         (0x00000000U)
   * FSMC_WRAP_MODE_DISABLE           (0x00000000U)
   * FSMC_PAGE_SIZE_NONE              (0x00000000U)
   * 
   */

  tmpreg = (0x00000040U) | (0x00000010U) | (0x00001000U);

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
          FSMC_BCRx_CBURSTRW             |
          FSMC_BCRx_WRAPMOD              |
          0x00070000U);
  
  MODIFY_REG(FSMC_Bank1->BTCR[0], mask, tmpreg);
  
  // Setup timing
  // Address and setup 15
  FSMC_Bank1->BTCR[1] = (15 << 8) | 15;

  // Setup hardware pins
  // these should probably come from printer.conf....






}
