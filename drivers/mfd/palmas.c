/*
 * TI Palmas MFD Driver
 *
 * Copyright 2011-2012 Texas Instruments Inc.
 *
 * Author: Graeme Gregory <gg@slimlogic.co.uk>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under  the terms of the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the License, or (at your
 *  option) any later version.
 *
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/regmap.h>
#include <linux/err.h>
#include <linux/mfd/core.h>
#include <linux/mfd/palmas.h>
#include <linux/of_platform.h>

enum palmas_ids {
	PALMAS_PMIC_ID,
	PALMAS_GPIO_ID,
	PALMAS_LEDS_ID,
	PALMAS_WDT_ID,
	PALMAS_RTC_ID,
	PALMAS_PWRBUTTON_ID,
	PALMAS_GPADC_ID,
	PALMAS_RESOURCE_ID,
	PALMAS_CLK_ID,
	PALMAS_PWM_ID,
	PALMAS_USB_ID,
	PALMAS_BQCHARGER_ID,
};

static const struct mfd_cell palmas_children[] = {
	{
		.name = "palmas-pmic",
		.id = PALMAS_PMIC_ID,
	},
	{
		.name = "palmas-gpio",
		.id = PALMAS_GPIO_ID,
	},
	{
		.name = "palmas-leds",
		.id = PALMAS_LEDS_ID,
	},
	{
		.name = "palmas-wdt",
		.id = PALMAS_WDT_ID,
	},
	{
		.name = "palmas-rtc",
		.id = PALMAS_RTC_ID,
	},
	{
		.name = "palmas-pwrbutton",
		.id = PALMAS_PWRBUTTON_ID,
	},
	{
		.name = "palmas-gpadc",
		.id = PALMAS_GPADC_ID,
	},
	{
		.name = "palmas-resource",
		.id = PALMAS_RESOURCE_ID,
	},
	{
		.name = "palmas-clk",
		.id = PALMAS_CLK_ID,
	},
	{
		.name = "palmas-pwm",
		.id = PALMAS_PWM_ID,
	},
	{
		.name = "palmas-usb",
		.id = PALMAS_USB_ID,
	}
};

static const struct mfd_cell palmas_charger_children[] = {
	{
		.name = "palmas-bqcharger",
		.id = PALMAS_BQCHARGER_ID,
	},
};

static const struct regmap_config
	palmas_regmap_config[PALMAS_MAX_NUM_CLIENTS] = {
	{
		.reg_bits = 8,
		.val_bits = 8,
		.max_register = PALMAS_BASE_TO_REG(PALMAS_PU_PD_OD_BASE,
					PALMAS_PRIMARY_SECONDARY_PAD3),
	},
	{
		.reg_bits = 8,
		.val_bits = 8,
		.max_register = PALMAS_BASE_TO_REG(PALMAS_GPADC_BASE,
					PALMAS_GPADC_SMPS_VSEL_MONITORING),
	},
	{
		.reg_bits = 8,
		.val_bits = 8,
		.max_register = PALMAS_BASE_TO_REG(PALMAS_TRIM_GPADC_BASE,
					PALMAS_GPADC_TRIM16),
	},
	{
		.reg_bits = 8,
		.val_bits = 8,
		.max_register = PALMAS_BASE_TO_REG(PALMAS_BQ24192_BASE,
					PALMAS_REG10),
	},
	{
		.reg_bits = 8,
		.val_bits = 8,
		.max_register = 0xff,
	},
};

static const int palmas_i2c_ids[PALMAS_MAX_NUM_CLIENTS] =
{
	0x48,
	0x49,
	0x4a,
	0x6a,
	0x22
};

static void __devinit palmas_dt_to_pdata(struct device_node *node,
		struct palmas_platform_data *pdata)
{
	int ret;
	u32 prop;

	ret = of_property_read_u32(node, "ti,mux_pad1", &prop);
	if (!ret) {
		pdata->mux_from_pdata = 1;
		pdata->pad1 = prop;
	}

	ret = of_property_read_u32(node, "ti,mux_pad2", &prop);
	if (!ret) {
		pdata->mux_from_pdata = 1;
		pdata->pad2 = prop;
	}

	ret = of_property_read_u32(node, "ti,mux_pad3", &prop);
	if (!ret) {
		pdata->mux_from_pdata = 1;
		pdata->pad3 = prop;
	}

	ret = of_property_read_u32(node, "ti,mux_pad4", &prop);
	if (!ret) {
		pdata->mux_from_pdata = 1;
		pdata->pad4 = prop;
	}
}

static int __devinit palmas_i2c_regmap(struct palmas *palmas, int i2c_start,
	int i2c_end)
{
	int ret, i;

	if (i2c_end > PALMAS_MAX_NUM_CLIENTS)
		return -EINVAL;

	for (i = i2c_start; i < i2c_end; i++) {
		if (i != 0) {
			palmas->i2c_clients[i] =
				i2c_new_dummy(palmas->i2c_clients[0]->adapter,
							palmas_i2c_ids[i]);
			if (!palmas->i2c_clients[i]) {
				dev_err(palmas->dev,
					"can't attach client %d\n", i);
				ret = -ENOMEM;
				goto err;
			}
		}
		palmas->regmap[i] = devm_regmap_init_i2c(palmas->i2c_clients[i],
				&palmas_regmap_config[i]);
		if (IS_ERR(palmas->regmap[i])) {
			ret = PTR_ERR(palmas->regmap[i]);
			dev_err(palmas->dev,
				"Failed to allocate regmap %d, err: %d\n",
				i, ret);
			goto err;
		}
	}

	return 0;

err:
	return ret;
}

static int __devinit palmas_i2c_probe(struct i2c_client *i2c,
			    const struct i2c_device_id *id)
{
	struct palmas *palmas;
	struct palmas_platform_data *pdata;
	struct device_node *node = i2c->dev.of_node;
	int ret = 0;
	unsigned int reg, addr;
	int slave;
	struct mfd_cell *children;

	pdata = dev_get_platdata(&i2c->dev);

	if (node && !pdata) {
		pdata = devm_kzalloc(&i2c->dev, sizeof(*pdata), GFP_KERNEL);

		if (!pdata)
			return -ENOMEM;

		palmas_dt_to_pdata(node, pdata);
	}

	if (!pdata)
		return -EINVAL;

	palmas = devm_kzalloc(&i2c->dev, sizeof(struct palmas), GFP_KERNEL);
	if (palmas == NULL)
		return -ENOMEM;

	i2c_set_clientdata(i2c, palmas);
	palmas->dev = &i2c->dev;
	palmas->irq = i2c->irq;

	palmas->i2c_clients[0] = i2c;

	ret = palmas_i2c_regmap(palmas, 0, PALMAS_BASE_NUM_CLIENTS);
	if (ret < 0)
		goto err;

	/* Read varient info from the device */
	slave = PALMAS_BASE_TO_SLAVE(PALMAS_ID_BASE);
	addr = PALMAS_BASE_TO_REG(PALMAS_ID_BASE, PALMAS_PRODUCT_ID_LSB);
	ret = regmap_read(palmas->regmap[slave], addr, &reg);
	if (ret < 0) {
		dev_err(palmas->dev, "Unable to read ID err: %d\n", ret);
		goto err;
	}

	palmas->id = reg;

	slave = PALMAS_BASE_TO_SLAVE(PALMAS_ID_BASE);
	addr = PALMAS_BASE_TO_REG(PALMAS_ID_BASE, PALMAS_PRODUCT_ID_MSB);
	ret = regmap_read(palmas->regmap[slave], addr, &reg);
	if (ret < 0) {
		dev_err(palmas->dev, "Unable to read ID err: %d\n", ret);
		goto err;
	}

	palmas->id |= reg << 8;

	dev_info(palmas->dev, "Product ID %x\n", palmas->id);

	slave = PALMAS_BASE_TO_SLAVE(PALMAS_DESIGNREV_BASE);
	addr = PALMAS_BASE_TO_REG(PALMAS_DESIGNREV_BASE, PALMAS_DESIGNREV);
	ret = regmap_read(palmas->regmap[slave], addr, &reg);
	if (ret < 0) {
		dev_err(palmas->dev, "Unable to read DESIGNREV err: %d\n", ret);
		goto err;
	}

	palmas->designrev = reg & PALMAS_DESIGNREV_DESIGNREV_MASK;

	dev_info(palmas->dev, "Product Design Rev %x\n", palmas->designrev);

	slave = PALMAS_BASE_TO_SLAVE(PALMAS_PMU_CONTROL_BASE);
	addr = PALMAS_BASE_TO_REG(PALMAS_PMU_CONTROL_BASE, PALMAS_SW_REVISION);
	ret = regmap_read(palmas->regmap[slave], addr, &reg);
	if (ret < 0) {
		dev_err(palmas->dev, "Unable to read SW_REVISION err: %d\n",
				ret);
		goto err;
	}

	palmas->sw_revision = reg;

	dev_info(palmas->dev, "Product SW Rev %x\n", palmas->sw_revision);

	if (is_palmas_charger(palmas->id)) {
		ret = palmas_i2c_regmap(palmas, PALMAS_BASE_NUM_CLIENTS,
					PALMAS_CHARGER_NUM_CLIENTS);
		if (ret < 0)
			goto err;
	}

	/* Change IRQ into clear on read mode for efficiency */
	slave = PALMAS_BASE_TO_SLAVE(PALMAS_INTERRUPT_BASE);
	addr = PALMAS_BASE_TO_REG(PALMAS_INTERRUPT_BASE, PALMAS_INT_CTRL);
	reg = PALMAS_INT_CTRL_INT_CLEAR;

	regmap_write(palmas->regmap[slave], addr, reg);

	ret = palmas_irq_init(palmas);
	if (ret < 0)
		goto err;

	slave = PALMAS_BASE_TO_SLAVE(PALMAS_PU_PD_OD_BASE);
	addr = PALMAS_BASE_TO_REG(PALMAS_PU_PD_OD_BASE,
			PALMAS_PRIMARY_SECONDARY_PAD1);

	if (pdata->mux_from_pdata) {
		reg = pdata->pad1;
		ret = regmap_write(palmas->regmap[slave], addr, reg);
		if (ret)
			goto err_irq;
	} else {
		ret = regmap_read(palmas->regmap[slave], addr, &reg);
		if (ret)
			goto err_irq;
	}

	if (!(reg & PALMAS_PRIMARY_SECONDARY_PAD1_GPIO_0))
		palmas->gpio_muxed |= PALMAS_GPIO_0_MUXED;
	if (!(reg & PALMAS_PRIMARY_SECONDARY_PAD1_GPIO_1_MASK))
		palmas->gpio_muxed |= PALMAS_GPIO_1_MUXED;
	else if ((reg & PALMAS_PRIMARY_SECONDARY_PAD1_GPIO_1_MASK) ==
			(2 << PALMAS_PRIMARY_SECONDARY_PAD1_GPIO_1_SHIFT))
		palmas->led_muxed |= PALMAS_LED1_MUXED;
	else if ((reg & PALMAS_PRIMARY_SECONDARY_PAD1_GPIO_1_MASK) ==
			(3 << PALMAS_PRIMARY_SECONDARY_PAD1_GPIO_1_SHIFT))
		palmas->pwm_muxed |= PALMAS_PWM1_MUXED;
	if (!(reg & PALMAS_PRIMARY_SECONDARY_PAD1_GPIO_2_MASK))
		palmas->gpio_muxed |= PALMAS_GPIO_2_MUXED;
	else if ((reg & PALMAS_PRIMARY_SECONDARY_PAD1_GPIO_2_MASK) ==
			(2 << PALMAS_PRIMARY_SECONDARY_PAD1_GPIO_2_SHIFT))
		palmas->led_muxed |= PALMAS_LED2_MUXED;
	else if ((reg & PALMAS_PRIMARY_SECONDARY_PAD1_GPIO_2_MASK) ==
			(3 << PALMAS_PRIMARY_SECONDARY_PAD1_GPIO_2_SHIFT))
		palmas->pwm_muxed |= PALMAS_PWM2_MUXED;
	if (!(reg & PALMAS_PRIMARY_SECONDARY_PAD1_GPIO_3))
		palmas->gpio_muxed |= PALMAS_GPIO_3_MUXED;

	addr = PALMAS_BASE_TO_REG(PALMAS_PU_PD_OD_BASE,
			PALMAS_PRIMARY_SECONDARY_PAD2);

	if (pdata->mux_from_pdata) {
		reg = pdata->pad2;
		ret = regmap_write(palmas->regmap[slave], addr, reg);
		if (ret)
			goto err_irq;
	} else {
		ret = regmap_read(palmas->regmap[slave], addr, &reg);
		if (ret)
			goto err_irq;
	}

	if (is_palmas_charger(palmas->id)) {
		if (!(reg & PALMAS_PRIMARY_SECONDARY_PAD2_GPIO_4))
			palmas->gpio_muxed |= PALMAS_GPIO_4_MUXED;
	} else {
		if (!(reg & (PALMASCH_PRIMARY_SECONDARY_PAD2_GPIO_4_LSB |
				PALMASCH_PRIMARY_SECONDARY_PAD2_GPIO_4_MSB)))
			palmas->gpio_muxed |= PALMAS_GPIO_4_MUXED;
	}
	if (!(reg & PALMAS_PRIMARY_SECONDARY_PAD2_GPIO_5_MASK))
		palmas->gpio_muxed |= PALMAS_GPIO_5_MUXED;
	if (!(reg & PALMAS_PRIMARY_SECONDARY_PAD2_GPIO_6))
		palmas->gpio_muxed |= PALMAS_GPIO_6_MUXED;
	if (!(reg & PALMAS_PRIMARY_SECONDARY_PAD2_GPIO_7_MASK))
		palmas->gpio_muxed |= PALMAS_GPIO_7_MUXED;

	if (is_palmas_charger(palmas->id)) {
		addr = PALMAS_BASE_TO_REG(PALMAS_PU_PD_OD_BASE,
				PALMAS_PRIMARY_SECONDARY_PAD3);

		if (pdata->mux_from_pdata) {
			reg = pdata->pad3;
			ret = regmap_write(palmas->regmap[slave], addr, reg);
			if (ret)
				goto err_irq;
		}

		addr = PALMAS_BASE_TO_REG(PALMAS_PU_PD_OD_BASE,
				PALMAS_PRIMARY_SECONDARY_PAD4);

		if (pdata->mux_from_pdata) {
			reg = pdata->pad4;
			ret = regmap_write(palmas->regmap[slave], addr, reg);
			if (ret)
				goto err_irq;
		} else {
			ret = regmap_read(palmas->regmap[slave], addr, &reg);
			if (ret)
				goto err_irq;
		}

		if (!(reg & PALMAS_PRIMARY_SECONDARY_PAD4_GPIO_8))
			palmas->gpio_muxed |= PALMAS_GPIO_8_MUXED;
		if (!(reg & PALMAS_PRIMARY_SECONDARY_PAD4_GPIO_9))
			palmas->gpio_muxed |= PALMAS_GPIO_9_MUXED;
		if (!(reg & PALMAS_PRIMARY_SECONDARY_PAD4_GPIO_10))
			palmas->gpio_muxed |= PALMAS_GPIO_10_MUXED;
		if (!(reg & PALMAS_PRIMARY_SECONDARY_PAD4_GPIO_11))
			palmas->gpio_muxed |= PALMAS_GPIO_11_MUXED;
		if (!(reg & PALMAS_PRIMARY_SECONDARY_PAD4_GPIO_12))
			palmas->gpio_muxed |= PALMAS_GPIO_12_MUXED;
		if (!(reg & PALMAS_PRIMARY_SECONDARY_PAD4_GPIO_14))
			palmas->gpio_muxed |= PALMAS_GPIO_14_MUXED;
		if (!(reg & PALMAS_PRIMARY_SECONDARY_PAD4_GPIO_15))
			palmas->gpio_muxed |= PALMAS_GPIO_15_MUXED;
	}

	dev_info(palmas->dev, "Muxing GPIO %x, PWM %x, LED %x\n",
			palmas->gpio_muxed, palmas->pwm_muxed,
			palmas->led_muxed);

	/*
	 * If we are probing with DT do this the DT way and return here
	 * otherwise continue and add devices using mfd helpers.
	 */
	if (node) {
		ret = of_platform_populate(node, NULL, NULL, &i2c->dev);
		if (ret < 0)
			goto err_irq;
		else
			return ret;
	}

	children = kmemdup(palmas_children, sizeof(palmas_children),
			   GFP_KERNEL);
	if (!children) {
		ret = -ENOMEM;
		goto err_irq;
	}

	children[PALMAS_PMIC_ID].platform_data = pdata->pmic_pdata;
	children[PALMAS_PMIC_ID].pdata_size = sizeof(*pdata->pmic_pdata);

	children[PALMAS_GPADC_ID].platform_data = pdata->gpadc_pdata;
	children[PALMAS_GPADC_ID].pdata_size = sizeof(*pdata->gpadc_pdata);

	children[PALMAS_RESOURCE_ID].platform_data = pdata->resource_pdata;
	children[PALMAS_RESOURCE_ID].pdata_size =
			sizeof(*pdata->resource_pdata);

	children[PALMAS_USB_ID].platform_data = pdata->usb_pdata;
	children[PALMAS_USB_ID].pdata_size = sizeof(*pdata->usb_pdata);

	children[PALMAS_CLK_ID].platform_data = pdata->clk_pdata;
	children[PALMAS_CLK_ID].pdata_size = sizeof(*pdata->clk_pdata);

	children[PALMAS_LEDS_ID].platform_data = pdata->leds_pdata;
	children[PALMAS_LEDS_ID].pdata_size = sizeof(*pdata->leds_pdata);

	ret = mfd_add_devices(palmas->dev, -1,
			      children, ARRAY_SIZE(palmas_children),
			      NULL, 0, NULL);
	kfree(children);

	if (ret < 0)
		goto err_devices;

	switch(palmas->id) {
	case PALMAS_CHIP_OLD_ID:
	case PALMAS_CHIP_ID:
		/* no extra children at this point */
		break;
	case PALMAS_CHIP_CHARGER_ID:
		children = kmemdup(palmas_charger_children,
				sizeof(palmas_charger_children),
				GFP_KERNEL);
		if (!children) {
			ret = -ENOMEM;
			goto err_irq;
		}

		children->platform_data = pdata->charger_pdata;
		children->pdata_size = sizeof(*pdata->charger_pdata);

		ret = mfd_add_devices(palmas->dev, -1, children,
				ARRAY_SIZE(palmas_charger_children),
				NULL, 0, NULL);
		kfree (children);

		if (ret < 0)
			goto err_devices;
		break;
	default:
		dev_info(palmas->dev, "Unknown Device ID for Subdevices\n");
	}

	return ret;

err_devices:
	mfd_remove_devices(palmas->dev);
err_irq:
	palmas_irq_exit(palmas);
err:
	return ret;
}

static int palmas_i2c_remove(struct i2c_client *i2c)
{
	struct palmas *palmas = i2c_get_clientdata(i2c);

	mfd_remove_devices(palmas->dev);
	palmas_irq_exit(palmas);

	return 0;
}

static const struct i2c_device_id palmas_i2c_id[] = {
	{ "palmas", },
	{ "twl6035", },
	{ "twl6037", },
	{ "tps65913", },
	{ "palmas-charger", },
	{ /* end */ }
};
MODULE_DEVICE_TABLE(i2c, palmas_i2c_id);

static struct of_device_id __devinitdata of_palmas_match_tbl[] = {
	{ .compatible = "ti,palmas", },
	{ /* end */ }
};

static struct i2c_driver palmas_i2c_driver = {
	.driver = {
		   .name = "palmas",
		   .of_match_table = of_palmas_match_tbl,
		   .owner = THIS_MODULE,
	},
	.probe = palmas_i2c_probe,
	.remove = palmas_i2c_remove,
	.id_table = palmas_i2c_id,
};

static int __init palmas_i2c_init(void)
{
	return i2c_add_driver(&palmas_i2c_driver);
}
/* init early so consumer devices can complete system boot */
subsys_initcall(palmas_i2c_init);

static void __exit palmas_i2c_exit(void)
{
	i2c_del_driver(&palmas_i2c_driver);
}
module_exit(palmas_i2c_exit);

MODULE_AUTHOR("Graeme Gregory <gg@slimlogic.co.uk>");
MODULE_DESCRIPTION("Palmas chip family multi-function driver");
MODULE_LICENSE("GPL");
