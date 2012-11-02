/*
 * Palmas Interrupt Support and Muxing
 *
 * Copyright 2012 Texas Instruments
 *
 * Author: Graeme Gregory <gg@slimlogic.co.uk>
 *
 * based on arizona-irq.c
 *
 * Copyright 2012 Wolfson Microelectronics plc
 *
 * Author: Mark Brown <broonie@opensource.wolfsonmicro.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/irqdomain.h>
#include <linux/module.h>
#include <linux/pm_runtime.h>
#include <linux/regmap.h>
#include <linux/slab.h>

#include <linux/mfd/palmas.h>

int palmas_map_irq(struct palmas *palmas, int irq)
{
	int i, ret = 0;

	for (i = 0; i < palmas->no_irq_slaves; i++) {
		ret = regmap_irq_get_virq(palmas->irq_data[i], irq);
		if (ret > 0)
			return ret;
	}

	return ret;
}
EXPORT_SYMBOL_GPL(palmas_map_irq);

int palmas_request_irq(struct palmas *palmas, int irq, char *name,
			   irq_handler_t handler, void *data)
{
	irq = palmas_map_irq(palmas, irq);
	if (irq < 0)
		return irq;

	return request_threaded_irq(irq, NULL, handler, IRQF_ONESHOT,
				    name, data);
}
EXPORT_SYMBOL_GPL(palmas_request_irq);

void palmas_free_irq(struct palmas *palmas, int irq, void *data)
{
	irq = palmas_map_irq(palmas, irq);
	if (irq < 0)
		return;

	free_irq(irq, data);
}
EXPORT_SYMBOL_GPL(palmas_free_irq);

int palmas_set_irq_wake(struct palmas *palmas, int irq, int on)
{
	irq = palmas_map_irq(palmas, irq);
	if (irq < 0)
		return irq;

	return irq_set_irq_wake(irq, on);
}
EXPORT_SYMBOL_GPL(palmas_set_irq_wake);

static irqreturn_t palmas_irq_thread(int irq, void *data)
{
	struct palmas *palmas = data;
	int i;

	/* Check all domains */
	for (i = 0; i < palmas->no_irq_slaves; i++)
		handle_nested_irq(irq_find_mapping(palmas->virq, i));

	return IRQ_HANDLED;
}

static void palmas_irq_enable(struct irq_data *data)
{
}

static void palmas_irq_disable(struct irq_data *data)
{
}

static struct irq_chip palmas_virq_chip = {
        .name                   = "palmas",
        .irq_disable            = palmas_irq_disable,
        .irq_enable             = palmas_irq_enable,
};

static int palmas_irq_map(struct irq_domain *h, unsigned int virq,
			      irq_hw_number_t hw)
{
	struct regmap_irq_chip_data *data = h->host_data;

	irq_set_chip_data(virq, data);
	irq_set_chip_and_handler(virq, &palmas_virq_chip, handle_edge_irq);
	irq_set_nested_thread(virq, 1);

	/* ARM needs us to explicitly flag the IRQ as valid
	 * and will set them noprobe when we do so. */
#ifdef CONFIG_ARM
	set_irq_flags(virq, IRQF_VALID);
#else
	irq_set_noprobe(virq);
#endif

	return 0;
}

static struct irq_domain_ops palmas_domain_ops = {
	.map	= palmas_irq_map,
	.xlate	= irq_domain_xlate_twocell,
};

static const struct regmap_irq palmas_irqs[] = {
	/* INT1 IRQs */
	[PALMAS_CHARG_DET_N_VBUS_OVV_IRQ] = {
		.mask = PALMAS_INT1_STATUS_CHARG_DET_N_VBUS_OVV,
	},
	[PALMAS_PWRON_IRQ] = {
		.mask = PALMAS_INT1_STATUS_PWRON,
	},
	[PALMAS_LONG_PRESS_KEY_IRQ] = {
		.mask = PALMAS_INT1_STATUS_LONG_PRESS_KEY,
	},
	[PALMAS_RPWRON_IRQ] = {
		.mask = PALMAS_INT1_STATUS_RPWRON,
	},
	[PALMAS_PWRDOWN_IRQ] = {
		.mask = PALMAS_INT1_STATUS_PWRDOWN,
	},
	[PALMAS_HOTDIE_IRQ] = {
		.mask = PALMAS_INT1_STATUS_HOTDIE,
	},
	[PALMAS_VSYS_MON_IRQ] = {
		.mask = PALMAS_INT1_STATUS_VSYS_MON,
	},
	[PALMAS_VBAT_MON_IRQ] = {
		.mask = PALMAS_INT1_STATUS_VBAT_MON,
	},
	/* INT2 IRQs*/
	[PALMAS_RTC_ALARM_IRQ] = {
		.mask = PALMAS_INT2_STATUS_RTC_ALARM,
		.reg_offset = 1,
	},
	[PALMAS_RTC_TIMER_IRQ] = {
		.mask = PALMAS_INT2_STATUS_RTC_TIMER,
		.reg_offset = 1,
	},
	[PALMAS_WDT_IRQ] = {
		.mask = PALMAS_INT2_STATUS_WDT,
		.reg_offset = 1,
	},
	[PALMAS_BATREMOVAL_IRQ] = {
		.mask = PALMAS_INT2_STATUS_BATREMOVAL,
		.reg_offset = 1,
	},
	[PALMAS_RESET_IN_IRQ] = {
		.mask = PALMAS_INT2_STATUS_RESET_IN,
		.reg_offset = 1,
	},
	[PALMAS_FBI_BB_IRQ] = {
		.mask = PALMAS_INT2_STATUS_FBI_BB,
		.reg_offset = 1,
	},
	[PALMAS_SHORT_IRQ] = {
		.mask = PALMAS_INT2_STATUS_SHORT,
		.reg_offset = 1,
	},
	[PALMAS_VAC_ACOK_IRQ] = {
		.mask = PALMAS_INT2_STATUS_VAC_ACOK,
		.reg_offset = 1,
	},
	/* INT3 IRQs */
	[PALMAS_GPADC_AUTO_0_IRQ] = {
		.mask = PALMAS_INT3_STATUS_GPADC_AUTO_0,
		.reg_offset = 2,
	},
	[PALMAS_GPADC_AUTO_1_IRQ] = {
		.mask = PALMAS_INT3_STATUS_GPADC_AUTO_1,
		.reg_offset = 2,
	},
	[PALMAS_GPADC_EOC_SW_IRQ] = {
		.mask = PALMAS_INT3_STATUS_GPADC_EOC_SW,
		.reg_offset = 2,
	},
	[PALMAS_GPADC_EOC_RT_IRQ] = {
		.mask = PALMAS_INT3_STATUS_GPADC_EOC_RT,
		.reg_offset = 2,
	},
	[PALMAS_ID_OTG_IRQ] = {
		.mask = PALMAS_INT3_STATUS_ID_OTG,
		.reg_offset = 2,
	},
	[PALMAS_ID_IRQ] = {
		.mask = PALMAS_INT3_STATUS_ID,
		.reg_offset = 2,
	},
	[PALMAS_VBUS_OTG_IRQ] = {
		.mask = PALMAS_INT3_STATUS_VBUS_OTG,
		.reg_offset = 2,
	},
	[PALMAS_VBUS_IRQ] = {
		.mask = PALMAS_INT3_STATUS_VBUS,
		.reg_offset = 2,
	},
	/* INT4 IRQs */
	[PALMAS_GPIO_0_IRQ] = {
		.mask = PALMAS_INT4_STATUS_GPIO_0,
		.reg_offset = 3,
	},
	[PALMAS_GPIO_1_IRQ] = {
		.mask = PALMAS_INT4_STATUS_GPIO_1,
		.reg_offset = 3,
	},
	[PALMAS_GPIO_2_IRQ] = {
		.mask = PALMAS_INT4_STATUS_GPIO_2,
		.reg_offset = 3,
	},
	[PALMAS_GPIO_3_IRQ] = {
		.mask = PALMAS_INT4_STATUS_GPIO_3,
		.reg_offset = 3,
	},
	[PALMAS_GPIO_4_IRQ] = {
		.mask = PALMAS_INT4_STATUS_GPIO_4,
		.reg_offset = 3,
	},
	[PALMAS_GPIO_5_IRQ] = {
		.mask = PALMAS_INT4_STATUS_GPIO_5,
		.reg_offset = 3,
	},
	[PALMAS_GPIO_6_IRQ] = {
		.mask = PALMAS_INT4_STATUS_GPIO_6,
		.reg_offset = 3,
	},
	[PALMAS_GPIO_7_IRQ] = {
		.mask = PALMAS_INT4_STATUS_GPIO_7,
		.reg_offset = 3,
	},
};

static struct regmap_irq_chip palmas_irq_chip = {
	.name = "palmas",
	.irqs = palmas_irqs,
	.num_irqs = ARRAY_SIZE(palmas_irqs),

	.num_regs = 4,
	.irq_reg_stride = 5,
	.status_base = PALMAS_BASE_TO_REG(PALMAS_INTERRUPT_BASE,
			PALMAS_INT1_STATUS),
	.mask_base = PALMAS_BASE_TO_REG(PALMAS_INTERRUPT_BASE,
			PALMAS_INT1_MASK),
};

static const struct regmap_irq palmas_charger_irqs[] = {
	/* INT5 IRQs */
	[PALMAS_GPIO_8_IRQ] = {
		.mask = PALMAS_INT5_STATUS_GPIO_8,
		.reg_offset = 0,
	},
	[PALMAS_GPIO_1_IRQ] = {
		.mask = PALMAS_INT5_STATUS_GPIO_9,
		.reg_offset = 0,
	},
	[PALMAS_GPIO_2_IRQ] = {
		.mask = PALMAS_INT5_STATUS_GPIO_10,
		.reg_offset = 0,
	},
	[PALMAS_GPIO_3_IRQ] = {
		.mask = PALMAS_INT5_STATUS_GPIO_11,
		.reg_offset = 0,
	},
	[PALMAS_GPIO_4_IRQ] = {
		.mask = PALMAS_INT5_STATUS_GPIO_12,
		.reg_offset = 0,
	},
	[PALMAS_GPIO_6_IRQ] = {
		.mask = PALMAS_INT5_STATUS_GPIO_14,
		.reg_offset = 0,
	},
	[PALMAS_GPIO_7_IRQ] = {
		.mask = PALMAS_INT5_STATUS_GPIO_15,
		.reg_offset = 0,
	},
	/* INT6 IRQs */
	[PALMAS_CC_EOC_IRQ] = {
		.mask = PALMAS_INT6_STATUS_CC_EOC,
		.reg_offset = 1,
	},
	[PALMAS_CC_SYNC_EOC_IRQ] = {
		.mask = PALMAS_INT6_STATUS_CC_SYNC_EOC,
		.reg_offset = 1,
	},
	[PALMAS_CC_OVC_LIMIT_IRQ] = {
		.mask = PALMAS_INT6_STATUS_CC_OVC_LIMIT,
		.reg_offset = 1,
	},
	[PALMAS_CC_BAT_STABLE_IRQ] = {
		.mask = PALMAS_INT6_STATUS_CC_BAT_STABLE,
		.reg_offset = 1,
	},
	[PALMAS_CC_AUTOCAL_IRQ] = {
		.mask = PALMAS_INT6_STATUS_CC_AUTOCAL,
		.reg_offset = 1,
	},
	[PALMAS_CHARGER_IRQ] = {
		.mask = PALMAS_INT6_STATUS_CHARGER,
		.reg_offset = 1,
	},
	[PALMAS_SIM1_IRQ] = {
		.mask = PALMAS_INT6_STATUS_SIM1,
		.reg_offset = 1,
	},
	[PALMAS_SIM2_IRQ] = {
		.mask = PALMAS_INT6_STATUS_SIM2,
		.reg_offset = 1,
	},
};

static struct regmap_irq_chip palmas_charger_irq_chip = {
	.name = "palmas-charger",
	.irqs = palmas_charger_irqs,
	.num_irqs = ARRAY_SIZE(palmas_charger_irqs),

	.num_regs = 2,
	.irq_reg_stride = 5,
	.status_base = PALMAS_BASE_TO_REG(PALMAS_INTERRUPT_BASE,
			PALMAS_INT5_STATUS),
	.mask_base = PALMAS_BASE_TO_REG(PALMAS_INTERRUPT_BASE,
			PALMAS_INT5_MASK),
};

static const struct regmap_irq palmas_charger_irqs2[] = {
	/* INT7 IRQs */
	[PALMAS_BAT_CONTACT_BREAK_IRQ] = {
		.mask = PALMAS_INT7_STATUS_BAT_CONTACT_BREAK,
		.reg_offset = 0,
	},
	[PALMAS_CHRG_IN_ANTICOLLAPSE_IRQ] = {
		.mask = PALMAS_INT7_STATUS_CHRG_IN_ANTICOLLAPSE,
		.reg_offset = 0,
	},
	[PALMAS_BAT_TEMP_FAULT_IRQ] = {
		.mask = PALMAS_INT7_STATUS_BAT_TEMP_FAULT,
		.reg_offset = 0,
	},
};

static struct regmap_irq_chip palmas_charger_irq_chip2 = {
	.name = "palmas-charger-bq",
	.irqs = palmas_charger_irqs2,
	.num_irqs = ARRAY_SIZE(palmas_charger_irqs2),

	.num_regs = 1,
	.status_base = PALMAS_BASE_TO_REG(PALMAS_INTERRUPT2_BASE,
			PALMAS_INT7_STATUS),
	.mask_base = PALMAS_BASE_TO_REG(PALMAS_INTERRUPT2_BASE,
			PALMAS_INT7_MASK),
};

int palmas_irq_init(struct palmas *palmas)
{
	int flags = IRQF_ONESHOT | IRQF_TRIGGER_LOW;
	int ret;

	switch (palmas->id) {
	case PALMAS_CHIP_ID:
	case PALMAS_CHIP_OLD_ID:
		palmas->no_irq_slaves = 1;
		break;
	case PALMAS_CHIP_CHARGER_ID:
		palmas->no_irq_slaves = 3;
		break;
	default:
		BUG_ON("Unknown Palmas class device" == NULL);
		return -EINVAL;
	}

	/* Allocate a virtual IRQ domain to distribute to the regmap domains */
	palmas->virq = irq_domain_add_linear(NULL, palmas->no_irq_slaves,
						&palmas_domain_ops, palmas);
	if (!palmas->virq) {
		ret = -EINVAL;
		goto err;
	}

	ret = regmap_add_irq_chip(palmas->regmap[1],
				  irq_create_mapping(palmas->virq, 0),
				  IRQF_ONESHOT, 0, &palmas_irq_chip,
				  &palmas->irq_data[0]);
	if (ret != 0) {
		dev_err(palmas->dev, "Failed to add Palmas IRQs: %d\n", ret);
		goto err_palmas_irq;
	}

	if (is_palmas_charger(palmas->id)) {
		ret = regmap_add_irq_chip(palmas->regmap[1],
				  irq_create_mapping(palmas->virq, 1),
				  IRQF_ONESHOT, 0, &palmas_charger_irq_chip,
				  &palmas->irq_data[1]);
		if (ret != 0) {
			dev_err(palmas->dev,
				"Failed to add Palmas Charger IRQs: %d\n", ret);
			goto err_palmas_charger;
		}

		ret = regmap_add_irq_chip(palmas->regmap[1],
				  irq_create_mapping(palmas->virq, 2),
				  IRQF_ONESHOT, 0, &palmas_charger_irq_chip2,
				  &palmas->irq_data[2]);
		if (ret != 0) {
			dev_err(palmas->dev,
					"Failed to add BQ IRQs: %d\n", ret);
			goto err_palmas_bq;
		}
	}

	ret = request_threaded_irq(palmas->irq, NULL, palmas_irq_thread,
				   flags, "palmas", palmas);

	if (ret != 0) {
		dev_err(palmas->dev, "Failed to request IRQ %d: %d\n",
			palmas->irq, ret);
		goto err_main_irq;
	}

	return 0;

err_main_irq:
	regmap_del_irq_chip(irq_create_mapping(palmas->virq, 2),
				palmas->irq_data[0]);
err_palmas_bq:
	regmap_del_irq_chip(irq_create_mapping(palmas->virq, 1),
				palmas->irq_data[1]);
err_palmas_charger:
	regmap_del_irq_chip(irq_create_mapping(palmas->virq, 0),
				palmas->irq_data[2]);
err_palmas_irq:
err:
	return ret;
}

int palmas_irq_exit(struct palmas *palmas)
{
	int i;

	for (i = 0; i < palmas->no_irq_slaves; i++)
		regmap_del_irq_chip(irq_create_mapping(palmas->virq, i),
				palmas->irq_data[i]);
	free_irq(palmas->irq, palmas);

	return 0;
}
