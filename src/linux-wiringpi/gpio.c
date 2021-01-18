// Basic support via libwiringPi
//
// Copyright (C) 2021  Rui Barreiros <rbarreiros@gmail.com>
//
// This file may be distributed under the terms of the GNU GPLv3 license.
#include "internal.h" // report_errno and autoconf
#include "gpio.h"
#include "command.h" // shutdown

#include <wiringPi.h>

DECL_ENUMERATION_RANGE("pin", "gpio0", 0, MAX_GPIO_LINES);

struct gpio_line {
  int pin;
  int state;
};

// Don't think wiringPi has more than 256 gpios
static struct gpio_line lines[MAX_GPIO_LINES];

struct gpio_out
gpio_out_setup(uint32_t pin, uint8_t val)
{
  struct gpio_line *line = &lines[pin];
  line->pin = pin;
  struct gpio_out g = { .line = line };
  gpio_out_reset(g, val);
  return g;
}

void
gpio_out_reset(struct gpio_out g, uint8_t val)
{
  int rv = wiringPiSetup();
  if(rv != 0)
  {
    report_errno("gpio_out_reset unable to initialize wiringPi", rv);
    shutdown("Unable to initialize wiringPi");
    return;
  }
  pinMode(g.line->pin, OUTPUT);
  g.line->state = val;
  gpio_out_write(g, val);
}

void
gpio_out_write(struct gpio_out g, uint8_t val)
{
  g.line->state = val;
  digitalWrite(g.line->pin, val);
}

void
gpio_out_toggle(struct gpio_out g)
{
  gpio_out_write(g,!g.line->state);
}

void
gpio_out_toggle_noirq(struct gpio_out g)
{
  gpio_out_toggle(g);
}

struct gpio_in
gpio_in_setup(uint32_t pin, int8_t pull_up)
{
  struct gpio_line *line = &lines[pin];
  line->pin = pin;
  struct gpio_in g = { .line = line };
  gpio_in_reset(g, pull_up);
  return g;
}

void
gpio_in_reset(struct gpio_in g, int8_t pull_up)
{
  int rv = wiringPiSetup();
  if(rv != 0)
  {
    report_errno("gpio_out_reset unable to initialize wiringPi", rv);
    shutdown("Unable to initialize wiringPi");
    return;
  }

  // Save it anyway, not that we're going to need it further down the line
  g.line->state = pull_up;
  pinMode(g.line->pin, INPUT);
  
  if(pull_up > 0)
    pullUpDnControl(g.line->pin, PUD_UP);
  else if(pull_up < 0)
    pullUpDnControl(g.line->pin, PUD_DOWN);
  else
    pullUpDnControl(g.line->pin, PUD_OFF);
}

uint8_t
gpio_in_read(struct gpio_in g)
{
  return digitalRead(g.line->pin);
}
