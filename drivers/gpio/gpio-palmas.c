/*
 * gpiolib support for Palmas Series PMICS
 *
 * Copyright 2011 Texas Instruments
 *
 * Author: Graeme Gregory <gg@slimlogic.co.uk>
 *
 * Based on gpio-wm831x.c
 *
 * Copyright 2009 Wolfson Microelectronics PLC.
 *
 * Author: Mark Brown <broonie@opensource.wolfsonmicro.com>
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 */

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/mfd/core.h>
#include <linux/platform_device.h>
#include <linux/seq_file.h>
#include <linux/regmap.h>
#include <linux/mfd/palmas.h>

struct palmas_gpio {
	struct palmas *palmas;
	struct gpio_chip gpio_chip;
};

#define GPIO_SLAVE		1

static int palmas_gpio_read(struct palmas *palmas, unsigned int reg,
		int gpio, unsigned int *dest)
{
	unsigned int addr;

	/* registers for second bank are identical and offset by 0x9 */
	if (gpio > 7)
		reg += PALMAS_GPIO_DATA_IN2;

	addr = PALMAS_BASE_TO_REG(PALMAS_GPIO_BASE, reg);

	return regmap_read(palmas->regmap[GPIO_SLAVE], addr, dest);
}

static int palmas_gpio_update_bits(struct palmas *palmas, unsigned int reg,
		int gpio, unsigned int mask, unsigned int data)
{
	unsigned int addr;

	/* registers for second bank are identical and offset by 0x9 */
	if (gpio > 7)
		reg += PALMAS_GPIO_DATA_IN2;

	addr = PALMAS_BASE_TO_REG(PALMAS_GPIO_BASE, reg);

	return regmap_update_bits(palmas->regmap[GPIO_SLAVE], addr, mask, data);
}

static struct palmas_gpio *to_palmas_gpio(struct gpio_chip *chip)
{
	return container_of(chip, struct palmas_gpio, gpio_chip);
}

static int palmas_gpio_direction_in(struct gpio_chip *chip, unsigned offset)
{
	struct palmas_gpio *palmas_gpio = to_palmas_gpio(chip);
	struct palmas *palmas = palmas_gpio->palmas;

	return palmas_gpio_update_bits(palmas, PALMAS_GPIO_DATA_DIR,
			offset, 1 << (offset % 8), 0);
}

static int palmas_gpio_get(struct gpio_chip *chip, unsigned offset)
{
	struct palmas_gpio *palmas_gpio = to_palmas_gpio(chip);
	struct palmas *palmas = palmas_gpio->palmas;
	unsigned int reg = 0;

	palmas_gpio_read(palmas, PALMAS_GPIO_DATA_IN, offset, &reg);

	return !!(reg & (1 << (offset % 8)));
}

static void palmas_gpio_set(struct gpio_chip *chip, unsigned offset, int value)
{
	struct palmas_gpio *palmas_gpio = to_palmas_gpio(chip);
	struct palmas *palmas = palmas_gpio->palmas;

	palmas_gpio_update_bits(palmas, PALMAS_GPIO_DATA_OUT,
			offset, 1 << (offset % 8), value << (offset % 8));
}

static int palmas_gpio_direction_out(struct gpio_chip *chip,
				     unsigned offset, int value)
{
	struct palmas_gpio *palmas_gpio = to_palmas_gpio(chip);
	struct palmas *palmas = palmas_gpio->palmas;

	return palmas_gpio_update_bits(palmas, PALMAS_GPIO_DATA_DIR,
			offset, 1 << (offset % 8), 0xFF);
}

static int palmas_gpio_to_irq(struct gpio_chip *chip, unsigned offset)
{
	struct palmas_gpio *palmas_gpio = to_palmas_gpio(chip);
	struct palmas *palmas = palmas_gpio->palmas;

	return palmas_map_irq(palmas, offset + PALMAS_GPIO_0_IRQ);
}

static int palmas_gpio_set_debounce(struct gpio_chip *chip, unsigned offset,
				    unsigned debounce)
{
	struct palmas_gpio *palmas_gpio = to_palmas_gpio(chip);
	struct palmas *palmas = palmas_gpio->palmas;
	unsigned int data = 0;

	if (debounce)
		data = 0xff;

	return palmas_gpio_update_bits(palmas, PALMAS_GPIO_DEBOUNCE_EN,
			offset, 1 << (offset % 8), data);
}

static const struct gpio_chip template_chip = {
	.label			= "palmas",
	.owner			= THIS_MODULE,
	.direction_input	= palmas_gpio_direction_in,
	.get			= palmas_gpio_get,
	.direction_output	= palmas_gpio_direction_out,
	.set			= palmas_gpio_set,
	.to_irq			= palmas_gpio_to_irq,
	.set_debounce		= palmas_gpio_set_debounce,
	.can_sleep		= 1,
	.ngpio			= 8,
};

static int __devinit palmas_gpio_probe(struct platform_device *pdev)
{
	struct palmas *palmas = dev_get_drvdata(pdev->dev.parent);
	struct palmas_platform_data *pdata = palmas->dev->platform_data;
	struct palmas_gpio *gpio;
	int ret;

	gpio = devm_kzalloc(&pdev->dev, sizeof(*gpio), GFP_KERNEL);
	if (!gpio)
		return -ENOMEM;

	gpio->palmas = palmas;
	gpio->gpio_chip.dev = &pdev->dev;

	memcpy(&gpio->gpio_chip, &template_chip, sizeof(template_chip));

	/* palmas charger has 16 gpios */
	if (is_palmas_charger(palmas->id))
		gpio->gpio_chip.ngpio = 16;

	if (pdata && pdata->gpio_base)
		gpio->gpio_chip.base = pdata->gpio_base;
	else
		gpio->gpio_chip.base = -1;

	ret = gpiochip_add(&gpio->gpio_chip);
	if (ret < 0) {
		dev_err(&pdev->dev, "Could not register gpiochip, %d\n",
			ret);
		return ret;
	}

	platform_set_drvdata(pdev, gpio);

	return ret;
}

static int __devexit palmas_gpio_remove(struct platform_device *pdev)
{
	struct palmas_gpio *gpio = platform_get_drvdata(pdev);

	return gpiochip_remove(&gpio->gpio_chip);
}

static struct of_device_id __devinitdata of_palmas_match_tbl[] = {
	{ .compatible = "ti,palmas-gpio", },
	{ /* end */ }
};

static struct platform_driver palmas_gpio_driver = {
	.driver = {
		.name = "palmas-gpio",
		.of_match_table = of_palmas_match_tbl,
		.owner = THIS_MODULE,
	},
	.probe = palmas_gpio_probe,
	.remove = __devexit_p(palmas_gpio_remove),
};

static int __init palmas_gpio_init(void)
{
	return platform_driver_register(&palmas_gpio_driver);
}
subsys_initcall(palmas_gpio_init);

static void __exit palmas_gpio_exit(void)
{
	platform_driver_unregister(&palmas_gpio_driver);
}
module_exit(palmas_gpio_exit);

MODULE_AUTHOR("Graeme Gregory <gg@slimlogic.co.uk>");
MODULE_DESCRIPTION("GPIO interface for the Palmas series chips");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:palmas-gpio");
MODULE_DEVICE_TABLE(of, of_palmas_match_tbl);

