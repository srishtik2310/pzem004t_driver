#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/sensor/pzem004t.h>


int main(void)
{
	const struct device *const dev = DEVICE_DT_GET(DT_ALIAS(pzem004t));

	if (!device_is_ready(dev)) {
		printk("Device %s is not ready\n", dev->name);
		return -ENODEV;
	}

	int ret;

	#if CONFIG_READ_MEASUREMENT_VALUES
	struct sensor_value voltage;
	struct sensor_value current;
	struct sensor_value power;
	struct sensor_value frequency;
	struct sensor_value energy;
	struct sensor_value power_factor;
	struct sensor_value alarm_status;
	#endif // READ_MEASUREMENT_VALUES

	#if CONFIG_READ_SENSOR_PARAMETERS

	struct sensor_value power_alarm_threshold;
	struct sensor_value modbus_rtu_address;

	#endif // READ_SENSOR_PARAMETERS

	#if CONFIG_SET_SENSOR_PARAMETERS
	struct sensor_value power_alarm_threshold_set = {
		.val1 = 1000,
		.val2 = 0
	};

	struct sensor_value modbus_rtu_address_set = {
		.val1 = 0x01,
		.val2 = 0
	};

	#endif // CONFIG_SET_SENSOR_PARAMETERS

	while (1) {
		#if CONFIG_READ_MEASUREMENT_VALUES
		/* Fetch the latest sample from the sensor */
		ret = sensor_sample_fetch(dev);
		if (ret) {
			printk("Failed to fetch sensor data: %d\n", ret);
		}

		ret = sensor_channel_get(dev, SENSOR_CHAN_VOLTAGE, &voltage);
		if (ret) {
			printk("Failed to get voltage: %d\n", ret);
		}
		else{
			printk("Voltage: %d.%d V\n", voltage.val1, voltage.val2);
		}
		

		ret = sensor_channel_get(dev, SENSOR_CHAN_CURRENT, &current);
		if (ret) {
			printk("Failed to get current: %d\n", ret);
		}
		else{
			printk("Current: %d.%d A\n", current.val1, current.val2);
		}


		ret = sensor_channel_get(dev, SENSOR_CHAN_POWER, &power);
		if (ret) {
			printk("Failed to get power: %d\n", ret);
		}
		else{
			printk("Power: %d.%d W\n", power.val1, power.val2);
		}

		ret = sensor_channel_get(dev, SENSOR_CHAN_FREQUENCY, &frequency);
		if (ret) {
			printk("Failed to get frequency: %d\n", ret);
		}
		else{
			printk("Frequency: %d.%d Hz\n", frequency.val1, frequency.val2);
		}


		ret = sensor_channel_get(dev, SENSOR_CHAN_ENERGY, &energy);
		if (ret) {
			printk("Failed to get energy: %d\n", ret);
		}
		else{
			printk("Energy: %d.%d Wh\n", energy.val1, energy.val2);
		}

		ret = sensor_channel_get(dev, SENSOR_CHAN_POWER_FACTOR, &power_factor);
		if (ret) {
			printk("Failed to get power factor: %d\n", ret);
		}
		else{
			printk("Power Factor: %d.%d\n", power_factor.val1, power_factor.val2);
		}

		ret = sensor_channel_get(dev, SENSOR_CHAN_ALARM_STATUS, &alarm_status);
		if (ret) {
			printk("Failed to get alarm status: %d\n", ret);
		}
		else{
			printk("Alarm Status: %d\n\n", alarm_status.val1);
		}

		k_msleep(1000);

		#endif // READ_MEASUREMENT_VALUES
		
		#if CONFIG_READ_SENSOR_PARAMETERS
		ret = sensor_attr_get(dev, SENSOR_CHAN_POWER_ALARM_THRESHOLD,
				      SENSOR_ATTR_POWER_ALARM_THRESHOLD, &power_alarm_threshold);
		if (ret) {
			printk("Failed to get power alarm threshold: %d\n", ret);
		}
		else{
			printk("Power Alarm Threshold: %d W\n", power_alarm_threshold.val1);
		}

		ret = sensor_attr_get(dev, SENSOR_CHAN_MODBUS_RTU_ADDRESS,
				      SENSOR_ATTR_MODBUS_RTU_ADDRESS, &modbus_rtu_address);
		if (ret) {
			printk("Failed to get Modbus RTU address: %d\n", ret);
		}
		else{
			printk("Modbus RTU Address: 0x%02x\n", modbus_rtu_address.val1);
			return 0;
		}
		return 0;

		#endif // READ_SENSOR_PARAMETERS

		#if CONFIG_SET_SENSOR_PARAMETERS
		
		ret = sensor_attr_set(dev, SENSOR_CHAN_POWER_ALARM_THRESHOLD,
				SENSOR_ATTR_POWER_ALARM_THRESHOLD,
				&power_alarm_threshold_set);

		if (ret) {
			printk("Failed to set power alarm threshold: %d\n", ret);
		}
		else{
			printk("Power alarm threshold set to: %d W\n", power_alarm_threshold_set.val1);
		}

		ret = sensor_attr_set(dev, SENSOR_CHAN_MODBUS_RTU_ADDRESS,
					SENSOR_ATTR_MODBUS_RTU_ADDRESS, &modbus_rtu_address_set);

		if (ret) {
			printk("Failed to set Modbus RTU address: %d\n", ret);
		}
		else{
			printk("Modbus RTU address set to: 0x%02x\n", modbus_rtu_address_set.val1);
		}
		return 0;
		#endif // CONFIG_SET_SENSOR_PARAMETERS

		#if CONFIG_RESET_ENERGY
		ret = sensor_attr_set(dev, SENSOR_CHAN_RESET_ENERGY,
				SENSOR_ATTR_RESET_ENERGY, NULL);
		if (ret) {
			printk("Failed to reset energy: %d\n", ret);
		}
		else{
			printk("Energy reset successfully\n");
		}
		return 0;
		#endif // CONFIG_RESET_ENERGY
	}

	return 0;
}