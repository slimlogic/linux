#include <linux/i2c.h>
#include <linux/i2c/twl.h>
#include <linux/gpio.h>
#include <linux/regulator/machine.h>
#include <linux/regulator/fixed.h>
#include <linux/mfd/palmas.h>
#include <plat/i2c.h>
#include "mux.h"

static struct palmas_gpadc_platform_data gpadc_data = {
	.ch3_current = 0,
	.ch0_current = 0,
	.bat_removal = 0,
	.start_polarity= 0,
};

/* Initialisation Data for Regulators */

static struct palmas_reg_init smps12_init = {
	.warm_reset = 0,
	.roof_floor = 0,
	.mode_sleep = 0,
	.tstep = 0,
};

static struct palmas_reg_init smps3_init = {
	.warm_reset = 0,
	.roof_floor = 0,
	.mode_sleep = 0,
	.tstep = 0,
};

static struct palmas_reg_init smps6_init = {
	.warm_reset = 0,
	.roof_floor = 0,
	.mode_sleep = 1,
	.tstep = 0,
};

static struct palmas_reg_init smps7_init = {
	.warm_reset = 0,
	.roof_floor = 0,
	.mode_sleep = 1,
};

static struct palmas_reg_init smps8_init = {
	.warm_reset = 0,
	.roof_floor = 0,
	.mode_sleep = 0,
	.tstep = 0,
};

static struct palmas_reg_init smps9_init = {
	.warm_reset = 0,
	.roof_floor = 0,
	.mode_sleep = 0,
	.vsel = 0xbd,
};

static struct palmas_reg_init ldo1_init = {
	.warm_reset = 0,
	.mode_sleep = 0,
};

static struct palmas_reg_init ldo2_init = {
	.warm_reset = 0,
	.mode_sleep = 0,
};

static struct palmas_reg_init ldo3_init = {
	.warm_reset = 0,
	.mode_sleep = 0,
};

static struct palmas_reg_init ldo4_init = {
	.warm_reset = 0,
	.mode_sleep = 0,
};

static struct palmas_reg_init ldo5_init = {
	.warm_reset = 0,
	.mode_sleep = 0,
};

static struct palmas_reg_init ldo6_init = {
	.warm_reset = 0,
	.mode_sleep = 0,
};

static struct palmas_reg_init ldo7_init = {
	.warm_reset = 0,
	.mode_sleep = 0,
};

static struct palmas_reg_init ldo8_init = {
	.warm_reset = 0,
	.mode_sleep = 0,
};

static struct palmas_reg_init ldo9_init = {
	.warm_reset = 0,
	.mode_sleep = 0,
};

static struct palmas_reg_init ldoln_init = {
	.warm_reset = 0,
	.mode_sleep = 0,
};

static struct palmas_reg_init ldousb_init = {
	.warm_reset = 0,
	.mode_sleep = 0,
};

static struct palmas_reg_init ldo10_init = {
	.warm_reset = 0,
	.mode_sleep = 0,
};

static struct palmas_reg_init ldo11_init = {
	.warm_reset = 0,
	.mode_sleep = 0,
};

static struct palmas_reg_init ldo12_init = {
	.warm_reset = 0,
	.mode_sleep = 0,
};

static struct palmas_reg_init ldo13_init = {
	.warm_reset = 0,
	.mode_sleep = 0,
};

static struct palmas_reg_init ldo14_init = {
	.warm_reset = 0,
	.mode_sleep = 0,
};

/* Constraints for Regulators */
static struct regulator_init_data smps12 = {
	.constraints = {
		.min_uV			= 600000,
		.max_uV			= 1310000,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask		= REGULATOR_CHANGE_VOLTAGE
					| REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
};

static struct regulator_init_data smps3 = {
	.constraints = {
		.min_uV			= 1000000,
		.max_uV			= 3300000,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask		= REGULATOR_CHANGE_VOLTAGE
					| REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
};

static struct regulator_init_data smps6 = {
	.constraints = {
		.min_uV			= 1200000,
		.max_uV			= 1200000,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask		= REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
};

static struct regulator_init_data smps7 = {
	.constraints = {
		.min_uV			= 1800000,
		.max_uV			= 1800000,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask		= REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
};

static struct regulator_init_data smps8 = {
	.constraints = {
		.min_uV			= 600000,
		.max_uV			= 1310000,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask		= REGULATOR_CHANGE_VOLTAGE
					| REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
};

static struct regulator_consumer_supply adac_supply[] = {
	REGULATOR_SUPPLY("vcc", "soc-audio"),
};

static struct regulator_init_data smps9 = {
	.constraints = {
		.min_uV			= 500000,
		.max_uV			= 1650000,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask		= REGULATOR_CHANGE_VOLTAGE
					| REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
	.num_consumer_supplies	= ARRAY_SIZE(adac_supply),
	.consumer_supplies	= adac_supply,

};

static struct regulator_init_data ldo1 = {
	.constraints = {
		.min_uV			= 2800000,
		.max_uV			= 2800000,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask		= REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
};

static struct regulator_init_data ldo2 = {
	.constraints = {
		.min_uV			= 2800000,
		.max_uV			= 2800000,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask		= REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
};

static struct regulator_init_data ldo3 = {
	.constraints = {
		.min_uV			= 3000000,
		.max_uV			= 3000000,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask		= REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
};

static struct regulator_init_data ldo4 = {
	.constraints = {
		.min_uV			= 2200000,
		.max_uV			= 2200000,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask		= REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
};

static struct regulator_init_data ldo5 = {
	.constraints = {
		.min_uV			= 1800000,
		.max_uV			= 1800000,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask		= REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
};

static struct regulator_init_data ldo6 = {
	.constraints = {
		.min_uV			= 1500000,
		.max_uV			= 1500000,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask		= REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
};

static struct regulator_init_data ldo7 = {
	.constraints = {
		.min_uV			= 1500000,
		.max_uV			= 1500000,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask		= REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
};

static struct regulator_init_data ldo8 = {
	.constraints = {
		.min_uV			= 1500000,
		.max_uV			= 1500000,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask		= REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
};

static struct regulator_init_data ldo9 = {
	.constraints = {
		.min_uV			= 1800000,
		.max_uV			= 1800000,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask		= REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
};

static struct regulator_init_data ldoln = {
	.constraints = {
		.min_uV			= 1800000,
		.max_uV			= 1800000,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask		= REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
};

static struct regulator_init_data ldousb = {
	.constraints = {
		.min_uV			= 3250000,
		.max_uV			= 3250000,
		.valid_modes_mask	= REGULATOR_MODE_NORMAL
					| REGULATOR_MODE_STANDBY,
		.valid_ops_mask		= REGULATOR_CHANGE_MODE
					| REGULATOR_CHANGE_STATUS,
	},
};

static struct regulator_init_data ldo10 = {
	.constraints = {
		.min_uV			= 900000,
		.max_uV			= 3300000,
		.valid_ops_mask		= REGULATOR_CHANGE_VOLTAGE
					| REGULATOR_CHANGE_STATUS,
	},
};

static struct regulator_init_data ldo11 = {
	.constraints = {
		.min_uV			= 900000,
		.max_uV			= 3300000,
		.valid_ops_mask		= REGULATOR_CHANGE_VOLTAGE
					| REGULATOR_CHANGE_STATUS,
	},
};

static struct regulator_init_data ldo12 = {
	.constraints = {
		.min_uV			= 900000,
		.max_uV			= 3300000,
		.valid_ops_mask		= REGULATOR_CHANGE_VOLTAGE
					| REGULATOR_CHANGE_STATUS,
	},
};

static struct regulator_init_data ldo13 = {
	.constraints = {
		.min_uV			= 900000,
		.max_uV			= 3300000,
		.valid_ops_mask		= REGULATOR_CHANGE_VOLTAGE
					| REGULATOR_CHANGE_STATUS,
	},
};

static struct regulator_init_data ldo14 = {
	.constraints = {
		.min_uV			= 900000,
		.max_uV			= 3300000,
		.valid_ops_mask		= REGULATOR_CHANGE_VOLTAGE
					| REGULATOR_CHANGE_STATUS,
	},
};

static struct regulator_consumer_supply vbus_supply[] = {
	REGULATOR_SUPPLY("vbus", "palmas-usb"),
};

static struct regulator_init_data boost = {
	.constraints = {
		.min_uV			= 5000000,
		.max_uV			= 5000000,
		.min_uA			= 500000,
		.max_uA			= 1300000,
		.valid_ops_mask		= REGULATOR_CHANGE_CURRENT |
						REGULATOR_CHANGE_STATUS,
	},
	.num_consumer_supplies	= ARRAY_SIZE(vbus_supply),
	.consumer_supplies	= vbus_supply,

};

static struct palmas_pmic_platform_data pmic_data = {
	.reg_data = {
			[PALMAS_REG_SMPS12] = &smps12,
			[PALMAS_REG_SMPS3] = &smps3,
			[PALMAS_REG_SMPS6] = &smps6,
			[PALMAS_REG_SMPS7] = &smps7,
			[PALMAS_REG_SMPS8] = &smps8,
			[PALMAS_REG_SMPS9] = &smps9,

			[PALMAS_REG_LDO1] = &ldo1,
			[PALMAS_REG_LDO2] = &ldo2,
			[PALMAS_REG_LDO3] = &ldo3,
			[PALMAS_REG_LDO4] = &ldo4,
			[PALMAS_REG_LDO5] = &ldo5,
			[PALMAS_REG_LDO6] = &ldo6,
			[PALMAS_REG_LDO7] = &ldo7,
			[PALMAS_REG_LDO8] = &ldo8,
			[PALMAS_REG_LDO9] = &ldo9,
			[PALMAS_REG_LDOLN] = &ldoln,
			[PALMAS_REG_LDOUSB] = &ldousb,
			[PALMAS_REG_LDO10] = &ldo10,
			[PALMAS_REG_LDO11] = &ldo11,
			[PALMAS_REG_LDO12] = &ldo12,
			[PALMAS_REG_LDO13] = &ldo13,
			[PALMAS_REG_LDO14] = &ldo14,

			[PALMAS_REG_BOOST] = &boost,
		},
	.reg_init = {
			[PALMAS_REG_SMPS12] = &smps12_init,
			[PALMAS_REG_SMPS3] = &smps3_init,
			[PALMAS_REG_SMPS6] = &smps6_init,
			[PALMAS_REG_SMPS7] = &smps7_init,
			[PALMAS_REG_SMPS8] = &smps8_init,
			[PALMAS_REG_SMPS9] = &smps9_init,

			[PALMAS_REG_LDO1] = &ldo1_init,
			[PALMAS_REG_LDO2] = &ldo2_init,
			[PALMAS_REG_LDO3] = &ldo3_init,
			[PALMAS_REG_LDO4] = &ldo4_init,
			[PALMAS_REG_LDO5] = &ldo5_init,
			[PALMAS_REG_LDO6] = &ldo6_init,
			[PALMAS_REG_LDO7] = &ldo7_init,
			[PALMAS_REG_LDO8] = &ldo8_init,
			[PALMAS_REG_LDO9] = &ldo9_init,
			[PALMAS_REG_LDOLN] = &ldoln_init,
			[PALMAS_REG_LDOUSB] = &ldousb_init,
			[PALMAS_REG_LDO10] = &ldo10_init,
			[PALMAS_REG_LDO11] = &ldo11_init,
			[PALMAS_REG_LDO12] = &ldo12_init,
			[PALMAS_REG_LDO13] = &ldo13_init,
			[PALMAS_REG_LDO14] = &ldo14_init,
		},
	.ldo6_vibrator = 0,
};

static struct palmas_resource_platform_data resource_data = {
	.regen1_mode_sleep = 0,
	.regen2_mode_sleep = 0,
	.sysen1_mode_sleep = 0,
	.sysen2_mode_sleep = 0,

	.nsleep_res = 0,
	.nsleep_smps = 0,
	.nsleep_ldo1 = 0,
	.nsleep_ldo2 = 0,

	.enable1_res = 0,
	.enable1_smps = 0,
	.enable1_ldo1 = 0,
	.enable1_ldo2 = 0,

	.enable2_res = 0,
	.enable2_smps = 0,
	.enable2_ldo1 = 0,
	.enable2_ldo2 = 0,
};

static struct palmas_clk_platform_data clk_data = {
	.clk32kg_mode_sleep = 0,
	.clk32kgaudio_mode_sleep = 0,
};

/* based on an old table for 10bit GPADC hence the *4 for 12bit GPADC */
static int palmas_batt_temperature_table[] = {
        /* adc code for temperature in degree C */
        929*4, 925*4, /* -2 ,-1 */
        920*4, 917*4, 912*4, 908*4, 904*4, 899*4, 895*4, 890*4, 885*4, 880*4, /* 00 - 09 */
        875*4, 869*4, 864*4, 858*4, 853*4, 847*4, 841*4, 835*4, 829*4, 823*4, /* 10 - 19 */
        816*4, 810*4, 804*4, 797*4, 790*4, 783*4, 776*4, 769*4, 762*4, 755*4, /* 20 - 29 */
        748*4, 740*4, 732*4, 725*4, 718*4, 710*4, 703*4, 695*4, 687*4, 679*4, /* 30 - 39 */
        671*4, 663*4, 655*4, 647*4, 639*4, 631*4, 623*4, 615*4, 607*4, 599*4, /* 40 - 49 */
        591*4, 583*4, 575*4, 567*4, 559*4, 551*4, 543*4, 535*4, 527*4, 519*4, /* 50 - 59 */
        511*4, 504*4, 496*4 /* 60 - 62 */
};

static struct palmas_batt_capacity_chart volt_cap_table[] = {
	{ .volt = 3345, .cap =  7 },
	{ .volt = 3450, .cap = 15 },
	{ .volt = 3500, .cap = 30 },
	{ .volt = 3600, .cap = 50 },
	{ .volt = 3650, .cap = 70 },
	{ .volt = 3780, .cap = 85 },
	{ .volt = 3850, .cap = 95 },
};

struct palmas_charger_platform_data charger_data = {
	/* time in seconds for current average readings */
	.current_avg_interval = 10,

	/* Battery is soldered in with no detection resistor */
	.battery_soldered = 0,

	/* Limit to use when on dedicated charger */
	.usb_charge_limit = PALMAS_IINLIM_2000MA,

	/* Default watchdog timeout */
	.watchdog_code = PALMAS_WATCHDOG_40S,

	/* How often we should update the info about charge status */
	.battery_status_interval = 10000,

	/* maximum capacity in mAh of battery */
	.battery_capacity_max = 4000,

	/* converstion table from V to capacity for first boot */
	.bat_chart = volt_cap_table,
	.bat_chart_size = ARRAY_SIZE(volt_cap_table),

	/* temperature conversion for this battery */
	.battery_temperature_chart = palmas_batt_temperature_table,
	.battery_temperature_chart_size =
			ARRAY_SIZE(palmas_batt_temperature_table),
};

struct palmas_usb_platform_data usb_data = {
	.no_control_vbus = 1,
};

static struct palmas_leds_platform_data leds_data = {
	.chrg_led_mode = 0,
};

static struct palmas_platform_data palmas_data = {
	.gpio_base = 0,

	.gpadc_pdata = &gpadc_data,
	.pmic_pdata = &pmic_data,
	.usb_pdata = &usb_data,
	.resource_pdata = &resource_data,
	.clk_pdata = &clk_data,
	.charger_pdata = &charger_data,
	.leds_pdata = &leds_data,

	.power_ctrl = 0x07,
	.mux_from_pdata = 0,
	.pad1 = 0x00,
	.pad2 = 0x00,
};

static struct i2c_board_info __initdata palmas_i2c[] = {
	{
		I2C_BOARD_INFO("twl6035", 0x48),
		.platform_data = &palmas_data,
	}
};

int palmas_i2c_init(void)
{
	printk(KERN_WARNING "Probing the Palmas Device\n");

	omap_mux_init_gpio(140, OMAP_PIN_INPUT);
	gpio_request_one(140, GPIOF_IN, "palmas irq");
	palmas_i2c[0].irq = gpio_to_irq(140);

	i2c_register_board_info(4, palmas_i2c, ARRAY_SIZE(palmas_i2c));
	return 0;
}
