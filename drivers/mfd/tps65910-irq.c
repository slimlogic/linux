/*
 * tps65910-irq.c  --  TI TPS6591x
 *
 * Copyright 2010 Texas Instruments Inc.
 *
 * Author: Liam Girdwood <lrg@slimlogic.co.uk>
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
#include <linux/init.h>
#include <linux/bug.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <linux/mfd/tps65910.h>

static inline int irq_to_tps65910_irq(struct tps65910 *tps65910,
							int irq)
{
	return (irq - tps65910->irq_base);
}

/*
 * This is a threaded IRQ handler so can access I2C/SPI.  Since all
 * interrupts are clear on read the IRQ line will be reasserted and
 * the physical IRQ will be handled again if another interrupt is
 * asserted while we run - in the normal course of events this is a
 * rare occurrence so we save I2C/SPI reads.  We're also assuming that
 * it's rare to get lots of interrupts firing simultaneously so try to
 * minimise I/O.
 */
static irqreturn_t tps65910_irq(int irq, void *irq_data)
{
	struct tps65910 *tps65910 = irq_data;
	u32 irq_sts;
	u32 irq_mask;
	u8 reg;
	int i;

	tps65910->read(tps65910, TPS65910_INT_STS, 1, &reg);
	irq_sts = reg;
	tps65910->read(tps65910, TPS65910_INT_STS2, 1, &reg);
	irq_sts |= reg << 8;
	if (tps_chip() == TPS65911) {
		tps65910->read(tps65910, TPS65910_INT_STS3, 1, &reg);
		irq_sts |= reg << 16;
	}

	irq_mask = tps65910->read(tps65910, TPS65910_INT_MSK, 1, &reg);
	irq_mask = reg;
	tps65910->read(tps65910, TPS65910_INT_MSK2, 1, &reg);
	irq_mask |= reg << 8;
	if (tps_chip() == TPS65911) {
		tps65910->read(tps65910, TPS65910_INT_MSK3, 1, &reg);
		irq_mask |= reg << 16;
	}

	irq_sts &= irq_mask;

	if (!irq_mask)
		return IRQ_NONE;

	for (i = 0; i < tps65910->irq_num; i++) {

		if (!(irq_sts & (1 << i)))
			continue;

		handle_nested_irq(tps65910->irq_base + i);
	}

	/* Write the STS register back to clear IRQs we handled */
	reg = irq_sts & 0xFF;
	irq_sts >>= 8;
	tps65910->write(tps65910, TPS65910_INT_STS, 1, &reg);
	reg = irq_sts & 0xFF;
	tps65910->write(tps65910, TPS65910_INT_STS2, 1, &reg);
	if (tps_chip() == TPS65911) {
		reg = irq_sts >> 8;
		tps65910->write(tps65910, TPS65910_INT_STS3, 1, &reg);
	}

	return IRQ_HANDLED;
}

static void tps65910_irq_lock(unsigned int irq)
{
	struct tps65910 *tps65910 = get_irq_chip_data(irq);

	mutex_lock(&tps65910->irq_lock);
}

static void tps65910_irq_sync_unlock(unsigned int irq)
{
	struct tps65910 *tps65910 = get_irq_chip_data(irq);
	u32 reg_mask;
	u8 reg;

	tps65910->read(tps65910, TPS65910_INT_MSK, 1, &reg);
	reg_mask = reg;
	tps65910->read(tps65910, TPS65910_INT_MSK2, 1, &reg);
	reg_mask |= reg << 8;
	if (tps_chip() == TPS65911) {
		tps65910->read(tps65910, TPS65910_INT_MSK3, 1, &reg);
		reg_mask |= reg << 16;
	}

	if (tps65910->irq_mask != reg_mask) {
		reg = tps65910->irq_mask & 0xFF;
		tps65910->write(tps65910, TPS65910_INT_MSK, 1, &reg);
		reg = tps65910->irq_mask >> 8 & 0xFF;
		if (tps_chip() == TPS65911) {
			tps65910->write(tps65910, TPS65910_INT_MSK2, 1, &reg);
			reg = tps65910->irq_mask >> 8;
		}
	}
	mutex_unlock(&tps65910->irq_lock);
}

static void tps65910_irq_enable(unsigned int irq)
{
	struct tps65910 *tps65910 = get_irq_chip_data(irq);

	tps65910->irq_mask &= ~( 1 << irq_to_tps65910_irq(tps65910, irq));
}

static void tps65910_irq_disable(unsigned int irq)
{
	struct tps65910 *tps65910 = get_irq_chip_data(irq);

	tps65910->irq_mask |= ( 1 << irq_to_tps65910_irq(tps65910, irq));
}

static struct irq_chip tps65910_irq_chip = {
	.name = "tps65910",
	.bus_lock = tps65910_irq_lock,
	.bus_sync_unlock = tps65910_irq_sync_unlock,
	.disable = tps65910_irq_disable,
	.enable = tps65910_irq_enable,
};

int tps65910_irq_init(struct tps65910 *tps65910, int irq,
		    struct tps65910_platform_data *pdata)
{
	int ret, cur_irq;
	int flags = IRQF_ONESHOT;
	u8 reg;

	if (!irq) {
		dev_warn(tps65910->dev, "No interrupt support, no core IRQ\n");
		return 0;
	}

	if (!pdata || !pdata->irq_base) {
		dev_warn(tps65910->dev, "No interrupt support, no IRQ base\n");
		return 0;
	}

	/* Mask top level interrupts */
	reg = 0xFF;
	tps65910->write(tps65910, TPS65910_INT_MSK, 1, &reg);
	reg = 0xFF;
	tps65910->write(tps65910, TPS65910_INT_MSK2, 1, &reg);
	if (tps_chip() == TPS65911) {
		reg = 0xFF;
		tps65910->write(tps65910, TPS65910_INT_MSK3, 1, &reg);
	}

	mutex_init(&tps65910->irq_lock);
	tps65910->chip_irq = irq;
	tps65910->irq_base = pdata->irq_base;

	if (tps_chip() == TPS65910)
		tps65910->irq_num = TPS65910_NUM_IRQ;
	else if (tps_chip() == TPS65911)
		tps65910->irq_num = TPS65911_NUM_IRQ;

	/* Register with genirq */
	for (cur_irq = tps65910->irq_base;
	     cur_irq < tps65910->irq_num + tps65910->irq_base;
	     cur_irq++) {
		set_irq_chip_data(cur_irq, tps65910);
		set_irq_chip_and_handler(cur_irq, &tps65910_irq_chip,
					 handle_edge_irq);
		set_irq_nested_thread(cur_irq, 1);

		/* ARM needs us to explicitly flag the IRQ as valid
		 * and will set them noprobe when we do so. */
#ifdef CONFIG_ARM
		set_irq_flags(cur_irq, IRQF_VALID);
#else
		set_irq_noprobe(cur_irq);
#endif
	}

	ret = request_threaded_irq(irq, NULL, tps65910_irq, flags,
				   "tps65910", tps65910);
	if (ret != 0)
		dev_err(tps65910->dev, "Failed to request IRQ: %d\n", ret);

	return ret;
}

int tps65910_irq_exit(struct tps65910 *tps65910)
{
	free_irq(tps65910->chip_irq, tps65910);
	return 0;
}
