# homesensor

## Description
This project, which is built around the esp32 microcontroller, started out as a simple [Spring Application](https://github.com/lal02/Centralized-Web-Application) while I was doing an internship as a part of my university's practical semester. 
The goal of the Spring Application was to get familiar with Spring in general.

The Spring application mentioned above has additional features that are not relevant to the homesensor setup and will therefore not be discussed in this repository.

The idea was to use the esp32 as a data source to get some hands-on experience with REST interfaces and Data Handling in Spring. 
Beyond just being a learning exercise, this project turned out to be really useful. 
By collecting and graphing temperature data, I was able to spot a problem with the underfloor heating in my apartment.

The esp32 collects data from a variety of sensors and transmits it to the Spring backend through REST interfaces. 
The Spring backend, in turn, offers multiple REST interfaces for both sending and retrieving data. 
Additionally, a Raspberry Pi controls a 64x64 LED matrix, which displays data received from the Spring backend. 
It also adjusts the brightness of the LED matrix based on the light intensity data obtained.

## Overview
This sketch illustrates the structure of all the hardware and software components, as well as how they interact with each other.

<br> </br>
![architecture.jpeg](https://github.com/lal02/homesensor/blob/main/architecture.jpg)

## Esp32 and Sensor Integration
The project commenced as a modest undertaking, utilising an ESP32 microcontroller and a DHT22 sensor to gather and transmit temperature and humidity data. 
Subsequent to this, the setup was expanded by the addition of further sensors, including the TSL2561 for the purpose of measuring light intensity, and subsequently the MQ7 and MQ135 for the purpose of measuring air quality. 
In order to enhance the ease of use, a number of quality-of-life features have been incorporated. 
These included an LCD screen that displayed the IP address and the time until the next sensor reading. 
Furthermore, an HTTP server was configured on the ESP32, incorporating two endpoints. 
The primary endpoint facilitates the retrieval of hardware-related information in the form of an HTML file, encompassing details such as the IP address, MAC address, host name, WiFi SSID, and the list of connected sensors along with their respective functions. 
In conclusion, a touch button was incorporated, the actuation of which initiates a sensor reading.
Presently, both the ESP32 and the sensors are stored within a cardboard box. 
This short term storage method will be replaced in the future by a custom 3D printed case.

## Raspberry Pi no.1 
The system is constructed around a Raspberry Pi 3, which hosts a variety of essential services. 
The system operates a Tomcat service, which contains a Spring application, thereby providing a robust backend for the management of a variety of tasks. 
Furthermore, the Raspberry Pi is equipped with a GitHub Actions runner, thereby enabling DevOps processes and continuous integration/continuous deployment (CI/CD) pipelines. 
It also manages a PostgreSQL database, thereby enabling efficient data storage and management.
It is evident that when a multitude of applications are operating concurrently, the Raspberry Pi 3 reaches its maximum performance capacity. 
During periods of substantial data loading, such as retrieving large amounts of data via a REST Interface Call, the system encounters performance bottlenecks. 
This performance limitation suggests that, in the future, it might be beneficial to transfer these operations to a machine with greater computing power to ensure smoother and more efficient processing.

## Raspberry Pi no.2
The Raspberry Pi is programmed to execute a Python script that enables the control of a connected 64x64 LED matrix. 
However, it should be noted that due to the utilisation of Python bindings, the performance is less than optimal which causes visible flickers caused by low Hz. 
The present script has been developed for interaction with the REST interface of a Spring application, with a view to both the display of data and the regulation of the brightness of the LED matrix. 
This objective is realised through the retrieval of the most recent light intensity data, which is then utilised to adjust the brightness. 
Furthermore, the script utilises the API of OpenWeatherMap to retrieve current weather information, thereby integrating external data into the display system.

<br> </br>

![matrix.jpeg](https://github.com/lal02/homesensor/blob/main/matrix.jpeg)


## Learnings
Throughout this project, I've gained a variety of skills and knowledge. 
This includes for example soldering and electronics, which were essential for building and troubleshooting hardware. 
I learned about the I2C protocol for device communication and improved my ability to read datasheets for component specifications. 
My Linux skills advanced, particularly in system configuration and hardware interaction. 
Additionally, I developed an understanding of hardware requirements, such as program storage size and computational power for running httpservers.


# Potential Upgrades
In the future, there are a number of potential enhancements that could be made to this project. 
A notable enhancement would be Over-the-Air (OTA) updates on the esp32. 
This feature facilitates seamless, wireless updates to the system, thereby ensuring that upgrades and maintenance can be effortlessly executed without the necessity for physical access to the device.
Furthermore, the utilisation of the available C++ bindings for the LED matrix is another possibility..
This would enhance performance and efficiency by leveraging the capabilities of C++, which has been demonstrated to offer superior resource management and faster execution compared to Python.
