#ifndef _CPP_COMPAT_H_
#define _CPP_COMPAT_H_

/*
 * This header provides compatibility functions and defines for 
 * transitioning from C to C++ and handling string conversion issues.
 */
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

#ifdef __cplusplus
/* This macro casts string literals to char* to prevent warnings in C++ */
#define STRING_LITERAL(s) const_cast<char*>(s)

/* Helper functions for safe string handling */
inline char* safe_strdup(const char* str) {
    return str ? strdup(str) : NULL;
}

/* This macro creates a mutable copy of the result of capitalize() */
#define MUTABLE_CAPITALIZE(s) safe_strdup(capitalize(s))

/* This macro wraps STRALLOC for capitalize() result to prevent warnings */
#define STRALLOC_CAPITALIZE(s) STRALLOC(const_cast<char*>(capitalize(s)))

/* Safe buffer handling functions */
inline int safe_snprintf(char *buf, size_t size, const char *format, ...) {
    va_list args;
    va_start(args, format);
    int result = vsnprintf(buf, size, format, args);
    va_end(args);
    buf[size-1] = '\0'; /* Ensure null termination */
    return result;
}

/* Two-stage buffer building to avoid truncation */
#define SAFE_BUFFER_COPY(dest_buf, dest_size, src_buf) \
    do { \
        strncpy((dest_buf), (src_buf), (dest_size)-1); \
        (dest_buf)[(dest_size)-1] = '\0'; \
    } while(0)

#define SAFE_BUFFER_CAT(dest_buf, dest_size, src_buf) \
    do { \
        strncat((dest_buf), (src_buf), (dest_size)-strlen(dest_buf)-1); \
        (dest_buf)[(dest_size)-1] = '\0'; \
    } while(0)

#else
/* In C mode, these are no-ops */
#define STRING_LITERAL(s) (s)
#define safe_strdup(str) strdup(str)
#define MUTABLE_CAPITALIZE(s) capitalize(s)
#define STRALLOC_CAPITALIZE(s) STRALLOC(capitalize(s))
#define safe_snprintf snprintf
#define SAFE_BUFFER_COPY(dest_buf, dest_size, src_buf) strncpy((dest_buf), (src_buf), (dest_size)-1); (dest_buf)[(dest_size)-1] = '\0'
#define SAFE_BUFFER_CAT(dest_buf, dest_size, src_buf) strncat((dest_buf), (src_buf), (dest_size)-strlen(dest_buf)-1); (dest_buf)[(dest_size)-1] = '\0'
#endif

#endif /* _CPP_COMPAT_H_ */
