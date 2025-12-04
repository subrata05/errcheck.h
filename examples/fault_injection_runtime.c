/**
 * =============================================================================
 * examples/fault_injection_runtime.c
 * 
 * RUNTIME (debugger-controlled) fault injection
 * 
 * Perfect for:
 *   • Manual testing on real hardware
 *   • Integration tests
 *   • Demonstrating recovery behavior to stakeholders
 *   • Safety certification (prove every error path is handled)
 * 
 * How to trigger failure from debugger (GDB, Ozone, PyOCD, etc.):
 *     (gdb) set var g_inject_error_flag = 1
 * 
 * The next CHECK with ERR_RADIO will instantly fail — even if radio_start() returns success!
 * =============================================================================
 */

#define ERRCHECK_ENABLE_RUNTIME_INJECTION   // ← Enables the injection hook in errcheck.h
#include "../errcheck.h"
#include <stdio.h>

/* -------------------------------------------------------------------------
 * User-defined error codes
 * ------------------------------------------------------------------------- */
typedef enum {
    ERR_NONE  = 0,
    ERR_RADIO = 1           // Radio bring-up or communication failure
} err_t;

#define ERR_FAILURE 0xFF

err_t g_last_error = ERR_NONE;

/* -------------------------------------------------------------------------
 * Global injection trigger — set this from debugger or test script
 * Only exists when ERRCHECK_ENABLE_RUNTIME_INJECTION is defined
 * ------------------------------------------------------------------------- */
volatile uint8_t g_inject_error_flag = 0;   // 0 = normal, 1 = force ERR_RADIO

/* -------------------------------------------------------------------------
 * Fake radio driver (in real project: the actual function)
 * ------------------------------------------------------------------------- */
int radio_start(void)
{
    printf("Radio hardware start → normally would succeed\n");
    return 1;               // Real hardware passes
}

/* -------------------------------------------------------------------------
 * System initialization using runtime-injectable check
 * ------------------------------------------------------------------------- */
err_t init(void)
{
    printf("Starting initialization...\n");

    /* This CHECK will:
         • Fail immediately if g_inject_error_flag == ERR_RADIO
         • Otherwise execute radio_start() normally */
    CHECK(radio_start(), ERR_RADIO);

    /* Only reached if radio passed (or wasn't injected) */
    printf("Radio initialized successfully!\n");
    return ERR_NONE;
}


int main(void)
{
    printf("=== Runtime Fault Injection Demo ===\n");
    printf("Watch this space in your debugger!\n\n");

    err_t result = init();

    if (result == ERR_FAILURE) {
        printf("\nInitialization FAILED!\n");
        printf("→ Error code = %d → ERR_RADIO (injected or real failure)\n", g_last_error);
        printf("   (This was triggered by setting g_inject_error_flag = 1)\n");
    } else {
        printf("\nAll good — initialization passed!\n");
    }

    printf("\nTip: In your debugger, try:\n");
    printf("    set var g_inject_error_flag = 1\n");
    printf("    run\n");
    printf("→ You will see instant failure with correct error code!\n");

    return 0;
}
