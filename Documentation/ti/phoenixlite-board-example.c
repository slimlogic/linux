#include <linux/i2c.h>
#include <linux/i2c/twl.h>
#include <linux/regulator/machine.h>
#include <linux/regulator/fixed.h>
#include <linux/i2c/twl.h>

/*
 * Voltage values in this file are examples, they should be configured to the
 * correct values for the target platform
 */

static struct regulator_init_data ldo1 = {
	.constraints = {
		.min_uV			= 1000000,
		.max_uV			= 3300000,
		.apply_uV		= true,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask	 = REGULATOR_CHANGE_VOLTAGE
					| REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
};

static struct regulator_init_data ldo2 = {
	.constraints = {
		.min_uV			= 1000000,
		.max_uV			= 3300000,
		.apply_uV		= true,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask	 = REGULATOR_CHANGE_VOLTAGE
					| REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
};

static struct regulator_init_data ldo3 = {
	.constraints = {
		.min_uV			= 1000000,
		.max_uV			= 3300000,
		.apply_uV		= true,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask	 = REGULATOR_CHANGE_VOLTAGE
					| REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
};

static struct regulator_init_data ldo4 = {
	.constraints = {
		.min_uV			= 1000000,
		.max_uV			= 3300000,
		.apply_uV		= true,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask	 = REGULATOR_CHANGE_VOLTAGE
					| REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
};

static struct regulator_init_data ldo5 = {
	.constraints = {
		.min_uV			= 1000000,
		.max_uV			= 3300000,
		.apply_uV		= true,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask	 = REGULATOR_CHANGE_VOLTAGE
					| REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
};

static struct regulator_init_data ldo6 = {
	.constraints = {
		.min_uV			= 1000000,
		.max_uV			= 3300000,
		.apply_uV		= true,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask	 = REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
};

static struct regulator_init_data ldo7 = {
	.constraints = {
		.min_uV			= 1000000,
		.max_uV			= 3300000,
		.apply_uV		= true,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask	 = REGULATOR_CHANGE_VOLTAGE
					| REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
};

static struct regulator_init_data ldoln = {
	.constraints = {
		.min_uV			= 1000000,
		.max_uV			= 3300000,
		.apply_uV		= true,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask	 = REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
};

static struct regulator_init_data ldousb = {
	.constraints = {
		.min_uV			= 1000000,
		.max_uV			= 3300000,
		.apply_uV		= true,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask	 =	REGULATOR_CHANGE_VOLTAGE
					| REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
};

static struct regulator_init_data smps3 = {
	.constraints = {
		.min_uV			= 600000,
		.max_uV			= 2100000,
		.apply_uV		= true,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask	 =	REGULATOR_CHANGE_VOLTAGE
					| REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
};

static struct regulator_init_data smps4 = {
	.constraints = {
		.min_uV			= 1852000,
		.max_uV			= 3241000,
		.apply_uV		= true,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask	 =	REGULATOR_CHANGE_VOLTAGE
					| REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
};

static struct regulator_init_data vana = {
	.constraints = {
		.min_uV			= 2100000,
		.max_uV			= 2100000,
		.apply_uV		= true,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask	 = REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
};

static struct regulator_init_data vio = {
	.constraints = {
		.min_uV			= 600000,
		.max_uV			= 2100000,
		.apply_uV		= true,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask	 =	REGULATOR_CHANGE_VOLTAGE
					| REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
};

/*
 * This table is used to determin battery temperature and will need to be
 * calibrated with the fitted thermistor
 */
static int batt_table[] = {
        /* adc code for temperature in degree C */
        929, 925, /* -2 ,-1 */
        920, 917, 912, 908, 904, 899, 895, 890, 885, 880, /* 00 - 09 */
        875, 869, 864, 858, 853, 847, 841, 835, 829, 823, /* 10 - 19 */
        816, 810, 804, 797, 790, 783, 776, 769, 762, 755, /* 20 - 29 */
        748, 740, 732, 725, 718, 710, 703, 695, 687, 679, /* 30 - 39 */
        671, 663, 655, 647, 639, 631, 623, 615, 607, 599, /* 40 - 49 */
        591, 583, 575, 567, 559, 551, 543, 535, 527, 519, /* 50 - 59 */
        511, 504, 496 /* 60 - 62 */
};

static struct twl4030_bci_platform_data bci_data = {
	/* How often the fuel gauging algorithm runs */
        .monitoring_interval            = 10,
	/* The max current for charger sets CHARGERUSB_VICHRG */
        .max_charger_currentmA          = 1500,
	/* The maximum charge voltage, sets CHARGERUSB_CTRLLIMIT1 */
        .max_charger_voltagemV          = 4560,
	/* The maximum battery voltage, sets CHARGERUSB_VOREG */
        .max_bat_voltagemV              = 4200,
	/* The termination current for charging, ITERM in CHARGERUSB_CTRL2 */
        .termination_currentmA          = 100,
	/* The temperature table from above */
        .battery_tmp_tbl                = batt_table,
        .tblsize                        = ARRAY_SIZE(batt_table),
	/* The size of sense resistor for current measurements */
        .sense_resistor_mohm            = 10,
};

/* this is not used on 6032 but is needed to probe driver */
static struct twl4030_madc_platform_data gpadc = {
        .irq_line = -1,
};

static struct twl4030_platform_data twl_platform_data = {
	/* Regulators */
	.ldo1		= &ldo1,
	.ldo2		= &ldo2,
	.ldo3		= &ldo3,
	.ldo4		= &ldo4,
	.ldo5		= &ldo5,
	.ldo6		= &ldo6,
	.ldo7		= &ldo7,
	.vana		= &vana,
	.ldoln		= &ldoln,
	.ldousb		= &ldousb,
	.smps3		= &smps3,
	.smps4		= &smps4,
	.vio6032	= &vio,

	/* Battery/Charger Information */
	.bci            = &bci_data,

	/* GPADC */
	.madc		= &gpadc,
};

