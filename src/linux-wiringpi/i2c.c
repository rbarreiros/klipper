// Linux i2c implementation using libwiringPi
//
// Copyright (C) 2021 Rui Barreiros <rbarreiros@gmail.com>
//
// This file may be distributed under the terms of the GNU GPLv3 license.

// TODO TODO TODO Needs Testing!!

#include "internal.h" // report_errno
#include "gpio.h" // i2c_setup
#include "command.h" // shutdown
#include "sched.h"
#include <wiringPiI2C.h>

DECL_ENUMERATION_RANGE("i2c_bus", "i2c.0", 0, 2);

struct i2c_s {
    uint32_t bus;
    uint8_t addr;
    int fd;
};

static struct i2c_s devices[16] = {0};
static int devices_count = 0;

static int
i2c_open(uint32_t bus, uint8_t addr)
{
    // Find existing device (if already opened)
    int i;
    for (i=0; i<devices_count; i++) {
        if (devices[i].bus == bus && devices[i].addr == addr) {
            return devices[i].fd;
        }
    }

    i = wiringPiI2CSetup(addr);
    if(i < 0)
    {
      report_errno("Unable to initialize I2C Bus/Address", i);
      shutdown("Unable to open I2C Bus/Address");
      return i;
    }
    
    devices[devices_count].bus = bus;
    devices[devices_count].addr = addr;
    devices[devices_count].fd = i;
    devices_count++;

    return i;
}

struct i2c_config
i2c_setup(uint32_t bus, uint32_t rate, uint8_t addr)
{
    int fd = i2c_open(bus, addr);
    return (struct i2c_config){.fd=fd};
}

void
i2c_write(struct i2c_config config, uint8_t write_len, uint8_t *data)
{
  int i, ret = 0;
  
  for(i = 0; i < write_len; i++)
  {
    ret = wiringPiI2CWrite(config.fd, data[i]);
    if(ret < 0)
    {
      report_errno("Error while trying to write I2C value", ret);
      try_shutdown("Unable to write I2C value");
    }
  }
}

void
i2c_read(struct i2c_config config, uint8_t reg_len, uint8_t *reg
         , uint8_t read_len, uint8_t *data)
{
  int i, ret = 0;
  
  if(reg_len != 0)
  {
    for(i = 0; i < reg_len; i++)
    {
      ret = wiringPiI2CWriteReg8(config.fd, reg[i], 0);
      if(ret < 0)
      {
	  report_errno("Error while trying to write I2C register", ret);
	  try_shutdown("Unable to write I2C register value");
	  return;
      }
    }
  }
  
  for(i = 0; i < read_len; i++)
  {
    data[i] = wiringPiI2CRead(config.fd);
  }
}
