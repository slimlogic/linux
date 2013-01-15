/*
 * Driver for Regulator part of Palmas PMIC Chips
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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/slab.h>
#include <linux/regmap.h>
#include <linux/mfd/palmas.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/regulator/of_regulator.h>

struct regs_info {
	char	*name;
	u8	vsel_addr;
	u8	ctrl_addr;
	u8	tstep_addr;
	u8	short_bit;
};

static const struct regs_info palmas_regs_info[] = {
	{
		.name		= "SMPS12",
		.vsel_addr	= PALMAS_SMPS12_VOLTAGE,
		.ctrl_addr	= PALMAS_SMPS12_CTRL,
		.tstep_addr	= PALMAS_SMPS12_TSTEP,
		.short_bit	= PALMAS_SMPS_SHORT_STATUS_SMPS12,
	},
	{
		.name		= "SMPS123",
		.vsel_addr	= PALMAS_SMPS12_VOLTAGE,
		.ctrl_addr	= PALMAS_SMPS12_CTRL,
		.tstep_addr	= PALMAS_SMPS12_TSTEP,
	},
	{
		.name		= "SMPS3",
		.vsel_addr	= PALMAS_SMPS3_VOLTAGE,
		.ctrl_addr	= PALMAS_SMPS3_CTRL,
		.short_bit	= PALMAS_SMPS_SHORT_STATUS_SMPS3,
	},
	{
		.name		= "SMPS45",
		.vsel_addr	= PALMAS_SMPS45_VOLTAGE,
		.ctrl_addr	= PALMAS_SMPS45_CTRL,
		.tstep_addr	= PALMAS_SMPS45_TSTEP,
		.short_bit	= PALMAS_SMPS_SHORT_STATUS_SMPS45,
	},
	{
		.name		= "SMPS457",
		.vsel_addr	= PALMAS_SMPS45_VOLTAGE,
		.ctrl_addr	= PALMAS_SMPS45_CTRL,
		.tstep_addr	= PALMAS_SMPS45_TSTEP,
	},
	{
		.name		= "SMPS6",
		.vsel_addr	= PALMAS_SMPS6_VOLTAGE,
		.ctrl_addr	= PALMAS_SMPS6_CTRL,
		.tstep_addr	= PALMAS_SMPS6_TSTEP,
		.short_bit	= PALMAS_SMPS_SHORT_STATUS_SMPS6,
	},
	{
		.name		= "SMPS7",
		.vsel_addr	= PALMAS_SMPS7_VOLTAGE,
		.ctrl_addr	= PALMAS_SMPS7_CTRL,
		.short_bit	= PALMAS_SMPS_SHORT_STATUS_SMPS7,
	},
	{
		.name		= "SMPS8",
		.vsel_addr	= PALMAS_SMPS8_VOLTAGE,
		.ctrl_addr	= PALMAS_SMPS8_CTRL,
		.tstep_addr	= PALMAS_SMPS8_TSTEP,
		.short_bit	= PALMAS_SMPS_SHORT_STATUS_SMPS8,
	},
	{
		.name		= "SMPS9",
		.vsel_addr	= PALMAS_SMPS9_VOLTAGE,
		.ctrl_addr	= PALMAS_SMPS9_CTRL,
		.short_bit	= PALMAS_SMPS_SHORT_STATUS_SMPS9,
	},
	{
		.name		= "SMPS10",
		.short_bit	= PALMAS_SMPS_SHORT_STATUS_SMPS10,
	},
	{
		.name		= "LDO1",
		.vsel_addr	= PALMAS_LDO1_VOLTAGE,
		.ctrl_addr	= PALMAS_LDO1_CTRL,
		.short_bit	= PALMAS_LDO_SHORT_STATUS1_LDO1,
	},
	{
		.name		= "LDO2",
		.vsel_addr	= PALMAS_LDO2_VOLTAGE,
		.ctrl_addr	= PALMAS_LDO2_CTRL,
		.short_bit	= PALMAS_LDO_SHORT_STATUS1_LDO2,
	},
	{
		.name		= "LDO3",
		.vsel_addr	= PALMAS_LDO3_VOLTAGE,
		.ctrl_addr	= PALMAS_LDO3_CTRL,
		.short_bit	= PALMAS_LDO_SHORT_STATUS1_LDO3,
	},
	{
		.name		= "LDO4",
		.vsel_addr	= PALMAS_LDO4_VOLTAGE,
		.ctrl_addr	= PALMAS_LDO4_CTRL,
		.short_bit	= PALMAS_LDO_SHORT_STATUS1_LDO4,
	},
	{
		.name		= "LDO5",
		.vsel_addr	= PALMAS_LDO5_VOLTAGE,
		.ctrl_addr	= PALMAS_LDO5_CTRL,
		.short_bit	= PALMAS_LDO_SHORT_STATUS1_LDO5,
	},
	{
		.name		= "LDO6",
		.vsel_addr	= PALMAS_LDO6_VOLTAGE,
		.ctrl_addr	= PALMAS_LDO6_CTRL,
		.short_bit	= PALMAS_LDO_SHORT_STATUS1_LDO6,
	},
	{
		.name		= "LDO7",
		.vsel_addr	= PALMAS_LDO7_VOLTAGE,
		.ctrl_addr	= PALMAS_LDO7_CTRL,
		.short_bit	= PALMAS_LDO_SHORT_STATUS1_LDO7,
	},
	{
		.name		= "LDO8",
		.vsel_addr	= PALMAS_LDO8_VOLTAGE,
		.ctrl_addr	= PALMAS_LDO8_CTRL,
		.short_bit	= PALMAS_LDO_SHORT_STATUS1_LDO8,
	},
	{
		.name		= "LDO9",
		.vsel_addr	= PALMAS_LDO9_VOLTAGE,
		.ctrl_addr	= PALMAS_LDO9_CTRL,
		.short_bit	= PALMAS_LDO_SHORT_STATUS2_LDO9,
	},
	{
		.name		= "LDOLN",
		.vsel_addr	= PALMAS_LDOLN_VOLTAGE,
		.ctrl_addr	= PALMAS_LDOLN_CTRL,
		.short_bit	= PALMAS_LDO_SHORT_STATUS2_LDOLN,
	},
	{
		.name		= "LDOUSB",
		.vsel_addr	= PALMAS_LDOUSB_VOLTAGE,
		.ctrl_addr	= PALMAS_LDOUSB_CTRL,
		.short_bit	= PALMAS_LDO_SHORT_STATUS2_LDOUSB,
	},
	{
		.name		= "LDO10",
		.vsel_addr	= PALMAS_LDO10_VOLTAGE,
		.ctrl_addr	= PALMAS_LDO10_CTRL,
	},
	{
		.name		= "LDO11",
		.vsel_addr	= PALMAS_LDO11_VOLTAGE,
		.ctrl_addr	= PALMAS_LDO11_CTRL,
	},
	{
		.name		= "LDO12",
		.vsel_addr	= PALMAS_LDO12_VOLTAGE,
		.ctrl_addr	= PALMAS_LDO12_CTRL,
	},
	{
		.name		= "LDO13",
		.vsel_addr	= PALMAS_LDO13_VOLTAGE,
		.ctrl_addr	= PALMAS_LDO13_CTRL,
	},
	{
		.name		= "LDO14",
		.vsel_addr	= PALMAS_LDO14_VOLTAGE,
		.ctrl_addr	= PALMAS_LDO14_CTRL,
	},
	{
		.name		= "BOOST",
	},
};

static const int palmas_smps_regulators[] = {
	PALMAS_REG_SMPS12,
	PALMAS_REG_SMPS123,
	PALMAS_REG_SMPS3,
	PALMAS_REG_SMPS45,
	PALMAS_REG_SMPS457,
	PALMAS_REG_SMPS6,
	PALMAS_REG_SMPS7,
	PALMAS_REG_SMPS8,
	PALMAS_REG_SMPS9,
	PALMAS_REG_SMPS10,
};

static const int palmas_ldo_regulators[] = {
	/* LDO regulators */
	PALMAS_REG_LDO1,
	PALMAS_REG_LDO2,
	PALMAS_REG_LDO3,
	PALMAS_REG_LDO4,
	PALMAS_REG_LDO5,
	PALMAS_REG_LDO6,
	PALMAS_REG_LDO7,
	PALMAS_REG_LDO8,
	PALMAS_REG_LDO9,
	PALMAS_REG_LDOLN,
	PALMAS_REG_LDOUSB,
};

static const int palmas_charger_smps_regulators[] = {
	/* SMPS regulators */
	PALMAS_REG_SMPS12,
	PALMAS_REG_SMPS123,
	PALMAS_REG_SMPS3,
	PALMAS_REG_SMPS6,
	PALMAS_REG_SMPS7,
	PALMAS_REG_SMPS8,
	PALMAS_REG_SMPS9,
};

static const int palmas_charger_ldo_regulators[] = {
	PALMAS_REG_LDO1,
	PALMAS_REG_LDO2,
	PALMAS_REG_LDO3,
	PALMAS_REG_LDO4,
	PALMAS_REG_LDO5,
	PALMAS_REG_LDO6,
	PALMAS_REG_LDO7,
	PALMAS_REG_LDO8,
	PALMAS_REG_LDO9,
	PALMAS_REG_LDOLN,
	PALMAS_REG_LDOUSB,
	PALMAS_REG_LDO10,
	PALMAS_REG_LDO11,
	PALMAS_REG_LDO12,
	PALMAS_REG_LDO13,
	PALMAS_REG_LDO14,
};

static const int palmas_charger_special_regulators[] = {
	PALMAS_REG_BOOST,
};

#define SMPS_CTRL_MODE_OFF		0x00
#define SMPS_CTRL_MODE_ON		0x01
#define SMPS_CTRL_MODE_ECO		0x02
#define SMPS_CTRL_MODE_PWM		0x03

/* These values are derived from the data sheet. And are the number of steps
 * where there is a voltage change, the ranges at beginning and end of register
 * max/min values where there are no change are ommitted.
 *
 * So they are basically (maxV-minV)/stepV
 */
#define PALMAS_SMPS_NUM_VOLTAGES	117
#define PALMAS_SMPS10_NUM_VOLTAGES	2
#define PALMAS_LDO_NUM_VOLTAGES		50

#define SMPS10_VSEL			(1<<3)
#define SMPS10_BOOST_EN			(1<<2)
#define SMPS10_BYPASS_EN		(1<<1)
#define SMPS10_SWITCH_EN		(1<<0)

#define REGULATOR_SLAVE			0

static int palmas_smps_read(struct palmas *palmas, unsigned int reg,
		unsigned int *dest)
{
	unsigned int addr;

	addr = PALMAS_BASE_TO_REG(PALMAS_SMPS_BASE, reg);

	return regmap_read(palmas->regmap[REGULATOR_SLAVE], addr, dest);
}

static int palmas_smps_write(struct palmas *palmas, unsigned int reg,
		unsigned int value)
{
	unsigned int addr;

	addr = PALMAS_BASE_TO_REG(PALMAS_SMPS_BASE, reg);

	return regmap_write(palmas->regmap[REGULATOR_SLAVE], addr, value);
}

static int palmas_ldo_read(struct palmas *palmas, unsigned int reg,
		unsigned int *dest)
{
	unsigned int addr;

	addr = PALMAS_BASE_TO_REG(PALMAS_LDO_BASE, reg);

	return regmap_read(palmas->regmap[REGULATOR_SLAVE], addr, dest);
}

static int palmas_ldo_write(struct palmas *palmas, unsigned int reg,
		unsigned int value)
{
	unsigned int addr;

	addr = PALMAS_BASE_TO_REG(PALMAS_LDO_BASE, reg);

	return regmap_write(palmas->regmap[REGULATOR_SLAVE], addr, value);
}

static int palmas_is_enabled_smps(struct regulator_dev *dev)
{
	struct palmas_pmic *pmic = rdev_get_drvdata(dev);
	int id = rdev_get_id(dev);
	unsigned int reg;

	palmas_smps_read(pmic->palmas, palmas_regs_info[id].ctrl_addr, &reg);

	reg &= PALMAS_SMPS12_CTRL_STATUS_MASK;
	reg >>= PALMAS_SMPS12_CTRL_STATUS_SHIFT;

	return !!(reg);
}

static int palmas_enable_smps(struct regulator_dev *dev)
{
	struct palmas_pmic *pmic = rdev_get_drvdata(dev);
	int id = rdev_get_id(dev);
	unsigned int reg;

	palmas_smps_read(pmic->palmas, palmas_regs_info[id].ctrl_addr, &reg);

	reg &= ~PALMAS_SMPS12_CTRL_MODE_ACTIVE_MASK;
	reg |= SMPS_CTRL_MODE_ON;

	palmas_smps_write(pmic->palmas, palmas_regs_info[id].ctrl_addr, reg);

	return 0;
}

static int palmas_disable_smps(struct regulator_dev *dev)
{
	struct palmas_pmic *pmic = rdev_get_drvdata(dev);
	int id = rdev_get_id(dev);
	unsigned int reg;

	palmas_smps_read(pmic->palmas, palmas_regs_info[id].ctrl_addr, &reg);

	reg &= ~PALMAS_SMPS12_CTRL_MODE_ACTIVE_MASK;

	palmas_smps_write(pmic->palmas, palmas_regs_info[id].ctrl_addr, reg);

	return 0;
}


static int palmas_set_mode_smps(struct regulator_dev *dev, unsigned int mode)
{
	struct palmas_pmic *pmic = rdev_get_drvdata(dev);
	int id = rdev_get_id(dev);
	unsigned int reg;

	palmas_smps_read(pmic->palmas, palmas_regs_info[id].ctrl_addr, &reg);
	reg &= ~PALMAS_SMPS12_CTRL_MODE_ACTIVE_MASK;

	switch (mode) {
	case REGULATOR_MODE_NORMAL:
		reg |= SMPS_CTRL_MODE_ON;
		break;
	case REGULATOR_MODE_IDLE:
		reg |= SMPS_CTRL_MODE_ECO;
		break;
	case REGULATOR_MODE_FAST:
		reg |= SMPS_CTRL_MODE_PWM;
		break;
	default:
		return -EINVAL;
	}
	palmas_smps_write(pmic->palmas, palmas_regs_info[id].ctrl_addr, reg);

	return 0;
}

static unsigned int palmas_get_mode_smps(struct regulator_dev *dev)
{
	struct palmas_pmic *pmic = rdev_get_drvdata(dev);
	int id = rdev_get_id(dev);
	unsigned int reg;

	palmas_smps_read(pmic->palmas, palmas_regs_info[id].ctrl_addr, &reg);
	reg &= PALMAS_SMPS12_CTRL_STATUS_MASK;
	reg >>= PALMAS_SMPS12_CTRL_STATUS_SHIFT;

	switch (reg) {
	case SMPS_CTRL_MODE_ON:
		return REGULATOR_MODE_NORMAL;
	case SMPS_CTRL_MODE_ECO:
		return REGULATOR_MODE_IDLE;
	case SMPS_CTRL_MODE_PWM:
		return REGULATOR_MODE_FAST;
	}

	return 0;
}

static int palmas_list_voltage_smps(struct regulator_dev *dev,
					unsigned selector)
{
	struct palmas_pmic *pmic = rdev_get_drvdata(dev);
	int id = rdev_get_id(dev);
	int mult = 1;

	if (!selector)
		return 0;

	/* Read the multiplier set in VSEL register to return
	 * the correct voltage.
	 */
	if (pmic->range[id])
		mult = 2;

	/* Voltage is (0.49V + (selector * 0.01V)) * RANGE
	 * as defined in data sheet. RANGE is either x1 or x2
	 */
	return  (490000 + (selector * 10000)) * mult;
}

static int palmas_get_voltage_smps_sel(struct regulator_dev *dev)
{
	struct palmas_pmic *pmic = rdev_get_drvdata(dev);
	int id = rdev_get_id(dev);
	int selector;
	unsigned int reg;
	unsigned int addr;

	addr = palmas_regs_info[id].vsel_addr;

	palmas_smps_read(pmic->palmas, addr, &reg);

	selector = reg & PALMAS_SMPS12_VOLTAGE_VSEL_MASK;

	/* Adjust selector to match list_voltage ranges */
	if ((selector > 0) && (selector < 6))
		selector = 6;
	if (!selector)
		selector = 5;
	if (selector > 121)
		selector = 121;
	selector -= 5;

	return selector;
}

static int palmas_set_voltage_smps_sel(struct regulator_dev *dev,
		unsigned selector)
{
	struct palmas_pmic *pmic = rdev_get_drvdata(dev);
	int id = rdev_get_id(dev);
	unsigned int reg = 0;
	unsigned int addr;

	addr = palmas_regs_info[id].vsel_addr;

	/* Make sure we don't change the value of RANGE */
	if (pmic->range[id])
		reg |= PALMAS_SMPS12_VOLTAGE_RANGE;

	/* Adjust the linux selector into range used in VSEL register */
	if (selector)
		reg |= selector + 5;

	palmas_smps_write(pmic->palmas, addr, reg);

	return 0;
}

static int palmas_suspend_enable_smps(struct regulator_dev *dev)
{
	struct palmas_pmic *pmic = rdev_get_drvdata(dev);
	int id = rdev_get_id(dev);
	unsigned int reg;

	palmas_smps_read(pmic->palmas, palmas_regs_info[id].ctrl_addr, &reg);

	reg &= ~PALMAS_SMPS12_CTRL_MODE_SLEEP_MASK;
	reg |= SMPS_CTRL_MODE_ON << PALMAS_SMPS12_CTRL_MODE_SLEEP_SHIFT;

	palmas_smps_write(pmic->palmas, palmas_regs_info[id].ctrl_addr, reg);

	return 0;
}

static int palmas_suspend_disable_smps(struct regulator_dev *dev)
{
	struct palmas_pmic *pmic = rdev_get_drvdata(dev);
	int id = rdev_get_id(dev);
	unsigned int reg;

	palmas_smps_read(pmic->palmas, palmas_regs_info[id].ctrl_addr, &reg);

	reg &= ~PALMAS_SMPS12_CTRL_MODE_SLEEP_MASK;

	palmas_smps_write(pmic->palmas, palmas_regs_info[id].ctrl_addr, reg);

	return 0;
}

static int palmas_map_voltage_smps(struct regulator_dev *rdev,
		int min_uV, int max_uV)
{
	struct palmas_pmic *pmic = rdev_get_drvdata(rdev);
	int id = rdev_get_id(rdev);
	int ret, voltage;

	if (min_uV == 0)
		return 0;

	if (pmic->range[id]) { /* RANGE is x2 */
		if (min_uV < 1000000)
			min_uV = 1000000;
		ret = DIV_ROUND_UP(min_uV - 1000000, 20000) + 1;
	} else {		/* RANGE is x1 */
		if (min_uV < 500000)
			min_uV = 500000;
		ret = DIV_ROUND_UP(min_uV - 500000, 10000) + 1;
	}

	/* Map back into a voltage to verify we're still in bounds */
	voltage = palmas_list_voltage_smps(rdev, ret);
	if (voltage < min_uV || voltage > max_uV)
		return -EINVAL;

	return ret;
}

static int palmas_suspend_enable_ldo(struct regulator_dev *dev)
{
	struct palmas_pmic *pmic = rdev_get_drvdata(dev);
	int id = rdev_get_id(dev);
	unsigned int reg;

	palmas_ldo_read(pmic->palmas, palmas_regs_info[id].ctrl_addr, &reg);

	reg |= PALMAS_LDO1_CTRL_MODE_SLEEP;

	palmas_ldo_write(pmic->palmas, palmas_regs_info[id].ctrl_addr, reg);

	return 0;
}

static int palmas_suspend_disable_ldo(struct regulator_dev *dev)
{
	struct palmas_pmic *pmic = rdev_get_drvdata(dev);
	int id = rdev_get_id(dev);
	unsigned int reg;

	palmas_ldo_read(pmic->palmas, palmas_regs_info[id].ctrl_addr, &reg);

	reg &= ~PALMAS_LDO1_CTRL_MODE_SLEEP;

	palmas_ldo_write(pmic->palmas, palmas_regs_info[id].ctrl_addr, reg);

	return 0;
}

static struct regulator_ops palmas_ops_smps = {
	.is_enabled		= palmas_is_enabled_smps,
	.enable			= palmas_enable_smps,
	.disable		= palmas_disable_smps,
	.set_mode		= palmas_set_mode_smps,
	.get_mode		= palmas_get_mode_smps,
	.get_voltage_sel	= palmas_get_voltage_smps_sel,
	.set_voltage_sel	= palmas_set_voltage_smps_sel,
	.list_voltage		= palmas_list_voltage_smps,
	.map_voltage		= palmas_map_voltage_smps,
	.set_suspend_enable	= palmas_suspend_enable_smps,
	.set_suspend_disable	= palmas_suspend_disable_smps,
};

/* @brief set or clear the bypass bit on SMPS10
 *
 * There is not a way to represent this function within the regulator
 * framework. This sets/clears the bypass of SMPS10 so voltage is obtained
 * from either SMPS10_IN or BOOST.
 *
 * @param palmas pointer to the palmas mfd structure
 * @param bypass boolean to indicate switch status
 * @return error or result
 */
int palmas_set_bypass_smps10(struct palmas *palmas, int bypass)
{
	unsigned int reg;

	palmas_smps_read(palmas, PALMAS_SMPS10_CTRL, &reg);

	if (bypass)
		reg |= SMPS10_BYPASS_EN;
	else
		reg &= ~SMPS10_BYPASS_EN;

	palmas_smps_write(palmas, PALMAS_SMPS10_CTRL, reg);

	return 0;
}
EXPORT_SYMBOL(palmas_set_bypass_smps10);

/* @brief set or clear the switch bit on SMPS10
 *
 * There is not a way to represent this function within the regulator
 * framework. This sets/clears the switch of SMPS10 so SMPS10_OUT1 and
 * SMPS10_OUT2 are shorted together.
 *
 * @param palmas pointer to the palmas mfd structure
 * @param sw boolean to indicate switch status
 * @return error or result
 */
int palmas_set_switch_smps10(struct palmas *palmas, int sw)
{
	unsigned int reg;

	palmas_smps_read(palmas, PALMAS_SMPS10_CTRL, &reg);

	if (sw)
		reg |= SMPS10_SWITCH_EN;
	else
		reg &= ~SMPS10_SWITCH_EN;

	palmas_smps_write(palmas, PALMAS_SMPS10_CTRL, reg);

	return 0;
}
EXPORT_SYMBOL(palmas_set_switch_smps10);

static struct regulator_ops palmas_ops_smps10 = {
	.is_enabled		= regulator_is_enabled_regmap,
	.enable			= regulator_enable_regmap,
	.disable		= regulator_disable_regmap,
	.get_voltage_sel	= regulator_get_voltage_sel_regmap,
	.set_voltage_sel	= regulator_set_voltage_sel_regmap,
	.list_voltage		= regulator_list_voltage_linear,
	.map_voltage		= regulator_map_voltage_linear,
};

static int palmas_is_enabled_ldo(struct regulator_dev *dev)
{
	struct palmas_pmic *pmic = rdev_get_drvdata(dev);
	int id = rdev_get_id(dev);
	unsigned int reg;

	palmas_ldo_read(pmic->palmas, palmas_regs_info[id].ctrl_addr, &reg);

	reg &= PALMAS_LDO1_CTRL_STATUS;

	return !!(reg);
}

static int palmas_list_voltage_ldo(struct regulator_dev *dev,
					unsigned selector)
{
	if (!selector)
		return 0;

	/* voltage is 0.85V + (selector * 0.05v) */
	return  850000 + (selector * 50000);
}

static int palmas_map_voltage_ldo(struct regulator_dev *rdev,
		int min_uV, int max_uV)
{
	int ret, voltage;

	if (min_uV == 0)
		return 0;

	if (min_uV < 900000)
		min_uV = 900000;
	ret = DIV_ROUND_UP(min_uV - 900000, 50000) + 1;

	/* Map back into a voltage to verify we're still in bounds */
	voltage = palmas_list_voltage_ldo(rdev, ret);
	if (voltage < min_uV || voltage > max_uV)
		return -EINVAL;

	return ret;
}

static struct regulator_ops palmas_ops_ldo = {
	.is_enabled		= palmas_is_enabled_ldo,
	.enable			= regulator_enable_regmap,
	.disable		= regulator_disable_regmap,
	.get_voltage_sel	= regulator_get_voltage_sel_regmap,
	.set_voltage_sel	= regulator_set_voltage_sel_regmap,
	.list_voltage		= palmas_list_voltage_ldo,
	.map_voltage		= palmas_map_voltage_ldo,
	.set_suspend_enable	= palmas_suspend_enable_ldo,
	.set_suspend_disable	= palmas_suspend_disable_ldo,
};

static int palmas_enable_booost(struct regulator_dev *dev)
{
	struct palmas_pmic *pmic = rdev_get_drvdata(dev);
	unsigned int addr;

	addr = PALMAS_BASE_TO_REG(PALMAS_PMU_CONTROL_BASE, PALMAS_CHRG_CTRL2);

	regmap_update_bits(pmic->palmas->regmap[0], addr,
			PALMAS_CHRG_CTRL2_BOOST_EN,
			PALMAS_CHRG_CTRL2_BOOST_EN);

	addr = PALMAS_BASE_TO_REG(PALMAS_BQ24192_BASE, PALMAS_REG01);

	regmap_update_bits(pmic->palmas->regmap[3], addr,
			PALMAS_REG01_CHG_CONFIG_MASK,
			0x2 << PALMAS_REG01_CHG_CONFIG_SHIFT);

	return 0;
}

static int palmas_disable_boost(struct regulator_dev *dev)
{
	struct palmas_pmic *pmic = rdev_get_drvdata(dev);
	unsigned int addr;

	addr = PALMAS_BASE_TO_REG(PALMAS_PMU_CONTROL_BASE, PALMAS_CHRG_CTRL2);

	regmap_update_bits(pmic->palmas->regmap[0], addr,
			PALMAS_CHRG_CTRL2_BOOST_EN, 0);

	addr = PALMAS_BASE_TO_REG(PALMAS_BQ24192_BASE, PALMAS_REG01);

	regmap_update_bits(pmic->palmas->regmap[3], addr,
			PALMAS_REG01_CHG_CONFIG_MASK, 0);

	return 0;
}

static int palmas_is_enabled_boost(struct regulator_dev *dev)
{
	struct palmas_pmic *pmic = rdev_get_drvdata(dev);
	unsigned int addr, reg1, reg2;

	addr = PALMAS_BASE_TO_REG(PALMAS_PMU_CONTROL_BASE, PALMAS_CHRG_CTRL2);

	regmap_read(pmic->palmas->regmap[0], addr, &reg1);

	addr = PALMAS_BASE_TO_REG(PALMAS_BQ24192_BASE, PALMAS_REG01);

	regmap_read(pmic->palmas->regmap[3], addr, &reg2);

	reg1 >>= PALMAS_CHRG_CTRL2_BOOST_EN_SHIFT;
	reg2 >>= PALMAS_REG01_CHG_CONFIG_SHIFT;

	return (reg1 && (reg2 & 0x02));
}

static int palmas_set_current_limit_boost(struct regulator_dev *dev,
		int min_uA, int max_uA)
{
	struct palmas_pmic *pmic = rdev_get_drvdata(dev);
	unsigned int addr;

	addr = PALMAS_BASE_TO_REG(PALMAS_BQ24192_BASE, PALMAS_REG01);

	if ((min_uA <= 500000) && (max_uA >= 500000))
	{
		regmap_update_bits(pmic->palmas->regmap[3], addr,
				PALMAS_REG01_BOOST_LIM, 0);
		return 0;
	} else if ((min_uA <=1300000) && (max_uA >=1300000)) {
		regmap_update_bits(pmic->palmas->regmap[3], addr,
				PALMAS_REG01_BOOST_LIM, 1);
		return 0;
	}

	return -EINVAL;
}

static int palmas_get_current_limit_boost(struct regulator_dev *dev)
{
	struct palmas_pmic *pmic = rdev_get_drvdata(dev);
	unsigned int addr, reg;

	addr = PALMAS_BASE_TO_REG(PALMAS_BQ24192_BASE, PALMAS_REG01);

	regmap_read(pmic->palmas->regmap[3], addr, &reg);

	if (reg & PALMAS_REG01_BOOST_LIM)
		return 1300000;

	return 500000;
}

static struct regulator_ops palmas_charger_ops_boost = {
	.list_voltage		= regulator_list_voltage_linear,
	.enable			= palmas_enable_booost,
	.disable		= palmas_disable_boost,
	.is_enabled		= palmas_is_enabled_boost,
	.set_current_limit	= palmas_set_current_limit_boost,
	.get_current_limit	= palmas_get_current_limit_boost,
};

/*
 * setup the hardware based sleep configuration of the SMPS/LDO regulators
 * from the platform data. This is different to the software based control
 * supported by the regulator framework as it is controlled by toggling
 * pins on the PMIC such as PREQ, SYSEN, ...
 */
static int palmas_smps_init(struct palmas *palmas, int id,
		struct palmas_reg_init *reg_init)
{
	unsigned int reg;
	unsigned int addr;
	int ret;

	addr = palmas_regs_info[id].ctrl_addr;

	ret = palmas_smps_read(palmas, addr, &reg);
	if (ret)
		return ret;

	switch (id) {
	case PALMAS_REG_SMPS10:
		if (reg_init->mode_sleep) {
			reg &= ~PALMAS_SMPS10_CTRL_MODE_SLEEP_MASK;
			reg |= reg_init->mode_sleep <<
					PALMAS_SMPS10_CTRL_MODE_SLEEP_SHIFT;
		}
		break;
	default:
		if (reg_init->warm_reset)
			reg |= PALMAS_SMPS12_CTRL_WR_S;

		if (reg_init->roof_floor)
			reg |= PALMAS_SMPS12_CTRL_ROOF_FLOOR_EN;

		if (reg_init->mode_sleep) {
			reg &= ~PALMAS_SMPS12_CTRL_MODE_SLEEP_MASK;
			reg |= reg_init->mode_sleep <<
					PALMAS_SMPS12_CTRL_MODE_SLEEP_SHIFT;
		}
	}

	ret = palmas_smps_write(palmas, addr, reg);
	if (ret)
		return ret;

	if (palmas_regs_info[id].tstep_addr && reg_init->tstep) {
		addr = palmas_regs_info[id].tstep_addr;

		reg = reg_init->tstep & PALMAS_SMPS12_TSTEP_TSTEP_MASK;

		ret = palmas_smps_write(palmas, addr, reg);
		if (ret)
			return ret;
	}

	if (palmas_regs_info[id].vsel_addr && reg_init->vsel) {
		addr = palmas_regs_info[id].vsel_addr;

		reg = reg_init->vsel;

		ret = palmas_smps_write(palmas, addr, reg);
		if (ret)
			return ret;
	}


	return 0;
}

static int palmas_ldo_init(struct palmas *palmas, int id,
		struct palmas_reg_init *reg_init)
{
	unsigned int reg;
	unsigned int addr;
	int ret;

	addr = palmas_regs_info[id].ctrl_addr;

	ret = palmas_ldo_read(palmas, addr, &reg);
	if (ret)
		return ret;

	if (reg_init->warm_reset)
		reg |= PALMAS_LDO1_CTRL_WR_S;

	if (reg_init->mode_sleep)
		reg |= PALMAS_LDO1_CTRL_MODE_SLEEP;

	ret = palmas_ldo_write(palmas, addr, reg);
	if (ret)
		return ret;

	return 0;
}

static struct of_regulator_match palmas_matches[] = {
	{ .name = "smps12", },
	{ .name = "smps123", },
	{ .name = "smps3", },
	{ .name = "smps45", },
	{ .name = "smps457", },
	{ .name = "smps6", },
	{ .name = "smps7", },
	{ .name = "smps8", },
	{ .name = "smps9", },
	{ .name = "smps10", },
	{ .name = "ldo1", },
	{ .name = "ldo2", },
	{ .name = "ldo3", },
	{ .name = "ldo4", },
	{ .name = "ldo5", },
	{ .name = "ldo6", },
	{ .name = "ldo7", },
	{ .name = "ldo8", },
	{ .name = "ldo9", },
	{ .name = "ldoln", },
	{ .name = "ldousb", },
	{ .name = "ldo10", },
	{ .name = "ldo11", },
	{ .name = "ldo12", },
	{ .name = "ldo13", },
	{ .name = "ldo14", },
	{ .name = "boost", },
};

static void __devinit palmas_dt_to_pdata(struct device *dev,
		struct device_node *node,
		struct palmas_pmic_platform_data *pdata)
{
	struct device_node *regulators;
	u32 prop;
	int idx, ret;

	regulators = of_find_node_by_name(node, "regulators");
	if (!regulators) {
		dev_info(dev, "regulator node not found\n");
		return;
	}

	ret = of_regulator_match(dev, regulators, palmas_matches,
			PALMAS_NUM_REGS);
	if (ret < 0) {
		dev_err(dev, "Error parsing regulator init data: %d\n", ret);
		return;
	}

	for (idx = 0; idx < PALMAS_NUM_REGS; idx++) {
		if (!palmas_matches[idx].init_data ||
				!palmas_matches[idx].of_node)
			continue;

		pdata->reg_data[idx] = palmas_matches[idx].init_data;

		pdata->reg_init[idx] = devm_kzalloc(dev,
				sizeof(struct palmas_reg_init), GFP_KERNEL);

		ret = of_property_read_u32(palmas_matches[idx].of_node,
				"ti,warm_reset", &prop);
		if (!ret)
			pdata->reg_init[idx]->warm_reset = prop;

		ret = of_property_read_u32(palmas_matches[idx].of_node,
				"ti,roof_floor", &prop);
		if (!ret)
			pdata->reg_init[idx]->roof_floor = prop;

		ret = of_property_read_u32(palmas_matches[idx].of_node,
				"ti,mode_sleep", &prop);
		if (!ret)
			pdata->reg_init[idx]->mode_sleep = prop;

		ret = of_property_read_u32(palmas_matches[idx].of_node,
				"ti,warm_reset", &prop);
		if (!ret)
			pdata->reg_init[idx]->warm_reset = prop;

		ret = of_property_read_u32(palmas_matches[idx].of_node,
				"ti,tstep", &prop);
		if (!ret)
			pdata->reg_init[idx]->tstep = prop;

		ret = of_property_read_u32(palmas_matches[idx].of_node,
				"ti,vsel", &prop);
		if (!ret)
			pdata->reg_init[idx]->vsel = prop;
	}

	ret = of_property_read_u32(node, "ti,ldo6_vibrator", &prop);
	if (!ret)
		pdata->ldo6_vibrator = prop;
}


static __devinit int palmas_probe(struct platform_device *pdev)
{
	struct palmas *palmas = dev_get_drvdata(pdev->dev.parent);
	struct palmas_pmic_platform_data *pdata = pdev->dev.platform_data;
	struct device_node *node = pdev->dev.of_node;
	struct regulator_dev *rdev;
	struct regulator_config config = { };
	struct palmas_pmic *pmic;
	struct palmas_reg_init *reg_init;
	int id = 0, i, ret;
	unsigned int addr, reg;
	const int *smps_regs = NULL, *ldo_regs = NULL, *spec_regs = NULL;
	int smps_size = 0, ldo_size = 0, spec_size = 0;

	if (node && !pdata) {
		pdata = devm_kzalloc(&pdev->dev, sizeof(*pdata), GFP_KERNEL);

		if (!pdata)
			return -ENOMEM;

		palmas_dt_to_pdata(&pdev->dev, node, pdata);
	}

	pmic = devm_kzalloc(&pdev->dev, sizeof(*pmic), GFP_KERNEL);
	if (!pmic)
		return -ENOMEM;

	pmic->dev = &pdev->dev;
	pmic->palmas = palmas;
	palmas->pmic = pmic;
	platform_set_drvdata(pdev, pmic);

	ret = palmas_smps_read(palmas, PALMAS_SMPS_CTRL, &reg);
	if (ret)
		return ret;

	switch(palmas->id) {
	case PALMAS_CHIP_ID:
	case PALMAS_CHIP_OLD_ID:
		if (reg & PALMAS_SMPS_CTRL_SMPS12_SMPS123_EN)
			pmic->smps123 = 1;

		if (reg & PALMAS_SMPS_CTRL_SMPS45_SMPS457_EN)
			pmic->smps457 = 1;

		smps_regs = palmas_smps_regulators;
		smps_size = ARRAY_SIZE(palmas_smps_regulators);

		ldo_regs = palmas_ldo_regulators;
		ldo_size = ARRAY_SIZE(palmas_ldo_regulators);

		spec_regs = NULL;
		spec_size = 0;
		break;
	case PALMAS_CHIP_CHARGER_ID:
		if (reg & PALMAS_SMPS_CTRL_SMPS12_SMPS123_EN)
			pmic->smps123 = 1;

		smps_regs = palmas_charger_smps_regulators;
		smps_size = ARRAY_SIZE(palmas_charger_smps_regulators);
		ldo_regs = palmas_charger_ldo_regulators;
		ldo_size = ARRAY_SIZE(palmas_charger_ldo_regulators);
		spec_regs = palmas_charger_special_regulators;
		spec_size = ARRAY_SIZE(palmas_charger_special_regulators);
	}

	config.regmap = palmas->regmap[REGULATOR_SLAVE];
	config.dev = &pdev->dev;
	config.driver_data = pmic;

	/* Process the SMPS type regulators */
	for (i = 0; i < smps_size; i++) {
		id = smps_regs[i];

		/*
		 * Miss out regulators which are not available due
		 * to slaving configurations.
		 */
		switch (id) {
		case PALMAS_REG_SMPS12:
		case PALMAS_REG_SMPS3:
			if (pmic->smps123)
				continue;
			break;
		case PALMAS_REG_SMPS123:
			if (!pmic->smps123)
				continue;
			break;
		case PALMAS_REG_SMPS45:
		case PALMAS_REG_SMPS7:
			if (pmic->smps457)
				continue;
			break;
		case PALMAS_REG_SMPS457:
			if (!pmic->smps457)
				continue;
		}

		/* Register the regulators */
		pmic->desc[id].name = palmas_regs_info[id].name;
		pmic->desc[id].id = id;

		switch (id) {
		case PALMAS_REG_SMPS10:
			pmic->desc[id].n_voltages = PALMAS_SMPS10_NUM_VOLTAGES;
			pmic->desc[id].ops = &palmas_ops_smps10;
			pmic->desc[id].vsel_reg =
					PALMAS_BASE_TO_REG(PALMAS_SMPS_BASE,
							PALMAS_SMPS10_CTRL);
			pmic->desc[id].vsel_mask = SMPS10_VSEL;
			pmic->desc[id].enable_reg =
					PALMAS_BASE_TO_REG(PALMAS_SMPS_BASE,
							PALMAS_SMPS10_STATUS);
			pmic->desc[id].enable_mask = SMPS10_BOOST_EN;
			pmic->desc[id].min_uV = 3750000;
			pmic->desc[id].uV_step = 1250000;
			break;
		default:
			pmic->desc[id].ops = &palmas_ops_smps;
			pmic->desc[id].n_voltages = PALMAS_SMPS_NUM_VOLTAGES;
		}

		pmic->desc[id].type = REGULATOR_VOLTAGE;
		pmic->desc[id].owner = THIS_MODULE;

		/* Initialise sleep/init values from platform data */
		if (pdata) {
			reg_init = pdata->reg_init[id];
			if (reg_init) {
				ret = palmas_smps_init(palmas, id, reg_init);
				if (ret)
					goto err_unregister_regulator;
			}
		}

		/*
		 * read and store the RANGE bit for later use
		 * This must be done before regulator is probed otherwise
		 * we error in probe with unsuportable ranges.
		 */
		if (id != PALMAS_REG_SMPS10) {
			addr = palmas_regs_info[id].vsel_addr;

			ret = palmas_smps_read(pmic->palmas, addr, &reg);
			if (ret)
				goto err_unregister_regulator;
			if (reg & PALMAS_SMPS12_VOLTAGE_RANGE)
				pmic->range[id] = 1;
		}

		if (pdata)
			config.init_data = pdata->reg_data[id];
		else
			config.init_data = NULL;

		config.of_node = palmas_matches[id].of_node;

		rdev = regulator_register(&pmic->desc[id], &config);
		if (IS_ERR(rdev)) {
			dev_err(&pdev->dev,
				"failed to register %s regulator\n",
				pdev->name);
			ret = PTR_ERR(rdev);
			goto err_unregister_regulator;
		}

		/* Save regulator for cleanup */
		pmic->rdev[id] = rdev;
	}

	/* Process the LDO type regulators */
	for (i = 0; i < ldo_size; i++) {
		id = ldo_regs[i];

		/* Miss out regulators which are not available due
		 * to alternate functions.
		 */

		/* Register the regulators */
		pmic->desc[id].name = palmas_regs_info[id].name;
		pmic->desc[id].id = id;
		pmic->desc[id].n_voltages = PALMAS_LDO_NUM_VOLTAGES;

		pmic->desc[id].ops = &palmas_ops_ldo;

		pmic->desc[id].type = REGULATOR_VOLTAGE;
		pmic->desc[id].owner = THIS_MODULE;
		pmic->desc[id].vsel_reg = PALMAS_BASE_TO_REG(PALMAS_LDO_BASE,
						palmas_regs_info[id].vsel_addr);
		pmic->desc[id].vsel_mask = PALMAS_LDO1_VOLTAGE_VSEL_MASK;
		pmic->desc[id].enable_reg = PALMAS_BASE_TO_REG(PALMAS_LDO_BASE,
						palmas_regs_info[id].ctrl_addr);
		pmic->desc[id].enable_mask = PALMAS_LDO1_CTRL_MODE_ACTIVE;

		if (pdata)
			config.init_data = pdata->reg_data[id];
		else
			config.init_data = NULL;

		config.of_node = palmas_matches[id].of_node;

		rdev = regulator_register(&pmic->desc[id], &config);
		if (IS_ERR(rdev)) {
			dev_err(&pdev->dev,
				"failed to register %s regulator\n",
				pdev->name);
			ret = PTR_ERR(rdev);
			goto err_unregister_regulator;
		}

		/* Save regulator for cleanup */
		pmic->rdev[id] = rdev;

		/* Initialise sleep/init values from platform data */
		if (pdata) {
			reg_init = pdata->reg_init[id];
			if (reg_init) {
				ret = palmas_ldo_init(palmas, id, reg_init);
				if (ret)
					goto err_unregister_regulator;
			}
		}
	}

	/* Process the Special type regulators */
	for (i = 0; i < spec_size; i++) {
		id = spec_regs[i];

		pmic->desc[id].name = palmas_regs_info[id].name;
		pmic->desc[id].id = id;

		switch (id) {
		case PALMAS_REG_BOOST:
			pmic->desc[id].n_voltages = 1;
			pmic->desc[id].ops = &palmas_charger_ops_boost;
			pmic->desc[id].min_uV = 5000000;
			break;
		}

		if (pdata)
			config.init_data = pdata->reg_data[id];
		else
			config.init_data = NULL;

		config.of_node = palmas_matches[id].of_node;

		rdev = regulator_register(&pmic->desc[id], &config);
		if (IS_ERR(rdev)) {
			dev_err(&pdev->dev,
				"failed to register %s regulator\n",
				pdev->name);
			ret = PTR_ERR(rdev);
			goto err_unregister_regulator;
		}

		/* Save regulator for cleanup */
		pmic->rdev[id] = rdev;
	}

	return 0;

err_unregister_regulator:
	while (--id >= 0)
		regulator_unregister(pmic->rdev[id]);
	return ret;
}

static int __devexit palmas_remove(struct platform_device *pdev)
{
	struct palmas_pmic *pmic = platform_get_drvdata(pdev);
	int id;

	for (id = 0; id < PALMAS_NUM_REGS; id++)
		regulator_unregister(pmic->rdev[id]);
	return 0;
}

static struct of_device_id __devinitdata of_palmas_match_tbl[] = {
	{ .compatible = "ti,palmas-pmic", },
	{ /* end */ }
};

static struct platform_driver palmas_driver = {
	.driver = {
		.name = "palmas-pmic",
		.of_match_table = of_palmas_match_tbl,
		.owner = THIS_MODULE,
	},
	.probe = palmas_probe,
	.remove = __devexit_p(palmas_remove),
};

static int __init palmas_init(void)
{
	return platform_driver_register(&palmas_driver);
}
subsys_initcall(palmas_init);

static void __exit palmas_exit(void)
{
	platform_driver_unregister(&palmas_driver);
}
module_exit(palmas_exit);

MODULE_AUTHOR("Graeme Gregory <gg@slimlogic.co.uk>");
MODULE_DESCRIPTION("Palmas voltage regulator driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:palmas-pmic");
MODULE_DEVICE_TABLE(of, of_palmas_match_tbl);
