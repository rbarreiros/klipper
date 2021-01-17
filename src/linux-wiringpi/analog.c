// Read analog values from Linux IIO device
//
// Copyright (C) 2017  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU GPLv3 license.

#include <fcntl.h> // open
#include <stdio.h> // snprintf
#include <stdlib.h> // atoi
#include <unistd.h> // read
#include "command.h" // shutdown
#include "gpio.h" // gpio_adc_setup
#include "internal.h" // report_errno
#include "sched.h" // sched_shutdown

#include <wiringPi.h>

DECL_ENUMERATION_RANGE("pin", "analog0", 0, 8);

struct gpio_adc
gpio_adc_setup(uint32_t pin)
{
  int rv = wiringPiSetup();
  if(rv != 0)
  {
    report_errno("gpio_adc_setup unable to initialize wiringPi", rv);
    shutdown("Unable to initialize wiringPi");
    return (struct gpio_adc){ .pin = -1 };
  }
  return (struct gpio_adc){ .pin = pin };
}

uint32_t
gpio_adc_sample(struct gpio_adc g)
{
  return 0;
}

uint16_t
gpio_adc_read(struct gpio_adc g)
{
  int val = analogRead(g.pin);
  if(val < 0)
  {
    report_errno("gpio_adc_read unable to read analog value or non existing adc", val);
    return 0;
  }
  
  return analogRead(g.pin);
}

void
gpio_adc_cancel_sample(struct gpio_adc g)
{
}
