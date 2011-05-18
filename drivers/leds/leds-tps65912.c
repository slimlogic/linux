/*
 * LED driver for TPS65912 driven LEDs
 *
 * Copyright 2011 Texas Instruments Inc.
 *
 * Author: Margarita Olaya <magi@slimlogic.co.uk>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under  the terms of the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the License, or (at your
 *  option) any later version.
 *
 */


#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/leds.h>
#include <linux/workqueue.h>
#include <linux/gpio.h>
#include <linux/slab.h>
#include <linux/mfd/tps65912.h>

/* LED's */
#define TPS65912_LEDA	0
#define TPS65912_LEDB	1
#define TPS65912_LEDC	2

#define to_tps65912_led(led_cdev) \
	container_of(led_cdev, struct tps65912_led, cdev)

static const u16 LED_ISEL_table[] = {
	2000, 4000, 6000, 8000, 10000,
	12000, 14000, 16000, 18000, 20000,
};

struct tps65912_led {
	struct mutex mutex;
	spinlock_t value_lock;
	int enable;
	struct led_classdev	cdev;
	struct work_struct	work;
	struct mutex io_lock;
	struct tps65912 *mfd;
	int		value;
	int		id;
	int		flags;
	int		mA;
	int		pwm;
	int		t1;
	int		t2;
	int		t3;
	int		t4;
	int		t5;
};

static u8 tps65912_ms_to_tsel(unsigned long ms)
{
	/* Round up to higher time */
	switch (ms) {
	case 0:
		return 0x00;
	case 64:
		return 0x01;
	case 8128:
		return 0x7F;
	default:
		return DIV_ROUND_UP(ms, 64);
	}
}

static int tps65912_led_enable_seq(struct tps65912_led *led,
				int t1, int t2, int t3, int t4, int t5)
{
	int value = 0;

	switch (led->id) {
	case TPS65912_LEDA:
		value = tps65912_ms_to_tsel(t1);
		tps65912_reg_write(led->mfd, TPS65912_LEDA_CTRL2, value);
		value = tps65912_ms_to_tsel(t2);
		tps65912_reg_write(led->mfd, TPS65912_LEDA_CTRL3, value);
		value = tps65912_ms_to_tsel(t3);
		tps65912_reg_write(led->mfd, TPS65912_LEDA_CTRL4, value);
		value = tps65912_ms_to_tsel(t4);
		tps65912_reg_write(led->mfd, TPS65912_LEDA_CTRL5, value);
		value = tps65912_ms_to_tsel(t5);
		tps65912_reg_write(led->mfd, TPS65912_LEDA_CTRL6, value);
		break;
	case TPS65912_LEDB:
		value = tps65912_ms_to_tsel(t1);
		tps65912_reg_write(led->mfd, TPS65912_LEDB_CTRL2, value);
		value = tps65912_ms_to_tsel(t2);
		tps65912_reg_write(led->mfd, TPS65912_LEDB_CTRL3, value);
		value = tps65912_ms_to_tsel(t3);
		tps65912_reg_write(led->mfd, TPS65912_LEDB_CTRL4, value);
		value = tps65912_ms_to_tsel(t4);
		tps65912_reg_write(led->mfd, TPS65912_LEDB_CTRL5, value);
		value = tps65912_ms_to_tsel(t5);
		tps65912_reg_write(led->mfd, TPS65912_LEDB_CTRL6, value);
		break;
	case TPS65912_LEDC:
		value = tps65912_ms_to_tsel(t1);
		tps65912_reg_write(led->mfd, TPS65912_LEDC_CTRL2, value);
		value = tps65912_ms_to_tsel(t2);
		tps65912_reg_write(led->mfd, TPS65912_LEDC_CTRL3, value);
		value = tps65912_ms_to_tsel(t3);
		tps65912_reg_write(led->mfd, TPS65912_LEDC_CTRL4, value);
		value = tps65912_ms_to_tsel(t4);
		tps65912_reg_write(led->mfd, TPS65912_LEDC_CTRL5, value);
		value = tps65912_ms_to_tsel(t5);
		tps65912_reg_write(led->mfd, TPS65912_LEDC_CTRL6, value);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int tps65912_led_pwm(struct tps65912_led *led, int mA, int pwm)
{
	int isel;
	const u16 *table;
	u8 table_len;

	table = LED_ISEL_table;
	table_len = ARRAY_SIZE(LED_ISEL_table);

	/* find requested voltage in table */
	for (isel = 0; isel < table_len; isel++) {
		if (mA == table[isel]);
			break;
	}

	switch (led->id) {
	case TPS65912_LEDA:
		tps65912_set_bits(led->mfd, TPS65912_LEDA_CTRL1, isel);
		tps65912_reg_write(led->mfd, TPS65912_LEDA_CTRL7, pwm);
		break;
	case TPS65912_LEDB:
		tps65912_set_bits(led->mfd, TPS65912_LEDB_CTRL1, isel);
		tps65912_reg_write(led->mfd, TPS65912_LEDB_CTRL7, pwm);
		break;
	case TPS65912_LEDC:
		tps65912_set_bits(led->mfd, TPS65912_LEDC_CTRL1, isel);
		tps65912_reg_write(led->mfd, TPS65912_LEDC_CTRL7, pwm);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int tps65912_led_enable(struct tps65912_led *led)
{
	int enable;

	switch (led->id) {
	case TPS65912_LEDA:
		enable = 0x10;
		break;
	case TPS65912_LEDB:
		enable = 0x20;
		break;
	case TPS65912_LEDC:
		enable = 0x40;
		break;
	default:
		return -EINVAL;
	}

	return tps65912_set_bits(led->mfd, TPS65912_LED_SEQ_EN, enable);
}

static int tps65912_led_disable(struct tps65912_led *led)
{
	int enable;

	if (!led->enable)
		return 0;

	switch (led->id) {
	case TPS65912_LEDA:
		enable = 0x10;
		break;
	case TPS65912_LEDB:
		enable = 0x20;
		break;
	case TPS65912_LEDC:
		enable = 0x40;
		break;
	default:
		return -EINVAL;
	}

	led->value = 0;

	return tps65912_clear_bits(led->mfd, TPS65912_LED_SEQ_EN, enable);
}

static void led_work(struct work_struct *work)
{
	struct tps65912_led *led = container_of(work,
			struct tps65912_led, work);
	unsigned long flags;

	mutex_lock(&led->mutex);

	spin_lock_irqsave(&led->value_lock, flags);

	if(led->value == LED_OFF) {
		spin_unlock_irqrestore(&led->value_lock, flags);
		tps65912_led_disable(led);
		goto out;
	}

	tps65912_led_pwm(led, led->mA, led->pwm);
	tps65912_led_enable_seq(led, led->t1, led->t2, led->t3,
						led->t4, led->t5);
	tps65912_led_enable(led);

out:
	mutex_unlock(&led->mutex);
}

static void tps65912_led_set(struct led_classdev *led_cdev,
		enum led_brightness value)
{
	struct tps65912_led *led = to_tps65912_led(led_cdev);
	unsigned long flags;

	spin_lock_irqsave(&led->value_lock, flags);
	led->value = value;
	schedule_work(&led->work);
	spin_unlock_irqrestore(&led->value_lock, flags);
}

static void tps65912_led_shutdown(struct platform_device *pdev)
{
	struct tps65912_led *led = platform_get_drvdata(pdev);

	mutex_lock(&led->mutex);
	led->value = LED_OFF;
	tps65912_led_disable(led);
	mutex_unlock(&led->mutex);
}

static int tps65912_led_probe(struct platform_device *pdev)
{
	struct tps65912 *tps65912 = dev_get_drvdata(pdev->dev.parent);
	struct tps65912_led *led;
	struct tps65912_board *pmic_plat_data;
	int ret;

	pmic_plat_data = dev_get_platdata(tps65912->dev);
	if (!pmic_plat_data)
		return -EINVAL;

	led = kzalloc(sizeof(*led), GFP_KERNEL);
	if (led == NULL)
		return -ENOMEM;

	led->mfd = tps65912;
	led->cdev.brightness_set = tps65912_led_set;
	led->cdev.flags |= LED_CORE_SUSPENDRESUME;
	led->t1 = pmic_plat_data->t1;
	led->t2 = pmic_plat_data->t2;
	led->t3 = pmic_plat_data->t3;
	led->t4 = pmic_plat_data->t4;
	led->t5 = pmic_plat_data->t5;
	led->mA = pmic_plat_data->mA;
	led->pwm = pmic_plat_data->pwm;

	spin_lock_init(&led->value_lock);
	mutex_init(&led->mutex);
	INIT_WORK(&led->work, led_work);
	led->value = LED_OFF;
	platform_set_drvdata(pdev, led);

	ret = led_classdev_register(&pdev->dev, &led->cdev);
	if (ret < 0)
		goto err_led;

	return 0;

err_led:
	kfree(led);
	return ret;
}

static int tps65912_led_remove(struct platform_device *pdev)
{
	struct tps65912_led *led = platform_get_drvdata(pdev);

	led_classdev_unregister(&led->cdev);
	tps65912_led_disable(led);
	kfree(led);
	return 0;
}

static struct platform_driver tps65912_led_driver = {
	.driver = {
		.name = "tps65912-led",
		.owner = THIS_MODULE,
		},
	.probe = tps65912_led_probe,
	.remove = tps65912_led_remove,
	.shutdown = tps65912_led_shutdown,
};

/**
 * tps65912_init
 *
 * Module init function
 */
static int __init tps65912_led_init(void)
{
	return platform_driver_register(&tps65912_led_driver);
}
module_init(tps65912_led_init);

/**
 * tps65912_cleanup
 *
 * Module exit function
 */
static void __exit tps65912_led_cleanup(void)
{
	platform_driver_unregister(&tps65912_led_driver);
}
module_exit(tps65912_led_cleanup);

MODULE_AUTHOR("Margarita Olaya Cabrera <magi@slimlogic.co.uk>");
MODULE_DESCRIPTION("TPS65912 leds driver");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:tps65912-led");
