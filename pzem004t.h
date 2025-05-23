/*
 * Copyright (c) 2025 Srishtik Bhandarkar
 * SPDX-License-Identifier: Apache-2.0
 */

/*
    To be placed in zephyr/include/drivers/sensor/
*/
 #ifndef ZEPHYR_INCLUDE_DRIVERS_SENSOR_PZEM004T_H_
 #define ZEPHYR_INCLUDE_DRIVERS_SENSOR_PZEM004T_H_
 
 #include <zephyr/drivers/sensor.h>
 
/* PZEM004T specific channels */
enum sensor_channel_pzem004t {
    SENSOR_CHAN_ENERGY = SENSOR_CHAN_PRIV_START,
    SENSOR_CHAN_POWER_FACTOR,
    SENSOR_CHAN_ALARM_STATUS,
    SENSOR_CHAN_POWER_ALARM_THRESHOLD,
    SENSOR_CHAN_MODBUS_RTU_ADDRESS,
    SENSOR_CHAN_RESET_ENERGY,
};
 
enum sensor_attribute_pzem004t {
    SENSOR_ATTR_POWER_ALARM_THRESHOLD = SENSOR_ATTR_PRIV_START,
    SENSOR_ATTR_MODBUS_RTU_ADDRESS,
    SENSOR_ATTR_RESET_ENERGY,
};

#endif