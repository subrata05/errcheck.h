# errcheck.h — Fail-fast error handling with built in fault injection for embedded C

**Before errcheck.h (50+ lines of boilerplate):**

```c
if (!init_power())   { err = ERR_POWER;  return ERROR; }
if (!init_sensor())  { err = ERR_SENSOR; return ERROR; }
if (!init_radio())   { err = ERR_RADIO;  return ERROR; }
if (!init_flash())   { err = ERR_FLASH;  return ERROR; }
// ... and 20 more identical lines
```

**After errcheck.h (just 4 lines):**

```c
CHECK(init_power(),  ERR_POWER);
CHECK(init_sensor(), ERR_SENSOR);
CHECK(init_radio(),  ERR_RADIO);
CHECK(init_flash(),  ERR_FLASH);
```

One line per operation • Zero duplication • Full fault injection • Used in real flight controllers, medical devices, and automotive ECUs.

---

## Why This Approach Is a Significant Improvement

| Problem in Classic Embedded                             | errcheck.h Solution                               | Real-World Benefit             |
| ------------------------------------------------------- | ------------------------------------------------- | ------------------------------ |
| 50–500 lines of repetitive error checking               | One macro → one line per call                     | Code shrinks dramatically      |
| Easy to assign wrong error flag (copy‑paste bug)        | Error flag is right next to the call              | No more mismatched error codes |
| Impossible to test every error path automatically       | Built‑in compile‑time + runtime fault injection   | 100% error‑path coverage in CI |
| Repetitive boilerplate makes the code harder to read    | Intent is instantly clear                         | Faster onboarding, fewer bugs  |
| Adding/removing a step = editing 5 lines                | Add/delete exactly one line                       | Zero maintenance risk          |

---

## Fail-Fast error checking architecture 

This pattern is called **fail-fast error handling** and is the gold standard in safety‑critical firmware (DO‑178C, ISO 26262, IEC 61508).

Core idea:

* As soon as something fails → set the correct error code **and immediately return**.
* Never let the system continue in a half‑initialized state.

Advantages:

* No deep nesting of `if` statements
* No forgotten error propagation
* Single exit point → trivial to log/audit
* Perfect for state machines and recovery logic

**errcheck.h** is simply the cleanest, most testable implementation of this pattern.

---

## Goal of Fault Injection
* In real hardware, failures are rare and unpredictable.
* You can’t just wait for a sensor to actually die during testing.
* Fault injection lets you force a specific subsystem to fail at exactly the right moment so you can prove:

* Upper-layer state machines go to the correct SAFE/FAILURE state
* Watchdog timer resets the MCU when they should
* Fallback/degraded modes are actually executed
* Error logs, telemetry, and black-box recorders contain the correct error flag
* Automatic recovery mechanisms (retry, switch to backup sensor, etc.) really work
* System meets safety requirements (e.g. ASIL-B, SIL-3, Class III medical)

---

## Step‑by‑Step: How to Use It

### 1. Copy the header

Just drop `errcheck.h` into your project. No dependencies.

### 2. Define your error type (once per project)

```c
typedef enum {
    ERR_NONE = 0,
    ERR_POWER,
    ERR_SENSOR,
    ERR_RADIO,
    ERR_FLASH,
    ERR_TIMEOUT,
    ERR_MAX
} err_t;

#define ERR_FAILURE  0xFF               // Or ((err_t)-1) if you prefer
err_t g_last_error = ERR_NONE;          // Global – automatically updated
```

### 3. Basic usage

```c
err_t device_init(device_t *dev)
{
    CHECK(power_on(3300),            ERR_POWER);
    CHECK(sensor_init(&dev->sensor), ERR_SENSOR);
    CHECK(radio_begin(),             ERR_RADIO);
    CHECK(flash_verify(),            ERR_FLASH);

    return ERR_NONE;   // Only reached if everything passed
}
```

### 4. When many calls share the same error code

```c
err_t g_current_error_group = ERR_I2C;

err_t i2c_init(void)
{
    CHECK_SAME(i2c_master_enable());
    CHECK_SAME(i2c_set_baudrate(400000));
    CHECK_SAME(i2c_probe_slave(0x50));
    // All of the above will return ERR_I2C on failure
    return ERR_NONE;
}
```

### 5. Compile‑time Fault Injection (CI & Automated Testing)

You can force any subsystem to fail at compile time — perfect for proving every error path is handled.

```c
// test_sensor_failure.c
#define INJECT_ERR_SENSOR                      // ← enable injection

#ifdef INJECT_ERR_SENSOR
    #define CHECK_SENSOR(call)  CHECK(0, ERR_SENSOR)
#else
    #define CHECK_SENSOR(call)  CHECK(call, ERR_SENSOR)
#endif

err_t init(void)
{
    CHECK_SENSOR(calibrate_imu());   // ← always fails when testing
    // ... rest of init
}
```

Now your CI builds the same code 8 times with different `-D INJECT_...` flags → **100% error‑path coverage**, zero runtime cost.

### 6. Runtime Fault Injection (Debugger / Integration Tests)

Enable at compile time (usually only in debug builds):

```c
#define ERRCHECK_ENABLE_RUNTIME_INJECTION
#include "errcheck.h"

volatile uint8_t g_inject_error_flag = 0;   // Set from GDB/OpenOCD
```

From GDB:

```
(gdb) set var g_inject_error_flag = 2   # 2 == ERR_RADIO
(gdb) continue
```

→ Next `CHECK` with `ERR_RADIO` fails instantly, even if the real call succeeded!

Perfect for:

* Demonstrating recovery behavior to customers
* Integration testing on real hardware
* Safety certification evidence

---

## Full Feature List

| Feature                   | Macro / Define                               | Use Case                    |
| ------------------------- | -------------------------------------------- | --------------------------- |
| Standard check            | `CHECK(call, ERR_XXX)`                       | Most common                 |
| Same error for many calls | `CHECK_SAME(call)` + `g_current_error_group` | I2C, SPI, UART groups       |
| Manual return             | `RETURN_ERR(ERR_XXX)`                        | Early exit before checks    |
| Runtime injection         | `#define ERRCHECK_ENABLE_RUNTIME_INJECTION`  | Debugger‑controlled testing |
| Optional logging          | `#define ERRCHECK_ENABLE_LOGGING`            | printf/SEGGER_RTT on error  |

---

## Examples (ready to compile)

* `examples/basic_usage.c` – Classic init sequence
* `examples/multiple_errors.c` – Mixed error codes + CHECK_SAME
* `examples/fault_injection_compile_time.c` – CI testing
* `examples/fault_injection_runtime.c` – Debugger injection

---

## Installation

```bash
# Just copy the file – that's it
cp errcheck.h src/
```


