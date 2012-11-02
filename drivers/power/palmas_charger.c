/*
 * Driver for the Palmas Embedded BQ24915 Battery Charger
 *
 * Copyright (C) 2012 Texas Instruments
 *
 * Author: Graeme Gregory <gg@slimlogic.co.uk>
 *
 * based on
 *
 * BQ24195 battery charging driver
 *
 * Copyright (C) 2012 Texas Instruments, Inc.
 * Author: Texas Instruments, Inc.
 *
 * This package is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.

 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#define DEBUG
#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/jiffies.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/i2c/twl.h>
#include <linux/usb/otg.h>
#include <mach/gpio.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/power_supply.h>
#include <linux/mfd/palmas.h>
#include <linux/byteorder/generic.h>

static const unsigned int fuelgauge_rate[4] = {1, 4, 16, 64};

#define BQ_SLAVE	3
#define FG_SLAVE	1

/* GPADC Channels */
#define VBAT_CHANNEL		6
#define VBUS_CHANNEL		10
#define TEMP_CHANNEL		1

/* Current Range Values */
#define CURRENT_RANGE_LOW	0
#define CURRENT_RANGE_HIGH	1

/* REG01 CHG_CONFIG Values */
#define CHG_CONFIG_DISABLE	0x00
#define CHG_CONFIG_CHARGE_BAT	0x01
#define CHG_CONFIG_OTG		0x02
#define CHG_CONFIG_OTG2		0x03

/* REG01 SYS_MIN Values */
#define SYSMIN_3v0		0x00
#define SYSMIN_3v1		0x01
#define SYSMIN_3v2		0x02
#define SYSMIN_3v3		0x03
#define SYSMIN_3v4		0x04
#define SYSMIN_3v5		0x05
#define SYSMIN_3v6		0x06
#define SYSMIN_3v7		0x07

/* REG01 BOOST_LIM Values */
#define BOOSTLIM_500mA		0x00
#define BOOSTLIM_1300mA		0x01

/* REG04 BATLOWV Values */
#define BATLOWV_2800mV		0x00
#define BATLOWV_3000mV		0x01

/* REG04 VRECHG */
#define VRECHG_100mV		0x00
#define VRECHG_300mV		0x01

/* USB_SUSPEND modes */
#define USB_SUSPEND_DISABLE	0x00
#define USB_SUSPEND_ENABLE	0x01

static const int iinlim_to_ichg[] = {
	506,
	506,
	506,
	948,
	1210,
	1530,
	2036,
	3060,
};

static int palmas_charger_read(struct palmas_charger_info *di,
		unsigned int reg, unsigned int *dest)
{
	struct palmas *palmas = di->palmas;
	unsigned int addr;

	addr = PALMAS_BASE_TO_REG(PALMAS_BQ24192_BASE, reg);

	return regmap_read(palmas->regmap[BQ_SLAVE], addr, dest);
}

static int palmas_charger_update_bits(struct palmas_charger_info *di,
		unsigned int reg, unsigned int mask, unsigned int data)
{
	struct palmas *palmas = di->palmas;
	unsigned int addr;

	addr = PALMAS_BASE_TO_REG(PALMAS_BQ24192_BASE, reg);

	return regmap_update_bits(palmas->regmap[BQ_SLAVE], addr, mask, data);
}

static int palmas_set_iinlim(struct palmas_charger_info *di,
		unsigned int code)
{

	int ret;

	if (code > PALMAS_IINLIM_3000MA)
		return -EINVAL;

	ret = palmas_charger_update_bits(di, PALMAS_REG00,
			PALMAS_REG00_IINLIM_MASK, code);
	if (ret < 0) {
		dev_err(di->dev, "Unable to update current limit\n");
		return ret;
	}

	return 0;
}

static int palmas_set_usbsuspend(struct palmas_charger_info *di,
		unsigned int enable)
{
	struct palmas *palmas = di->palmas;
	unsigned int addr, reg;
	int ret;

	if (enable > USB_SUSPEND_ENABLE)
		return -EINVAL;

	reg = enable << PALMAS_USB_CHGCTL1_USB_SUSPEND_SHIFT;
	addr = PALMAS_BASE_TO_REG(PALMAS_PMU_CONTROL_BASE, PALMAS_USB_CHGCTL1);

	ret = regmap_update_bits(palmas->regmap[0], addr,
			PALMAS_USB_CHGCTL1_USB_SUSPEND, reg);
	if (ret < 0) {
		dev_err(di->dev, "Unable to update usb suspend\n");
		return ret;
	}

	return 0;

}

static int palmas_reset(struct palmas_charger_info *di)
{
	int ret;

	ret = palmas_charger_update_bits(di, PALMAS_REG01,
			PALMAS_REG01_REGISTER_RESET,
			PALMAS_REG01_REGISTER_RESET);
	if (ret < 0) {
		dev_err(di->dev, "Register reset failed\n");
		return ret;
	}

	return 0;
}

static int palmas_reset_watchdog(struct palmas_charger_info *di)
{
	int ret;

	ret = palmas_charger_update_bits(di, PALMAS_REG01,
			PALMAS_REG01_I2C_WATCHDOG_TIMER_RESET,
			PALMAS_REG01_I2C_WATCHDOG_TIMER_RESET);
	if (ret < 0) {
		dev_err(di->dev, "Watchdog reset failed\n");
		return ret;
	}

	return 0;
}


static int palmas_set_chgconfig(struct palmas_charger_info *di,
		unsigned int code)
{
	int ret;

	if (code > CHG_CONFIG_OTG)
		return -EINVAL;

	ret = palmas_charger_update_bits(di, PALMAS_REG01,
			PALMAS_REG01_CHG_CONFIG_MASK,
			code << PALMAS_REG01_CHG_CONFIG_SHIFT);
	if (ret < 0) {
		dev_err(di->dev, "CHG_CONFIG set failed\n");
		return ret;
	}

	return 0;
}

static int palmas_set_sysmin(struct palmas_charger_info *di,
		unsigned int vlimit)
{
	int ret;

	if (vlimit > SYSMIN_3v7)
		return -EINVAL;

	ret = palmas_charger_update_bits(di, PALMAS_REG01,
			PALMAS_REG01_SYS_MIN_MASK,
			vlimit << PALMAS_REG01_SYS_MIN_SHIFT);
	if (ret < 0) {
		dev_err(di->dev, "SYS_MIN set failed\n");
		return ret;
	}

	return 0;

}

static int palmas_set_chargecurrent(struct palmas_charger_info *di,
		unsigned int ichg)
{
	int ret;
	unsigned int reg = 0;

	switch (di->current_range) {
	case CURRENT_RANGE_LOW:
		if ((ichg < 250) || (ichg > 2266))
			return -EINVAL;
		/* minimum current is 250, and increases in 32mA steps */
		reg = (ichg - 250) / 32;
		break;
	case CURRENT_RANGE_HIGH:
		if ((ichg < 500) || (ichg > 4532))
			return -EINVAL;
		/* minimum current is 500, and increases in 64mA steps */
		reg = (ichg - 500) / 64;
		break;
	}

	ret = palmas_charger_update_bits(di, PALMAS_REG02,
			PALMAS_REG02_ICHG_MASK, reg << PALMAS_REG02_ICHG_SHIFT);
	if (ret < 0) {
		dev_err(di->dev, "ICHG set failed\n");
		return ret;
	}

	return 0;
}

static int palmas_set_prechargecurrent(struct palmas_charger_info *di,
		unsigned int iprechg)
{
	int ret;
	unsigned int reg = 0;

	switch (di->current_range) {
	case CURRENT_RANGE_LOW:
		if ((iprechg < 64) || (iprechg > 1024))
			return -EINVAL;
		/* minimum current is 64mA, and increases in 64mA steps */
		reg = (iprechg - 64) / 64;
		break;
	case CURRENT_RANGE_HIGH:
		if ((iprechg < 128) || (iprechg > 2048))
			return -EINVAL;
		/* minimum current is 128mA, and increases in 128mA steps */
		reg = (iprechg - 128) / 128;
		break;
	}

	ret = palmas_charger_update_bits(di, PALMAS_REG03,
			PALMAS_REG03_IPRECHG_MASK,
			reg << PALMAS_REG03_IPRECHG_SHIFT);
	if (ret < 0) {
		dev_err(di->dev, "IPRECHG set failed\n");
		return ret;
	}

	return 0;
}

static int palmas_set_terminationcurrent(struct palmas_charger_info *di,
		unsigned int iterm)
{
	int ret;
	unsigned int reg = 0;

	switch (di->current_range) {
	case CURRENT_RANGE_LOW:
		if ((iterm < 64) || (iterm > 1024))
			return -EINVAL;
		/* minimum current is 64mA, and increases in 64mA steps */
		reg = (iterm - 64) / 64;
		break;
	case CURRENT_RANGE_HIGH:
		if ((iterm < 128) || (iterm > 2048))
			return -EINVAL;
		/* minimum current is 128mA, and increases in 128mA steps */
		reg = (iterm - 128) / 128;
		break;
	}

	ret = palmas_charger_update_bits(di, PALMAS_REG03,
			PALMAS_REG03_ITERM_MASK, reg);
	if (ret < 0) {
		dev_err(di->dev, "ITERM set failed\n");
		return ret;
	}

	return 0;
}

static int palmas_set_vreg(struct palmas_charger_info *di, unsigned int vreg)
{
	int ret;
	unsigned int reg;

	if ((vreg < 3504000) || (vreg > 4400000))
		return -EINVAL;
	/* minimum current is 3.504v, and increases in 16mV steps */
	reg = (vreg - 3504000) / 16000;

	ret = palmas_charger_update_bits(di, PALMAS_REG04,
			PALMAS_REG04_VREG_MASK,
			reg << PALMAS_REG04_VREG_SHIFT);
	if (ret < 0) {
		dev_err(di->dev, "VREG set failed\n");
		return ret;
	}

	return 0;
}

static int palmas_set_batlowv(struct palmas_charger_info *di,
		unsigned int setting)
{
	int ret;

	if (setting > BATLOWV_3000mV)
		return -EINVAL;

	ret = palmas_charger_update_bits(di, PALMAS_REG04,
			PALMAS_REG04_BATLOWV,
			setting << PALMAS_REG04_BATLOWV_SHIFT);
	if (ret < 0) {
		dev_err(di->dev, "BATLOWV set failed\n");
		return ret;
	}

	return 0;
}

static int palmas_set_vrechg(struct palmas_charger_info *di,
		unsigned int setting)
{
	int ret;

	if (setting > VRECHG_300mV)
		return -EINVAL;

	ret = palmas_charger_update_bits(di, PALMAS_REG04,
			PALMAS_REG04_VRECHG,
			setting << PALMAS_REG04_VRECHG_SHIFT);
	if (ret < 0) {
		dev_err(di->dev, "VRECHG set failed\n");
		return ret;
	}

	return 0;
}

static int palmas_set_watchdog(struct palmas_charger_info *di,
		unsigned int code)
{
	int ret;
	int time;

	if (code > PALMAS_WATCHDOG_160S)
		return -EINVAL;

	ret = palmas_charger_update_bits(di, PALMAS_REG05,
			PALMAS_REG05_WATCHDOG_MASK,
			code << PALMAS_REG05_WATCHDOG_SHIFT);
	if (ret < 0) {
		dev_err(di->dev, "WATCHDOG set failed\n");
		return ret;
	}

	/*
	 * If watchdog is set then make sure that we service it 5s
	 * before it is due to expire in case of delay. Otherwise
	 * disable the work.
	 */
	if (code) {
		time = (1 << (code - 1)) * 40000;
		time -= 5000;
		di->watchdog_time = time;
		cancel_delayed_work(&di->palmas_charger_work);
		palmas_reset_watchdog(di);
		schedule_delayed_work(&di->palmas_charger_work,
			msecs_to_jiffies(time));
	} else {
		cancel_delayed_work(&di->palmas_charger_work);
	}

	return 0;
}

static void palmas_charger_handle_status(struct palmas_charger_info *di)
{
	int charge_state;
	int vsys_state;

	charge_state = (di->reg08 & PALMAS_REG08_CHRG_STAT_MASK) >>
			PALMAS_REG08_CHRG_STAT_SHIFT;

	vsys_state = di->reg08 & PALMAS_REG08_VSYS_STAT;

	switch (charge_state) {
	case 3:
		dev_dbg(di->dev, "CHARGE STATE : CHARGE DONE\n");
		di->battery_charge_status = POWER_SUPPLY_STATUS_FULL;
		di->battery_health = POWER_SUPPLY_HEALTH_GOOD;
		break;
	case 2:
		dev_dbg(di->dev, "CHARGE STATE : FAST CHARGING\n");
		di->battery_charge_status = POWER_SUPPLY_STATUS_CHARGING;
		di->battery_health = POWER_SUPPLY_HEALTH_GOOD;
		/*
		 * a fault resets the charge params so if its cleared set them
		 */
		if (di->battery_had_fault) {
			schedule_work(&di->usb_event_work);
			di->battery_had_fault = 0;
		}
		break;
	case 1:
		dev_dbg(di->dev, "CHARGE STATE : PRECHARGE\n");
		di->battery_charge_status = POWER_SUPPLY_STATUS_CHARGING;
		di->battery_health = POWER_SUPPLY_HEALTH_GOOD;
		/*
		 * a fault resets the charge params so if its cleared set them
		 */
		if (di->battery_had_fault) {
			schedule_work(&di->usb_event_work);
			di->battery_had_fault = 0;
		}
		break;
	case 0:
		dev_dbg(di->dev, "CHARGE STATE : NOT CHARGING\n");
		if (di->wireless_online_status || di->usb_online_status)
			di->battery_charge_status =
					POWER_SUPPLY_STATUS_NOT_CHARGING;
		else
			di->battery_charge_status =
					POWER_SUPPLY_STATUS_DISCHARGING;
		break;
	}

	if (vsys_state) {
		dev_dbg(di->dev, "Battery too low\n");
		di->battery_capacity = 0;
	}
}

static void palmas_charger_handle_status_irq(struct palmas_charger_info *di)
{
	int charge_state;

	charge_state = (di->reg08 & PALMAS_REG08_CHRG_STAT_MASK) >>
			PALMAS_REG08_CHRG_STAT_SHIFT;

	if (charge_state == PALMAS_REG08_CHRG_STAT_MASK) {
		dev_dbg(di->dev, "CHARGE STATE : CHARGE DONE\n");
		di->battery_charge_status = POWER_SUPPLY_STATUS_FULL;
		di->battery_health = POWER_SUPPLY_HEALTH_GOOD;
	}
}

static void palmas_charger_handle_error(struct palmas_charger_info *di)
{
	u8 chrg_fault = (di->reg09 & PALMAS_REG09_CHRG_FAULT_MASK) >>
			PALMAS_REG09_CHRG_FAULT_SHIFT;
	u8 charge_gated = di->reg09 & PALMAS_REG09_CHARGE_GATED_MASK;

	switch (chrg_fault) {
	case 3:
		dev_dbg(di->dev, "CHARGER TIMER EXPIRATION\n");
		di->battery_charge_status = POWER_SUPPLY_STATUS_NOT_CHARGING;
		di->battery_health = POWER_SUPPLY_HEALTH_UNSPEC_FAILURE;
		break;
	case 2:
		dev_dbg(di->dev, "CHARGER THERMAL SHUTDOWN\n");
		di->battery_charge_status = POWER_SUPPLY_STATUS_NOT_CHARGING;
		di->battery_health = POWER_SUPPLY_HEALTH_UNSPEC_FAILURE;
		break;
	case 1:
		dev_dbg(di->dev, "CHARGER INPUT OVER/UNDERVOLTAGE\n");
		di->battery_charge_status = POWER_SUPPLY_STATUS_NOT_CHARGING;
		di->battery_health = POWER_SUPPLY_HEALTH_UNSPEC_FAILURE;
		break;
	}

	switch (charge_gated) {
	case 6:
		dev_dbg(di->dev, "CHARGE GATED");
		di->battery_charge_status = POWER_SUPPLY_STATUS_NOT_CHARGING;
		break;
	case 0:
		break;
	}

	if (di->reg09 & PALMAS_REG09_WATCHDOG_FAULT)
		dev_dbg(di->dev, "CHARGER WATCHDOG EXPIRED\n");

	if (di->reg09 & PALMAS_REG09_OTG_FAULT)
		dev_dbg(di->dev, "CHARGER OTG FAULT\n");

	if (di->reg09 & PALMAS_REG09_BAT_FAULT) {
		dev_dbg(di->dev, "BATTERY OVP\n");
		di->battery_health = POWER_SUPPLY_HEALTH_OVERVOLTAGE;
	}

	di->battery_had_fault = 1;
}

static irqreturn_t palmas_charger_irq(int irq, void *_di)
{
	struct palmas_charger_info *di = _di;
	unsigned int reg08, reg09;
	int ret;

	dev_dbg(di->dev, "Charger IRQ\n");

	ret = palmas_charger_read(di, PALMAS_REG08, &reg08);
	if (ret) {
		dev_err(di->dev, "Unable to read reg08 from charger\n");
		return IRQ_HANDLED;
	}

	ret = palmas_charger_read(di, PALMAS_REG09, &reg09);
	if (ret) {
		dev_err(di->dev, "Unable to read reg09 from charger\n");
		return IRQ_HANDLED;
	}

	di->reg08 = reg08;
	di->reg09 = reg09;

	palmas_charger_handle_status_irq(di);
	palmas_charger_handle_error(di);

	return IRQ_HANDLED;
}

static void palmas_event_work_func(struct work_struct *work)
{
	struct palmas_charger_info *di = container_of(work,
					      struct palmas_charger_info,
					      usb_event_work);

	switch (di->usb_event) {
	case PALMAS_USB_100mA:
		dev_dbg(di->dev, "Charger PALMAS_USB_100mA\n");
		palmas_set_iinlim(di, PALMAS_IINLIM_100MA);
		palmas_set_chargecurrent(di,
				iinlim_to_ichg[PALMAS_IINLIM_100MA]);
		palmas_set_watchdog(di, di->watchdog_code);
		palmas_set_chgconfig(di, CHG_CONFIG_CHARGE_BAT);
		palmas_set_usbsuspend(di, USB_SUSPEND_DISABLE);
		power_supply_changed(&di->usb);
		break;
	case PALMAS_USB_500mA:
		dev_dbg(di->dev, "Charger PALMAS_USB_500mA\n");
		palmas_set_iinlim(di, PALMAS_IINLIM_500MA);
		palmas_set_chargecurrent(di,
				iinlim_to_ichg[PALMAS_IINLIM_500MA]);
		palmas_set_watchdog(di, di->watchdog_code);
		palmas_set_chgconfig(di, CHG_CONFIG_CHARGE_BAT);
		palmas_set_usbsuspend(di, USB_SUSPEND_DISABLE);
		power_supply_changed(&di->usb);
		break;
	case PALMAS_USB_CHARGER:
		dev_dbg(di->dev, "Charger PALMAS_USB_CHARGER\n");
		palmas_set_iinlim(di, di->usb_charge_limit);
		palmas_set_chargecurrent(di,
				iinlim_to_ichg[di->usb_charge_limit]);
		palmas_set_watchdog(di, di->watchdog_code);
		palmas_set_chgconfig(di, CHG_CONFIG_CHARGE_BAT);
		palmas_set_usbsuspend(di, USB_SUSPEND_DISABLE);
		power_supply_changed(&di->usb);
		break;
	case PALMAS_USB_SUSPEND:
		dev_dbg(di->dev, "Charger PALMAS_USB_SUSPEND\n");
		palmas_set_watchdog(di, PALMAS_WATCHDOG_DISABLE);
		palmas_set_usbsuspend(di, USB_SUSPEND_ENABLE);
		power_supply_changed(&di->usb);
		break;
	default:
		dev_dbg(di->dev, "Charger Unknown EVENT\n");
	}
}

static void palmas_charger_work(struct work_struct *work)
{
	struct palmas_charger_info *di = container_of(work,
			struct palmas_charger_info, palmas_charger_work.work);

	dev_dbg(di->dev, "Charger WORK %d\n", di->watchdog_time);

	palmas_reset_watchdog(di);

	schedule_delayed_work(&di->palmas_charger_work,
			msecs_to_jiffies(di->watchdog_time));
}

static void palmas_calculate_capacity(struct palmas_charger_info *di)
{
	int accumulated_charge, curr_capacity;

	/*
	 * 3000 mA maps to a count of 4096 per sample
	 * We have 4 samples per second
	 * Charge added in one second == (acc_value * 3000 / (4 * 4096))
	 * mAh added == (Charge added in one second / 3600)
	 * mAh added == acc_value * (3000/3600) / (4 * 4096)
	 * mAh added == acc_value * (5/6) / (2^14)
	 * Using 5/6 instead of 3000 to avoid overflow
	 * FIXME: revisit this code for overflows
	 * FIXME: Take care of different value of samples/sec
	 */

	accumulated_charge = (((di->battery_charge_n1 -
			(di->cc_offset * di->battery_timer_n1)) * 5) / 6) >> 14;
	curr_capacity = (di->battery_boot_capacity_mAh + accumulated_charge) /
					(di->battery_capacity_max / 100);
	dev_dbg(di->dev, "voltage %d\n", di->battery_voltage_uV);
	dev_dbg(di->dev,
		"initial capacity %d mAh, accumulated %d mAh, total %d mAh\n",
			di->battery_boot_capacity_mAh, accumulated_charge,
			di->battery_boot_capacity_mAh + accumulated_charge);
	dev_dbg(di->dev, "percentage_capacity %d\n", curr_capacity);

	if (curr_capacity > 99)
		curr_capacity = 99;

	if (di->battery_charge_status == POWER_SUPPLY_STATUS_FULL)
		curr_capacity = 100;

	if (curr_capacity != di->battery_prev_capacity) {
		di->battery_capacity = curr_capacity;
		di->battery_prev_capacity = curr_capacity;
		power_supply_changed(&di->battery);
	}

	return;
}

static void palmas_battery_current_avg(struct work_struct *work)
{
	struct palmas_charger_info *di = container_of(work,
		struct palmas_charger_info,
		battery_current_avg_work.work);
	struct palmas *palmas = di->palmas;
	s32 samples = 0;
	s16 cc_offset = 0;
	unsigned int addr;
	int current_avg_uA = 0, ret;
	u8 temp[4];

	di->battery_charge_n2 = di->battery_charge_n1;
	di->battery_timer_n2 = di->battery_timer_n1;

	/* pause FG updates to get consistant data */
	addr = PALMAS_BASE_TO_REG(PALMAS_FUEL_GAUGE_BASE, PALMAS_FG_REG_00);
	ret = regmap_update_bits(palmas->regmap[FG_SLAVE], addr,
			PALMAS_FG_REG_00_CC_PAUSE, PALMAS_FG_REG_00_CC_PAUSE);
	if (ret < 0) {
		dev_err(di->dev, "Error pausing FG FG_REG_00\n");
		return;
	}

	/* FG_REG_01, 02, 03 is 24 bit unsigned sample counter value */
	addr = PALMAS_BASE_TO_REG(PALMAS_FUEL_GAUGE_BASE, PALMAS_FG_REG_01);

	ret = regmap_bulk_read(palmas->regmap[FG_SLAVE], addr, temp, 3);
	if (ret < 0) {
		dev_err(di->dev, "Error reading FG_REG_01-03\n");
		return;
	}

	temp[3] = 0;

	di->battery_timer_n1 = le32_to_cpup((u32 *)temp);

	/*
	 * FG_REG_04, 5, 6, 7 is 32 bit signed accumulator value
	 * accumulates instantaneous current value
	 */
	addr = PALMAS_BASE_TO_REG(PALMAS_FUEL_GAUGE_BASE, PALMAS_FG_REG_04);

	ret = regmap_bulk_read(palmas->regmap[FG_SLAVE], addr, temp, 4);
	if (ret < 0) {
		dev_err(di->dev, "Error reading FG_REG_04-07\n");
		return;
	}

	di->battery_charge_n1 = le32_to_cpup((u32 *)temp);

	/* FG_REG_08, 09 is 10 bit signed calibration offset value */
	addr = PALMAS_BASE_TO_REG(PALMAS_FUEL_GAUGE_BASE, PALMAS_FG_REG_08);

	ret = regmap_bulk_read(palmas->regmap[FG_SLAVE], addr, temp, 2);
	if (ret < 0) {
		dev_err(di->dev, "Error reading FG_REG_09-09\n");
		return;
	}

	cc_offset = le16_to_cpup((u16 *)temp);
	cc_offset = ((s16)(cc_offset << 6) >> 6);
	di->cc_offset = cc_offset;

	/* resume FG updates */
	addr = PALMAS_BASE_TO_REG(PALMAS_FUEL_GAUGE_BASE, PALMAS_FG_REG_00);
	ret = regmap_update_bits(palmas->regmap[FG_SLAVE], addr,
			PALMAS_FG_REG_00_CC_PAUSE, 0);
	if (ret < 0) {
		dev_dbg(di->dev, "Error resuming FG FG_REG_00\n");
		return;
	}

	samples = di->battery_timer_n1 - di->battery_timer_n2;
	/* check for timer overflow */
	if (di->battery_timer_n1 < di->battery_timer_n2)
		samples = samples + (1 << 24);

	/* offset is accumulative over number of samples */
	cc_offset = cc_offset * samples;

	current_avg_uA = ((di->battery_charge_n1 - di->battery_charge_n2
					- cc_offset)
					* di->current_max_scale) /
					fuelgauge_rate[di->fuelgauge_mode];
	/* clock is a fixed 32Khz */
	current_avg_uA >>= 15;

	/* Correct for the fuelguage sampling rate */
	samples /= fuelgauge_rate[di->fuelgauge_mode] * 4;

	/*
	 * Only update the current average if we have had a valid number
	 * of samples in the accumulation.
	 */
	if (samples) {
		current_avg_uA = current_avg_uA / samples;
		di->battery_current_avg_uA = current_avg_uA * 1000;
	}

	palmas_calculate_capacity(di);

	schedule_delayed_work(&di->battery_current_avg_work,
		msecs_to_jiffies(1000 * di->battery_current_avg_interval));
	return;
}

static void palmas_battery_current_now(struct palmas_charger_info *di)
{
	struct palmas *palmas = di->palmas;
	int ret = 0;
	unsigned int reg, addr;
	s16 temp = 0;
	int current_now = 0;

	/* pause FG updates to get consistant data */
	addr = PALMAS_BASE_TO_REG(PALMAS_FUEL_GAUGE_BASE, PALMAS_FG_REG_00);
	ret = regmap_update_bits(palmas->regmap[FG_SLAVE], addr,
			PALMAS_FG_REG_00_CC_PAUSE, PALMAS_FG_REG_00_CC_PAUSE);
	if (ret < 0) {
		dev_err(di->dev, "Error pausing FG FG_REG_00\n");
		return;
	}

	addr = PALMAS_BASE_TO_REG(PALMAS_FUEL_GAUGE_BASE, PALMAS_FG_REG_10);

	ret = regmap_read(palmas->regmap[FG_SLAVE], addr, &reg);
	if (ret < 0) {
		dev_dbg(di->dev, "failed to read FG_REG_10\n");
		return;
	}

	temp = reg;

	addr = PALMAS_BASE_TO_REG(PALMAS_FUEL_GAUGE_BASE, PALMAS_FG_REG_11);

	ret = regmap_read(palmas->regmap[FG_SLAVE], addr, &reg);
	if (ret < 0) {
		dev_dbg(di->dev, "failed to read FG_REG_11\n");
		return;
	}

	temp |= reg << 8;

	/* resume FG updates */
	addr = PALMAS_BASE_TO_REG(PALMAS_FUEL_GAUGE_BASE, PALMAS_FG_REG_00);
	ret = regmap_update_bits(palmas->regmap[FG_SLAVE], addr,
			PALMAS_FG_REG_00_CC_PAUSE, 0);
	if (ret < 0) {
		dev_dbg(di->dev, "Error resuming FG FG_REG_00\n");
		return;
	}

	/* sign extend the result */
	temp = ((s16)(temp << 2) >> 2);
	current_now = temp - di->cc_offset;

	/* current drawn per sec */
	current_now = current_now * fuelgauge_rate[di->fuelgauge_mode];
	/* current in mAmperes */
	current_now = (current_now * di->current_max_scale) >> 13;
	/* current in uAmperes */
	current_now = current_now * 1000;
	di->battery_current_uA = current_now;

	return;
}

static void palmas_battery_voltage_now(struct palmas_charger_info *di)
{
	struct palmas *palmas = di->palmas;
	struct palmas_gpadc_result res;
	int ret;

	ret = palmas_gpadc_conversion(palmas->gpadc, VBAT_CHANNEL, &res);
	if (ret < 0)
		dev_err(di->dev, "Error in Battery Voltage conversion\n");

	di->battery_voltage_uV = res.result * 1000;
}

static void palmas_battery_temperature(struct palmas_charger_info *di)
{
	struct palmas *palmas = di->palmas;
	struct palmas_gpadc_result res;
	int temp, adc_code, ret;

	ret = palmas_gpadc_conversion(palmas->gpadc, TEMP_CHANNEL, &res);
	if (ret < 0)
		dev_err(di->dev, "Error in Battery Voltage conversion\n");

	adc_code = res.corrected_code;

	for (temp = 0; temp < di->battery_temperature_chart_size; temp++) {
		if (adc_code >= di->battery_temperature_chart[temp])
			break;
	}

	/*
	 * first 2 values are for negative temperature
	 * temperature in tenths of degree Celsius
	 */
	di->battery_termperature_tenthC = (temp - 2) * 10;
}

void palmas_charger_usb_vbus_state(struct palmas_charger_info *di, int state)
{
	dev_dbg(di->dev, "VBUS State %d\n", state);

	di->usb_online_status = state;

	/*
	 * Make sure charger is is suspend state until we get notification
	 * of supply type from USB subsystem.
	 */
	palmas_set_watchdog(di, PALMAS_WATCHDOG_DISABLE);
	palmas_set_usbsuspend(di, USB_SUSPEND_ENABLE);

	power_supply_changed(&di->usb);
}
EXPORT_SYMBOL(palmas_charger_usb_vbus_state);

static void palmas_usb_voltage_now(struct palmas_charger_info *di)
{
	struct palmas *palmas = di->palmas;
	struct palmas_gpadc_result res;
	int ret;

	ret = palmas_gpadc_conversion(palmas->gpadc, VBUS_CHANNEL, &res);
	if (ret < 0)
		dev_err(di->dev, "Error in Battery Voltage conversion\n");

	di->battery_voltage_uV = res.result * 1000;
}

static enum power_supply_property palmas_battery_props[] = {
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_HEALTH,
	POWER_SUPPLY_PROP_ONLINE,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_CURRENT_NOW,
	POWER_SUPPLY_PROP_CURRENT_AVG,
	POWER_SUPPLY_PROP_CAPACITY,
	POWER_SUPPLY_PROP_TEMP,
};

static enum power_supply_property palmas_usb_props[] = {
	POWER_SUPPLY_PROP_ONLINE,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
};

static enum power_supply_property palmas_wireless_props[] = {
	POWER_SUPPLY_PROP_ONLINE,
};

static int palmas_battery_get_props(struct power_supply *psy,
					enum power_supply_property psp,
					union power_supply_propval *val)
{
	struct palmas_charger_info *di = container_of(psy,
			struct palmas_charger_info, battery);

	switch (psp) {
	case POWER_SUPPLY_PROP_STATUS:
		val->intval = di->battery_charge_status;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		palmas_battery_voltage_now(di);
		val->intval = di->battery_voltage_uV;
		break;
	case POWER_SUPPLY_PROP_CURRENT_NOW:
		palmas_battery_current_now(di);
		val->intval = di->battery_current_uA;
		break;
	case POWER_SUPPLY_PROP_TEMP:
		palmas_battery_temperature(di);
		val->intval = di->battery_termperature_tenthC;
		break;
	case POWER_SUPPLY_PROP_ONLINE:
		val->intval = (di->battery_online || di->battery_soldered);
		break;
	case POWER_SUPPLY_PROP_CURRENT_AVG:
		val->intval = di->battery_current_avg_uA;
		break;
	case POWER_SUPPLY_PROP_HEALTH:
		val->intval = di->battery_health;
		break;
	case POWER_SUPPLY_PROP_CAPACITY:
		val->intval = di->battery_capacity;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static int palmas_usb_get_property(struct power_supply *psy,
		enum power_supply_property psp,
		union power_supply_propval *val)
{

	struct palmas_charger_info *di = container_of(psy,
			struct palmas_charger_info, usb);

	switch (psp) {
	case POWER_SUPPLY_PROP_ONLINE:
		val->intval = di->usb_online_status;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		palmas_usb_voltage_now(di);
		val->intval = di->battery_voltage_uV;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int palmas_wireless_get_property(struct power_supply *psy,
		enum power_supply_property psp,
		union power_supply_propval *val)
{
	struct palmas_charger_info *di = container_of(psy,
			struct palmas_charger_info,
			wireless);

	switch (psp) {
	case POWER_SUPPLY_PROP_ONLINE:
		val->intval = di->wireless_online_status;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int palmas_current_setup(struct palmas_charger_info *di,
		struct palmas_charger_platform_data *pdata)
{
	struct palmas *palmas = di->palmas;
	int ret = 0;
	unsigned int addr, reg = 0;
	u8 temp[4];

	addr = PALMAS_BASE_TO_REG(PALMAS_FUEL_GAUGE_BASE, PALMAS_FG_REG_00);

	/*
	 * Enable the AUTOCLEAR so that any FG is in known state, and
	 * enabled the FG
	 */
	reg = PALMAS_FG_REG_00_CC_AUTOCLEAR | PALMAS_FG_REG_00_CC_FG_EN |
		PALMAS_FG_REG_00_CC_DITH_EN | PALMAS_FG_REG_00_CC_CAL_EN;

	ret = regmap_update_bits(palmas->regmap[FG_SLAVE], addr, reg, reg);
	if (ret < 0)
		dev_err(di->dev, "failed to write FG_REG00\n");


	/* initialise the current average values */
	di->battery_current_avg_interval = pdata->current_avg_interval;

	/* pause FG updates to get consistant data */
	addr = PALMAS_BASE_TO_REG(PALMAS_FUEL_GAUGE_BASE, PALMAS_FG_REG_00);
	ret = regmap_update_bits(palmas->regmap[FG_SLAVE], addr,
			PALMAS_FG_REG_00_CC_PAUSE, PALMAS_FG_REG_00_CC_PAUSE);
	if (ret < 0) {
		dev_err(di->dev, "Error pausing FG FG_REG_00\n");
		return ret;
	}

	/* FG_REG_01, 02, 03 is 24 bit unsigned sample counter value */
	addr = PALMAS_BASE_TO_REG(PALMAS_FUEL_GAUGE_BASE, PALMAS_FG_REG_01);

	ret = regmap_bulk_read(palmas->regmap[FG_SLAVE], addr, temp, 3);
	if (ret < 0) {
		dev_err(di->dev, "Error reading FG_REG_01-03\n");
		return ret;
	}

	temp[3] = 0;

	di->battery_timer_n2 = le32_to_cpup((u32 *)temp);

	/*
	 * FG_REG_04, 5, 6, 7 is 32 bit signed accumulator value
	 * accumulates instantaneous current value
	 */
	addr = PALMAS_BASE_TO_REG(PALMAS_FUEL_GAUGE_BASE, PALMAS_FG_REG_04);

	ret = regmap_bulk_read(palmas->regmap[FG_SLAVE], addr, temp, 4);
	if (ret < 0) {
		dev_err(di->dev, "Error reading FG_REG_04-07\n");
		return ret;
	}

	di->battery_charge_n2 = le32_to_cpup((u32 *)temp);

	/* FG_REG_08, 09 is 10 bit signed calibration offset value */
	addr = PALMAS_BASE_TO_REG(PALMAS_FUEL_GAUGE_BASE, PALMAS_FG_REG_08);

	ret = regmap_bulk_read(palmas->regmap[FG_SLAVE], addr, temp, 2);
	if (ret < 0) {
		dev_err(di->dev, "Error reading FG_REG_09-09\n");
		return ret;
	}

	di->cc_offset = le16_to_cpup((u16 *)temp);
	di->cc_offset = ((s16)(di->cc_offset << 6) >> 6);

	INIT_DEFERRABLE_WORK(&di->battery_current_avg_work,
						palmas_battery_current_avg);

	schedule_delayed_work(&di->battery_current_avg_work,
		msecs_to_jiffies(1000 * di->battery_current_avg_interval));

	return ret;
}

static irqreturn_t palmas_batremoval_irq(int irq, void *_di)
{
	struct palmas_charger_info *di = _di;
	struct palmas *palmas = di->palmas;
	int addr, reg;

	addr = PALMAS_BASE_TO_REG(PALMAS_INTERRUPT_BASE,
			PALMAS_INT2_LINE_STATE);
	regmap_read(palmas->regmap[1], addr, &reg);

	if (!(reg & PALMAS_INT2_LINE_STATE_BATREMOVAL)) {
		di->battery_online = 1;
		di->battery_health = POWER_SUPPLY_HEALTH_GOOD;
	} else {
		di->battery_online = 0;
		di->battery_health = POWER_SUPPLY_HEALTH_DEAD;
		di->usb_event = PALMAS_USB_SUSPEND;
		schedule_work(&di->usb_event_work);
	}

	return IRQ_HANDLED;
}

static int palmas_batremoval_setup(struct palmas_charger_info *di,
		struct palmas_charger_platform_data *pdata)
{
	struct palmas *palmas = di->palmas;
	int ret = 0;
	unsigned int addr, reg;

	/* If battery detect is not neeeded do not setup */
	if (pdata->battery_soldered) {
		di->battery_soldered = 1;
		return 0;
	}

	ret = palmas_gpadc_current_src_ch0(palmas->gpadc, 1);
	if (ret < 0) {
		dev_err(di->dev, "Error setting CHO current source on\n");
		return ret;
	}

	ret = palmas_gpadc_bat_removal(palmas->gpadc, 1);
	if (ret < 0) {
		dev_err(di->dev, "Error setting BAT_REMOVAL_DET on\n");
		return ret;
	}

	/* let the GPADC settle down */
	msleep(50);

	addr = PALMAS_BASE_TO_REG(PALMAS_INTERRUPT_BASE,
			PALMAS_INT2_LINE_STATE);
	regmap_read(palmas->regmap[1], addr, &reg);

	if (!(reg & PALMAS_INT2_LINE_STATE_BATREMOVAL))
		di->battery_online = 1;

	ret = palmas_request_irq(palmas, PALMAS_BATREMOVAL_IRQ,
				"palmas_batremoval", palmas_batremoval_irq, di);
	if (ret < 0)
		dev_err(di->dev, "can't get IRQ %d, err %d\n",
				PALMAS_BATREMOVAL_IRQ, ret);

	return ret;
}

static int palmas_batremoval_shutdown(struct palmas_charger_info *di)
{
	struct palmas *palmas = di->palmas;
	int ret;

	if (di->battery_soldered)
		return 0;

	palmas_free_irq(palmas, PALMAS_BATREMOVAL_IRQ, di);

	ret = palmas_gpadc_current_src_ch0(palmas->gpadc, 0);
	if (ret < 0) {
		dev_err(di->dev, "Error setting CHO current source off\n");
		return ret;
	}

	ret = palmas_gpadc_bat_removal(palmas->gpadc, 0);
	if (ret < 0)
		dev_err(di->dev, "Error setting BAT_REMOVAL_DET off\n");

	return ret;
}

static irqreturn_t palmas_anticollapse_irq(int irq, void *_di)
{
	struct palmas_charger_info *di = _di;
	struct palmas *palmas = di->palmas;
	int addr, reg;

	addr = PALMAS_BASE_TO_REG(PALMAS_INTERRUPT2_BASE,
			PALMAS_INT7_LINE_STATE);
	regmap_read(palmas->regmap[1], addr, &reg);

	if ((reg & PALMAS_INT7_LINE_STATE_CHRG_IN_ANTICOLLAPSE))
		dev_dbg(di->dev, "Charger in ANTICOLLAPSE\n");
	else
		dev_dbg(di->dev, "Charger not in ANTICOLLAPSE\n");

	return IRQ_HANDLED;
}

static irqreturn_t palmas_bat_temp_fault_irq(int irq, void *_di)
{
	struct palmas_charger_info *di = _di;
	struct palmas *palmas = di->palmas;
	int addr, reg;

	addr = PALMAS_BASE_TO_REG(PALMAS_INTERRUPT2_BASE,
			PALMAS_INT7_LINE_STATE);
	regmap_read(palmas->regmap[1], addr, &reg);

	if (reg & PALMAS_INT7_LINE_STATE_BAT_TEMP_FAULT) {
		dev_dbg(di->dev, "Battery Over/Under Temp\n");
		di->battery_health = POWER_SUPPLY_HEALTH_OVERHEAT;
	} else {
		dev_dbg(di->dev, "Battery Not Over/Under Temp\n");
		di->battery_health = POWER_SUPPLY_HEALTH_GOOD;
	}

	return IRQ_HANDLED;
}

static irqreturn_t palmas_vac_acok_irq(int irq, void *_di)
{
	struct palmas_charger_info *di = _di;
	struct palmas *palmas = di->palmas;
	int addr, reg;

	addr = PALMAS_BASE_TO_REG(PALMAS_INTERRUPT_BASE,
			PALMAS_INT2_LINE_STATE);
	regmap_read(palmas->regmap[1], addr, &reg);

	if (reg & PALMAS_INT2_LINE_STATE_VAC_ACOK) {
		dev_dbg(di->dev, "Charger Wireless Inserted\n");
		di->wireless_online_status = 1;
	} else {
		dev_dbg(di->dev, "Charger Wireless Removed\n");
		di->wireless_online_status = 0;
	}

	return IRQ_HANDLED;
}

static void palmas_battery_charge_status_work(struct work_struct *work)
{
	struct palmas_charger_info *di = container_of(work,
		struct palmas_charger_info,
		palmas_battery_status_work.work);
	int ret;
	unsigned int reg08;

	ret = palmas_charger_read(di, PALMAS_REG08, &reg08);
	if (ret) {
		dev_err(di->dev, "Unable to read reg08 from charger\n");
		return;
	}

	di->reg08 = reg08;

	palmas_charger_handle_status(di);

	schedule_delayed_work(&di->palmas_battery_status_work,
		msecs_to_jiffies(di->battery_status_interval));
}

static ssize_t set_usb_event(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct palmas_charger_info *di = dev_get_drvdata(dev);
	int status = count;

	if (!strncmp(buf, "palmas_usb_100mA", 16))
		di->usb_event = PALMAS_USB_100mA;
	else if (!strncmp(buf, "palmas_usb_500mA", 16))
		di->usb_event = PALMAS_USB_500mA;
	else if (!strncmp(buf, "palmas_usb_charger", 18))
		di->usb_event = PALMAS_USB_CHARGER;
	else if (!strncmp(buf, "palmas_usb_suspend", 18))
		di->usb_event = PALMAS_USB_SUSPEND;
	else
		return status;

	schedule_work(&di->usb_event_work);

	return status;
}

static ssize_t set_regulation_voltage(struct device *dev,
        struct device_attribute *attr, const char *buf, size_t count)
{
	struct palmas_charger_info *di = dev_get_drvdata(dev);
	long val;
	int ret;

	if (strict_strtol(buf, 10, &val) < 0)
		return -EINVAL;

	ret = palmas_set_vreg(di, val);
	if (ret < 0)
		return ret;

	return count;
}

static ssize_t show_regulation_voltage(struct device *dev,
        struct device_attribute *attr, char *buf)
{
	struct palmas_charger_info *di = dev_get_drvdata(dev);
	unsigned int reg;
	int ret;

	ret = palmas_charger_read(di, PALMAS_REG04, &reg);
	if (ret < 0)
		return ret;

	reg &= PALMAS_REG04_VREG_MASK;
	reg >>= PALMAS_REG04_VREG_SHIFT;

        reg = reg * 16000 + 3504000;

        return sprintf(buf, "%u\n", reg);
}

static ssize_t set_vsys_minimum(struct device *dev,
        struct device_attribute *attr, const char *buf, size_t count)
{
	struct palmas_charger_info *di = dev_get_drvdata(dev);
	long val;
	int ret;

	if (strict_strtol(buf, 10, &val) < 0)
		return -EINVAL;

	/* 3.0v - 3.7v in 0.1v steps */
	val -= 3000000;
	val /= 100000;

	ret = palmas_set_sysmin(di, val);
	if (ret < 0)
		return ret;

	return count;
}

static ssize_t show_vsys_minimum(struct device *dev,
        struct device_attribute *attr, char *buf)
{
	struct palmas_charger_info *di = dev_get_drvdata(dev);
	unsigned int reg;
	int ret;

	ret = palmas_charger_read(di, PALMAS_REG01, &reg);
	if (ret < 0)
		return ret;

	reg &= PALMAS_REG01_SYS_MIN_MASK;
	reg >>= PALMAS_REG01_SYS_MIN_SHIFT;

        reg = reg * 100000 + 3000000;

        return sprintf(buf, "%u\n", reg);
}

static ssize_t set_precharge_current(struct device *dev,
        struct device_attribute *attr, const char *buf, size_t count)
{
	struct palmas_charger_info *di = dev_get_drvdata(dev);
	long val;
	int ret;

	if (strict_strtol(buf, 10, &val) < 0)
		return -EINVAL;

	if (di->current_range) {
		/* 128mA - 2048mA in 128mA steps */
		val -= 128000;
		val /= 128000;
	} else {
		/* 64mA - 1024mA in 64mA steps */
		val -= 64000;
		val /= 64000;
	}

	ret = palmas_set_prechargecurrent(di, val);
	if (ret < 0)
		return ret;

	return count;
}

static ssize_t show_precharge_current(struct device *dev,
        struct device_attribute *attr, char *buf)
{
	struct palmas_charger_info *di = dev_get_drvdata(dev);
	unsigned int reg;
	int ret;

	ret = palmas_charger_read(di, PALMAS_REG03, &reg);
	if (ret < 0)
		return ret;

	reg &= PALMAS_REG03_IPRECHG_MASK;
	reg >>= PALMAS_REG03_IPRECHG_SHIFT;

	if (di->current_range)
		reg = reg * 128000 + 128000;
	else
		reg = reg * 64000 + 64000;

        return sprintf(buf, "%u\n", reg);
}

static ssize_t set_termination_current(struct device *dev,
        struct device_attribute *attr, const char *buf, size_t count)
{
	struct palmas_charger_info *di = dev_get_drvdata(dev);
	long val;
	int ret;

	if (strict_strtol(buf, 10, &val) < 0)
		return -EINVAL;

	if (di->current_range) {
		/* 128mA - 2048mA in 128mA steps */
		val -= 128000;
		val /= 128000;
	} else {
		/* 64mA - 1024mA in 64mA steps */
		val -= 64000;
		val /= 64000;
	}

	ret = palmas_set_terminationcurrent(di, val);
	if (ret < 0)
		return ret;

	return count;
}

static ssize_t show_termination_current(struct device *dev,
        struct device_attribute *attr, char *buf)
{
	struct palmas_charger_info *di = dev_get_drvdata(dev);
	unsigned int reg;
	int ret;

	ret = palmas_charger_read(di, PALMAS_REG03, &reg);
	if (ret < 0)
		return ret;

	reg &= PALMAS_REG03_ITERM_MASK;

	if (di->current_range)
		reg = reg * 128000 + 128000;
	else
		reg = reg * 64000 + 64000;

        return sprintf(buf, "%u\n", reg);
}

static ssize_t set_batlowv(struct device *dev,
        struct device_attribute *attr, const char *buf, size_t count)
{
	struct palmas_charger_info *di = dev_get_drvdata(dev);
	long val;
	int ret;

	if (strict_strtol(buf, 10, &val) < 0)
		return -EINVAL;

	val -= 2800000;
	val /= 200000;

	ret = palmas_set_batlowv(di, val);
	if (ret < 0)
		return ret;

	return count;
}

static ssize_t show_batlowv(struct device *dev,
        struct device_attribute *attr, char *buf)
{
	struct palmas_charger_info *di = dev_get_drvdata(dev);
	unsigned int reg;
	int ret;

	ret = palmas_charger_read(di, PALMAS_REG04, &reg);
	if (ret < 0)
		return ret;

	reg &= PALMAS_REG04_BATLOWV;

	if (reg)
		reg = 3000000;
	else
		reg = 2800000;

        return sprintf(buf, "%u\n", reg);
}

static ssize_t set_vrechg(struct device *dev,
        struct device_attribute *attr, const char *buf, size_t count)
{
	struct palmas_charger_info *di = dev_get_drvdata(dev);
	long val;
	int ret;

	if (strict_strtol(buf, 10, &val) < 0)
		return -EINVAL;

	val -= 100000;
	val /= 200000;

	ret = palmas_set_vrechg(di, val);
	if (ret < 0)
		return ret;

	return count;
}

static ssize_t show_vrechg(struct device *dev,
        struct device_attribute *attr, char *buf)
{
	struct palmas_charger_info *di = dev_get_drvdata(dev);
	unsigned int reg;
	int ret;

	ret = palmas_charger_read(di, PALMAS_REG04, &reg);
	if (ret < 0)
		return ret;

	reg &= PALMAS_REG04_VRECHG;

	if (reg)
		reg = 300000;
	else
		reg = 100000;

        return sprintf(buf, "%u\n", reg);
}

static DEVICE_ATTR(usb_event, S_IWUSR, NULL, set_usb_event);
static DEVICE_ATTR(regulation_voltage, S_IWUSR | S_IRUGO,
		show_regulation_voltage, set_regulation_voltage);
static DEVICE_ATTR(vsys_minimum, S_IWUSR | S_IRUGO,
		show_vsys_minimum, set_vsys_minimum);
static DEVICE_ATTR(precharge_current, S_IWUSR | S_IRUGO,
		show_precharge_current, set_precharge_current);
static DEVICE_ATTR(termination_current, S_IWUSR | S_IRUGO,
		show_termination_current, set_termination_current);
static DEVICE_ATTR(batlowv, S_IWUSR | S_IRUGO,
		show_batlowv, set_batlowv);
static DEVICE_ATTR(vrechg, S_IWUSR | S_IRUGO,
		show_vrechg, set_vrechg);

static struct attribute *palmas_attributes[] = {
	&dev_attr_usb_event.attr,
	&dev_attr_regulation_voltage.attr,
	&dev_attr_vsys_minimum.attr,
	&dev_attr_precharge_current.attr,
	&dev_attr_termination_current.attr,
	&dev_attr_batlowv.attr,
	&dev_attr_vrechg.attr,
	NULL,
};

static const struct attribute_group palmas_attr_group = {
	.attrs = palmas_attributes,
};

static int palmas_capacity_lookup(struct palmas_charger_info *di, int volt)
{
	int i, table_size;

	table_size = di->bat_chart_size;

	for (i = 1; i < table_size; i++) {
		if (volt < di->bat_chart[i].volt)
			break;
	}

	return di->bat_chart[i-1].cap;
}

static __devinit int palmas_charger_probe(struct platform_device *pdev)
{
	struct palmas *palmas = dev_get_drvdata(pdev->dev.parent);
	struct palmas_charger_platform_data *pdata = pdev->dev.platform_data;
	struct device_node *node = pdev->dev.of_node;
	struct palmas_charger_info *di;
	unsigned int addr, reg;
	int ret = 0, status;

	/* Charger needs GPADC to probe first */
	if (!palmas->gpadc)
		return -EPROBE_DEFER;

	if (node && !pdata) {
		pdata = devm_kzalloc(&pdev->dev, sizeof(*pdata), GFP_KERNEL);

		if (!pdata)
			return -ENOMEM;

		//palmas_dt_to_pdata(&pdev->dev, node, pdata);
	}

	if (!pdata)
		return -EINVAL;

	di = devm_kzalloc(&pdev->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	/* copy the battery chart from pdata */
	di->bat_chart = devm_kzalloc(&pdev->dev,
			sizeof(*pdata->bat_chart) * pdata->bat_chart_size,
					GFP_KERNEL);
	if (!di->bat_chart)
		return -ENOMEM;

	memcpy(di->bat_chart, pdata->bat_chart,
			sizeof(*pdata->bat_chart) * pdata->bat_chart_size);
	di->bat_chart_size = pdata->bat_chart_size;

	/* copy the battery temperature chart from pdata */
	di->battery_temperature_chart = devm_kzalloc(&pdev->dev,
			sizeof(int) * pdata->battery_temperature_chart_size,
					GFP_KERNEL);
	if (!di->battery_temperature_chart)
		return -ENOMEM;

	memcpy(di->battery_temperature_chart, pdata->battery_temperature_chart,
			sizeof(int) * pdata->battery_temperature_chart_size);
	di->battery_temperature_chart_size =
			pdata->battery_temperature_chart_size;

	di->palmas = palmas;
	di->dev = &pdev->dev;
	palmas->charger = di;

	platform_set_drvdata(pdev, di);

	/* Initialise Values from pdata */
	di->watchdog_code = pdata->watchdog_code;

	if (pdata->usb_charge_limit)
		di->usb_charge_limit = pdata->usb_charge_limit;
	else
		di->usb_charge_limit = PALMAS_IINLIM_1200MA;

	di->battery_status_interval = pdata->battery_status_interval;
	di->battery_capacity_max = pdata->battery_capacity_max;

	/* Start with battery health good until we get an IRQ fault */
	di->battery_health = POWER_SUPPLY_HEALTH_GOOD;

	/* Initialise the fuelgauge */

	/* calculate current max scale from sense */
	if (pdata->sense_resistor_mohm) {
		di->current_max_scale = (62000) / pdata->sense_resistor_mohm;
	} else {
		/* Set sensible defaults if platform data is missing */
		di->current_max_scale = 6200;
	}

	palmas_current_setup(di, pdata);

	INIT_DELAYED_WORK(&di->palmas_charger_work,
			palmas_charger_work);
	INIT_DELAYED_WORK(&di->palmas_battery_status_work,
			palmas_battery_charge_status_work);
	INIT_WORK(&di->usb_event_work, palmas_event_work_func);


	di->battery.name = "palmas-battery";
	di->battery.type = POWER_SUPPLY_TYPE_BATTERY;
	di->battery.properties = palmas_battery_props;
	di->battery.num_properties = ARRAY_SIZE(palmas_battery_props);
	di->battery.get_property = palmas_battery_get_props;
	di->battery.external_power_changed = NULL;

	di->usb.name = "palmas-usb";
	di->usb.type = POWER_SUPPLY_TYPE_USB;
	di->usb.properties = palmas_usb_props;
	di->usb.num_properties = ARRAY_SIZE(palmas_usb_props);
	di->usb.get_property = palmas_usb_get_property;
	di->usb.external_power_changed = NULL;

	di->wireless.name = "palmas-wireless";
	di->wireless.type = POWER_SUPPLY_TYPE_MAINS;
	di->wireless.properties = palmas_wireless_props;
	di->wireless.num_properties = ARRAY_SIZE(palmas_wireless_props);
	di->wireless.get_property = palmas_wireless_get_property;
	di->wireless.external_power_changed = NULL;

	palmas_reset(di);

	/* Initial boot capacity */
	palmas_battery_voltage_now(di);
	di->battery_capacity = palmas_capacity_lookup(di,
			di->battery_voltage_uV / 1000);
	di->battery_boot_capacity_mAh = (di->battery_capacity_max *
			di->battery_capacity) / 100;

	ret = power_supply_register(di->dev, &di->battery);
	if (ret) {
		dev_err(di->dev, "failed to register battery: %d\n", ret);
		goto err;
	}

	ret = power_supply_register(di->dev, &di->usb);
	if (ret) {
		dev_err(di->dev, "failed to register usb: %d\n", ret);
		goto err_usb;
	}

	ret = power_supply_register(di->dev, &di->wireless);
	if (ret) {
		dev_err(di->dev, "failed to register wireless: %d\n", ret);
		goto err_wireless;
	}

	status = palmas_request_irq(palmas, PALMAS_CHARGER_IRQ,
				"palmas_charger", palmas_charger_irq, di);
	if (status < 0) {
		dev_err(&pdev->dev, "can't get IRQ %d, err %d\n",
				PALMAS_CHARGER_IRQ, status);
		goto err_irq;
	}

	status = palmas_request_irq(palmas, PALMAS_CHRG_IN_ANTICOLLAPSE_IRQ,
				"palmas_anticollapse", palmas_anticollapse_irq,
				di);
	if (status < 0) {
		dev_err(&pdev->dev, "can't get IRQ %d, err %d\n",
				PALMAS_CHRG_IN_ANTICOLLAPSE_IRQ, status);
		goto err_irq2;
	}

	status = palmas_request_irq(palmas, PALMAS_BAT_TEMP_FAULT_IRQ,
				"palmas_battemp", palmas_bat_temp_fault_irq,
				di);
	if (status < 0) {
		dev_err(&pdev->dev, "can't get IRQ %d, err %d\n",
				PALMAS_BAT_TEMP_FAULT_IRQ, status);
		goto err_irq3;
	}

	/* Initial state of AC line then register IRQ */
	addr = PALMAS_BASE_TO_REG(PALMAS_INTERRUPT_BASE,
			PALMAS_INT2_LINE_STATE);
	regmap_read(palmas->regmap[1], addr, &reg);

	if (reg & PALMAS_INT2_LINE_STATE_VAC_ACOK) {
		di->wireless_online_status = 1;
	} else {
		di->wireless_online_status = 0;
	}

	status = palmas_request_irq(palmas, PALMAS_VAC_ACOK_IRQ,
				"palmas_battemp", palmas_vac_acok_irq,
				di);
	if (status < 0) {
		dev_err(&pdev->dev, "can't get IRQ %d, err %d\n",
				PALMAS_VAC_ACOK_IRQ, status);
		goto err_irq4;
	}

	ret = palmas_batremoval_setup(di, pdata);
	if (ret < 0)
		goto err_irq5;

	/* Set boot status info */
	palmas_charger_irq(PALMAS_CHARGER_IRQ, di);

	/* Schedule the battery status polling */
	schedule_delayed_work(&di->palmas_battery_status_work,
		msecs_to_jiffies(di->battery_status_interval));

	ret = sysfs_create_group(&di->dev->kobj, &palmas_attr_group);

	return 0;

err_irq5:
	palmas_free_irq(palmas, PALMAS_VAC_ACOK_IRQ, di);
err_irq4:
	palmas_free_irq(palmas, PALMAS_BAT_TEMP_FAULT_IRQ, di);
err_irq3:
	palmas_free_irq(palmas, PALMAS_CHRG_IN_ANTICOLLAPSE_IRQ, di);
err_irq2:
	palmas_free_irq(palmas, PALMAS_CHARGER_IRQ, di);
err_irq:
	power_supply_unregister(&di->wireless);
err_wireless:
	power_supply_unregister(&di->usb);
err_usb:
	power_supply_unregister(&di->battery);
err:
	return ret;
}

static int __devexit palmas_charger_remove(struct platform_device *pdev)
{
	struct palmas_charger_info *di = platform_get_drvdata(pdev);
	struct palmas *palmas = di->palmas;

	palmas_reset(di);
	palmas_batremoval_shutdown(di);
	palmas_free_irq(palmas, PALMAS_BAT_TEMP_FAULT_IRQ, di);
	palmas_free_irq(palmas, PALMAS_CHRG_IN_ANTICOLLAPSE_IRQ, di);
	palmas_free_irq(palmas, PALMAS_CHARGER_IRQ, di);
	cancel_delayed_work(&di->palmas_charger_work);
	flush_scheduled_work();
	power_supply_unregister(&di->battery);
	power_supply_unregister(&di->usb);
	power_supply_unregister(&di->wireless);

	return 0;
}

static int palmas_charger_suspend(struct device *dev)
{
	return 0;
}

static int palmas_charger_resume(struct device *dev)
{
	return 0;
}

static const struct dev_pm_ops pm_ops = {
	.suspend	= palmas_charger_suspend,
	.resume		= palmas_charger_resume,
};

static struct of_device_id __devinitdata of_palmas_match_tbl[] = {
	{ .compatible = "ti,palmas-bqcharger", },
	{ /* end */ }
};

static struct platform_driver palmas_charger_driver = {
	.probe = palmas_charger_probe,
	.remove = __devexit_p(palmas_charger_remove),
	.driver = {
		.name = "palmas-bqcharger",
		.of_match_table = of_palmas_match_tbl,
		.owner = THIS_MODULE,
		.pm = &pm_ops,
	},
};

static int __init palmas_charger_init(void)
{
	return platform_driver_register(&palmas_charger_driver);
}
subsys_initcall(palmas_charger_init);

static void __exit palmas_charger_exit(void)
{
	platform_driver_unregister(&palmas_charger_driver);
}
module_exit(palmas_charger_exit);

MODULE_ALIAS("platform:palmas-bqcharger");
MODULE_AUTHOR("Graeme Gregory <gg@slimlogic.co.uk>");
MODULE_DESCRIPTION("Palmas USB transceiver driver");
MODULE_LICENSE("GPL");
MODULE_DEVICE_TABLE(of, of_palmas_match_tbl);
