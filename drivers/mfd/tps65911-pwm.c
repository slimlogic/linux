/*
 * tps65910.c  --  TI TPS6591x
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
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/mfd/tps65910.h>
#include <linux/slab.h>

#define PWM_CTRL2_MAX			255

struct tps65910 *tps_mfd;

static const u16 PWM_FREQ_table[] = {
	5000, 2500, 1250, 625
};

struct pwm_device {
	const char *label;
	unsigned int pwm_id;
};

int pwm_config(struct pwm_device *pwm, int duty_cycle, int frequency)
{
	u8 pwm_freq;

	if (pwm == NULL)
		return -EINVAL;

	pwm_freq = 0;
	frequency *= 10;
	while(pwm_freq < 4) {
		if (frequency >= PWM_FREQ_table[pwm_freq])
			break;
		else
			pwm_freq++;
	}

	if (pwm_freq > 3)
		pwm_freq = 3;
		
	tps_mfd->write(tps_mfd, TPS65911_PWM_CTRL1, 1, &pwm_freq);

	if (duty_cycle > 255)
		duty_cycle = 255;
	tps_mfd->write(tps_mfd, TPS65911_PWM_CTRL2, 1, &duty_cycle);

	pr_err("%s: pwm configured: duty_cycle: %d, duty_freq: %d\n", __func__,
						duty_cycle, frequency);

	return 0;
}
EXPORT_SYMBOL(pwm_config);
/*
int pwm_enable(struct pwm_device *pwm)
{

}
EXPORT_SYMBOL(pwm_enable);
*/
void pwm_disable(struct pwm_device *pwm)
{
	u8 freq_duty;
	int ret;

	freq_duty = 0;

	ret = tps_mfd->write(tps_mfd, TPS65911_PWM_CTRL2, 1, &freq_duty);
	if (ret < 0)
		pr_err("%s: Failed to disable PWM, Error %d\n",
			pwm->label, ret);
}
EXPORT_SYMBOL(pwm_disable);

struct pwm_device *pwm_request(int pwm_id, const char *label)
{
	struct pwm_device *pwm;

	pwm = kzalloc(sizeof(struct pwm_device), GFP_KERNEL);
	if (pwm == NULL) {
		pr_err("%s: failed to allocate memory\n", label);
		return NULL;
	}

	pwm->label = label;
	pwm->pwm_id = pwm_id;

	return pwm;
}
EXPORT_SYMBOL(pwm_request);

void pwm_free(struct pwm_device *pwm)
{
	pwm_disable(pwm);
	kfree(pwm);
}
EXPORT_SYMBOL(pwm_free);

void tps65911_pwm_init(struct tps65910 *tps65910)
{
	tps_mfd = tps65910;
}
EXPORT_SYMBOL(tps65911_pwm_init);
