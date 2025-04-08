.. zephyr:code-sample:: pzem004t
   :name: PZEM004T Multi function AC Sensor
   :relevant-api: sensor_interface
   :relevant-api: modbus

   Get AC sample data from a PZEM004T sensor (Voltage, Current, Power,
   Energy, Frequency, Power Factor) using modbus protocol over UART. 
   Get sensor parameter like power alarm threshold and modbus address and also 
   set sensor parameter like power alarm threshold and modbus address.

Overview
********

This is a sample application to read an external PZEM004T multi function AC sensor
over UART using modbus protocol. The sample demonstrates how to read the sensor
data like voltage, current, power, energy, frequency, power factor and also 
how to set the sensor parameters like power alarm threshold and modbus address. 

Requirements
************

- PZEM004T wired to your board UART bus

Building and Running
********************

This sample can be built with any board that supports UART. A sample overlay is
provided for the NUCLEO-G071RB board.

.. zephyr-app-commands::
   :zephyr-app: samples/sensor/pzem004t
   :board: nucleo_g071rb
   :goals: build flash
   :compact:

Sample Output
============
The application will read and print sensor data every second.

.. code-block:: console

   Temperature: 25.25 C
   Temperature: 25.50 C

   <repeats endlessly every second>