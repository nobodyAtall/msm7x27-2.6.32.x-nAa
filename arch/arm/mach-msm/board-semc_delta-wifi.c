/*
 * Copyright (C) 2013 Vassilis Tsogkas (tsogkas@ceid.upatras.gr)
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <asm/gpio.h>
#include <asm/mach/mmc.h>
#include <linux/wl12xx.h>

#define DELTA_WIFI_PMENA_GPIO	93  //delta
#define DELTA_WIFI_IRQ_GPIO	27  //delta

static char *driver = "WL12XX";

module_param(driver, charp, 0000);
MODULE_PARM_DESC(driver, "Wifi driver: platform_wifi / WL12XX");

extern int msm_add_sdcc(unsigned int controller, struct mmc_platform_data *plat);
extern uint32_t msm_sdcc_setup_power(struct device *dv, unsigned int vdd);
extern uint32_t wifi_setup_power(struct device *dv, unsigned int vdd);

static struct mmc_platform_data msm7x27_sdcc_data2_TIWLAN = {
	.ocr_mask	= MMC_VDD_28_29,
	.translate_vdd	= msm_sdcc_setup_power,
};

static struct mmc_platform_data msm7x27_sdcc_data2 = {
	.ocr_mask = MMC_VDD_28_29,
	.translate_vdd = wifi_setup_power,
	.mmc_bus_width = MMC_CAP_4_BIT_DATA | MMC_CAP_POWER_OFF_CARD,
	.sdiowakeup_irq = MSM_GPIO_TO_INT(66),
	.msmsdcc_fmin = 144000,
	.msmsdcc_fmid = 24576000,
	.msmsdcc_fmax = 49152000,
	.nonremovable = 1,
};

struct wl12xx_platform_data delta_wlan_data = {
	.irq = MSM_GPIO_TO_INT(DELTA_WIFI_IRQ_GPIO),
#ifdef CONFIG_MACH_MSM7X27_SHAKIRA
	.board_ref_clock = WL12XX_REFCLOCK_19,
#else
	.board_ref_clock = WL12XX_REFCLOCK_38_XTAL,
#endif
	.platform_quirks = WL12XX_PLATFORM_QUIRK_EDGE_IRQ,
};

static int delta_wifi_init(void)
{
	int ret;

	ret = gpio_request(DELTA_WIFI_IRQ_GPIO, "wifi_irq");
	if (ret < 0) {
		printk(KERN_ERR "%s: can't reserve GPIO: %d\n", __func__,
			DELTA_WIFI_IRQ_GPIO);
		goto out;
	}
	ret = gpio_request(DELTA_WIFI_PMENA_GPIO, "wifi_pmena");
	if (ret < 0) {
		printk(KERN_ERR "%s: can't reserve GPIO: %d\n", __func__,
			DELTA_WIFI_PMENA_GPIO);
		gpio_free(DELTA_WIFI_IRQ_GPIO);
		goto out;
	}
	gpio_direction_input(DELTA_WIFI_IRQ_GPIO);
	gpio_direction_output(DELTA_WIFI_PMENA_GPIO, 0);
	if (wl12xx_set_platform_data(&delta_wlan_data))
		pr_err("error setting wl12xx data\n");
out:
	return ret;
}

static int __init delta_platform_wifi_init(void)
{
	if (!strncmp("TI1271", driver, 6)) {
		printk(KERN_INFO "Adding platform driver for TI1271...\n");
		return msm_add_sdcc(3, &msm7x27_sdcc_data2_TIWLAN);
	}
	else {
		printk(KERN_INFO "Adding platform driver for WL12XX...\n");
		delta_wifi_init();
		return msm_add_sdcc(2, &msm7x27_sdcc_data2);		
	}
}

static void __exit delta_platform_wifi_exit(void)
{
	return;
}

module_init(delta_platform_wifi_init);
module_exit(delta_platform_wifi_exit);
MODULE_DESCRIPTION("delta_wifi_platform");
MODULE_AUTHOR("Vassilis Tsogkas <tsogkas@ceid.upatras.gr>");
MODULE_LICENSE("GPL");
