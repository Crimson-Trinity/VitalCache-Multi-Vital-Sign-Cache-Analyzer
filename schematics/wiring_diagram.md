# VitalCache — Wiring Diagram

## Component Connections

### PPG Sensor (Analog Pulse Sensor)

| PPG Pin | ESP32 Pin |
|---------|-----------|
| VCC | 3.3V |
| GND | GND |
| Signal | GPIO 34 |

### LM35 Temperature Sensor

| LM35 Pin | ESP32 Pin |
|----------|-----------|
| VCC | 3.3V |
| GND | GND |
| Vout | GPIO 35 |

### MAX30102 SpO₂ & Heart Rate Sensor (I2C)

| MAX30102 Pin | ESP32 Pin |
|--------------|-----------|
| VIN | 3.3V |
| GND | GND |
| SDA | GPIO 21 |
| SCL | GPIO 22 |

## Wiring Diagram (ASCII)

```
                    ┌──────────────────────┐
                    │     ESP32 Dev Module  │
                    │                      │
   PPG Sensor       │                      │     MAX30102
  ┌─────────┐       │                      │    ┌──────────┐
  │  VCC ───┤───────┤── 3.3V          3.3V ├────┤── VIN    │
  │  GND ───┤───────┤── GND           GND  ├────┤── GND    │
  │  SIG ───┤───────┤── GPIO 34             │    │          │
  └─────────┘       │                      │    │          │
                    │              GPIO 21 ├────┤── SDA    │
   LM35 Temp        │              GPIO 22 ├────┤── SCL    │
  ┌─────────┐       │                      │    └──────────┘
  │  VCC ───┤───────┤── 3.3V              │
  │  GND ───┤───────┤── GND               │
  │  OUT ───┤───────┤── GPIO 35            │
  └─────────┘       │                      │
                    │              USB ────┤──── PC (Serial Monitor)
                    └──────────────────────┘
```

## Notes

- All sensors operate at **3.3V** (ESP32 logic level)
- The LM35 outputs 10mV/°C — ESP32 ADC is configured for 12-bit resolution
- MAX30102 uses I2C protocol with default address `0x57`
- PPG sensor threshold is set to `550` in firmware (adjustable via `THRESHOLD` constant)
- Ensure proper grounding between all components
