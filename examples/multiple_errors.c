/**
 * =============================================================================
 * examples/multiple_errors.c
 * 
 * Shows how to handle different error codes for different subsystems
 * and how to use CHECK_SAME() when several calls share the same error type
 * =============================================================================
 */

#include <stdio.h>
#include "../errcheck.h"

/* -------------------------------------------------------------------------
 * User-defined error codes
 * ------------------------------------------------------------------------- */
typedef enum {
    ERR_NONE = 0,       // Success
    ERR_I2C,            // Any I2C-related failure
    ERR_SPI,            // SPI peripheral failure
    ERR_UART,           // UART initialization or config error
    ERR_TIMEOUT         // Communication timeout
} err_t;

/* Returned by any failing CHECK */
#define ERR_FAILURE 0xFF                    

/* Global variables required by errcheck.h */
err_t g_last_error = ERR_NONE;              // Stores which error occurred

/* Optional: used by CHECK_SAME() — lets many calls share one error code */
err_t g_current_error_group = ERR_I2C;      // Currently treating I2C calls as a group

/* -------------------------------------------------------------------------
 * Driver functions (replace with real HAL/driver calls in your project)
 * ------------------------------------------------------------------------- */
int i2c_write(void)  { printf("I2C write  → OK\n");  return 1; }
int i2c_read(void)   { printf("I2C read   → FAILED\n"); return 0; }  // ← Intentional failure

int spi_test(void)   { printf("SPI test   → OK\n");  return 1; }
int uart_init(void)  { printf("UART init  → OK\n");  return 1; }

/* -------------------------------------------------------------------------
 * Bus initialization using mixed error handling styles
 * ------------------------------------------------------------------------- */
err_t bus_init(void)
{
    /* CHECK_SAME(call) = shortcut when many calls should return the same error code
       It uses the current value of g_current_error_group (here: ERR_I2C) */
    CHECK_SAME(i2c_write());        // Succeeds → continues
    CHECK(i2c_read(), ERR_I2C);     // Fails → sets g_last_error = ERR_I2C and returns ERR_FAILURE

    /* Execution stops at the line above — lines below are never reached on failure */
    CHECK(spi_test(), ERR_SPI);     // Only runs if all previous checks passed
    CHECK(uart_init(), ERR_UART);

    /* Only reached if everything succeeded */
    return ERR_NONE;
}


int main(void)
{
    printf("=== Starting bus initialization ===\n");

    err_t result = bus_init();

    if (result == ERR_FAILURE) {
        printf("\nBus initialization FAILED!\n");
        printf("→ Last error code: %d ", g_last_error);

        /* UART based logging of errors */
        switch (g_last_error) {
            case ERR_I2C:     printf("(I2C communication error)\n"); break;
            case ERR_SPI:     printf("(SPI error)\n"); break;
            case ERR_UART:    printf("(UART error)\n"); break;
            case ERR_TIMEOUT: printf("(Timeout)\n"); break;
            default:          printf("(Unknown error)\n"); break;
        }
    } else {
        printf("\nAll buses initialized successfully!\n");
    }

    return 0;
}
