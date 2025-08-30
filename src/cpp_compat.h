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

#else /* __cplusplus */
/* In C mode, these are no-ops */
#define STRING_LITERAL(s) (s)
#define safe_strdup(str) strdup(str)
#define MUTABLE_CAPITALIZE(s) capitalize(s)
#define STRALLOC_CAPITALIZE(s) STRALLOC(capitalize(s))
#define safe_snprintf snprintf
#define SAFE_BUFFER_COPY(dest_buf, dest_size, src_buf) strncpy((dest_buf), (src_buf), (dest_size)-1); (dest_buf)[(dest_size)-1] = '\0'
#define SAFE_BUFFER_CAT(dest_buf, dest_size, src_buf) strncat((dest_buf), (src_buf), (dest_size)-strlen(dest_buf)-1); (dest_buf)[(dest_size)-1] = '\0'
#endif

/* BSD string functions for systems that don't have them */
#ifndef HAVE_STRLCPY
#ifdef __cplusplus
inline size_t strlcpy(char *dst, const char *src, size_t size) {
#else
static size_t strlcpy(char *dst, const char *src, size_t size) {
#endif
    size_t srclen = strlen(src);
    if (size > 0) {
        size_t len = srclen < (size - 1) ? srclen : (size - 1);
        memcpy(dst, src, len);
        dst[len] = '\0';
    }
    return srclen;
}
#endif

#ifndef HAVE_STRLCAT
#ifdef __cplusplus
inline size_t strlcat(char *dst, const char *src, size_t size) {
#else
static size_t strlcat(char *dst, const char *src, size_t size) {
#endif
    size_t dstlen = strlen(dst);
    size_t srclen = strlen(src);
    
    if (dstlen >= size) {
        return size + srclen;
    }
    
    if (dstlen + srclen < size) {
        memcpy(dst + dstlen, src, srclen + 1);
    } else {
        memcpy(dst + dstlen, src, size - dstlen - 1);
        dst[size - 1] = '\0';
    }
    
    return dstlen + srclen;
}
#endif

#endif /* _CPP_COMPAT_H_ */
