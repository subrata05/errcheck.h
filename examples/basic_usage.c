/**
 * =============================================================================
 * examples/basic_usage.c
 * 
 * Demonstrates the most common use case of errcheck.h:
 * =============================================================================
 */

#include <stdio.h>      
#include "../errcheck.h" 

/* -------------------------------------------------------------------------
 * User-defined error type and constants
 * ------------------------------------------------------------------------- */
typedef enum {
    ERR_NONE = 0,       // Success
    ERR_POWER,          // Power regulator failed
    ERR_SENSOR,         // Sensor initialization failed
    ERR_RADIO           // Radio module failed
} err_t;

/* Value returned on any failure */
#define ERR_FAILURE 0xFF             

/* Global variable that stores the last error code (set automatically by CHECK) */
err_t g_last_error = ERR_NONE;

/* -------------------------------------------------------------------------
 * Hardware initialization functions (replace with real drivers)
 * ------------------------------------------------------------------------- */
int init_power(void)
{
    printf("Power regulator: OK\n");
    return 1;                           // 1 = success, 0 = failure
}

int init_sensor(void)
{
    printf("Sensor: OK\n");
    return 1;
}

int init_radio(void)
{
    printf("Radio: FAILED\n");
    return 0;                           // ← This one intentionally fails
}

/* -------------------------------------------------------------------------
 * Device initialization using errcheck.h
 * If any step fails → immediately return with correct error code
 * ------------------------------------------------------------------------- */
err_t device_init(void)
{
    /* CHECK(call, error_flag) does this automatically:
       if the call returns 0 → store error_flag in g_last_error and return ERR_FAILURE */
    CHECK(init_power(),  ERR_POWER);    // Line 1: will succeed
    CHECK(init_sensor(), ERR_SENSOR);   // Line 2: will succeed
    CHECK(init_radio(),  ERR_RADIO);    // Line 3: will fail → function returns here!

    // This line is only reached if ALL checks passed
    return ERR_NONE;
}

int main(void)
{
    printf("Starting device initialization...\n");

    err_t result = device_init();

    if (result == ERR_FAILURE) {
        printf("Initialization FAILED!\n");
        printf("→ Error code = %d (ERR_RADIO = %d)\n", g_last_error, ERR_RADIO);
        // Upper layers can now take recovery action based on the exact error
    } else {
        printf("Initialization successful!\n");
    }

    return 0;
}
