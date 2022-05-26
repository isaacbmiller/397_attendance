/* main.c - Application main entry point */

/*
 * Copyright (c) 2015-2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/types.h>
#include <stddef.h>
#include <sys/printk.h>
#include <sys/util.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>

#include <device.h>
#include <zephyr.h>
#include <inttypes.h>
#include <drivers/gpio.h>

#define SLEEP_TIME_MS	1

/*
 * Get button configuration from the devicetree sw0 alias. This is mandatory.
 */
#define SW0_NODE	DT_ALIAS(sw0)
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET_OR(SW0_NODE, gpios,
							      {0});
static struct gpio_callback button_cb_data;

static struct gpio_dt_spec led = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led0), gpios,
						     {0});
int ad_num = 0;
int has_started = 0;



#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)
	
// ada implementing the Eddystone UUID scheme
static const struct bt_data student_ad1[] = {
	BT_DATA_BYTES(BT_DATA_SVC_DATA16,
			  0x00, /* Frame type */
			  0x00, /* Tx power at 0m */
			  's', 't', 'u', 'd', 'e', 'n', 't', 0x00, 0x00, 0x00, /* 10-byte namespace */
			  0x45, 0x45, 0x45, 0x0, 0x0, 0x0, /* 6-byte instance id */
			  0x00, /* RFU */
			  0x00 /* RFU */
			  ),
	BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_NO_BREDR),
	BT_DATA_BYTES(BT_DATA_UUID16_ALL, 0x12, 0x34),
	
};

static const struct bt_data student_ad2[] = {
	BT_DATA_BYTES(BT_DATA_SVC_DATA16,
			  0x00, /* Frame type */
			  0x00, /* Tx power at 0m */
			  's', 't', 'u', 'd', 'e', 'n', 't', 0x00, 0x00, 0x00, /* 10-byte namespace */
			  0x0b, 0x0b, 0x0b, 0x0, 0x0, 0x0, /* 6-byte instance id */
			  0x00, /* RFU */
			  0x00 /* RFU */
			  ),
	BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_NO_BREDR),
	BT_DATA_BYTES(BT_DATA_UUID16_ALL, 0x12, 0x34),
	
};

static const struct bt_data student_ad3[] = {
	BT_DATA_BYTES(BT_DATA_SVC_DATA16,
			  0x00, /* Frame type */
			  0x00, /* Tx power at 0m */
			  's', 't', 'u', 'd', 'e', 'n', 't', 0x00, 0x00, 0x00, /* 10-byte namespace */
			  0x16, 0x16, 0x16, 0x0, 0x0, 0x0, /* 6-byte instance id */
			  0x00, /* RFU */
			  0x00 /* RFU */
			  ),
	BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_NO_BREDR),
	BT_DATA_BYTES(BT_DATA_UUID16_ALL, 0x12, 0x34),
	
};

static const struct bt_data student_ad4[] = {
	BT_DATA_BYTES(BT_DATA_SVC_DATA16,
			  0x00, /* Frame type */
			  0x00, /* Tx power at 0m */
			  's', 't', 'u', 'd', 'e', 'n', 't', 0x00, 0x00, 0x00, /* 10-byte namespace */
			  0x21, 0x21, 0x21, 0x0, 0x0, 0x0, /* 6-byte instance id */
			  0x00, /* RFU */
			  0x00 /* RFU */
			  ),
	BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_NO_BREDR),
	
};

/* Set Scan Response data */
static const struct bt_data sd[] = {
	BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
};

int start_adv_data(void) {
	/* Start advertising */
	int err;
	if (ad_num == 0) {
		err = bt_le_adv_start(BT_LE_ADV_NCONN_IDENTITY, student_ad1, ARRAY_SIZE(student_ad1),
				sd, ARRAY_SIZE(sd));
	}
	else if (ad_num == 1) {
		err = bt_le_adv_start(BT_LE_ADV_NCONN_IDENTITY, student_ad2, ARRAY_SIZE(student_ad2),
				sd, ARRAY_SIZE(sd));
	}
	else if (ad_num == 2) {
		printk("\nAd 2 starting\n");
		err = bt_le_adv_start(BT_LE_ADV_NCONN_IDENTITY, student_ad3, ARRAY_SIZE(student_ad3),
				sd, ARRAY_SIZE(sd));
	}
	else if (ad_num == 3){
		err = bt_le_adv_start(BT_LE_ADV_NCONN_IDENTITY, student_ad4, ARRAY_SIZE(student_ad4),
				sd, ARRAY_SIZE(sd));
	} else {
		printk("Unknown ad_num%d\n", ad_num);
	}
	// err = bt_le_adv_start(BT_LE_ADV_NCONN_IDENTITY, student_ad2, ARRAY_SIZE(student_ad2),
	// 		      sd, ARRAY_SIZE(sd));
	if (err) {
		printk("Advertising failed to update (err %d)\n", err);
		return err;
	}
	return err;

}
static void bt_ready(int err)
{
	printk("Bluetooth initializing\n");
	char addr_s[BT_ADDR_LE_STR_LEN];
	bt_addr_le_t addr = {0};
	size_t count = 1;

	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return;
	}

	printk("Bluetooth initialized\n");

	
	/* Start advertising */
	err = start_adv_data();
	if (err) {
		printk("Advertising failed to start (err %d)\n", err);
		return;
	}

	bt_id_get(&addr, &count);
	bt_addr_le_to_str(&addr, addr_s, sizeof(addr_s));

	printk("Beacon started, advertising as %s\n", addr_s);
}

void update_adv_data(void)
{
	int err;
	printk("Now advertising ad %d\n", ad_num);
	/* Start advertising */
	
	if (ad_num == 0) {
		err = bt_le_adv_update_data(student_ad1, ARRAY_SIZE(student_ad1),
					sd, ARRAY_SIZE(sd));
	}
	else if (ad_num == 1) {
		err = bt_le_adv_update_data(student_ad2, ARRAY_SIZE(student_ad2),
				sd, ARRAY_SIZE(sd));
	}
	else if (ad_num == 2) {
		err = bt_le_adv_update_data(student_ad3, ARRAY_SIZE(student_ad3),
				sd, ARRAY_SIZE(sd));
	}
	else {
		err = bt_le_adv_update_data(student_ad4, ARRAY_SIZE(student_ad4),
				sd, ARRAY_SIZE(sd));
	}
	if (err) {
		printk("Advertising failed to update (err %d)\n", err);
		return;
	}
}

// Increase the ad num and call the update helper function
void increment_ad_num(void) {
	ad_num = (ad_num + 1) % 4;
	printk("ad_num is now %d\n", ad_num);
	update_adv_data();
}

void button_pressed(const struct device *dev, struct gpio_callback *cb,
		    uint32_t pins)
{
	printk("Button pressed at %" PRIu32 "\n", k_cycle_get_32());
	// For some reason when this callback is used to update the bluetooth, it crashes, most liklely a multithreading issue
}

void main(void)
{

	int ret;
	int err;
	err = bt_enable(bt_ready);
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
	} else {
		has_started = 1;
	}
	if (!device_is_ready(button.port)) {
		printk("Error: button device %s is not ready\n",
		       button.port->name);
		return;
	}

	ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
	if (ret != 0) {
		printk("Error %d: failed to configure %s pin %d\n",
		       ret, button.port->name, button.pin);
		return;
	}

	ret = gpio_pin_interrupt_configure_dt(&button,
					      GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) {
		printk("Error %d: failed to configure interrupt on %s pin %d\n",
			ret, button.port->name, button.pin);
		return;
	}

	gpio_init_callback(&button_cb_data, button_pressed, BIT(button.pin));
	gpio_add_callback(button.port, &button_cb_data);
	printk("Set up button at %s pin %d\n", button.port->name, button.pin);

	if (led.port && !device_is_ready(led.port)) {
		printk("Error %d: LED device %s is not ready; ignoring it\n",
		       ret, led.port->name);
		led.port = NULL;
	}

	if (led.port) {
		ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT);
		if (ret != 0) {
			printk("Error %d: failed to configure LED device %s pin %d\n",
			       ret, led.port->name, led.pin);
			led.port = NULL;
		} else {
			printk("Set up LED at %s pin %d\n", led.port->name, led.pin);
		}
	}

	
	if (led.port) {
		while (1) {
			/* If we have an LED, match its state to the button's. */
			int val = gpio_pin_get_dt(&button);

			if (val >= 0) {
				gpio_pin_set_dt(&led, val);
				// Update the advertising data on a button press
				if (val) {
					increment_ad_num();
					k_sleep(K_MSEC(1000));
				}

			}
			k_msleep(SLEEP_TIME_MS);
		}
	}
}
