/*
 * tps65910.h  --  TI TPS6591x
 *
 * Copyright 2010-2011 Texas Instruments Inc.
 *
 * Author: Liam Girdwood <lrg@slimlogic.co.uk>
 * Author: Graeme Gregory <gg@slimlogic.co.uk>
 * Author: Arnaud Deconinck <a-deconinck@ti.com>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under  the terms of the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the License, or (at your
 *  option) any later version.
 *
 */

#ifndef __LINUX_MFD_TPS65910_H
#define __LINUX_MFD_TPS65910_H

/* TPS chip id list */
#define TPS65910			910
#define TPS65911			911

/* TPS regulator type list */
#define REGULATOR_LDO			0
#define REGULATOR_DCDC			1

/* PWM id list */
#define PWM_ID				0
#define PWM_LED1_ID			1
#define PWM_LED2_ID			2

/*
 * List of registers for component TPS65910
 *
 */

#define TPS65910_SECONDS                                            0x0
#define TPS65910_MINUTES                                            0x1
#define TPS65910_HOURS                                              0x2
#define TPS65910_DAYS                                               0x3
#define TPS65910_MONTHS                                             0x4
#define TPS65910_YEARS                                              0x5
#define TPS65910_WEEKS                                              0x6
#define TPS65910_ALARM_SECONDS                                      0x8
#define TPS65910_ALARM_MINUTES                                      0x9
#define TPS65910_ALARM_HOURS                                        0xA
#define TPS65910_ALARM_DAYS                                         0xB
#define TPS65910_ALARM_MONTHS                                       0xC
#define TPS65910_ALARM_YEARS                                        0xD
#define TPS65910_RTC_CTRL                                           0x10
#define TPS65910_RTC_STATUS                                         0x11
#define TPS65910_RTC_INTERRUPTS                                     0x12
#define TPS65910_RTC_COMP_LSB                                       0x13
#define TPS65910_RTC_COMP_MSB                                       0x14
#define TPS65910_RTC_RES_PROG                                       0x15
#define TPS65910_RTC_RESET_STATUS                                   0x16
#define TPS65910_BCK1                                               0x17
#define TPS65910_BCK2                                               0x18
#define TPS65910_BCK3                                               0x19
#define TPS65910_BCK4                                               0x1A
#define TPS65910_BCK5                                               0x1B
#define TPS65910_PUADEN                                             0x1C
#define TPS65910_REF                                                0x1D
#define TPS65910_VRTC                                               0x1E
#define TPS65910_VIO                                                0x20
#define TPS65910_VDD1                                               0x21
#define TPS65910_VDD1_OP                                            0x22
#define TPS65910_VDD1_SR                                            0x23
#define TPS65910_VDD2                                               0x24
#define TPS65910_VDD2_OP                                            0x25
#define TPS65910_VDD2_SR                                            0x26
#define TPS65910_VDD3                                               0x27
#define TPS65910_VDIG1                                              0x30
#define TPS65910_VDIG2                                              0x31
#define TPS65910_VAUX1                                              0x32
#define TPS65910_VAUX2                                              0x33
#define TPS65910_VAUX33                                             0x34
#define TPS65910_VMMC                                               0x35
#define TPS65910_VPLL                                               0x36
#define TPS65910_VDAC                                               0x37
#define TPS65910_THERM                                              0x38
#define TPS65910_BBCH                                               0x39
#define TPS65910_DCDCCTRL                                           0x3E
#define TPS65910_DEVCTRL                                            0x3F
#define TPS65910_DEVCTRL2                                           0x40
#define TPS65910_SLEEP_KEEP_LDO_ON                                  0x41
#define TPS65910_SLEEP_KEEP_RES_ON                                  0x42
#define TPS65910_SLEEP_SET_LDO_OFF                                  0x43
#define TPS65910_SLEEP_SET_RES_OFF                                  0x44
#define TPS65910_EN1_LDO_ASS                                        0x45
#define TPS65910_EN1_SMPS_ASS                                       0x46
#define TPS65910_EN2_LDO_ASS                                        0x47
#define TPS65910_EN2_SMPS_ASS                                       0x48
#define TPS65910_EN3_LDO_ASS                                        0x49
#define TPS65910_SPARE                                              0x4A
#define TPS65910_INT_STS                                            0x50
#define TPS65910_INT_MSK                                            0x51
#define TPS65910_INT_STS2                                           0x52
#define TPS65910_INT_MSK2                                           0x53
#define TPS65910_INT_STS3                                           0x54
#define TPS65910_INT_MSK3                                           0x55
#define TPS65910_GPIO0                                              0x60
#define TPS65910_GPIO1                                              0x61
#define TPS65910_GPIO2                                              0x62
#define TPS65910_GPIO3                                              0x63
#define TPS65910_GPIO4                                              0x64
#define TPS65910_GPIO5                                              0x65
#define TPS65911_GPIO6						    0x66
#define TPS65911_GPIO7						    0x67
#define TPS65911_GPIO8						    0x68
#define TPS65910_VERNUM                                       	    0x80
#define TPS65910_MAX_REGISTER                                       0x80

/*
 * List of registers specific to TPS65911
 */
#define TPS65911_VDDCTRL					    0x27
#define TPS65911_VDDCTRL_OP					    0x28
#define TPS65911_VDDCTRL_SR					    0x29
#define TPS65911_LDO1						    0x30
#define TPS65911_LDO2						    0x31
#define TPS65911_LDO5						    0x32
#define TPS65911_LDO8						    0x33
#define TPS65911_LDO7						    0x34
#define TPS65911_LDO6						    0x35
#define TPS65911_LDO4						    0x36
#define TPS65911_LDO3						    0x37
#define TPS65911_WATCHDOG					    0x69
#define TPS65911_VMBCH						    0x6A
#define TPS65911_VMBCH2						    0x6B
#define TPS65911_LED_CTRL1					    0x6C
#define TPS65911_LED_CTRL2					    0x6D
#define TPS65911_PWM_CTRL1					    0X6E
#define TPS65911_PWM_CTRL2					    0x6F

/*
 * List of register bitfields for component TPS65910
 *
 */


/*Register SECONDS  (0x80) register.RegisterDescription */
#define SECONDS_SEC1_MASK                             0x70
#define SECONDS_SEC1_SHIFT                            4
#define SECONDS_SEC0_MASK                             0x0F
#define SECONDS_SEC0_SHIFT                            0


/*Register MINUTES  (0x80) register.RegisterDescription */
#define MINUTES_MIN1_MASK                             0x70
#define MINUTES_MIN1_SHIFT                            4
#define MINUTES_MIN0_MASK                             0x0F
#define MINUTES_MIN0_SHIFT                            0


/*Register HOURS  (0x80) register.RegisterDescription */
#define HOURS_PM_NAM_MASK                             0x80
#define HOURS_PM_NAM_SHIFT                            7
#define HOURS_HOUR1_MASK                              0x30
#define HOURS_HOUR1_SHIFT                             4
#define HOURS_HOUR0_MASK                              0x0F
#define HOURS_HOUR0_SHIFT                             0


/*Register DAYS  (0x80) register.RegisterDescription */
#define DAYS_DAY1_MASK                                0x30
#define DAYS_DAY1_SHIFT                               4
#define DAYS_DAY0_MASK                                0x0F
#define DAYS_DAY0_SHIFT                               0


/*Register MONTHS  (0x80) register.RegisterDescription */
#define MONTHS_MONTH1_MASK                            0x10
#define MONTHS_MONTH1_SHIFT                           4
#define MONTHS_MONTH0_MASK                            0x0F
#define MONTHS_MONTH0_SHIFT                           0


/*Register YEARS  (0x80) register.RegisterDescription */
#define YEARS_YEAR1_MASK                              0xF0
#define YEARS_YEAR1_SHIFT                             4
#define YEARS_YEAR0_MASK                              0x0F
#define YEARS_YEAR0_SHIFT                             0


/*Register WEEKS  (0x80) register.RegisterDescription */
#define WEEKS_WEEK_MASK                               0x07
#define WEEKS_WEEK_SHIFT                              0


/*Register ALARM_SECONDS  (0x80) register.RegisterDescription */
#define ALARM_SECONDS_ALARM_SEC1_MASK                 0x70
#define ALARM_SECONDS_ALARM_SEC1_SHIFT                4
#define ALARM_SECONDS_ALARM_SEC0_MASK                 0x0F
#define ALARM_SECONDS_ALARM_SEC0_SHIFT                0


/*Register ALARM_MINUTES  (0x80) register.RegisterDescription */
#define ALARM_MINUTES_ALARM_MIN1_MASK                 0x70
#define ALARM_MINUTES_ALARM_MIN1_SHIFT                4
#define ALARM_MINUTES_ALARM_MIN0_MASK                 0x0F
#define ALARM_MINUTES_ALARM_MIN0_SHIFT                0


/*Register ALARM_HOURS  (0x80) register.RegisterDescription */
#define ALARM_HOURS_ALARM_PM_NAM_MASK                 0x80
#define ALARM_HOURS_ALARM_PM_NAM_SHIFT                7
#define ALARM_HOURS_ALARM_HOUR1_MASK                  0x30
#define ALARM_HOURS_ALARM_HOUR1_SHIFT                 4
#define ALARM_HOURS_ALARM_HOUR0_MASK                  0x0F
#define ALARM_HOURS_ALARM_HOUR0_SHIFT                 0


/*Register ALARM_DAYS  (0x80) register.RegisterDescription */
#define ALARM_DAYS_ALARM_DAY1_MASK                    0x30
#define ALARM_DAYS_ALARM_DAY1_SHIFT                   4
#define ALARM_DAYS_ALARM_DAY0_MASK                    0x0F
#define ALARM_DAYS_ALARM_DAY0_SHIFT                   0


/*Register ALARM_MONTHS  (0x80) register.RegisterDescription */
#define ALARM_MONTHS_ALARM_MONTH1_MASK                0x10
#define ALARM_MONTHS_ALARM_MONTH1_SHIFT               4
#define ALARM_MONTHS_ALARM_MONTH0_MASK                0x0F
#define ALARM_MONTHS_ALARM_MONTH0_SHIFT               0


/*Register ALARM_YEARS  (0x80) register.RegisterDescription */
#define ALARM_YEARS_ALARM_YEAR1_MASK                  0xF0
#define ALARM_YEARS_ALARM_YEAR1_SHIFT                 4
#define ALARM_YEARS_ALARM_YEAR0_MASK                  0x0F
#define ALARM_YEARS_ALARM_YEAR0_SHIFT                 0


/*Register RTC_CTRL  (0x80) register.RegisterDescription */
#define RTC_CTRL_RTC_V_OPT_MASK                       0x80
#define RTC_CTRL_RTC_V_OPT_SHIFT                      7
#define RTC_CTRL_GET_TIME_MASK                        0x40
#define RTC_CTRL_GET_TIME_SHIFT                       6
#define RTC_CTRL_SET_32_COUNTER_MASK                  0x20
#define RTC_CTRL_SET_32_COUNTER_SHIFT                 5
#define RTC_CTRL_TEST_MODE_MASK                       0x10
#define RTC_CTRL_TEST_MODE_SHIFT                      4
#define RTC_CTRL_MODE_12_24_MASK                      0x08
#define RTC_CTRL_MODE_12_24_SHIFT                     3
#define RTC_CTRL_AUTO_COMP_MASK                       0x04
#define RTC_CTRL_AUTO_COMP_SHIFT                      2
#define RTC_CTRL_ROUND_30S_MASK                       0x02
#define RTC_CTRL_ROUND_30S_SHIFT                      1
#define RTC_CTRL_STOP_RTC_MASK                        0x01
#define RTC_CTRL_STOP_RTC_SHIFT                       0


/*Register RTC_STATUS  (0x80) register.RegisterDescription */
#define RTC_STATUS_POWER_UP_MASK                      0x80
#define RTC_STATUS_POWER_UP_SHIFT                     7
#define RTC_STATUS_ALARM_MASK                         0x40
#define RTC_STATUS_ALARM_SHIFT                        6
#define RTC_STATUS_EVENT_1D_MASK                      0x20
#define RTC_STATUS_EVENT_1D_SHIFT                     5
#define RTC_STATUS_EVENT_1H_MASK                      0x10
#define RTC_STATUS_EVENT_1H_SHIFT                     4
#define RTC_STATUS_EVENT_1M_MASK                      0x08
#define RTC_STATUS_EVENT_1M_SHIFT                     3
#define RTC_STATUS_EVENT_1S_MASK                      0x04
#define RTC_STATUS_EVENT_1S_SHIFT                     2
#define RTC_STATUS_RUN_MASK                           0x02
#define RTC_STATUS_RUN_SHIFT                          1


/*Register RTC_INTERRUPTS  (0x80) register.RegisterDescription */
#define RTC_INTERRUPTS_IT_SLEEP_MASK_EN_MASK          0x10
#define RTC_INTERRUPTS_IT_SLEEP_MASK_EN_SHIFT         4
#define RTC_INTERRUPTS_IT_ALARM_MASK                  0x08
#define RTC_INTERRUPTS_IT_ALARM_SHIFT                 3
#define RTC_INTERRUPTS_IT_TIMER_MASK                  0x04
#define RTC_INTERRUPTS_IT_TIMER_SHIFT                 2
#define RTC_INTERRUPTS_EVERY_MASK                     0x03
#define RTC_INTERRUPTS_EVERY_SHIFT                    0


/*Register RTC_COMP_LSB  (0x80) register.RegisterDescription */
#define RTC_COMP_LSB_RTC_COMP_LSB_MASK                0xFF
#define RTC_COMP_LSB_RTC_COMP_LSB_SHIFT               0


/*Register RTC_COMP_MSB  (0x80) register.RegisterDescription */
#define RTC_COMP_MSB_RTC_COMP_MSB_MASK                0xFF
#define RTC_COMP_MSB_RTC_COMP_MSB_SHIFT               0


/*Register RTC_RES_PROG  (0x80) register.RegisterDescription */
#define RTC_RES_PROG_SW_RES_PROG_MASK                 0x3F
#define RTC_RES_PROG_SW_RES_PROG_SHIFT                0


/*Register RTC_RESET_STATUS  (0x80) register.RegisterDescription */
#define RTC_RESET_STATUS_RESET_STATUS_MASK            0x01
#define RTC_RESET_STATUS_RESET_STATUS_SHIFT           0


/*Register BCK1  (0x80) register.RegisterDescription */
#define BCK1_BCKUP_MASK                               0xFF
#define BCK1_BCKUP_SHIFT                              0


/*Register BCK2  (0x80) register.RegisterDescription */
#define BCK2_BCKUP_MASK                               0xFF
#define BCK2_BCKUP_SHIFT                              0


/*Register BCK3  (0x80) register.RegisterDescription */
#define BCK3_BCKUP_MASK                               0xFF
#define BCK3_BCKUP_SHIFT                              0


/*Register BCK4  (0x80) register.RegisterDescription */
#define BCK4_BCKUP_MASK                               0xFF
#define BCK4_BCKUP_SHIFT                              0


/*Register BCK5  (0x80) register.RegisterDescription */
#define BCK5_BCKUP_MASK                               0xFF
#define BCK5_BCKUP_SHIFT                              0


/*Register PUADEN  (0x80) register.RegisterDescription */
#define PUADEN_EN3P_MASK                              0x80
#define PUADEN_EN3P_SHIFT                             7
#define PUADEN_I2CCTLP_MASK                           0x40
#define PUADEN_I2CCTLP_SHIFT                          6
#define PUADEN_I2CSRP_MASK                            0x20
#define PUADEN_I2CSRP_SHIFT                           5
#define PUADEN_PWRONP_MASK                            0x10
#define PUADEN_PWRONP_SHIFT                           4
#define PUADEN_SLEEPP_MASK                            0x08
#define PUADEN_SLEEPP_SHIFT                           3
#define PUADEN_PWRHOLDP_MASK                          0x04
#define PUADEN_PWRHOLDP_SHIFT                         2
#define PUADEN_BOOT1P_MASK                            0x02
#define PUADEN_BOOT1P_SHIFT                           1
#define PUADEN_BOOT0P_MASK                            0x01
#define PUADEN_BOOT0P_SHIFT                           0


/*Register REF  (0x80) register.RegisterDescription */
#define REF_VMBCH_SEL_MASK                            0x0C
#define REF_VMBCH_SEL_SHIFT                           2
#define REF_ST_MASK                                   0x03
#define REF_ST_SHIFT                                  0


/*Register VRTC  (0x80) register.RegisterDescription */
#define VRTC_VRTC_OFFMASK_MASK                        0x08
#define VRTC_VRTC_OFFMASK_SHIFT                       3
#define VRTC_ST_MASK                                  0x03
#define VRTC_ST_SHIFT                                 0


/*Register VIO  (0x80) register.RegisterDescription */
#define VIO_ILMAX_MASK                                0xC0
#define VIO_ILMAX_SHIFT                               6
#define VIO_SEL_MASK                                  0x0C
#define VIO_SEL_SHIFT                                 2
#define VIO_ST_MASK                                   0x03
#define VIO_ST_SHIFT                                  0


/*Register VDD1  (0x80) register.RegisterDescription */
#define VDD1_VGAIN_SEL_MASK                           0xC0
#define VDD1_VGAIN_SEL_SHIFT                          6
#define VDD1_ILMAX_MASK                               0x20
#define VDD1_ILMAX_SHIFT                              5
#define VDD1_TSTEP_MASK                               0x1C
#define VDD1_TSTEP_SHIFT                              2
#define VDD1_ST_MASK                                  0x03
#define VDD1_ST_SHIFT                                 0


/*Register VDD1_OP  (0x80) register.RegisterDescription */
#define VDD1_OP_CMD_MASK                              0x80
#define VDD1_OP_CMD_SHIFT                             7
#define VDD1_OP_SEL_MASK                              0x7F
#define VDD1_OP_SEL_SHIFT                             0


/*Register VDD1_SR  (0x80) register.RegisterDescription */
#define VDD1_SR_SEL_MASK                              0x7F
#define VDD1_SR_SEL_SHIFT                             0


/*Register VDD2  (0x80) register.RegisterDescription */
#define VDD2_VGAIN_SEL_MASK                           0xC0
#define VDD2_VGAIN_SEL_SHIFT                          6
#define VDD2_ILMAX_MASK                               0x20
#define VDD2_ILMAX_SHIFT                              5
#define VDD2_TSTEP_MASK                               0x1C
#define VDD2_TSTEP_SHIFT                              2
#define VDD2_ST_MASK                                  0x03
#define VDD2_ST_SHIFT                                 0


/*Register VDD2_OP  (0x80) register.RegisterDescription */
#define VDD2_OP_CMD_MASK                              0x80
#define VDD2_OP_CMD_SHIFT                             7
#define VDD2_OP_SEL_MASK                              0x7F
#define VDD2_OP_SEL_SHIFT                             0


/*Register VDD2_SR  (0x80) register.RegisterDescription */
#define VDD2_SR_SEL_MASK                              0x7F
#define VDD2_SR_SEL_SHIFT                             0


/*Register VDD3  (0x80) register.RegisterDescription */
#define VDD3_CKINEN_MASK                              0x04
#define VDD3_CKINEN_SHIFT                             2
#define VDD3_ST_MASK                                  0x03
#define VDD3_ST_SHIFT                                 0


/*Register VDIG1  (0x80) register.RegisterDescription */
#define VDIG1_SEL_MASK                                0x0C
#define VDIG1_SEL_SHIFT                               2
#define VDIG1_ST_MASK                                 0x03
#define VDIG1_ST_SHIFT                                0


/*Register VDIG2  (0x80) register.RegisterDescription */
#define VDIG2_SEL_MASK                                0x0C
#define VDIG2_SEL_SHIFT                               2
#define VDIG2_ST_MASK                                 0x03
#define VDIG2_ST_SHIFT                                0


/*Register VAUX1  (0x80) register.RegisterDescription */
#define VAUX1_SEL_MASK                                0x0C
#define VAUX1_SEL_SHIFT                               2
#define VAUX1_ST_MASK                                 0x03
#define VAUX1_ST_SHIFT                                0


/*Register VAUX2  (0x80) register.RegisterDescription */
#define VAUX2_SEL_MASK                                0x0C
#define VAUX2_SEL_SHIFT                               2
#define VAUX2_ST_MASK                                 0x03
#define VAUX2_ST_SHIFT                                0


/*Register VAUX33  (0x80) register.RegisterDescription */
#define VAUX33_SEL_MASK                               0x0C
#define VAUX33_SEL_SHIFT                              2
#define VAUX33_ST_MASK                                0x03
#define VAUX33_ST_SHIFT                               0


/*Register VMMC  (0x80) register.RegisterDescription */
#define VMMC_SEL_MASK                                 0x0C
#define VMMC_SEL_SHIFT                                2
#define VMMC_ST_MASK                                  0x03
#define VMMC_ST_SHIFT                                 0


/*Register VPLL  (0x80) register.RegisterDescription */
#define VPLL_SEL_MASK                                 0x0C
#define VPLL_SEL_SHIFT                                2
#define VPLL_ST_MASK                                  0x03
#define VPLL_ST_SHIFT                                 0


/*Register VDAC  (0x80) register.RegisterDescription */
#define VDAC_SEL_MASK                                 0x0C
#define VDAC_SEL_SHIFT                                2
#define VDAC_ST_MASK                                  0x03
#define VDAC_ST_SHIFT                                 0

/* LDO Masks*/
#define LDO_SEL_MASK				      0xFC
#define LDO_SEL_SHIFT				      2
#define LDO_ST_MASK				      0x03
#define LDO_ST_SHIFT				      0


/*Register THERM  (0x80) register.RegisterDescription */
#define THERM_THERM_HD_MASK                           0x20
#define THERM_THERM_HD_SHIFT                          5
#define THERM_THERM_TS_MASK                           0x10
#define THERM_THERM_TS_SHIFT                          4
#define THERM_THERM_HDSEL_MASK                        0x0C
#define THERM_THERM_HDSEL_SHIFT                       2
#define THERM_RSVD1_MASK                              0x02
#define THERM_RSVD1_SHIFT                             1
#define THERM_THERM_STATE_MASK                        0x01
#define THERM_THERM_STATE_SHIFT                       0


/*Register BBCH  (0x80) register.RegisterDescription */
#define BBCH_BBSEL_MASK                               0x06
#define BBCH_BBSEL_SHIFT                              1
#define BBCH_BBCHEN_MASK                              0x01
#define BBCH_BBCHEN_SHIFT                             0


/*Register DCDCCTRL  (0x80) register.RegisterDescription */
#define DCDCCTRL_VDD2_PSKIP_MASK                      0x20
#define DCDCCTRL_VDD2_PSKIP_SHIFT                     5
#define DCDCCTRL_VDD1_PSKIP_MASK                      0x10
#define DCDCCTRL_VDD1_PSKIP_SHIFT                     4
#define DCDCCTRL_VIO_PSKIP_MASK                       0x08
#define DCDCCTRL_VIO_PSKIP_SHIFT                      3
#define DCDCCTRL_DCDCCKEXT_MASK                       0x04
#define DCDCCTRL_DCDCCKEXT_SHIFT                      2
#define DCDCCTRL_DCDCCKSYNC_MASK                      0x03
#define DCDCCTRL_DCDCCKSYNC_SHIFT                     0


/*Register DEVCTRL  (0x80) register.RegisterDescription */
#define DEVCTRL_RTC_PWDN_MASK                         0x40
#define DEVCTRL_RTC_PWDN_SHIFT                        6
#define DEVCTRL_CK32K_CTRL_MASK                       0x20
#define DEVCTRL_CK32K_CTRL_SHIFT                      5
#define DEVCTRL_SR_CTL_I2C_SEL_MASK                   0x10
#define DEVCTRL_SR_CTL_I2C_SEL_SHIFT                  4
#define DEVCTRL_DEV_OFF_RST_MASK                      0x08
#define DEVCTRL_DEV_OFF_RST_SHIFT                     3
#define DEVCTRL_DEV_ON_MASK                           0x04
#define DEVCTRL_DEV_ON_SHIFT                          2
#define DEVCTRL_DEV_SLP_MASK                          0x02
#define DEVCTRL_DEV_SLP_SHIFT                         1
#define DEVCTRL_DEV_OFF_MASK                          0x01
#define DEVCTRL_DEV_OFF_SHIFT                         0


/*Register DEVCTRL2  (0x80) register.RegisterDescription */
#define DEVCTRL2_TSLOT_LENGTH_MASK                    0x30
#define DEVCTRL2_TSLOT_LENGTH_SHIFT                   4
#define DEVCTRL2_SLEEPSIG_POL_MASK                    0x08
#define DEVCTRL2_SLEEPSIG_POL_SHIFT                   3
#define DEVCTRL2_PWON_LP_OFF_MASK                     0x04
#define DEVCTRL2_PWON_LP_OFF_SHIFT                    2
#define DEVCTRL2_PWON_LP_RST_MASK                     0x02
#define DEVCTRL2_PWON_LP_RST_SHIFT                    1
#define DEVCTRL2_IT_POL_MASK                          0x01
#define DEVCTRL2_IT_POL_SHIFT                         0


/*Register SLEEP_KEEP_LDO_ON  (0x80) register.RegisterDescription */
#define SLEEP_KEEP_LDO_ON_VDAC_KEEPON_MASK            0x80
#define SLEEP_KEEP_LDO_ON_VDAC_KEEPON_SHIFT           7
#define SLEEP_KEEP_LDO_ON_VPLL_KEEPON_MASK            0x40
#define SLEEP_KEEP_LDO_ON_VPLL_KEEPON_SHIFT           6
#define SLEEP_KEEP_LDO_ON_VAUX33_KEEPON_MASK          0x20
#define SLEEP_KEEP_LDO_ON_VAUX33_KEEPON_SHIFT         5
#define SLEEP_KEEP_LDO_ON_VAUX2_KEEPON_MASK           0x10
#define SLEEP_KEEP_LDO_ON_VAUX2_KEEPON_SHIFT          4
#define SLEEP_KEEP_LDO_ON_VAUX1_KEEPON_MASK           0x08
#define SLEEP_KEEP_LDO_ON_VAUX1_KEEPON_SHIFT          3
#define SLEEP_KEEP_LDO_ON_VDIG2_KEEPON_MASK           0x04
#define SLEEP_KEEP_LDO_ON_VDIG2_KEEPON_SHIFT          2
#define SLEEP_KEEP_LDO_ON_VDIG1_KEEPON_MASK           0x02
#define SLEEP_KEEP_LDO_ON_VDIG1_KEEPON_SHIFT          1
#define SLEEP_KEEP_LDO_ON_VMMC_KEEPON_MASK            0x01
#define SLEEP_KEEP_LDO_ON_VMMC_KEEPON_SHIFT           0


/*Register SLEEP_KEEP_RES_ON  (0x80) register.RegisterDescription */
#define SLEEP_KEEP_RES_ON_THERM_KEEPON_MASK           0x80
#define SLEEP_KEEP_RES_ON_THERM_KEEPON_SHIFT          7
#define SLEEP_KEEP_RES_ON_CLKOUT32K_KEEPON_MASK       0x40
#define SLEEP_KEEP_RES_ON_CLKOUT32K_KEEPON_SHIFT      6
#define SLEEP_KEEP_RES_ON_VRTC_KEEPON_MASK            0x20
#define SLEEP_KEEP_RES_ON_VRTC_KEEPON_SHIFT           5
#define SLEEP_KEEP_RES_ON_I2CHS_KEEPON_MASK           0x10
#define SLEEP_KEEP_RES_ON_I2CHS_KEEPON_SHIFT          4
#define SLEEP_KEEP_RES_ON_VDD3_KEEPON_MASK            0x08
#define SLEEP_KEEP_RES_ON_VDD3_KEEPON_SHIFT           3
#define SLEEP_KEEP_RES_ON_VDD2_KEEPON_MASK            0x04
#define SLEEP_KEEP_RES_ON_VDD2_KEEPON_SHIFT           2
#define SLEEP_KEEP_RES_ON_VDD1_KEEPON_MASK            0x02
#define SLEEP_KEEP_RES_ON_VDD1_KEEPON_SHIFT           1
#define SLEEP_KEEP_RES_ON_VIO_KEEPON_MASK             0x01
#define SLEEP_KEEP_RES_ON_VIO_KEEPON_SHIFT            0


/*Register SLEEP_SET_LDO_OFF  (0x80) register.RegisterDescription */
#define SLEEP_SET_LDO_OFF_VDAC_SETOFF_MASK            0x80
#define SLEEP_SET_LDO_OFF_VDAC_SETOFF_SHIFT           7
#define SLEEP_SET_LDO_OFF_VPLL_SETOFF_MASK            0x40
#define SLEEP_SET_LDO_OFF_VPLL_SETOFF_SHIFT           6
#define SLEEP_SET_LDO_OFF_VAUX33_SETOFF_MASK          0x20
#define SLEEP_SET_LDO_OFF_VAUX33_SETOFF_SHIFT         5
#define SLEEP_SET_LDO_OFF_VAUX2_SETOFF_MASK           0x10
#define SLEEP_SET_LDO_OFF_VAUX2_SETOFF_SHIFT          4
#define SLEEP_SET_LDO_OFF_VAUX1_SETOFF_MASK           0x08
#define SLEEP_SET_LDO_OFF_VAUX1_SETOFF_SHIFT          3
#define SLEEP_SET_LDO_OFF_VDIG2_SETOFF_MASK           0x04
#define SLEEP_SET_LDO_OFF_VDIG2_SETOFF_SHIFT          2
#define SLEEP_SET_LDO_OFF_VDIG1_SETOFF_MASK           0x02
#define SLEEP_SET_LDO_OFF_VDIG1_SETOFF_SHIFT          1
#define SLEEP_SET_LDO_OFF_VMMC_SETOFF_MASK            0x01
#define SLEEP_SET_LDO_OFF_VMMC_SETOFF_SHIFT           0


/*Register SLEEP_SET_RES_OFF  (0x80) register.RegisterDescription */
#define SLEEP_SET_RES_OFF_DEFAULT_VOLT_MASK           0x80
#define SLEEP_SET_RES_OFF_DEFAULT_VOLT_SHIFT          7
#define SLEEP_SET_RES_OFF_RSVD_MASK                   0x60
#define SLEEP_SET_RES_OFF_RSVD_SHIFT                  5
#define SLEEP_SET_RES_OFF_SPARE_SETOFF_MASK           0x10
#define SLEEP_SET_RES_OFF_SPARE_SETOFF_SHIFT          4
#define SLEEP_SET_RES_OFF_VDD3_SETOFF_MASK            0x08
#define SLEEP_SET_RES_OFF_VDD3_SETOFF_SHIFT           3
#define SLEEP_SET_RES_OFF_VDD2_SETOFF_MASK            0x04
#define SLEEP_SET_RES_OFF_VDD2_SETOFF_SHIFT           2
#define SLEEP_SET_RES_OFF_VDD1_SETOFF_MASK            0x02
#define SLEEP_SET_RES_OFF_VDD1_SETOFF_SHIFT           1
#define SLEEP_SET_RES_OFF_VIO_SETOFF_MASK             0x01
#define SLEEP_SET_RES_OFF_VIO_SETOFF_SHIFT            0


/*Register EN1_LDO_ASS  (0x80) register.RegisterDescription */
#define EN1_LDO_ASS_VDAC_EN1_MASK                     0x80
#define EN1_LDO_ASS_VDAC_EN1_SHIFT                    7
#define EN1_LDO_ASS_VPLL_EN1_MASK                     0x40
#define EN1_LDO_ASS_VPLL_EN1_SHIFT                    6
#define EN1_LDO_ASS_VAUX33_EN1_MASK                   0x20
#define EN1_LDO_ASS_VAUX33_EN1_SHIFT                  5
#define EN1_LDO_ASS_VAUX2_EN1_MASK                    0x10
#define EN1_LDO_ASS_VAUX2_EN1_SHIFT                   4
#define EN1_LDO_ASS_VAUX1_EN1_MASK                    0x08
#define EN1_LDO_ASS_VAUX1_EN1_SHIFT                   3
#define EN1_LDO_ASS_VDIG2_EN1_MASK                    0x04
#define EN1_LDO_ASS_VDIG2_EN1_SHIFT                   2
#define EN1_LDO_ASS_VDIG1_EN1_MASK                    0x02
#define EN1_LDO_ASS_VDIG1_EN1_SHIFT                   1
#define EN1_LDO_ASS_VMMC_EN1_MASK                     0x01
#define EN1_LDO_ASS_VMMC_EN1_SHIFT                    0


/*Register EN1_SMPS_ASS  (0x80) register.RegisterDescription */
#define EN1_SMPS_ASS_RSVD_MASK                        0xE0
#define EN1_SMPS_ASS_RSVD_SHIFT                       5
#define EN1_SMPS_ASS_SPARE_EN1_MASK                   0x10
#define EN1_SMPS_ASS_SPARE_EN1_SHIFT                  4
#define EN1_SMPS_ASS_VDD3_EN1_MASK                    0x08
#define EN1_SMPS_ASS_VDD3_EN1_SHIFT                   3
#define EN1_SMPS_ASS_VDD2_EN1_MASK                    0x04
#define EN1_SMPS_ASS_VDD2_EN1_SHIFT                   2
#define EN1_SMPS_ASS_VDD1_EN1_MASK                    0x02
#define EN1_SMPS_ASS_VDD1_EN1_SHIFT                   1
#define EN1_SMPS_ASS_VIO_EN1_MASK                     0x01
#define EN1_SMPS_ASS_VIO_EN1_SHIFT                    0


/*Register EN2_LDO_ASS  (0x80) register.RegisterDescription */
#define EN2_LDO_ASS_VDAC_EN2_MASK                     0x80
#define EN2_LDO_ASS_VDAC_EN2_SHIFT                    7
#define EN2_LDO_ASS_VPLL_EN2_MASK                     0x40
#define EN2_LDO_ASS_VPLL_EN2_SHIFT                    6
#define EN2_LDO_ASS_VAUX33_EN2_MASK                   0x20
#define EN2_LDO_ASS_VAUX33_EN2_SHIFT                  5
#define EN2_LDO_ASS_VAUX2_EN2_MASK                    0x10
#define EN2_LDO_ASS_VAUX2_EN2_SHIFT                   4
#define EN2_LDO_ASS_VAUX1_EN2_MASK                    0x08
#define EN2_LDO_ASS_VAUX1_EN2_SHIFT                   3
#define EN2_LDO_ASS_VDIG2_EN2_MASK                    0x04
#define EN2_LDO_ASS_VDIG2_EN2_SHIFT                   2
#define EN2_LDO_ASS_VDIG1_EN2_MASK                    0x02
#define EN2_LDO_ASS_VDIG1_EN2_SHIFT                   1
#define EN2_LDO_ASS_VMMC_EN2_MASK                     0x01
#define EN2_LDO_ASS_VMMC_EN2_SHIFT                    0


/*Register EN2_SMPS_ASS  (0x80) register.RegisterDescription */
#define EN2_SMPS_ASS_RSVD_MASK                        0xE0
#define EN2_SMPS_ASS_RSVD_SHIFT                       5
#define EN2_SMPS_ASS_SPARE_EN2_MASK                   0x10
#define EN2_SMPS_ASS_SPARE_EN2_SHIFT                  4
#define EN2_SMPS_ASS_VDD3_EN2_MASK                    0x08
#define EN2_SMPS_ASS_VDD3_EN2_SHIFT                   3
#define EN2_SMPS_ASS_VDD2_EN2_MASK                    0x04
#define EN2_SMPS_ASS_VDD2_EN2_SHIFT                   2
#define EN2_SMPS_ASS_VDD1_EN2_MASK                    0x02
#define EN2_SMPS_ASS_VDD1_EN2_SHIFT                   1
#define EN2_SMPS_ASS_VIO_EN2_MASK                     0x01
#define EN2_SMPS_ASS_VIO_EN2_SHIFT                    0


/*Register EN3_LDO_ASS  (0x80) register.RegisterDescription */
#define EN3_LDO_ASS_VDAC_EN3_MASK                     0x80
#define EN3_LDO_ASS_VDAC_EN3_SHIFT                    7
#define EN3_LDO_ASS_VPLL_EN3_MASK                     0x40
#define EN3_LDO_ASS_VPLL_EN3_SHIFT                    6
#define EN3_LDO_ASS_VAUX33_EN3_MASK                   0x20
#define EN3_LDO_ASS_VAUX33_EN3_SHIFT                  5
#define EN3_LDO_ASS_VAUX2_EN3_MASK                    0x10
#define EN3_LDO_ASS_VAUX2_EN3_SHIFT                   4
#define EN3_LDO_ASS_VAUX1_EN3_MASK                    0x08
#define EN3_LDO_ASS_VAUX1_EN3_SHIFT                   3
#define EN3_LDO_ASS_VDIG2_EN3_MASK                    0x04
#define EN3_LDO_ASS_VDIG2_EN3_SHIFT                   2
#define EN3_LDO_ASS_VDIG1_EN3_MASK                    0x02
#define EN3_LDO_ASS_VDIG1_EN3_SHIFT                   1
#define EN3_LDO_ASS_VMMC_EN3_MASK                     0x01
#define EN3_LDO_ASS_VMMC_EN3_SHIFT                    0


/*Register SPARE  (0x80) register.RegisterDescription */
#define SPARE_SPARE_MASK                              0xFF
#define SPARE_SPARE_SHIFT                             0


/*Register INT_STS  (0x80) register.RegisterDescription */
#define INT_STS_RTC_PERIOD_IT_MASK                    0x80
#define INT_STS_RTC_PERIOD_IT_SHIFT                   7
#define INT_STS_RTC_ALARM_IT_MASK                     0x40
#define INT_STS_RTC_ALARM_IT_SHIFT                    6
#define INT_STS_HOTDIE_IT_MASK                        0x20
#define INT_STS_HOTDIE_IT_SHIFT                       5
#define INT_STS_PWRHOLD_IT_MASK                       0x10
#define INT_STS_PWRHOLD_IT_SHIFT                      4
#define INT_STS_PWRON_LP_IT_MASK                      0x08
#define INT_STS_PWRON_LP_IT_SHIFT                     3
#define INT_STS_PWRON_IT_MASK                         0x04
#define INT_STS_PWRON_IT_SHIFT                        2
#define INT_STS_VMBHI_IT_MASK                         0x02
#define INT_STS_VMBHI_IT_SHIFT                        1
#define INT_STS_VMBDCH_IT_MASK                        0x01
#define INT_STS_VMBDCH_IT_SHIFT                       0


/*Register INT_MSK  (0x80) register.RegisterDescription */
#define INT_MSK_RTC_PERIOD_IT_MSK_MASK                0x80
#define INT_MSK_RTC_PERIOD_IT_MSK_SHIFT               7
#define INT_MSK_RTC_ALARM_IT_MSK_MASK                 0x40
#define INT_MSK_RTC_ALARM_IT_MSK_SHIFT                6
#define INT_MSK_HOTDIE_IT_MSK_MASK                    0x20
#define INT_MSK_HOTDIE_IT_MSK_SHIFT                   5
#define INT_MSK_PWRHOLD_IT_MSK_MASK                   0x10
#define INT_MSK_PWRHOLD_IT_MSK_SHIFT                  4
#define INT_MSK_PWRON_LP_IT_MSK_MASK                  0x08
#define INT_MSK_PWRON_LP_IT_MSK_SHIFT                 3
#define INT_MSK_PWRON_IT_MSK_MASK                     0x04
#define INT_MSK_PWRON_IT_MSK_SHIFT                    2
#define INT_MSK_VMBHI_IT_MSK_MASK                     0x02
#define INT_MSK_VMBHI_IT_MSK_SHIFT                    1
#define INT_MSK_VMBDCH_IT_MSK_MASK                    0x01
#define INT_MSK_VMBDCH_IT_MSK_SHIFT                   0


/*Register INT_STS2  (0x80) register.RegisterDescription */
#define INT_STS2_GPIO3_F_IT_MASK                      0x80
#define INT_STS2_GPIO3_F_IT_SHIFT                     7
#define INT_STS2_GPIO3_R_IT_MASK                      0x40
#define INT_STS2_GPIO3_R_IT_SHIFT                     6
#define INT_STS2_GPIO2_F_IT_MASK                      0x20
#define INT_STS2_GPIO2_F_IT_SHIFT                     5
#define INT_STS2_GPIO2_R_IT_MASK                      0x10
#define INT_STS2_GPIO2_R_IT_SHIFT                     4
#define INT_STS2_GPIO1_F_IT_MASK                      0x08
#define INT_STS2_GPIO1_F_IT_SHIFT                     3
#define INT_STS2_GPIO1_R_IT_MASK                      0x04
#define INT_STS2_GPIO1_R_IT_SHIFT                     2
#define INT_STS2_GPIO0_F_IT_MASK                      0x02
#define INT_STS2_GPIO0_F_IT_SHIFT                     1
#define INT_STS2_GPIO0_R_IT_MASK                      0x01
#define INT_STS2_GPIO0_R_IT_SHIFT                     0


/*Register INT_MSK2  (0x80) register.RegisterDescription */
#define INT_MSK2_GPIO3_F_IT_MSK_MASK                  0x80
#define INT_MSK2_GPIO3_F_IT_MSK_SHIFT                 7
#define INT_MSK2_GPIO3_R_IT_MSK_MASK                  0x40
#define INT_MSK2_GPIO3_R_IT_MSK_SHIFT                 6
#define INT_MSK2_GPIO2_F_IT_MSK_MASK                  0x20
#define INT_MSK2_GPIO2_F_IT_MSK_SHIFT                 5
#define INT_MSK2_GPIO2_R_IT_MSK_MASK                  0x10
#define INT_MSK2_GPIO2_R_IT_MSK_SHIFT                 4
#define INT_MSK2_GPIO1_F_IT_MSK_MASK                  0x08
#define INT_MSK2_GPIO1_F_IT_MSK_SHIFT                 3
#define INT_MSK2_GPIO1_R_IT_MSK_MASK                  0x04
#define INT_MSK2_GPIO1_R_IT_MSK_SHIFT                 2
#define INT_MSK2_GPIO0_F_IT_MSK_MASK                  0x02
#define INT_MSK2_GPIO0_F_IT_MSK_SHIFT                 1
#define INT_MSK2_GPIO0_R_IT_MSK_MASK                  0x01
#define INT_MSK2_GPIO0_R_IT_MSK_SHIFT                 0


/*Register INT_STS3  (0x80) register.RegisterDescription */
#define INT_STS3_GPIO5_F_IT_MASK                      0x08
#define INT_STS3_GPIO5_F_IT_SHIFT                     3
#define INT_STS3_GPIO5_R_IT_MASK                      0x04
#define INT_STS3_GPIO5_R_IT_SHIFT                     2
#define INT_STS3_GPIO4_F_IT_MASK                      0x02
#define INT_STS3_GPIO4_F_IT_SHIFT                     1
#define INT_STS3_GPIO4_R_IT_MASK                      0x01
#define INT_STS3_GPIO4_R_IT_SHIFT                     0


/*Register INT_MSK3  (0x80) register.RegisterDescription */
#define INT_MSK3_GPIO5_F_IT_MSK_MASK                  0x08
#define INT_MSK3_GPIO5_F_IT_MSK_SHIFT                 3
#define INT_MSK3_GPIO5_R_IT_MSK_MASK                  0x04
#define INT_MSK3_GPIO5_R_IT_MSK_SHIFT                 2
#define INT_MSK3_GPIO4_F_IT_MSK_MASK                  0x02
#define INT_MSK3_GPIO4_F_IT_MSK_SHIFT                 1
#define INT_MSK3_GPIO4_R_IT_MSK_MASK                  0x01
#define INT_MSK3_GPIO4_R_IT_MSK_SHIFT                 0


/*Register GPIO  (0x80) register.RegisterDescription */
#define GPIO_DEB_MASK                           0x10
#define GPIO_DEB_SHIFT                          4
#define GPIO_PUEN_MASK                          0x08
#define GPIO_PUEN_SHIFT                         3
#define GPIO_CFG_MASK                           0x04
#define GPIO_CFG_SHIFT                          2
#define GPIO_STS_MASK                           0x02
#define GPIO_STS_SHIFT                          1
#define GPIO_SET_MASK                           0x01
#define GPIO_SET_SHIFT                          0


/*Register JTAGVERNUM  (0x80) register.RegisterDescription */
#define JTAGVERNUM_VERNUM_MASK                        0x0F
#define JTAGVERNUM_VERNUM_SHIFT                       0


/* Register VDDCTRL (0x27) bit definitions */
#define VDDCTRL_ST_MASK                                  0x03
#define VDDCTRL_ST_SHIFT                                 0


/*Register VDDCTRL_OP  (0x28) bit definitios */
#define VDDCTRL_OP_CMD_MASK                              0x80
#define VDDCTRL_OP_CMD_SHIFT                             7
#define VDDCTRL_OP_SEL_MASK                              0x7F
#define VDDCTRL_OP_SEL_SHIFT                             0


/*Register VDDCTRL_SR  (0x29) bit definitions */
#define VDDCTRL_SR_SEL_MASK                              0x7F
#define VDDCTRL_SR_SEL_SHIFT                             0


/* IRQ Definitions */
#define TPS65910_IRQ_VBAT_VMBDCH		0
#define TPS65910_IRQ_VBAT_VMHI			1
#define TPS65910_IRQ_PWRON			2
#define TPS65910_IRQ_PWRON_LP			3
#define TPS65910_IRQ_PWRHOLD			4
#define TPS65910_IRQ_HOTDIE			5
#define TPS65910_IRQ_RTC_ALARM			6
#define TPS65910_IRQ_RTC_PERIOD			7
#define TPS65910_IRQ_GPIO_R			8
#define TPS65910_IRQ_GPIO_F			9
#define TPS65910_NUM_IRQ			10

#define TPS65911_IRQ_VBAT_VMBDCH		0
#define TPS65911_IRQ_VBAT_VMBDCH2L		1
#define TPS65911_IRQ_VBAT_VMBDCH2H		2
#define TPS65911_IRQ_VBAT_VMHI			3
#define TPS65911_IRQ_PWRON			4
#define TPS65911_IRQ_PWRON_LP			5
#define TPS65911_IRQ_PWRHOLD_F			6
#define TPS65911_IRQ_PWRHOLD_R			7
#define TPS65911_IRQ_HOTDIE			8
#define TPS65911_IRQ_RTC_ALARM			9
#define TPS65911_IRQ_RTC_PERIOD			10
#define TPS65911_IRQ_GPIO0_R			11
#define TPS65911_IRQ_GPIO0_F			12
#define TPS65911_IRQ_GPIO1_R			13
#define TPS65911_IRQ_GPIO1_F			14
#define TPS65911_IRQ_GPIO2_R			15
#define TPS65911_IRQ_GPIO2_F			16
#define TPS65911_IRQ_GPIO3_R			17
#define TPS65911_IRQ_GPIO3_F			18
#define TPS65911_IRQ_GPIO4_R			19
#define TPS65911_IRQ_GPIO4_F			20
#define TPS65911_IRQ_GPIO5_R			21
#define TPS65911_IRQ_GPIO5_F			22
#define TPS65911_IRQ_WTCHDG			23
#define TPS65911_IRQ_PWRDN			24

#define TPS65911_NUM_IRQ			25


/* GPIO Register Definitions */
#define TPS65910_GPIO_DEB			BIT(2)
#define TPS65910_GPIO_PUEN			BIT(3)
#define TPS65910_GPIO_CFG			BIT(2)
#define TPS65910_GPIO_STS			BIT(1)
#define TPS65910_GPIO_SET			BIT(0)

/* LED_CTRL Register Definitions */
#define LED_CTRL1_LED1_OFF			0xF8
#define LED_CTRL1_LED2_OFF			0xC7
#define LED_CTRL1_LED1_PERIOD_MASK		0x07
#define LED_CTRL1_LED2_PERIOD_MASK		0x38
#define LED_CTRL2_LED1_ON_TIME_MASK		0x03
#define LED_CTRL2_LED2_ON_TIME_MASK		0X0c
#define LED_CTRL1_LED2_OFFSET			3
#define LED_CTRL2_LED2_OFFSET			2

/* WATCHDOG Register Definitions */
#define WTCHDG_TIME_MASK			0x07
#define WTCHDG_MODE_MASK			0x08
#define WTCHDG_TIME_OFFSET			0
#define WTCHDG_MODE_OFFSET			3

#define WTCHDG_TIME_DEFAULT_ACTIVE		0x111
#define WTCHDG_TIME_INACTIVE			0X000

#define WTCHDG_IT_MASK				0x10

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
	int irq_num;
	u32 irq_mask;
};

struct tps65910_platform_data {
	int irq_base;
	int gpio_base;
	int vmbch_threshold;
	int vmbch2_threshold;
	struct regulator_init_data *tps65910_pmic_init_data;
};

struct pmic_data {
	int id;
	void * pmic;
};

int tps65910_irq_init(struct tps65910 *tps65910, int irq,
		    struct tps65910_platform_data *pdata);

void tps6591x_gpio_init(struct tps65910 *tps65910, int gpio_base);

void tps65911_pwm_init(struct tps65910 *tps65910);

unsigned int tps_chip(void);

#endif /*  __LINUX_MFD_TPS65910_H */
