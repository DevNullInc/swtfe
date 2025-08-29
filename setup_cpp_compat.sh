#!/bin/bash

# Helper script to fix common string conversion issues in the codebase

# Fix 1: Create a helper header with useful definitions
cat > /workspaces/swtfe/src/cpp_compat.h << 'HEADER_EOF'
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
#include <stddef.h>

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

/* Safe buffer handling for truncation prevention */
inline void safe_strcat(char *dest, const char *src, size_t dest_size) {
    size_t cur_len = strlen(dest);
    if (cur_len < dest_size - 1) {
        strncat(dest, src, dest_size - cur_len - 1);
        dest[dest_size - 1] = '\0';  /* Ensure null termination */
    }
}

#define SAFE_BUFFER_COPY(dest_buf, dest_size, src_buf) do { \
    strncpy((dest_buf), (src_buf), (dest_size)-1); \
    (dest_buf)[(dest_size)-1] = '\0'; \
} while(0)

#define SAFE_BUFFER_CAT(dest_buf, dest_size, src_buf) do { \
    safe_strcat((dest_buf), (src_buf), (dest_size)); \
} while(0)

#else
/* In C mode, these are no-ops or simpler implementations */
#define STRING_LITERAL(s) (s)
#define safe_strdup(str) strdup(str)
#define MUTABLE_CAPITALIZE(s) capitalize(s)
#define STRALLOC_CAPITALIZE(s) STRALLOC(capitalize(s))
#define SAFE_BUFFER_COPY(dest_buf, dest_size, src_buf) do { \
    strncpy((dest_buf), (src_buf), (dest_size)-1); \
    (dest_buf)[(dest_size)-1] = '\0'; \
} while(0)
#define SAFE_BUFFER_CAT(dest_buf, dest_size, src_buf) do { \
    size_t len = strlen((dest_buf)); \
    if (len < ((dest_size)-1)) { \
        strncat((dest_buf), (src_buf), ((dest_size)-len-1)); \
        (dest_buf)[(dest_size)-1] = '\0'; \
    } \
} while(0)
#endif

#endif /* _CPP_COMPAT_H_ */
HEADER_EOF

# Fix 2: Update mud.h to include our compatibility header
if grep -q "cpp_compat.h" /workspaces/swtfe/src/mud.h; then
    echo "cpp_compat.h already included in mud.h"
else
    # Insert after the first few includes
    sed -i '/#include <math.h>/a #include "cpp_compat.h"' /workspaces/swtfe/src/mud.h
    echo "Added cpp_compat.h to mud.h"
fi

echo "Created compatibility header and updated mud.h"
echo ""
echo "Usage suggestions:"
echo "1. Replace string literals with STRING_LITERAL() where needed"
echo "   Example: ch_printf(ch, \"Text\") -> ch_printf(ch, STRING_LITERAL(\"Text\"))"
echo ""
echo "2. Use safe_strdup() instead of strdup() for const char* inputs"
echo "   Example: char *name = strdup(capitalize(arg)) -> char *name = safe_strdup(capitalize(arg))"
echo ""
echo "3. Use MUTABLE_CAPITALIZE() for capitalizing strings that need to be mutable"
echo "   Example: char *name = capitalize(arg) -> char *name = MUTABLE_CAPITALIZE(arg)"
echo ""
echo "4. Use STRALLOC_CAPITALIZE() to prevent warnings with STRALLOC(capitalize())"
echo "   Example: STRALLOC(capitalize(arg)) -> STRALLOC_CAPITALIZE(arg)"
echo ""
echo "5. Use safe buffer handling macros to avoid truncation warnings:"
echo "   SAFE_BUFFER_COPY(dest_buf, dest_size, src_buf)"
echo "   SAFE_BUFFER_CAT(dest_buf, dest_size, src_buf)"
echo ""
echo "Compatibility header created successfully!"
