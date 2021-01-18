// SPI Support vial linux through libwiringPi
//
// Copyright (C) 2021 Rui Barreiros <rbarreiros@gmail.com>
//
// This file may be distributed under the terms of the GNU GPLv3 license.

// TODO TODO TODO Needs Testing!!

#include "command.h" // DECL_COMMAND
#include "gpio.h" // spi_setup
#include "internal.h" // report_errno
#include "sched.h" // shutdown

#include <wiringPiSPI.h>

// are there boards with more than 8 spi busses ?
DECL_ENUMERATION_RANGE("spi_bus", "spidev0.0", 0, 8);

struct spi_config
spi_setup(uint32_t bus, uint8_t mode, uint32_t rate)
{
  if(rate < 500000)
    rate = 500000;
  else if(rate > 32000000)
    rate = 32000000;
  
  int rv = wiringPiSPISetup(bus, rate);
  if(rv < 0)
  {
    report_errno("Unable to setup wiringPiSPI", rv);
    shutdown("Unable to setup wiringPiSPI");
  }
  
  return (struct spi_config) { rv, rate};
}

void
spi_prepare(struct spi_config config)
{
}

void
spi_transfer(struct spi_config config, uint8_t receive_data
             , uint8_t len, uint8_t *data)
{
  int rv;

  rv = wiringPiSPIDataRW(config.fd, data, len);
  if(rv < 0)
  {
    report_errno("Unable to read/write SPI", rv);
    shutdown("Unable to read/write SPI");
  }
}
