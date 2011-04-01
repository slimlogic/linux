/*
 * tps65911_wdt.c  --  TI TPS6591x WDT
 *
 * Copyright 2010 Texas Instruments Inc.
 *
 * Author: Jorge Eduardo Candelaria <jedu@slimlogic.co.uk>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under  the terms of the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the License, or (at your
 *  option) any later version.
 *
 */

#include <linux/module.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/watchdog.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>
#include <linux/mfd/tps65910.h>

#define WTCHDG_TIME_MAX			0x111
#define TPS65911_WDT_STATE_ACTIVE	1

static struct platform_device *tps65911_wdt_dev;

struct tps65911_wdt {
	struct miscdevice	miscdev;
	struct tps65910		*tps65910;
	unsigned long		state;
	int			timer_margin;
};

static int tps65911_wdt_enable(struct tps65911_wdt *wdt)
{
	struct tps65910 *tps65910 = wdt->tps65910;
	u8 val;
	int ret;

	ret = tps65910->read(tps65910, TPS65911_WATCHDOG, 1, &val);
	if (ret < 0)
		return ret;

	val |= WTCHDG_TIME_DEFAULT_ACTIVE & WTCHDG_TIME_MASK;

	ret = tps65910->write(tps65910, TPS65911_WATCHDOG, 1, &val);
	return ret;
}

static int tps65911_wdt_disable(struct tps65911_wdt *wdt)
{
	struct tps65910 *tps65910 = wdt->tps65910;
	int val, ret;

	ret = tps65910->read(tps65910, TPS65911_WATCHDOG, 1, &val);
	if (ret < 0)
		return ret;

	val |= WTCHDG_TIME_INACTIVE & WTCHDG_TIME_MASK;

	ret = tps65910->write(tps65910, TPS65911_WATCHDOG, 1, &val);
	return ret;
}

static int tps65911_wdt_set_timeout(struct tps65911_wdt *wdt, int timeout)
{
	struct tps65910 *tps65910 = wdt->tps65910;
	int val, ret;

	if (timeout > WTCHDG_TIME_MAX)
		return -EINVAL;

	ret = tps65910->read(tps65910, TPS65911_WATCHDOG, 1, &val);
	if (ret < 0)
		return ret;

	val |= timeout & WTCHDG_TIME_MASK;

	ret = tps65910->write(tps65910, TPS65911_WATCHDOG, 1, &val);
	return ret;
}

static int tps65911_wdt_keepalive(struct tps65911_wdt *wdt)
{
	struct tps65910 *tps65910 = wdt->tps65910;
	u8 val = WTCHDG_IT_MASK;

	return tps65910->write(tps65910, TPS65910_INT_STS3, 1, &val);
}

static ssize_t tps65911_wdt_write_fop(struct file *file,
		const char __user *data, size_t len, loff_t *ppos)
{
	struct tps65911_wdt *wdt = file->private_data;

	if (len)
		tps65911_wdt_keepalive(wdt);

	return len;
}

static long tps65911_wdt_ioctl(struct file *file,
		unsigned int cmd, unsigned long arg)
{
	void __user *argp = (void __user *)arg;
	int __user *p = argp;
	int new_margin;
	struct tps65911_wdt *wdt = file->private_data;

	static const struct watchdog_info tps65911_wd_ident = {
		.identity = "TPS65911 Watchdog",
		.options = WDIOF_SETTIMEOUT,
		.firmware_version = 0,
	};

	switch (cmd) {
	case WDIOC_GETSUPPORT:
		return copy_to_user(argp, &tps65911_wd_ident,
				sizeof(tps65911_wd_ident)) ? -EFAULT : 0;

	case WDIOC_GETSTATUS:
	case WDIOC_GETBOOTSTATUS:
		return put_user(0, p);

	case WDIOC_KEEPALIVE:
		return tps65911_wdt_keepalive(wdt);

	case WDIOC_SETTIMEOUT:
		if (get_user(new_margin, p))
			return -EFAULT;
		if (tps65911_wdt_set_timeout(wdt, new_margin))
			return -EINVAL;
		return put_user(wdt->timer_margin, p);

	case WDIOC_GETTIMEOUT:
		return put_user(wdt->timer_margin, p);

	default:
		return -ENOTTY;
	}

	return 0;
}

static int tps65911_wdt_open(struct inode *inode, struct file *file)
{
	struct tps65911_wdt *wdt = platform_get_drvdata(tps65911_wdt_dev);

	/* /dev/watchdog can only be opened once */
	if (test_and_set_bit(0, &wdt->state))
		return -EBUSY;

	wdt->state |= TPS65911_WDT_STATE_ACTIVE;
	file->private_data= (void *) wdt;

	tps65911_wdt_enable(wdt);
	return nonseekable_open(inode, file);
}

static int tps65911_wdt_release(struct inode *inode, struct file *file)
{
	struct tps65911_wdt *wdt = file->private_data;

	if (tps65911_wdt_disable(wdt))
		return -EFAULT;
	wdt->state &= ~TPS65911_WDT_STATE_ACTIVE;

	clear_bit(0, &wdt->state);
	return 0;
}

static const struct file_operations tps65911_wdt_fops = {
	.owner		= THIS_MODULE,
	.llseek		= no_llseek,
	.open		= tps65911_wdt_open,
	.release	= tps65911_wdt_release,
	.unlocked_ioctl	= tps65911_wdt_ioctl,
	.write		= tps65911_wdt_write_fop,
};

static int __devinit tps65911_wdt_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct tps65911_wdt *wdt;
	struct tps65910 *tps65910 = dev_get_platdata(&pdev->dev);

	wdt = kzalloc(sizeof(struct tps65911_wdt), GFP_KERNEL);
	if (!wdt)
		return -ENOMEM;

	wdt->tps65910		= tps65910;
	wdt->state		= 0;
	wdt->timer_margin	= 100;
	wdt->miscdev.parent	= &pdev->dev;
	wdt->miscdev.fops	= &tps65911_wdt_fops;
	wdt->miscdev.minor	= WATCHDOG_MINOR;
	wdt->miscdev.name	= "watchdog";

	platform_set_drvdata(pdev, wdt);

	tps65911_wdt_dev = pdev;

	tps65911_wdt_disable(wdt);

	ret = misc_register(&wdt->miscdev);
	if (ret) {
		dev_err(wdt->miscdev.parent,
			"Failed to register misc device\n");
		platform_set_drvdata(pdev, NULL);
		kfree(wdt);
		tps65911_wdt_dev = NULL;
		return ret;
	}

	return 0;
}

static int __devexit tps65911_wdt_remove(struct platform_device *pdev)
{
	struct tps65911_wdt *wdt = platform_get_drvdata(pdev);

	if (wdt->state & TPS65911_WDT_STATE_ACTIVE)
		if (tps65911_wdt_disable(wdt))
			return -EFAULT;

	wdt->state &= ~TPS65911_WDT_STATE_ACTIVE;
	misc_deregister(&wdt->miscdev);

	platform_set_drvdata(pdev, NULL);
	kfree(wdt);
	tps65911_wdt_dev = NULL;

	return 0;
}

static struct platform_driver tps65911_wdt_driver = {
	.probe		= tps65911_wdt_probe,
	.remove		= __devexit_p(tps65911_wdt_remove),
	.driver		= {
		.owner	= THIS_MODULE,
		.name	= "tps65911_wdt",
	},
};

static int __devinit tps65911_wdt_init(void)
{
	return platform_driver_register(&tps65911_wdt_driver);
}
module_init(tps65911_wdt_init);

static void __devexit tps65911_wdt_exit(void)
{
	platform_driver_unregister(&tps65911_wdt_driver);
}
module_exit(tps65911_wdt_exit);

MODULE_AUTHOR("Jorge Eduardo Candelaria");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:tps65911_wdt");
