#include <zephyr/kernel.h>
#include <stdio.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>


#include <zephyr/sys/printk.h>

void main(void)
{
    const struct device *bme680 = DEVICE_DT_GET_ONE(bosch_bme680);

    if (!device_is_ready(bme680)) {
        printk("BME680 sensor not found!\n");
        return;
    }

    printk("BME680 sensor found!\n");
    
    struct sensor_value temp;
    
    while (1) {

        if (sensor_sample_fetch(bme680) < 0) {
            printk("Failed to fetch sample from BME680\n");
            continue;
        }

        if (sensor_channel_get(bme680, SENSOR_CHAN_AMBIENT_TEMP, &temp) < 0) {
            printk("Failed to get temperature data\n");
            continue;
        }

        printk("Temperature: %d.%06d°C\n", temp.val1, temp.val2);
        printf("Real temp: %f C\n", sensor_value_to_double(&temp));

        k_sleep(K_SECONDS(2));  // Read every 2 seconds
    }
}
