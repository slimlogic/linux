/*
 * tps65910-gpio.c  --  TI TPS6591x
 *
 * Copyright 2010 Texas Instruments Inc.
 *
 * Author: Liam Girdwood <lrg@slimlogic.co.uk>
 * Author: Graeme Gregory <gg@slimlogic.co.uk>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under  the terms of the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the License, or (at your
 *  option) any later version.
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/mfd/tps65910.h>

static int tps6591x_gpio_get(struct gpio_chip *gc, unsigned offset)
{
	struct tps65910 *tps65910 = container_of(gc, struct tps65910, gpio);
	uint8_t val;

	tps65910->read(tps65910, TPS65910_GPIO0 + offset, 1, &val);

	if (val & GPIO_STS_MASK)
		return 1;

	return 0;
}

static void tps6591x_gpio_set(struct gpio_chip *gc, unsigned offset,
			      int value)
{
	struct tps65910 *tps65910 = container_of(gc, struct tps65910, gpio);
	uint8_t val;

	tps65910->read(tps65910, TPS65910_GPIO0 + offset, 1, &val);

	pr_err("%s: val - 0x%x  offset - %d\n", __func__, val, offset);

	if (value)
		val |= GPIO_SET_MASK;
	else
		val &= ~GPIO_SET_MASK;

	tps65910->write(tps65910, TPS65910_GPIO0 + offset, 1, &val);

	pr_err("%s: val - 0x%x\n", __func__, val);
}

static int tps6591x_gpio_output(struct gpio_chip *gc, unsigned offset,
				int value)
{
	struct tps65910 *tps65910 = container_of(gc, struct tps65910, gpio);
	uint8_t val;

	tps65910->read(tps65910, TPS65910_GPIO0 + offset, 1, &val);

	val |= GPIO_CFG_MASK;

	/* Disable GPIO pad pulldown in tps65911 */
	if (tps_chip() == TPS65911)
		val &= ~GPIO_PUEN_MASK;

	/* Set the initial value */
	tps6591x_gpio_set(gc, offset, value);

	return tps65910->write(tps65910, TPS65910_GPIO0 + offset, 1, &val);
}

static int tps6591x_gpio_input(struct gpio_chip *gc, unsigned offset)
{
	struct tps65910 *tps65910 = container_of(gc, struct tps65910, gpio);
	uint8_t val;

	tps65910->read(tps65910, TPS65910_GPIO0 + offset, 1, &val);

	val &= ~GPIO_CFG_MASK;

	return tps65910->write(tps65910, TPS65910_GPIO0 + offset, 1, &val);
}

void tps6591x_gpio_init(struct tps65910 *tps65910, int gpio_base)
{
	int ret;

	if (!gpio_base)
		return;

	tps65910->gpio.owner		= THIS_MODULE;
	tps65910->gpio.label		= tps65910->i2c_client->name;
	tps65910->gpio.dev		= tps65910->dev;
	tps65910->gpio.base		= gpio_base;
	if (tps_chip() == TPS65910)
		tps65910->gpio.ngpio		= 6;
	else if (tps_chip() == TPS65911)
		tps65910->gpio.ngpio		= 9;
	else
		tps65910->gpio.ngpio		= 0;
	tps65910->gpio.can_sleep	= 1;

	tps65910->gpio.direction_input	= tps6591x_gpio_input;
	tps65910->gpio.direction_output	= tps6591x_gpio_output;
	tps65910->gpio.set		= tps6591x_gpio_set;
	tps65910->gpio.get		= tps6591x_gpio_get;

	ret = gpiochip_add(&tps65910->gpio);

	if (ret)
		dev_warn(tps65910->dev, "GPIO registration failed: %d\n", ret);
}
