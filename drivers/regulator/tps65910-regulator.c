/*
 * tps65910.c  --  TI tps65910
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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
/*#include <linux/regulator/tps65910.h>*/
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/mfd/tps65910.h>

#define tps65910_VRTC		0
#define tps65910_VIO		1
#define tps65910_VDD1		2
#define tps65910_VDD2		3
#define tps65910_VDD3		4
#define tps65910_VDIG1		5
#define tps65910_VDIG2		6
#define tps65910_VPLL		7
#define tps65910_VDAC		8
#define tps65910_VAUX1		9
#define tps65910_VAUX2		10
#define tps65910_VAUX33		11
#define tps65910_VMMC		12

#define tps65910_NUM_REGULATOR		13

#define tps65910_VRTC_REG	0x1e
#define tps65910_VIO_REG	0x20
#define tps65910_VDD1_REG	0x21
#define tps65910_VDD2_REG	0x24
#define tps65910_VDD3_REG	0x27
#define tps65910_VDIG1_REG	0x30
#define tps65910_VDIG2_REG	0x31
#define tps65910_VAUX1_REG	0x32
#define tps65910_VAUX2_REG	0x33
#define tps65910_VAUX33_REG	0x34
#define tps65910_VMMC_REG	0x35
#define tps65910_VPLL_REG	0x36
#define tps65910_VDAC_REG	0x37

#define tps65910_SUPPLY_STATE_ENABLED	0x1
#define tps65910_SUPPLY_MODE_SLEEP	0x2

/* supported VIO voltages in milivolts */
static const u16 VIO_VSEL_table[] = {
	1500, 1800, 2500, 3300,
};

/* supported VDD 1 & 2 voltages (in 0.1 milliVolts) at 1x */
static const u16 VDD1_2_VSEL_table[] = {
	6000, 6125, 6250, 6375, 6500, 6625, 6750, 6875,
	7000, 7125, 7250, 7375, 7500, 7625, 7750, 7875,
	8000, 8125, 8250, 8375, 8500, 8625, 8750, 8875,
	9000, 9125, 9250, 9375, 9500, 9625, 9750, 9875,
	10000, 10125, 10250, 10375, 10500, 10625, 10750, 10875,
	11000, 11125, 11250, 11375, 11500, 11625, 11750, 11875,
	12000, 12125, 12250, 12375, 12500, 12625, 12750, 12875,
	13000, 13125, 13250, 13375, 13500, 13625, 13750, 13875,
	14000, 14125, 14250, 14375, 14500, 14625, 14750, 14875,
	15000,
};

/* supported VIO voltages in milivolts */
static const u16 VDD3_VSEL_table[] = {
	5000,
};

/* supported VDIG1 voltages in milivolts */
static const u16 VDIG1_VSEL_table[] = {
	1200, 1500, 1800, 2700,
};

/* supported VDIG2 voltages in milivolts */
static const u16 VDIG2_VSEL_table[] = {
	1000, 1100, 1200, 1800,
};

/* supported VPLL voltages in milivolts */
static const u16 VPLL_VSEL_table[] = {
	1000, 1100, 1800, 2500,
};

/* supported VDAC voltages in milivolts */
static const u16 VDAC_VSEL_table[] = {
	1800, 2600, 2800, 2850,
};

/* supported VAUX1 voltages in milivolts */
static const u16 VAUX1_VSEL_table[] = {
	1800, 2500, 2800, 2850,
};

/* supported VAUX2 voltages in milivolts */
static const u16 VAUX2_VSEL_table[] = {
	1800, 2800, 2900, 3300,
};

/* supported VAUX33 voltages in milivolts */
static const u16 VAUX33_VSEL_table[] = {
	1800, 2000, 2800, 3300,
};

/* supported VMMC voltages in milivolts */
static const u16 VMMC_VSEL_table[] = {
	1800, 2800, 3000, 3300,
};

static unsigned int num_voltages[] = {
	0, /* VRTC */
	ARRAY_SIZE(VIO_VSEL_table),
	ARRAY_SIZE(VDD1_2_VSEL_table),
	ARRAY_SIZE(VDD1_2_VSEL_table),
	ARRAY_SIZE(VDD3_VSEL_table),
	ARRAY_SIZE(VDIG1_VSEL_table),
	ARRAY_SIZE(VDIG2_VSEL_table),
	ARRAY_SIZE(VPLL_VSEL_table),
	ARRAY_SIZE(VDAC_VSEL_table),
	ARRAY_SIZE(VAUX1_VSEL_table),
	ARRAY_SIZE(VAUX2_VSEL_table),
	ARRAY_SIZE(VAUX33_VSEL_table),
	ARRAY_SIZE(VMMC_VSEL_table),
};

struct tps_info {
	const char *name;
	unsigned min_uV;
	unsigned max_uV;
	u8 table_len;
	const u16 *table;
	int vmult;
};

static struct tps_info tps65910_regs[] = {
	{
		.name = "VRTC",
	},
	{
		.name = "VIO",
		.min_uV = 1500000,
		.max_uV = 3300000,
		.table_len = ARRAY_SIZE(VIO_VSEL_table),
		.table = VIO_VSEL_table,
	},
	{
		.name = "VDD1",
		.min_uV = 600000,
		.max_uV = 4500000,
		.table_len = ARRAY_SIZE(VDD1_2_VSEL_table),
		.table = VDD1_2_VSEL_table,
	},
	{
		.name = "VDD2",
		.min_uV = 600000,
		.max_uV = 4500000,
		.table_len = ARRAY_SIZE(VDD1_2_VSEL_table),
		.table = VDD1_2_VSEL_table,
	},
	{
		.name = "VDD3",
		.min_uV = 5000000,
		.max_uV = 5000000,
		.table_len = ARRAY_SIZE(VDD3_VSEL_table),
		.table = VDD3_VSEL_table,
	},
	{
		.name = "VDIG1",
		.min_uV = 1200000,
		.max_uV = 2700000,
		.table_len = ARRAY_SIZE(VDIG1_VSEL_table),
		.table = VDIG1_VSEL_table,
	},
	{
		.name = "VDIG2",
		.min_uV = 1000000,
		.max_uV = 1800000,
		.table_len = ARRAY_SIZE(VDIG2_VSEL_table),
		.table = VDIG2_VSEL_table,
	},
	{
		.name = "VPLL",
		.min_uV = 1000000,
		.max_uV = 2500000,
		.table_len = ARRAY_SIZE(VPLL_VSEL_table),
		.table = VPLL_VSEL_table,
	},
	{
		.name = "VDAC",
		.min_uV = 1800000,
		.max_uV = 2850000,
		.table_len = ARRAY_SIZE(VDAC_VSEL_table),
		.table = VDAC_VSEL_table,
	},
	{
		.name = "VAUX1",
		.min_uV = 1800000,
		.max_uV = 2850000,
		.table_len = ARRAY_SIZE(VAUX1_VSEL_table),
		.table = VAUX1_VSEL_table,
	},
	{
		.name = "VAUX2",
		.min_uV = 1800000,
		.max_uV = 3300000,
		.table_len = ARRAY_SIZE(VAUX2_VSEL_table),
		.table = VAUX2_VSEL_table,
	},
	{
		.name = "VAUX33",
		.min_uV = 1800000,
		.max_uV = 3300000,
		.table_len = ARRAY_SIZE(VAUX33_VSEL_table),
		.table = VAUX33_VSEL_table,
	},
	{
		.name = "VMMC",
		.min_uV = 1800000,
		.max_uV = 3300000,
		.table_len = ARRAY_SIZE(VMMC_VSEL_table),
		.table = VMMC_VSEL_table,
	},
};

struct tps65910_reg {
	struct regulator_desc desc[tps65910_NUM_REGULATOR];
	struct tps65910 *mfd;
	struct regulator_dev *rdev[tps65910_NUM_REGULATOR];
	struct tps_info *info[tps65910_NUM_REGULATOR];
	struct mutex mutex;
	int mode;
};

static inline int tps65910_read(struct tps65910_reg *pmic, u8 reg)
{
	u8 val;
	int err;

	err = pmic->mfd->read(pmic->mfd, reg, 1, &val);
	if (err)
		return err;

	return val;
}

static inline int tps65910_write(struct tps65910_reg *pmic, u8 reg, u8 val)
{
	return pmic->mfd->write(pmic->mfd, reg, 1, &val);
}

static int tps65910_set_bits(struct tps65910_reg *pmic, u8 reg, u8 mask)
{
	int err, data;

	mutex_lock(&pmic->mutex);

	data = tps65910_read(pmic, reg);
	if (data < 0) {
		dev_err(pmic->mfd->dev, "Read from reg 0x%x failed\n", reg);
		err = data;
		goto out;
	}

	data |= mask;
	err = tps65910_write(pmic, reg, data);
	if (err)
		dev_err(pmic->mfd->dev, "Write for reg 0x%x failed\n", reg);

out:
	mutex_unlock(&pmic->mutex);
	return err;
}

static int tps65910_clear_bits(struct tps65910_reg *pmic, u8 reg, u8 mask)
{
	int err, data;

	mutex_lock(&pmic->mutex);

	data = tps65910_read(pmic, reg);
	if (data < 0) {
		dev_err(pmic->mfd->dev, "Read from reg 0x%x failed\n", reg);
		err = data;
		goto out;
	}

	data &= ~mask;
	err = tps65910_write(pmic, reg, data);
	if (err)
		dev_err(pmic->mfd->dev, "Write for reg 0x%x failed\n", reg);

out:
	mutex_unlock(&pmic->mutex);
	return err;
}

static int tps65910_reg_read(struct tps65910_reg *pmic, u8 reg)
{
	int data;

	mutex_lock(&pmic->mutex);

	data = tps65910_read(pmic, reg);
	if (data < 0)
		dev_err(pmic->mfd->dev, "Read from reg 0x%x failed\n", reg);

	mutex_unlock(&pmic->mutex);
	return data;
}

static int tps65910_reg_write(struct tps65910_reg *pmic, u8 reg, u8 val)
{
	int err;

	mutex_lock(&pmic->mutex);

	err = tps65910_write(pmic, reg, val);
	if (err < 0)
		dev_err(pmic->mfd->dev, "Write for reg 0x%x failed\n", reg);

	mutex_unlock(&pmic->mutex);
	return err;
}

static int tps65910_get_ctrl_register(int id)
{
	switch (id) {
	case tps65910_VRTC:
		return tps65910_VRTC_REG;
	case tps65910_VIO:
		return tps65910_VIO_REG;
	case tps65910_VDD1:
		return tps65910_VDD1_REG;
	case tps65910_VDD2:
		return tps65910_VDD2_REG;
	case tps65910_VDD3:
		return tps65910_VDD3_REG;
	case tps65910_VDIG1:
		return tps65910_VDIG1_REG;
	case tps65910_VDIG2:
		return tps65910_VDIG2_REG;
	case tps65910_VPLL:
		return tps65910_VPLL_REG;
	case tps65910_VDAC:
		return tps65910_VDAC_REG;
	case tps65910_VAUX1:
		return tps65910_VAUX1_REG;
	case tps65910_VAUX2:
		return tps65910_VAUX2_REG;
	case tps65910_VAUX33:
		return tps65910_VAUX33_REG;
	case tps65910_VMMC:
		return tps65910_VMMC_REG;
	default:
		return -EINVAL;
	}
}

static int tps65910_is_enabled(struct regulator_dev *dev)
{
	struct tps65910_reg *pmic = rdev_get_drvdata(dev);
	int reg, value, id = rdev_get_id(dev);

	reg = tps65910_get_ctrl_register(id);
	if (reg < 0)
		return reg;

	value = tps65910_reg_read(pmic, reg);
	if (value < 0)
		return value;

	return value & tps65910_SUPPLY_STATE_ENABLED;
}

static int tps65910_enable(struct regulator_dev *dev)
{
	struct tps65910_reg *pmic = rdev_get_drvdata(dev);
	int reg, value, id = rdev_get_id(dev);

	reg = tps65910_get_ctrl_register(id);
	if (reg < 0)
		return reg;

	value = tps65910_reg_read(pmic, reg);
	if (value < 0)
		return value;

	value |= tps65910_SUPPLY_STATE_ENABLED;
	return tps65910_reg_write(pmic, reg, value);
}

static int tps65910_disable(struct regulator_dev *dev)
{
	struct tps65910_reg *pmic = rdev_get_drvdata(dev);
	int reg, value, id = rdev_get_id(dev);

	reg = tps65910_get_ctrl_register(id);
	if (reg < 0)
		return reg;

	value = tps65910_reg_read(pmic, reg);
	if (value < 0)
		return value;

	value &= ~tps65910_SUPPLY_STATE_ENABLED;
	return tps65910_reg_write(pmic, reg, value);
}


static int tps65910_set_mode(struct regulator_dev *dev, unsigned int mode)
{
	struct tps65910_reg *pmic = rdev_get_drvdata(dev);
	int reg, value, id = rdev_get_id(dev);
// PFM MODE etc - How do the sleep ST bit relate to MODE ???????
	reg = tps65910_get_ctrl_register(id);
	if (reg < 0)
		return reg;

	value = tps65910_reg_read(pmic, reg);
	if (value < 0)
		return value;

	value &= ~0x2; // TODO: use MASK macro
	switch (mode) {
	case REGULATOR_MODE_NORMAL:
		break;
	case REGULATOR_MODE_STANDBY:
		value |= tps65910_SUPPLY_MODE_SLEEP;
		break;
	default:
		return -EINVAL;
	}

	return tps65910_reg_write(pmic, reg, value);
}

static unsigned int tps65910_get_mode(struct regulator_dev *dev)
{
	struct tps65910_reg *pmic = rdev_get_drvdata(dev);
	int reg, value, id = rdev_get_id(dev);

	reg = tps65910_get_ctrl_register(id);
	if (reg < 0)
		return reg;

	value = tps65910_reg_read(pmic, reg);
	if (value < 0)
		return value;

	if (value & tps65910_SUPPLY_MODE_SLEEP)
		return REGULATOR_MODE_STANDBY;
	else
		return REGULATOR_MODE_NORMAL;
}

static int tps65910_get_voltage(struct regulator_dev *dev)
{
	struct tps65910_reg *pmic = rdev_get_drvdata(dev);
	int reg, value, id = rdev_get_id(dev), voltage;

	reg = tps65910_get_ctrl_register(id);
	if (reg < 0)
		return reg;

	value = tps65910_reg_read(pmic, reg);
	if (value < 0)
		return value;

	switch (id) {
	case tps65910_VDD1:
	case tps65910_VDD2:
		value &= 0x3f; // TODO MASK MACRO
		break;
	case tps65910_VDD3:
		return 5 * 1000 * 1000;
	case tps65910_VIO:
	case tps65910_VDIG1:
	case tps65910_VDIG2:
	case tps65910_VPLL:
	case tps65910_VDAC:
	case tps65910_VAUX1:
	case tps65910_VAUX2:
	case tps65910_VAUX33:
	case tps65910_VMMC:
		value >>= 2;
		value &= 0x3; // TODO MASK MACRO
		break;
	default:
		return -EINVAL;
	}

	voltage = pmic->info[id]->table[value] * 1000;

	/* VDIG1 and VDIG2 have a multiplier */
	if (id == tps65910_VDIG1_REG || id == tps65910_VDIG2_REG) {
		if (pmic->info[id]->vmult)
			voltage *= pmic->info[id]->vmult;
	}
	return voltage;
}

static int tps65910_set_voltage(struct regulator_dev *dev,
				int min_uV, int max_uV, unsigned *selector)
{
	struct tps65910_reg *pmic = rdev_get_drvdata(dev);
	int reg, value, id = rdev_get_id(dev), vsel;

	/* check valid voltage */
	if (min_uV < pmic->info[id]->min_uV
		|| min_uV > pmic->info[id]->max_uV)
		return -EINVAL;
	if (max_uV < pmic->info[id]->min_uV
		|| max_uV > pmic->info[id]->max_uV)
		return -EINVAL;

	/* find requested voltage in table */
	for (vsel = 0; vsel < pmic->info[id]->table_len; vsel++) {
		int mV = pmic->info[id]->table[vsel];
		int uV = mV * 1000;

		/* Break at the first in-range value */
		if (min_uV <= uV && uV <= max_uV)
			break;
	}

	reg = tps65910_get_ctrl_register(id);
	if (reg < 0)
		return reg;

	value = tps65910_reg_read(pmic, reg);
	if (value < 0)
		return value;

	switch (id) {
	case tps65910_VDD1:
	case tps65910_VDD2:
		value &= ~0x3f; // TODO MASK MACRO
		value |= vsel;
		break;
	case tps65910_VDD3:
		return 0;
	case tps65910_VIO:
	case tps65910_VDIG1:
	case tps65910_VDIG2:
	case tps65910_VPLL:
	case tps65910_VDAC:
	case tps65910_VAUX1:
	case tps65910_VAUX2:
	case tps65910_VAUX33:
	case tps65910_VMMC:
		value &= ~(0x3 << 2); // TODO MASK MACRO
		value |= (vsel << 2);
		break;
	default:
		return -EINVAL;
	}

	return tps65910_reg_write(pmic, reg, value);
}

static int tps65910_list_voltage(struct regulator_dev *dev,
					unsigned selector)
{
	struct tps65910_reg *pmic = rdev_get_drvdata(dev);
	int id = rdev_get_id(dev), voltage;

	if (id < tps65910_VIO || id > tps65910_VMMC)
		return -EINVAL;

	if (selector >= pmic->info[id]->table_len)
		return -EINVAL;
	else
		voltage = pmic->info[id]->table[selector] * 1000;

	/* VDIG1 and VDIG2 have a multiplier */
	if (id == tps65910_VDIG1_REG || id == tps65910_VDIG2_REG) {
		if (pmic->info[id]->vmult)
			voltage *= pmic->info[id]->vmult;
	}
	return voltage;
}


static int tps65910_set_suspend_mode(struct regulator_dev *dev,
	unsigned int mode)
{
	return 0;
}

/* Regulator ops (except VRTC) */
static struct regulator_ops tps65910_ops = {
	.is_enabled	= tps65910_is_enabled,
	.enable		= tps65910_enable,
	.disable	= tps65910_disable,
	.set_mode	= tps65910_set_mode,
	.get_mode	= tps65910_get_mode,
	.get_voltage	= tps65910_get_voltage,
	.set_voltage	= tps65910_set_voltage,
	.list_voltage	= tps65910_list_voltage,
	.set_suspend_mode = tps65910_set_suspend_mode,
};

static int tps65910_vrtc_set_suspend_mode(struct regulator_dev *dev,
	unsigned int mode)
{
	return 0;
}

static int tps65910_set_suspend_enable(struct regulator_dev *dev)
{
//SLEEP_KEEP_LDO_ON_REG = 1
//SLEEP_KEEP_RES_ON_REG = 1
//SLEEP_SET_LDO_OFF_REG = 0
//SLEEP_SET_RES_OFF_REG = 0
	return 0;
}

static int tps65910_set_suspend_disable(struct regulator_dev *dev)
{
//SLEEP_KEEP_LDO_ON_REG = 0
//SLEEP_KEEP_RES_ON_REG = 0
//SLEEP_SET_LDO_OFF_REG = 1
//SLEEP_SET_RES_OFF_REG = 1
	return 0;
}

/* VRTC Regulator ops */
static struct regulator_ops tps65910_vrtc_ops = {
	.is_enabled	= tps65910_is_enabled,
	.set_suspend_mode = tps65910_vrtc_set_suspend_mode,
	.set_suspend_enable = tps65910_set_suspend_enable,
	.set_suspend_disable = tps65910_set_suspend_disable,
};

static __devinit int tps65910_probe(struct platform_device *pdev)
{
	struct tps65910 *tps65910 = dev_get_drvdata(pdev->dev.parent);
	struct tps_info *info = tps65910_regs;
	struct regulator_init_data *reg_data;
	struct regulator_dev *rdev;
	struct tps65910_reg *pmic;
	struct tps65910_board *pmic_plat_data;
	static int desc_id;
	int i, err;

	pmic_plat_data = dev_get_platdata(tps65910->dev);
	if (!pmic_plat_data)
		return -EINVAL;

	reg_data = pmic_plat_data->tps65910_pmic_init_data;
	if (!reg_data)
		return -EINVAL;

	pmic = kzalloc(sizeof(*pmic), GFP_KERNEL);
	if (!pmic)
		return -ENOMEM;

	mutex_init(&pmic->mutex);
	pmic->mfd = tps65910;
	platform_set_drvdata(pdev, pmic);

	for (i = 0; i < tps65910_NUM_REGULATOR; i++, info++, reg_data++) {
		/* Register the regulators */
		pmic->info[i] = info;

		pmic->desc[i].name = info->name;
		pmic->desc[i].id = desc_id++;
		pmic->desc[i].n_voltages = num_voltages[i];
		pmic->desc[i].ops = &tps65910_ops;
		pmic->desc[i].type = REGULATOR_VOLTAGE;
		pmic->desc[i].owner = THIS_MODULE;

		rdev = regulator_register(&pmic->desc[i],
				tps65910->dev, reg_data, pmic);
		if (IS_ERR(rdev)) {
			dev_err(tps65910->dev,
				"failed to register %s regulator\n",
				pdev->name);
			err = PTR_ERR(rdev);
			goto err;
		}

		/* Save regulator for cleanup */
		pmic->rdev[i] = rdev;
	}
	return 0;

err:
	while (--i >= 0)
		regulator_unregister(pmic->rdev[i]);

	kfree(pmic);
	return err;
}

static int __devexit tps65910_remove(struct platform_device *pdev)
{
	struct tps65910_reg *tps65910_reg = platform_get_drvdata(pdev);
	int i;

	for (i = 0; i < tps65910_NUM_REGULATOR; i++)
		regulator_unregister(tps65910_reg->rdev[i]);

	kfree(tps65910_reg);
	return 0;
}

static struct platform_driver tps65910_driver = {
	.driver = {
		.name = "tps65910-pmic",
		.owner = THIS_MODULE,
	},
	.probe = tps65910_probe,
	.remove = __devexit_p(tps65910_remove),
};

static int __init tps65910_init(void)
{
	return platform_driver_register(&tps65910_driver);
}
subsys_initcall(tps65910_init);

static void __exit tps65910_cleanup(void)
{
	platform_driver_unregister(&tps65910_driver);
}
module_exit(tps65910_cleanup);

MODULE_AUTHOR("Liam Girdwood <lrg@slimlogic.co.uk>");
MODULE_DESCRIPTION("TPS6507x voltage regulator driver");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:tps65910-pmic");

