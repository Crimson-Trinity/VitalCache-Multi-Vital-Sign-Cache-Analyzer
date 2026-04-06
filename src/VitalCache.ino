/*
 * ============================================================================
 *  VitalCache: Multi-Vital Sign Cache Analyzer
 * ============================================================================
 *  
 *  Bridging Computer Architecture and Biomedical Systems through 
 *  real-time embedded cache simulation.
 *
 *  Authors:
 *    PRAGALYA M  — CB.AI.U4AIM24032
 *    RAMKUMAR R  — CB.AI.U4AIM24033
 *    YOUVASHREE K — CB.AI.U4AIM24051
 *
 *  Course: 24AIM204 – Foundations of Computer Architecture
 *
 *  Hardware:
 *    - ESP32 Dev Module
 *    - PPG Sensor (Analog)     → GPIO 34
 *    - LM35 Temperature Sensor → GPIO 35
 *    - MAX30102 (I2C)          → SDA: GPIO 21, SCL: GPIO 22
 *
 *  Serial Monitor: 115200 baud
 *  Input Format:   <mappingType> <blockIndex>
 *    - 0 = Direct Mapping
 *    - 1 = Fully Associative
 *    - 2 = 2-Way Set Associative
 *
 * ============================================================================
 */

#include <Wire.h>
#include "MAX30105.h"
#include "spo2_algorithm.h"

MAX30105 particleSensor;

// =============================================================================
//  CACHE SIMULATION CONFIGURATION
// =============================================================================

#define CACHE_LINES   4    // Number of cache lines
#define MAIN_BLOCKS   16   // Number of main memory blocks

/**
 * @brief Represents a single block of main memory.
 *        Each block stores one snapshot of vital sign readings.
 */
struct Block {
  float temp;   // Temperature in °C
  int   hr;     // Heart rate in bpm
  int   spo2;   // Blood oxygen saturation in %
};

/**
 * @brief Represents a single cache line.
 *        Contains validity bit, tag, cached data, and timestamp for FIFO.
 */
struct CacheLine {
  bool          valid;  // Whether this line holds valid data
  int           tag;    // Block index stored in this line
  Block         data;   // Cached block data
  unsigned long time;   // Timestamp for FIFO replacement
};

// Memory arrays
Block     mainMem[MAIN_BLOCKS];   // Simulated main memory
CacheLine cache[CACHE_LINES];     // Simulated cache memory
unsigned long timeCounter = 0;    // Global timestamp counter for FIFO

// =============================================================================
//  SENSOR CONFIGURATION
// =============================================================================

const int PPG_PIN   = 34;    // Analog PPG sensor input
const int TEMP_PIN  = 35;    // LM35 temperature sensor input
const int THRESHOLD = 550;   // PPG pulse detection threshold

// PPG variables
float         bpmPPG        = 0;
bool          pulseDetected = false;
unsigned long lastBeatTime  = 0;
unsigned long lastPrint     = 0;

// MAX30102 variables
int32_t spo2, hr;
int8_t  validSPO2, validHR;

// =============================================================================
//  FUNCTION PROTOTYPES
// =============================================================================

void  processRequest(int mapType, int block);
void  handleUserInput();
void  initCache();
float getTemp();
float getBPM();
float getSpO2();

// =============================================================================
//  SETUP
// =============================================================================

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);  // I2C: SDA=21, SCL=22
  delay(1000);

  Serial.println("============================================");
  Serial.println("  VitalCache: Multi-Vital Sign Cache Analyzer");
  Serial.println("============================================");
  Serial.println();
  Serial.println("Initializing sensors...");

  // Initialize MAX30102
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("[ERROR] MAX30102 not found! Check wiring.");
  } else {
    particleSensor.setup(60, 4, 2, 50, 411, 4096);
    Serial.println("[OK] MAX30102 ready!");
  }

  // Configure ADC resolution for ESP32 (12-bit)
  analogReadResolution(12);

  // Initialize cache memory
  initCache();

  Serial.println("[OK] Cache initialized (4 lines, 16 blocks)");
  Serial.println();
  Serial.println("Enter commands as: <mappingType> <blockIndex>");
  Serial.println("  0 = Direct Mapping");
  Serial.println("  1 = Fully Associative");
  Serial.println("  2 = 2-Way Set Associative");
  Serial.println("  blockIndex: 0-15");
  Serial.println("============================================");
  Serial.println();
}

// =============================================================================
//  MAIN LOOP
// =============================================================================

void loop() {
  unsigned long now = millis();

  // ----- Analog PPG Heart Rate Detection -----
  int ppgVal = analogRead(PPG_PIN);
  if (ppgVal > THRESHOLD && !pulseDetected) {
    pulseDetected = true;
    unsigned long interval = now - lastBeatTime;
    if (lastBeatTime > 0 && interval > 300) {
      bpmPPG = 60000.0 / interval;
    }
    lastBeatTime = now;
  }
  if (ppgVal < THRESHOLD - 50) {
    pulseDetected = false;
  }

  // ----- MAX30102 SpO₂ & Heart Rate -----
  if (particleSensor.check()) {
    static uint32_t ir[100], red[100];
    static byte i = 0;
    ir[i]  = particleSensor.getIR();
    red[i] = particleSensor.getRed();
    i++;
    if (i >= 100) {
      maxim_heart_rate_and_oxygen_saturation(
        ir, 100, red, &spo2, &validSPO2, &hr, &validHR
      );
      i = 0;
    }
  }

  // ----- LM35 Temperature Reading -----
  int   tempVal = analogRead(TEMP_PIN);
  float tempC   = (tempVal / 4095.0) * 3.3 * 100.0;

  // ----- Periodic Sensor Output (Non-blocking, every 1s) -----
  if (now - lastPrint > 1000) {
    lastPrint = now;
    Serial.print("PPG BPM: ");
    Serial.print(bpmPPG);
    Serial.print(" | Temp: ");
    Serial.print(tempC, 1);
    if (validSPO2 && validHR) {
      Serial.print(" | SpO2: ");
      Serial.print(spo2);
    }
    Serial.println();
  }

  // ----- Handle user input for cache simulation -----
  handleUserInput();
}

// =============================================================================
//  CACHE SIMULATION FUNCTIONS
// =============================================================================

/**
 * @brief Direct Mapping: Compute cache slot from block index.
 * @param b Block index
 * @return Cache line index
 */
int directSlot(int b) {
  return b % CACHE_LINES;
}

/**
 * @brief FIFO Replacement: Find the oldest cache line.
 * @return Index of the cache line to evict
 */
int fifoReplace() {
  int idx = 0;
  for (int i = 1; i < CACHE_LINES; i++) {
    if (cache[i].time < cache[idx].time) {
      idx = i;
    }
  }
  return idx;
}

/**
 * @brief Search cache for a specific block.
 * @param b Block index to search for
 * @return Cache line index if found, -1 otherwise
 */
int findBlock(int b) {
  for (int i = 0; i < CACHE_LINES; i++) {
    if (cache[i].valid && cache[i].tag == b) {
      return i;
    }
  }
  return -1;
}

/**
 * @brief Update a cache line with new block data.
 * @param idx  Cache line index
 * @param tag  Block index (tag)
 * @param blk  Block data to store
 */
void updateCacheLine(int idx, int tag, Block blk) {
  cache[idx].valid = true;
  cache[idx].tag   = tag;
  cache[idx].data  = blk;
  cache[idx].time  = ++timeCounter;
}

/**
 * @brief Initialize all cache lines to invalid state.
 */
void initCache() {
  for (int i = 0; i < CACHE_LINES; i++) {
    cache[i].valid = false;
    cache[i].tag   = -1;
  }
}

/**
 * @brief Process a cache request using the specified mapping technique.
 * 
 * Simulates the Fetch → Decode → Execute → Writeback pipeline:
 *   - Fetch:   Identify mapping type and block
 *   - Decode:  Read sensor values into the block
 *   - Execute: Compute aggregate sum
 *   - Output:  Hit/Miss/Conflict metrics + health alerts
 *
 * @param mapType  0=Direct, 1=Fully Associative, 2=Set-Associative
 * @param block    Block index (0 to MAIN_BLOCKS-1)
 */
void processRequest(int mapType, int block) {
  if (block < 0 || block >= MAIN_BLOCKS) {
    Serial.println("[ERROR] Block index out of range (0-15).");
    return;
  }

  // Update main memory block with current sensor readings
  mainMem[block].temp = getTemp();
  mainMem[block].hr   = getBPM();
  mainMem[block].spo2 = getSpO2();

  Serial.println();
  Serial.println("============================");
  Serial.printf("Fetch:   Mapping %d, Block %d\n", mapType, block);
  Serial.printf("Decode:  %.1f°C, %d bpm, %d%%\n",
                mainMem[block].temp,
                mainMem[block].hr,
                mainMem[block].spo2);

  int hit = 0, miss = 0, conflict = 0;
  int found = findBlock(block);

  // ---- Mode 0: Direct Mapping ----
  if (mapType == 0) {
    int slot = directSlot(block);
    if (found == slot) {
      hit = 1;
    } else {
      if (cache[slot].valid && cache[slot].tag != block) {
        conflict = 1;
      }
      updateCacheLine(slot, block, mainMem[block]);
      miss = 1;
    }
  }

  // ---- Mode 1: Fully Associative Mapping ----
  else if (mapType == 1) {
    if (found != -1) {
      hit = 1;
    } else {
      int idx = fifoReplace();
      updateCacheLine(idx, block, mainMem[block]);
      miss = 1;
    }
  }

  // ---- Mode 2: 2-Way Set Associative Mapping ----
  else if (mapType == 2) {
    int set    = block % 2;
    int base   = set * 2;
    int chosen = -1;

    // Search within the set
    for (int i = base; i < base + 2; i++) {
      if (cache[i].valid && cache[i].tag == block) {
        hit    = 1;
        chosen = i;
        break;
      }
    }

    // Miss: find empty line or evict via FIFO
    if (!hit) {
      for (int i = base; i < base + 2; i++) {
        if (!cache[i].valid) {
          chosen = i;
          break;
        }
      }
      if (chosen == -1) {
        chosen = (cache[base].time < cache[base + 1].time) ? base : base + 1;
      }
      updateCacheLine(chosen, block, mainMem[block]);
      miss = 1;
    }
  }

  // Compute aggregate
  int sum = mainMem[block].temp + mainMem[block].hr + mainMem[block].spo2;
  Serial.printf("Execute: Sum = %d\n", sum);
  Serial.printf("Hit:%d  Miss:%d  Conflict:%d\n", hit, miss, conflict);

  // ----- Health Alerts -----
  if (mainMem[block].temp > 38) {
    Serial.println("⚠  Fever detected!");
  }
  if (mainMem[block].hr < 60 || mainMem[block].hr > 100) {
    Serial.println("⚠  Abnormal heart rate!");
  }
  if (mainMem[block].spo2 < 95) {
    Serial.println("⚠  Low SpO₂!");
  }

  Serial.println("============================");
}

// =============================================================================
//  SENSOR HELPER FUNCTIONS
// =============================================================================

/**
 * @brief Read current temperature from LM35 sensor.
 * @return Temperature in °C
 */
float getTemp() {
  int tempVal = analogRead(TEMP_PIN);
  return (tempVal / 4095.0) * 3.3 * 100.0;
}

/**
 * @brief Get current heart rate from PPG sensor.
 * @return Heart rate in bpm
 */
float getBPM() {
  return bpmPPG;
}

/**
 * @brief Get current SpO₂ from MAX30102 sensor.
 * @return SpO₂ percentage, or 0 if not valid
 */
float getSpO2() {
  if (validSPO2) return spo2;
  return 0;
}

// =============================================================================
//  SERIAL INPUT HANDLER
// =============================================================================

/**
 * @brief Handle user input from Serial Monitor.
 *        Expected format: <mappingType> <blockIndex>
 *        Example: "0 5" for Direct Mapping on Block 5
 */
void handleUserInput() {
  if (Serial.available()) {
    String line = Serial.readStringUntil('\n');
    line.trim();
    if (line.length() > 0) {
      int mapType, block;
      if (sscanf(line.c_str(), "%d %d", &mapType, &block) == 2) {
        processRequest(mapType, block);
      } else {
        Serial.println("[INFO] Enter as: <mappingType> <blockIndex>");
        Serial.println("       Example: 0 5  (Direct Mapping, Block 5)");
      }
    }
  }
}
