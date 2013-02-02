#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/err.h>

#include <asm/gpio.h>
#include <asm/io.h>
#include <linux/wl12xx.h>

#define DELTA_WIFI_PMENA_GPIO	93  //delta
#define DELTA_WIFI_IRQ_GPIO	27  //delta

static int delta_wifi_power_state;

int delta_wifi_power(int on)
{
	printk(KERN_ERR "%s: %d\n", __func__, on);
	if (on && (on == delta_wifi_power_state))
		return 0;
	if (on) {
		printk(KERN_ERR "%s: turning on\n", __func__);
		gpio_set_value(DELTA_WIFI_PMENA_GPIO, 1);
		mdelay(15);
		gpio_set_value(DELTA_WIFI_PMENA_GPIO, 0);
		mdelay(1);
		gpio_set_value(DELTA_WIFI_PMENA_GPIO, 1);
		mdelay(70);
	} else {
		printk(KERN_ERR "%s: turning off\n", __func__);
		gpio_set_value(DELTA_WIFI_PMENA_GPIO, 0);
	}
	delta_wifi_power_state = on;
	return 0;
}

struct wl12xx_platform_data delta_wlan_data __initdata = {
	.irq = MSM_GPIO_TO_INT(DELTA_WIFI_IRQ_GPIO),
	.board_ref_clock = WL12XX_REFCLOCK_19,
	.platform_quirks = WL12XX_PLATFORM_QUIRK_EDGE_IRQ,
};

static int __init delta_wifi_init(void)
{
	int ret;
printk(KERN_ERR "%s\n", __func__);
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

device_initcall(delta_wifi_init);
