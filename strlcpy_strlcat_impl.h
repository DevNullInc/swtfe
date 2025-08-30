// Add these function prototypes to a header file like cpp_compat.h

#ifndef STRLCPY_STRLCAT_DEFINED
#define STRLCPY_STRLCAT_DEFINED

#include <cstring>
#include <cstddef>

// strlcpy - Copy string with length checking
inline size_t strlcpy(char *dst, const char *src, size_t size) {
    size_t srclen = strlen(src);
    if (size > 0) {
        size_t len = srclen < (size - 1) ? srclen : (size - 1);
        memcpy(dst, src, len);
        dst[len] = '\0';
    }
    return srclen;
}

// strlcat - Concatenate strings with length checking
inline size_t strlcat(char *dst, const char *src, size_t size) {
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

#endif /* STRLCPY_STRLCAT_DEFINED */
