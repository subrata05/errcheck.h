/**
 * =============================================================================
 * examples/fault_injection_compile_time.c
 * 
 * Demonstrates COMPILE-TIME fault injection – the #1 way to automatically
 * test every error path in CI and on real hardware without changing runtime code.
 * 
 * How to test a failure:
 *   gcc this_file.c ../errcheck.h -o test_imu_fail -D INJECT_ERR_SENSOR
 * 
 * The IMU calibration will be FORCED to fail – proving the error handling works!
 * =============================================================================
 */

#include <stdio.h>
#include "../errcheck.h"

/* -------------------------------------------------------------------------
 * 1. COMPILE-TIME FAULT INJECTION CONTROL
 * ------------------------------------------------------------------------- 
 * Just define one of these before compiling (e.g. via -D flag or in CI script)
 * This is the cleanest, zero-overhead way to test error paths
 */
#define INJECT_ERR_SENSOR          // ← Remove/comment this line for normal behavior

/* Redefine the CHECK for sensor-related calls based on injection flag */
#ifdef INJECT_ERR_SENSOR
    /* Force failure: pretend the call returned 0 and inject ERR_SENSOR */
    #define CHECK_SENSOR(call)  CHECK(0, ERR_SENSOR)
#else
    /* Normal behavior: actually execute the call */
    #define CHECK_SENSOR(call)  CHECK(call, ERR_SENSOR)
#endif

/* -------------------------------------------------------------------------
 * User-defined error codes and globals
 * ------------------------------------------------------------------------- */
typedef enum {
    ERR_NONE    = 0,
    ERR_SENSOR  = 1          // IMU, accelerometer, gyro, etc.
} err_t;

#define ERR_FAILURE 0xFF

err_t g_last_error = ERR_NONE;   // Automatically set by CHECK() on failure

/* -------------------------------------------------------------------------
 * Fake IMU calibration function (in real code: testing driver)
 * ------------------------------------------------------------------------- */
int calibrate_imu(void)
{
    printf("IMU calibration: would normally pass\n");
    return 1;                // Success in real life
}

/* -------------------------------------------------------------------------
 * System initialization – uses the injectable macro
 * ------------------------------------------------------------------------- */
err_t init(void)
{
    printf("Starting initialization with fault injection test...\n");

    /* This line will:
         • With INJECT_ERR_SENSOR  → always fail with ERR_SENSOR
         • Without the define      → behave normally (pass) */
    CHECK_SENSOR(calibrate_imu());

    /* Only reached if calibration passed */
    printf("IMU calibration successful!\n");
    return ERR_NONE;
}


int main(void)
{
    err_t result = init();

    if (result == ERR_FAILURE) {
        printf("\nInitialization FAILED (as expected in fault injection mode)\n");
        printf("→ Error code = %d → ERR_SENSOR\n", g_last_error);
    } else {
        printf("\nAll good – initialization passed!\n");
    }

    return 0;
}
