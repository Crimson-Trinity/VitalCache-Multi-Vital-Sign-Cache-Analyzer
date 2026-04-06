***

# VitalCache: Multi-Vital Sign Cache Analyzer

Bridging Computer Architecture and Biomedical Systems through real-time embedded cache simulation.

## Overview
VitalCache is an embedded system project that integrates real-time biomedical sensing with cache memory architecture concepts.  
The system collects live physiological data — heart rate (PPG), temperature, and SpO₂ — and simulates how this data is stored and retrieved using different cache mapping techniques.  
This project demonstrates how cache performance impacts real-time healthcare systems, making it both a learning tool and a research prototype.

## Team Members
PRAGALYA M , YOUVASHREE K , YOUVASHREE K
 
## Objectives
Simulate cache memory interaction with biomedical data

Compare:
- Direct Mapping
- Fully Associative Mapping
- Set-Associative Mapping

Analyze cache performance metrics (hit, miss, conflict)  
Demonstrate real-time embedded system integration

## Key Features
- Real-time sensor data acquisition (PPG, Temp, SpO₂)
- Cache simulation with 3 mapping techniques
- FIFO-based replacement policy
- Live performance metrics (Hit / Miss / Conflict)
- Health alerts:
  - Fever detection
  - Abnormal heart rate
  - Low SpO₂

## Hardware Used
- ESP32 Microcontroller
- PPG Sensor (Analog Pulse Sensor)
- MAX30102 / MAX30105 (SpO₂ + HR)
- LM35 Temperature Sensor

## Pin Configuration
| Component | Pin |
|---|---|
| PPG Sensor | GPIO 34 |
| LM35 Temp | GPIO 35 |
| MAX30102 SDA | GPIO 21 |
| MAX30102 SCL | GPIO 22 |

(Based on system design from project PPT)

## Concepts Implemented
- Cache Memory Fundamentals
- Temporal & Spatial Locality
- Cache Lines / Blocks
- Tag, Index, Offset

Mapping Techniques
- Direct Mapping
- Fully Associative Mapping
- 2-Way Set Associative Mapping

Metrics
- Cache Hit
- Cache Miss
- Conflict Miss

## System Workflow
Sensors collect real-time data  
Data stored in main memory blocks  
User requests block via serial input  
Cache mapping algorithm processes request  

Output:
- Hit / Miss / Conflict
- Sensor values
- Execution result
- Health alerts triggered if abnormal

## How to Run
### 1. Setup
Install Arduino IDE  
Install required libraries:
- MAX30105
- spo2_algorithm

### 2. Upload Code
Select board: ESP32 Dev Module  
Upload the provided `.ino` file

### 3. Open Serial Monitor
Baud rate: 115200

## Input Format
Enter in Serial Monitor:

`<mappingType> <blockIndex>`

Example:
- `0 5` → Direct Mapping
- `1 3` → Fully Associative
- `2 7` → Set Associative

## Sample Output
Fetch: Mapping 1, Block 4  
Decode: 36.5°C, 78 bpm, 98%  
Execute: Sum = 212  
Hit:1  Miss:0  Conflict:0

## Results & Insights
From experimental observations:

| Mapping Type | Hit Ratio |
|---|---|
| Direct Mapping | ~0.32 |
| Fully Associative | ~0.36 |
| Set-Associative | ~0.36 |

 Associative and Set-Associative mapping perform better due to reduced conflicts.

## Health Monitoring Alerts
- Temperature > 38°C → Fever
- HR < 60 or > 100 → Abnormal
- SpO₂ < 95% → Low Oxygen

## Applications
- Smart healthcare monitoring systems
- Embedded system education
- Cache architecture visualization tool
- IoT-based health analytics

## Innovation / Contribution
Combines hardware-level biomedical sensing with cache simulation  
Provides real-time evaluation of memory architectures  
Acts as a practical learning bridge between:

- Computer Architecture
- Embedded Systems
- Biomedical Engineering

## References
Key research areas:
- Cache-assisted health monitoring
- IoT healthcare systems
- Memory hierarchy optimization

## Future Work
- Add cloud integration (IoT dashboard)
- Implement LRU replacement policy
- Improve signal processing accuracy
- Add mobile/web visualization

## License
MIT

## Acknowledgment
Developed as part of:  
24AIM204 – Foundations of Computer Architecture

***
