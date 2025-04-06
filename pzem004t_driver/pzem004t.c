/*
 * Copyright (c) 2025 Srishtik Bhandarkar
 *
 * SPDX-License-Identifier: Apache-2.0
 */

 #define DT_DRV_COMPAT peacefair_pzem004t
 
 #include <errno.h>
 #include <zephyr/devicetree.h>
 #include <zephyr/arch/cpu.h>
 #include <zephyr/init.h>
 #include <zephyr/kernel.h>
 #include <zephyr/drivers/sensor.h>
 #include <zephyr/drivers/uart.h>
 #include <zephyr/logging/log.h>
 #include <zephyr/modbus/modbus.h>
 #include <zephyr/drivers/sensor/pzem004t.h>
 #include "pzem004t.h"
 
LOG_MODULE_REGISTER(PZEM004T, CONFIG_SENSOR_LOG_LEVEL);

static int shared_modbus_iface = -1; 
static bool modbus_initialized = false; 

#if CONFIG_PZEM004T_ENABLE_RESET_ENERGY
/* Custom function code handler */
static bool custom_fc_handler(const int iface,
                            const struct modbus_adu *rx_adu,
                            struct modbus_adu *tx_adu,
                            uint8_t *const excep_code,
                            void *const user_data)
{
    LOG_INF("Custom Modbus handler for function code 0x42 called");

    /* Validate the received function code */
    if (rx_adu->fc != 0x42) {
        LOG_ERR("Unexpected function code: 0x%02X", rx_adu->fc);
        *excep_code = MODBUS_EXC_ILLEGAL_FC; 
        return true;
    }

    return true;
}

static void register_custom_fc(const int iface)
{
    /* Define the custom function code structure */
    MODBUS_CUSTOM_FC_DEFINE(custom_fc, custom_fc_handler, 0x42, NULL);

    /* Register the custom function code */
    int err = modbus_register_user_fc(iface, &modbus_cfg_custom_fc);
    if (err) {
        LOG_ERR("Failed to register custom function code (err %d)", err);
    } else {
        LOG_INF("Custom function code 0x42 registered successfully");
    }
}

static int pzem004t_reset_energy(int iface, uint8_t address){
    struct modbus_adu adu = {
        .unit_id = address,
        .fc = 0x42,
        .length = 0,
    };                

    /* Send the custom command */
    int err = modbus_raw_backend_txn(iface, &adu);

    if (err) {
        LOG_ERR("Failed to send custom command (err %d)", err);
        return err;
    }

    /* Handle the response */
    if (adu.fc == 0x42) {
        LOG_INF("Energy Successfully reset");
        return 0;
    } 
    else if (adu.fc == 0xC2) {
        LOG_ERR("Received error response for Reset Energy");
        return -EIO;
    } 
    else {
        LOG_ERR("Unexpected response function code: 0x%02X", adu.fc);
        return -EIO;
    }
    return 0;
}

#endif /* CONFIG_PZEM004T_ENABLE_RESET_ENERGY */
 
static int pzem004t_init(const struct device *dev)
{
    const struct pzem004t_config *config = dev->config;
    struct pzem004t_data *data = dev->data;

    LOG_INF("Initializing PZEM004T device with Modbus interface: %s", config->modbus_iface_name);
    LOG_INF("Modbus address: 0x%02X", config->modbus_address);

    // Initialize the shared Modbus interface only once
    if (!modbus_initialized) {
        shared_modbus_iface = modbus_iface_get_by_name(config->modbus_iface_name);
        if (shared_modbus_iface < 0) {
            LOG_ERR("Failed to get Modbus interface: %s", config->modbus_iface_name);
            return -ENODEV;
        }

        int err = modbus_init_client(shared_modbus_iface, config->client_param);
        if (err) {
            LOG_ERR("Modbus RTU client initialization failed");
            return err;
        }

        modbus_initialized = true;
        LOG_INF("Shared Modbus interface initialized: %d", shared_modbus_iface);
    }

    // Store the shared Modbus interface in the instance data
    data->iface = shared_modbus_iface;

    LOG_INF("PZEM004T device initialized with Modbus address: 0x%02X", config->modbus_address);

    #if CONFIG_PZEM004T_ENABLE_RESET_ENERGY
        register_custom_fc(data->iface);
    #endif /* CONFIG_PZEM004T_ENABLE_RESET_ENERGY */

    return 0;
}

static int pzem004t_sample_fetch(const struct device *dev, 
                                enum sensor_channel chan)
{
    const struct pzem004t_config *config = dev->config;
    struct pzem004t_data *sensor_data = dev->data;

    uint16_t reg_buf[MESUREMENT_REGISTER_TOTAL_LENGTH] = {0};
	int err;
	err = modbus_read_input_regs(sensor_data->iface, config->modbus_address, 
                                MESUREMENT_REGISTER_START_ADDRESS, 
                                reg_buf,
                                MESUREMENT_REGISTER_TOTAL_LENGTH);
	if (err != 0) {
		LOG_ERR("Failed to fetch sensor data at address 0x%02x: %d", config->modbus_address, err);
		return -1;
	}

    sensor_data->voltage = reg_buf[0];
    sensor_data->current = ((reg_buf[2] << 16) | reg_buf[1]);
    sensor_data->power = ((reg_buf[4] << 16) | reg_buf[3]);
    sensor_data->energy = ((reg_buf[6] << 16) | reg_buf[5]);
    sensor_data->frequency = reg_buf[7];
    sensor_data->power_factor = reg_buf[8];
    sensor_data->alarm_status = reg_buf[9];
    LOG_INF("Voltage = %d", sensor_data->voltage);
    LOG_INF("Current = %d", sensor_data->current);
    LOG_INF("Power = %d", sensor_data->power);
    LOG_INF("Energy = %d", sensor_data->energy);
    LOG_INF("Frequency = %d", sensor_data->frequency);
    LOG_INF("Power Factor = %d", sensor_data->power_factor);
    LOG_INF("Alarm Status = %d", sensor_data->alarm_status);

    return 0;
}

static int pzem004t_channel_get(const struct device *dev,
                            enum sensor_channel chan,
                            struct sensor_value *val)
{
    struct pzem004t_data *sensor_data = dev->data;

    switch ((uint32_t)chan) {
    case SENSOR_CHAN_VOLTAGE:
        val->val1 = sensor_data->voltage / PZEM004T_VOLTAGE_SCALE;
        val->val2 = (sensor_data->voltage % PZEM004T_VOLTAGE_SCALE);
        break;
    case SENSOR_CHAN_CURRENT:
        val->val1 = sensor_data->current / PZEM004T_CURRENT_SCALE;
        val->val2 = (sensor_data->current % PZEM004T_CURRENT_SCALE);
        break;
    case SENSOR_CHAN_POWER:
        val->val1 = sensor_data->power / PZEM004T_POWER_SCALE;
        val->val2 = (sensor_data->power % PZEM004T_POWER_SCALE);
        break;
    case SENSOR_CHAN_ENERGY:
        val->val1 = sensor_data->energy / PZEM004T_ENERGY_SCALE;
        val->val2 = (sensor_data->energy % PZEM004T_ENERGY_SCALE);
        break;
    case SENSOR_CHAN_FREQUENCY:
        val->val1 = sensor_data->frequency / PZEM004T_FREQUENCY_SCALE;
        val->val2 = (sensor_data->frequency % PZEM004T_FREQUENCY_SCALE);
        break;
    case (enum sensor_channel)SENSOR_CHAN_POWER_FACTOR:
        val->val1 = sensor_data->power_factor / PZEM004T_POWER_FACTOR_SCALE;
        val->val2 = (sensor_data->power_factor % PZEM004T_POWER_FACTOR_SCALE);
        break;
    case SENSOR_CHAN_ALARM_STATUS:
        val->val1 = sensor_data->alarm_status;
        val->val2 = 0;
        break;
    default:
        LOG_ERR("Unsupported channel");
        return -ENOTSUP;
    }

    return 0;
}

static int pzem004t_attr_get(const struct device *dev,
                        enum sensor_channel chan,
                        enum sensor_attribute attr,
                        struct sensor_value *val)
{
    const struct pzem004t_config *config = dev->config;
    struct pzem004t_data *data = dev->data;

    int err;
    uint16_t reg_buf[1] = {0};

    if (chan != (enum sensor_channel)SENSOR_CHAN_POWER_ALARM_THRESHOLD && 
        chan != (enum sensor_channel)SENSOR_CHAN_MODBUS_RTU_ADDRESS) {
        LOG_ERR("Channel not supported for attribute request");
        return -ENOTSUP;
    }

    switch ((uint32_t)attr){
        case SENSOR_ATTR_POWER_ALARM_THRESHOLD:
            err = modbus_read_holding_regs(data->iface, config->modbus_address,
                                        POWER_ALARM_THRESHOLD_ADDRESS, 
                                        reg_buf, POWER_ALARM_THRESHOLD_REGISTER_LENGTH);
            val->val1 = reg_buf[0];
            val->val2 = 0;
            break;

        case SENSOR_ATTR_MODBUS_RTU_ADDRESS:
            err = modbus_read_holding_regs(data->iface, config->modbus_address,    
                                        MODBUS_RTU_ADDRESS_REGISTER, 
                                        reg_buf, MODBUS_RTU_ADDRESS_REGISTER_LENGTH);
                val->val1 = reg_buf[0];
                val->val2 = 0;  
                break;

        default:
            LOG_ERR("Unsupported channel");
            return -ENOTSUP;
            break;
    }

    return 0;
}

static int pzem004t_attr_set(const struct device *dev,
                            enum sensor_channel chan,
                            enum sensor_attribute attr,
                            const struct sensor_value *val)
{
    const struct pzem004t_config *config = dev->config;
    struct pzem004t_data *data = dev->data;
    int err; 

    if (chan != (enum sensor_channel)SENSOR_CHAN_POWER_ALARM_THRESHOLD && 
        chan != (enum sensor_channel)SENSOR_CHAN_MODBUS_RTU_ADDRESS    &&
        chan != (enum sensor_channel)SENSOR_CHAN_RESET_ENERGY) {
        LOG_ERR("Channel not supported for attribute request");
        return -ENOTSUP;
    }

    switch((uint32_t)attr){
        case SENSOR_ATTR_POWER_ALARM_THRESHOLD:
            if(val->val1 < 0 || val->val1 > PZEM004T_MAX_POWER_ALARM_THRESHOLD) {
                LOG_ERR("Invalid value for power alarm threshold");
                return -EINVAL;
            }
            
            err = modbus_write_holding_reg(data->iface, config->modbus_address,
                                        POWER_ALARM_THRESHOLD_ADDRESS, 
                                        val->val1);
            if (err != 0) {
                LOG_ERR("FC06 failed with %d", err);
                return -1;
            }
            break;

        case SENSOR_ATTR_MODBUS_RTU_ADDRESS:
            if(val->val1 < 0 || val->val1 > PZEM004T_MAX_MODBUS_RTU_ADDRESS) {
                LOG_ERR("Invalid value for Modbus RTU address");
                return -EINVAL;
            }

            err = modbus_write_holding_reg(data->iface, config->modbus_address,
                                        MODBUS_RTU_ADDRESS_REGISTER, 
                                        val->val1);

            if (err != 0) {
                LOG_ERR("FC06 failed with %d", err);
                return -1;
            }
            break;
            
        #if CONFIG_PZEM004T_ENABLE_RESET_ENERGY
            case SENSOR_ATTR_RESET_ENERGY:
                err = pzem004t_reset_energy( data->iface, config->modbus_address);
                if (err != 0) {
                    LOG_ERR("Failed to reset energy");
                    return -1;
                }
                break;
        #else
            case SENSOR_ATTR_RESET_ENERGY:
                LOG_ERR("Reset energy is not enabled by default. Enable CONFIG_PZEM004T_ENABLE_RESET_ENERGY in prj.conf.");
                return -ENOTSUP;
        #endif /* CONFIG_PZEM004T_ENABLE_RESET_ENERGY */

        default:
            LOG_ERR("Unsupported Attribute");
            return -ENOTSUP;
            break;
    }

    return 0;
}


static DEVICE_API(sensor, pzem004t_api) = {
    .sample_fetch = pzem004t_sample_fetch,
    .channel_get = pzem004t_channel_get,
    .attr_get = pzem004t_attr_get,
    .attr_set = pzem004t_attr_set,
};
    
#define PZEM004T_DEFINE(inst)                                      \
    static const struct pzem004t_config pzem004t_config_##inst = { \
        .modbus_iface_name = DEVICE_DT_NAME(DT_PARENT(DT_INST(inst, peacefair_pzem004t))), \
        .modbus_address = DT_INST_PROP(inst, modbus_address),      \
        .client_param = {                                          \
            .mode = MODBUS_MODE_RTU,                               \
            .rx_timeout = 100000,                                  \
            .serial = {                                            \
                .baud = 9600,                                      \
                .parity = UART_CFG_PARITY_NONE,                    \
                .stop_bits_client = UART_CFG_STOP_BITS_1,          \
            },                                                     \
        },                                                         \
    };                                                             \
                                                                   \
    static struct pzem004t_data pzem004t_data_##inst;              \
                                                                   \
    SENSOR_DEVICE_DT_INST_DEFINE(inst,                            \
                                  &pzem004t_init,                 \
                                  NULL,                           \
                                  &pzem004t_data_##inst,          \
                                  &pzem004t_config_##inst,        \
                                  POST_KERNEL,                    \
                                  CONFIG_SENSOR_INIT_PRIORITY,    \
                                  &pzem004t_api);

DT_INST_FOREACH_STATUS_OKAY(PZEM004T_DEFINE)
 