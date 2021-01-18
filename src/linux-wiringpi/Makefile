# Additional linux build rules

dirs-y += src/linux-wiringpi src/generic

src-y += linux-wiringpi/main.c linux-wiringpi/timer.c linux-wiringpi/console.c linux-wiringpi/watchdog.c
src-y += linux-wiringpi/spidev.c linux-wiringpi/analog.c linux-wiringpi/hard_pwm.c
src-y += linux-wiringpi/i2c.c linux-wiringpi/gpio.c generic/crc16_ccitt.c generic/alloc.c

CFLAGS_klipper.elf += -lutil -lwiringPi -lpthread -lcrypt -lm -lrt

flash: $(OUT)klipper.elf
	@echo "  Flashing"
	$(Q)sudo ./scripts/flash-linux.sh
