// HW PWM support via wiringPi
//
// Copyright (C) 2021 Rui Barreiros <rbarreiros@gmail.com>
//
// This file may be distributed under the terms of the GNU GPLv3 license.

#include "gpio.h" // struct gpio_pwm
#include "internal.h" // NSECS_PER_TICK
#include "command.h" // shutdown
#include "sched.h" // sched_shutdown

#include <wiringPi.h>

DECL_ENUMERATION_RANGE("pin", "pwm0", 0, 8);

struct gpio_pwm gpio_pwm_setup(uint32_t pin, uint32_t cycle_time, uint16_t val)
{
  struct gpio_pwm g = {0};
  int rv = wiringPiSetup();
  
  rv = wiringPiSetup();
  if(rv < 0)
  {
    report_errno("Error setting up wiringPiSetup", rv);
    shutdown("Unable to config pwm device");
    return g;
  }
  
  g.pin = pin;

  pinMode(pin, PWM_OUTPUT);
  gpio_pwm_write(g, val);
  return g;
}


void gpio_pwm_write(struct gpio_pwm g, uint16_t val)
{
  if(val > 1024)
    val = 1024;
  
  pwmWrite(g.pin, val);
}
