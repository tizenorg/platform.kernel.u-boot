/*
 * Copyright (C) 2010 Samsung Electronics
 * Sanghee Kim <sh0130.kim@samsung.com>
 */

#include <common.h>
#include <serial.h>

static struct serial_device *serial_current = &s5p_serial2_device;

struct serial_device *__default_serial_console(void)
{
	return &s5p_serial2_device;
}

struct serial_device *default_serial_console(void)
	__attribute__((weak, alias("__default_serial_console")));

void serial_putc(const char c)
{
	if (!serial_current) {
		struct serial_device *dev = default_serial_console();

		dev->putc(c);
		return;
	}

	serial_current->putc(c);
}

void serial_puts(const char *s)
{
	if (!serial_current) {
		struct serial_device *dev = default_serial_console();

		dev->puts(s);
		return;
	}

	serial_current->puts(s);
}

int serial_init(void)
{
	if (!serial_current) {
		struct serial_device *dev = default_serial_console();

		return dev->init();
	}

	return serial_current->init();
}
