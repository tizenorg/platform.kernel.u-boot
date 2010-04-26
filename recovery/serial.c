/*
 * (C) Copyright 2010 Samsung Electronics
 * Sanghee Kim <sh0130.kim@samsung.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <serial.h>

static struct serial_device *serial_current = NULL;

struct serial_device *__default_serial_console (void)
{
	return &s5pc1xx_serial2_device;
}

struct serial_device *default_serial_console(void) __attribute__((weak, alias("__default_serial_console")));

void serial_putc(const char c)
{
	if (!serial_current) {
		struct serial_device *dev = default_serial_console ();

		dev->putc (c);
		return;
	}

	serial_current->putc (c);
}

void serial_puts(const char *s)
{
	if (!serial_current) {
		struct serial_device *dev = default_serial_console ();

		dev->puts (s);
		return;
	}

	serial_current->puts (s);
}

int serial_init(void)
{
	if (!serial_current) {
		struct serial_device *dev = default_serial_console ();

		return dev->init ();
	}

	return serial_current->init ();
}
