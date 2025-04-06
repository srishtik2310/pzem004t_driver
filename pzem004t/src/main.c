#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/sensor/pzem004t.h>

#define READ_MEASUREMENT_VALUES 
// #define READ_SENSOR_PARAMETERS


int main(void)
{
    const struct device *const dev = DEVICE_DT_GET(DT_ALIAS(pzem004t1));
    const struct device *const dev2 = DEVICE_DT_GET(DT_ALIAS(pzem004t2));

    if(!device_is_ready(dev)) {
        return -ENODEV;
    }

    // printk("device is %p, name is %s\n", dev, dev->name);

    struct sensor_value power_alarm_threshold_set;
    struct sensor_value modbus_rtu_address_set;

    power_alarm_threshold_set.val1 = 200; // Set the power alarm threshold to 1000 W
    modbus_rtu_address_set.val1 = 1; // Set the Modbus RTU address to 0xF7


    while (1) {
        #ifdef READ_MEASUREMENT_VALUES
        struct sensor_value voltage;
        struct sensor_value current;
        int ret;

        /* Fetch the latest sample from the sensor */
        ret = sensor_sample_fetch(dev);
        if (ret) {
            printk("Failed to fetch sensor data: %d\n", ret);
            k_sleep(K_SECONDS(1));
            continue;
        }

        ret = sensor_sample_fetch(dev2);
        if (ret) {
            printk("Failed to fetch sensor data: %d\n", ret);
            // k_sleep(K_SECONDS(1));
            // continue;
        }

        /* Get the voltage value */
        ret = sensor_channel_get(dev, SENSOR_CHAN_VOLTAGE, &voltage);
        if (ret) {
            printk("Failed to get voltage: %d\n", ret);
            k_sleep(K_SECONDS(1));
            continue;
        }

        /* Print the voltage value */
        printk("Voltage: %d.%d V\n", voltage.val1, voltage.val2);

        /* Get the current value */
        ret = sensor_channel_get(dev, SENSOR_CHAN_CURRENT, &current);
        if (ret) {
            printk("Failed to get current: %d\n", ret);
            k_sleep(K_SECONDS(1));
            continue;
        }
        /* Print the current value */
        printk("Current: %d.%d A\n", current.val1, current.val2);

        /* Get the power value */
        struct sensor_value power;
        ret = sensor_channel_get(dev, SENSOR_CHAN_POWER, &power);
        if (ret) {
            printk("Failed to get power: %d\n", ret);
            k_sleep(K_SECONDS(1));
            continue;
        }
        /* Print the power value */
        printk("Power: %d.%d W\n", power.val1, power.val2);

        /* Get the frequency value */
        struct sensor_value frequency;
        ret = sensor_channel_get(dev, SENSOR_CHAN_FREQUENCY, &frequency);
        if (ret) {
            printk("Failed to get frequency: %d\n", ret);
            k_sleep(K_SECONDS(1));
            continue;
        }
        /* Print the frequency value */
        printk("Frequency: %d.%d Hz\n", frequency.val1, frequency.val2);

        /* Get the power ENERGY value */
        struct sensor_value energy;
        ret = sensor_channel_get(dev, SENSOR_CHAN_ENERGY, &energy);
        if (ret) {
            printk("Failed to get energy: %d\n", ret);
            k_sleep(K_SECONDS(1));
            continue;
        }
        /* Print the energy value */
        printk("Energy: %d.%d Wh\n", energy.val1, energy.val2);

        /* Get the power factor value */
        struct sensor_value power_factor;
        ret = sensor_channel_get(dev, SENSOR_CHAN_POWER_FACTOR, &power_factor);
        if (ret) {
            printk("Failed to get power factor: %d\n", ret);
            k_sleep(K_SECONDS(1));
            continue;
        }
        /* Print the power factor value */
        printk("Power Factor: %d.%d\n", power_factor.val1, power_factor.val2);
        /* Get the alarm status value */
        struct sensor_value alarm_status;
        ret = sensor_channel_get(dev, SENSOR_CHAN_ALARM_STATUS, &alarm_status);
        if (ret) {
            printk("Failed to get alarm status: %d\n", ret);
            k_sleep(K_SECONDS(1));
            continue;
        }
        /* Print the alarm status value */
        printk("Alarm Status: %d\n", alarm_status.val1);

        /* Wait for 1 second before fetching again */
        k_sleep(K_SECONDS(1));

        ret = sensor_attr_set(dev, SENSOR_CHAN_RESET_ENERGY, SENSOR_ATTR_RESET_ENERGY, NULL);
        if (ret) {
            printk("Failed to reset energy: %d\n", ret);
            k_sleep(K_SECONDS(1));
            continue;
        }
      
        printk("Energy reset successfully\n");
        #endif // READ_MEASUREMENT_VALUES 

        #ifdef READ_SENSOR_PARAMETERS

        struct sensor_value power_alarm_threshold;
        struct sensor_value modbus_rtu_address;

        /* Set the power alarm threshold value */
        int ret = sensor_attr_set(dev, SENSOR_CHAN_POWER_ALARM_THRESHOLD, SENSOR_ATTR_POWER_ALARM_THRESHOLD, &power_alarm_threshold_set);
        if (ret) {
            printk("Failed to set power alarm threshold: %d\n", ret);
            k_sleep(K_SECONDS(1));
            continue;
        }
        /* Set the Modbus RTU address value */
        ret = sensor_attr_set(dev, SENSOR_CHAN_MODBUS_RTU_ADDRESS, SENSOR_ATTR_MODBUS_RTU_ADDRESS, &modbus_rtu_address_set);
        if (ret) {
            printk("Failed to set Modbus RTU address: %d\n", ret);
            k_sleep(K_SECONDS(1));
            continue;
        }

        /* Get the power alarm threshold value */
        ret = sensor_attr_get(dev, SENSOR_CHAN_POWER_ALARM_THRESHOLD, SENSOR_ATTR_POWER_ALARM_THRESHOLD, &power_alarm_threshold);
        if (ret) {
            printk("Failed to get power alarm threshold: %d\n", ret);
            k_sleep(K_SECONDS(1));
            continue;
        }
        /* Print the power alarm threshold value */
        printk("Power Alarm Threshold: %d W\n", power_alarm_threshold.val1);

        /* Get the Modbus RTU address value */
        ret = sensor_attr_get(dev, SENSOR_CHAN_MODBUS_RTU_ADDRESS, SENSOR_ATTR_MODBUS_RTU_ADDRESS, &modbus_rtu_address);
        if (ret) {
            printk("Failed to get Modbus RTU address: 0x%x\n", ret);
            k_sleep(K_SECONDS(1));
            continue;
        }
        /* Print the Modbus RTU address value */
        printk("Modbus RTU Address: 0x%x\n", modbus_rtu_address.val1);

        // power_alarm_threshold_set.val1 += 0; // Reset the power alarm threshold to 0 W
        // modbus_rtu_address_set.val1 += 10;

        k_msleep(1000);

        #endif // READ_SENSOR_PARAMETERS
    }

    return 0;
}