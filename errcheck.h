/**
 * =============================================================================
 * errcheck.h 
 * Clean, Testable, Fail-Fast Error Handling for Embedded C
 * =============================================================================
 * GitHub : https://github.com/subrata05/errcheck.h
 * =============================================================================
 */

#ifndef ERRCHECK_H
#define ERRCHECK_H

#include <stdint.h>

/* User must define these before including errcheck.h (or after with care) */
#ifndef ERR_T
    typedef uint8_t err_t;
    #define ERR_T
#endif

#ifndef ERR_FAILURE
    #define ERR_FAILURE ((err_t)0xFF)
#endif

extern err_t g_last_error;

/* ========================================================================= */
/* Core Macros                                                               */
/* ========================================================================= */

/* Standard check with specific error code */
#define CHECK(call, err_flag) do {                     \
    if ((call) == 0) {                                 \
        g_last_error = (err_flag);                     \
        return ERR_FAILURE;                            \
    }                                                  \
} while (0)

/* When many calls share the same error code */
#define CHECK_SAME(call) CHECK((call), g_current_error_group)

/* Manual return with error */
#define RETURN_ERR(err_flag) do {                      \
    g_last_error = (err_flag);                         \
    return ERR_FAILURE;                                \
} while (0)

/* ========================================================================= */
/* Optional: Runtime Fault Injection (Debug builds only)                     */
/* ========================================================================= */
#ifdef ERRCHECK_ENABLE_RUNTIME_INJECTION
    extern volatile uint8_t g_inject_error_flag;
    #undef CHECK
    #define CHECK(call, err_flag) do {                 \
        if ((call) == 0 || g_inject_error_flag == (err_flag)) { \
            g_last_error = (err_flag);                 \
            g_inject_error_flag = 0;                   \
            return ERR_FAILURE;                        \
        }                                              \
    } while (0)
#endif

/* ========================================================================= */
/* Optional: Error Logging                                                   */
/* ========================================================================= */
#ifdef ERRCHECK_ENABLE_LOGGING
    #define ERR_LOG(...) printf(__VA_ARGS__)
#else
    #define ERR_LOG(...)
#endif

#endif /* ERRCHECK_H */
