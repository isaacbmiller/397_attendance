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
#include <stdlib.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include "common/log.h"

#include <device.h>
#include <devicetree.h>
#include <drivers/gpio.h>

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS   1000

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)

// #if DT_NODE_HAS_STATUS(LED0_NODE, okay)
#define LED0	DT_GPIO_LABEL(LED0_NODE, gpios)
#define PIN1	DT_GPIO_PIN(LED0_NODE, gpios)
#define FLAGS1	DT_GPIO_FLAGS(LED0_NODE, gpios)

/* The devicetree node identifier for the "led1" alias. */
#define LED1_NODE DT_ALIAS(led1)

// #if DT_NODE_HAS_STATUS(LED1_NODE, okay)
#define LED1	DT_GPIO_LABEL(LED1_NODE, gpios)
#define PIN2	DT_GPIO_PIN(LED1_NODE, gpios)
#define FLAGS2	DT_GPIO_FLAGS(LED1_NODE, gpios)

/* The devicetree node identifier for the "led2" alias. */
#define LED2_NODE DT_ALIAS(led2)

// #if DT_NODE_HAS_STATUS(LED2_NODE, okay)
#define LED2	DT_GPIO_LABEL(LED2_NODE, gpios)
#define PIN3	DT_GPIO_PIN(LED2_NODE, gpios)
#define FLAGS3	DT_GPIO_FLAGS(LED2_NODE, gpios)

/* The devicetree node identifier for the "led3" alias. */
#define LED3_NODE DT_ALIAS(led3)

// #if DT_NODE_HAS_STATUS(LED3_NODE, okay)
#define LED3	DT_GPIO_LABEL(LED3_NODE, gpios)
#define PIN4	DT_GPIO_PIN(LED3_NODE, gpios)
#define FLAGS4	DT_GPIO_FLAGS(LED3_NODE, gpios)


// static const struct bt_data ad[] = {
// 	BT_DATA(BT_DATA_MANUFACTURER_DATA, mfg_data, 3),
// };
static struct bt_conn *default_conn;

int student_ids[4] = {696969, 111111, 222222, 333333};
int student_attendance[4] = {0, 0, 0, 0};

// make an array of PIN1, PIN2
int pin_array[4] = {PIN1, PIN2, PIN3, PIN4};

static void scan_cb(const bt_addr_le_t *addr, int8_t rssi, uint8_t adv_type,
		    struct net_buf_simple *buf)
{
	// Check to see if the packet starts with the student identifier
	if (strncmp(buf->data + 4, "student", 8) == 0) {
		int id_str[3];
		for (int i = 0; i < 3; i++) {
			id_str[i] = (int)buf->data[14 + i];
		}
		// Construct the student ID from the packet
		int found_student_id = id_str[0] * 10000 + id_str[1] * 100 + id_str[2];
		for (int i = 0; i < 4; i++) {
			// Check if the student ID is in the array of student IDs
			// If so increment the attendance for that student
			if (found_student_id == student_ids[i] && student_attendance[i] == 0) {
				student_attendance[i] = 1;
				printk("\nStudent %d attended\n", student_ids[i]);
				break;
			}
		}
		// Check the total attendance and stop scanning if all 4 students have attended
		int total_attendance = 0;
		for (int j = 0; j < 4; j++) {
			if (student_attendance[j] == 1) {
				total_attendance++;
			}
		}
		if (total_attendance == 4) {
			printk("\nAll students attended\n");
			bt_le_scan_stop();
		}

	}
}

void main(void)
{

	const struct device *dev;
	bool led_is_on = true;
	int ret;

	dev = device_get_binding(LED0);
	if (dev == NULL) {
		return;
	}

	ret = gpio_pin_configure(dev, PIN1, GPIO_OUTPUT_ACTIVE | FLAGS1);
	if (ret < 0) {
		printk("Cannot configure LED1 pin (err %d)\n", ret);
		return;
	}

	ret = gpio_pin_configure(dev, PIN2, GPIO_OUTPUT_ACTIVE | FLAGS2);
	if (ret < 0) {
		printk("Cannot configure LED2 pin (err %d)\n", ret);
		return;
	}

	ret = gpio_pin_configure(dev, PIN3, GPIO_OUTPUT_ACTIVE | FLAGS3);
	if (ret < 0) {
		printk("Cannot configure LED3 pin (err %d)\n", ret);
		return;
	}

	ret = gpio_pin_configure(dev, PIN4, GPIO_OUTPUT_ACTIVE | FLAGS4);
	if (ret < 0) {
		printk("Cannot configure LED4 pin (err %d)\n", ret);
		return;
	}

	struct bt_le_scan_param scan_param = {
		.type       = BT_HCI_LE_SCAN_PASSIVE,
		.options    = BT_LE_SCAN_OPT_NONE,
		.interval   = 0x0010,
		.window     = 0x0010,
	};
	int err;

	printk("Starting Student Scanner\n");
	/* Start blinking the LED */
	

	/* Initialize the Bluetooth Subsystem */
	err = bt_enable(NULL);
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return;
	}

	printk("Bluetooth initialized\n");

	err = bt_le_scan_start(&scan_param, scan_cb);
	if (err) {
		printk("Starting scanning failed (err %d)\n", err);
		return;
	}

	while (1) {
		for (int pin = 0; pin < 4; pin++) {
			if (student_attendance[pin] == 1) {
				gpio_pin_set(dev, pin_array[pin], 1);
			} else {
				gpio_pin_set(dev, pin_array[pin], led_is_on ? 1 : 0);
			}
			
		}
		// k_sleep(K_MSEC(500));
		led_is_on = !led_is_on;
		k_msleep(SLEEP_TIME_MS);
	}
}
