# Cache Memory Concepts — VitalCache

This document provides a detailed reference for the cache memory concepts implemented in the VitalCache system.

---

## 1. Cache Memory Fundamentals

Cache memory is a small, fast memory located between the CPU and main memory. It stores frequently accessed data to reduce access time.

### Locality Principles

| Principle | Description | VitalCache Example |
|-----------|-------------|--------------------|
| **Temporal Locality** | Recently accessed data will likely be accessed again soon | Repeated queries for the same patient block |
| **Spatial Locality** | Data near recently accessed locations will likely be accessed | Sequential block queries (Block 4, then Block 5) |

### Cache Structure

Each cache line in VitalCache contains:

```
┌─────────┬──────┬──────────────────────────┬────────────┐
│  Valid   │  Tag │         Data             │  Timestamp │
│  (bool)  │(int) │ (temp, hr, spo2)         │  (ulong)   │
└─────────┴──────┴──────────────────────────┴────────────┘
```

- **Valid bit**: Indicates whether the cache line holds meaningful data
- **Tag**: Identifies which main memory block is stored
- **Data**: The actual vital sign readings (temperature, heart rate, SpO₂)
- **Timestamp**: Used for FIFO replacement policy

---

## 2. Address Decomposition

In a real cache system, memory addresses are split into:

```
┌────────────────────────────────────────┐
│  Memory Address                        │
├──────────┬──────────┬─────────────────┤
│   Tag    │  Index   │     Offset      │
│ (block   │ (cache   │  (byte within   │
│  ID)     │  line)   │   block)        │
└──────────┴──────────┴─────────────────┘
```

In VitalCache:
- **Tag** = Block index (0–15)
- **Index** = Computed based on mapping technique
- **Offset** = N/A (entire block is one unit)

---

## 3. Mapping Techniques

### 3.1 Direct Mapping (Mode 0)

Each main memory block maps to exactly **one** cache line.

```
Formula: cache_line = block_index % CACHE_LINES

Block  0 → Line 0     Block  4 → Line 0     Block  8 → Line 0
Block  1 → Line 1     Block  5 → Line 1     Block  9 → Line 1
Block  2 → Line 2     Block  6 → Line 2     Block 10 → Line 2
Block  3 → Line 3     Block  7 → Line 3     Block 11 → Line 3
```

**Pros**: Simple, fast lookup  
**Cons**: High conflict miss rate (blocks competing for same line)

### 3.2 Fully Associative Mapping (Mode 1)

Any block can be placed in **any** cache line.

```
Block 5 → Line 0, 1, 2, or 3 (wherever there's space)

Search: Must check ALL lines for a match
Eviction: FIFO — oldest entry is replaced
```

**Pros**: No conflict misses, maximum flexibility  
**Cons**: Slower lookup (must search all lines), more complex hardware

### 3.3 2-Way Set Associative Mapping (Mode 2)

Cache is divided into **sets**, each containing **2 lines**.

```
Set 0: Lines 0, 1     (for even-indexed blocks)
Set 1: Lines 2, 3     (for odd-indexed blocks)

Formula: set = block_index % 2
         base_line = set * 2

Block 0 → Set 0 (Line 0 or 1)
Block 1 → Set 1 (Line 2 or 3)
Block 2 → Set 0 (Line 0 or 1)
Block 3 → Set 1 (Line 2 or 3)
```

**Pros**: Balanced — fewer conflicts than direct, simpler than fully associative  
**Cons**: Still some conflict potential within sets

---

## 4. Replacement Policy — FIFO

When the cache is full and a new block must be loaded, the **First-In, First-Out (FIFO)** policy is used:

1. Each cache line has a **timestamp** (incremented on every write)
2. On eviction, the line with the **smallest timestamp** (oldest) is replaced
3. This applies to Fully Associative and Set-Associative modes

```
Example:
  Time 1: Load Block 3 → Line 0 (timestamp=1)
  Time 2: Load Block 7 → Line 1 (timestamp=2)
  Time 3: Load Block 2 → Line 2 (timestamp=3)
  Time 4: Load Block 9 → Line 3 (timestamp=4)
  Time 5: Load Block 5 → Evict Line 0 (oldest, timestamp=1)
```

---

## 5. Performance Metrics

| Metric | Definition | Impact |
|--------|-----------|--------|
| **Hit** | Requested block found in cache | Fast access, no main memory read needed |
| **Miss** | Block not found; loaded from main memory | Slower access, cache updated |
| **Conflict** | Miss caused by another block occupying the target line | Only in Direct Mapping |

### Hit Ratio Formula

```
Hit Ratio = Total Hits / Total Requests
```

---

## 6. VitalCache Implementation Details

### Memory Configuration

| Parameter | Value |
|-----------|-------|
| Cache Lines | 4 |
| Main Memory Blocks | 16 |
| Block Size | 3 fields (temp, hr, spo2) |
| Replacement Policy | FIFO |
| Sets (Set-Associative) | 2 sets × 2 ways |

### Data Flow

```
Sensor Read → Main Memory Update → Cache Lookup → Hit/Miss → Output
```

---

## References

1. Patterson, D. A., & Hennessy, J. L. — *Computer Organization and Design: The Hardware/Software Interface*
2. Stallings, W. — *Computer Organization and Architecture: Designing for Performance*
3. Hamacher, C. et al. — *Computer Organization and Embedded Systems*
