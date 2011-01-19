/*
 * tps65910.h  --  TI TPS6591x
 *
 * Copyright 2010 Texas Instruments Inc.
 *
 * Author: Liam Girdwood <lrg@slimlogic.co.uk>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under  the terms of the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the License, or (at your
 *  option) any later version.
 *
 */

#ifndef __LINUX_MFD_TPS6591X_H
#define __LINUX_MFD_TPS6591X_H

#define TPS6591X_INT_STS			0x50
#define TPS6591X_INT_MSK			0x51
#define TPS6591X_INT_STS2			0x52
#define TPS6591X_INT_MSK2			0x53
#define TPS6591X_GPIO0				0x60
#define TPS6591X_MAX_REGISTER			0x80

/* IRQ Definitions */
#define TPS6591X_IRQ_VBAT_VMBDCH		0
#define TPS6591X_IRQ_VBAT_VMHI			1
#define TPS6591X_IRQ_PWRON			2
#define TPS6591X_IRQ_PWRON_LP			3
#define TPS6591X_IRQ_PWRHOLD			4
#define TPS6591X_IRQ_HOTDIE			5
#define TPS6591X_IRQ_RTC_ALARM			6
#define TPS6591X_IRQ_RTC_PERIOD			7
#define TPS6591X_IRQ_GPIO_R				8
#define TPS6591X_IRQ_GPIO_F				9
#define TPS6591X_NUM_IRQ			10

/* GPIO Register Definitions */
#define TPS6591X_GPIO_DEB			BIT(2)
#define TPS6591X_GPIO_PUEN			BIT(3)
#define TPS6591X_GPIO_CFG			BIT(2)
#define TPS6591X_GPIO_STS			BIT(1)
#define TPS6591X_GPIO_SET			BIT(0)

/**
 * struct tps65910_board
 * Board platform data may be used to initialize regulators.
 */

struct tps65910_board {
	struct regulator_init_data *tps65910_pmic_init_data;
};

/**
 * struct tps65910 - tps65910 sub-driver chip access routines
 */

struct tps65910 {
	struct device *dev;
	struct i2c_client *i2c_client;
	int (*read)(struct tps65910 *tps65910, u8 reg, int size, void *dest);
	int (*write)(struct tps65910 *tps65910, u8 reg, int size, void *src);

	/* Client devices */
	struct tps65910_pmic *pmic;
	struct tps65910_rtc *rtc;
	struct tps65910_power *power;

	/* GPIO Handling */
	struct gpio_chip gpio;

	/* IRQ Handling */
	struct mutex irq_lock;
	int chip_irq;
	int irq_base;
	u16 irq_mask;
};

struct tps65910_platform_data {
	int irq_base;
};

#endif /*  __LINUX_MFD_TPS6591X_H */
