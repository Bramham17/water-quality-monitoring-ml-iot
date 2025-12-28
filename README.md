# Water Quality Monitoring System with ML

## Overview

An intelligent water quality monitoring system that uses Arduino UNO microcontroller with pH and Turbidity sensors to measure water parameters in real-time. The system integrates a Random Forest machine learning algorithm (R² = 0.933) for accurate water quality assessment and uploads data to Blynk IoT cloud platform for remote monitoring and analysis.

## Project Description

Water quality monitoring is essential for environmental protection, agriculture, and industrial applications. This project develops an automated system that continuously monitors key water quality parameters and uses machine learning for intelligent analysis and prediction. The system provides real-time feedback through both local sensors and cloud-based dashboards.

## Hardware Components

### Microcontroller
- **Arduino UNO**: 16 MHz ATmega328P processor with 2KB SRAM, 32KB Flash memory, 14 digital pins, 6 analog input pins

### Sensors
- **pH Sensor (DFRobot SEN0161)**: Measures acidity/alkalinity (0-14 pH range, ±0.1 accuracy)
- **Turbidity Sensor (DFRobot SEN0189)**: Measures water clarity (0-800 NTU range)
- **Temperature Sensor (DS18B20)**: 1-Wire digital temperature sensor for thermal compensation

### Communication & Cloud
- **ESP8266 WiFi Module**: Enables wireless connectivity for IoT cloud integration
- **Blynk IoT Platform**: Real-time data visualization and remote monitoring
- **USB Cable**: Programming and power supply
 
## License

This project is licensed under the MIT License - see LICENSE file for details.

## Author

**P.Bramham** - Water Quality Monitoring System with ML

For more information and updates, visit the [GitHub Repository](https://github.com/Bramham17/water-quality-monitoring-ml-iot)

---

**Last Updated**: 2025
**Project Status**: Active Development
**Contributors**: Open for collaboration and contributions
