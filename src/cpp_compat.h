/*****************************************************************************************
 *                      .___________. __    __   _______                                 *
 *                      |           ||  |  |  | |   ____|                                *
 *                      `---|  |----`|  |__|  | |  |__                                   *
 *                          |  |     |   __   | |   __|                                  *
 *                          |  |     |  |  |  | |  |____                                 *
 *                          |__|     |__|  |__| |_______|                                *
 *                                                                                       *
 *                _______  __  .__   __.      ___       __                               *
 *               |   ____||  | |  \ |  |     /   \     |  |                              *
 *               |  |__   |  | |   \|  |    /  ^  \    |  |                              *
 *               |   __|  |  | |  . `  |   /  /_\  \   |  |                              *
 *               |  |     |  | |  |\   |  /  _____  \  |  `----.                         *
 *               |__|     |__| |__| \__| /__/     \__\ |_______|                         *
 *                                                                                       *
 *      _______ .______    __       _______.  ______    _______   _______                *
 *     |   ____||   _  \  |  |     /       | /  __  \  |       \ |   ____|               *
 *     |  |__   |  |_)  | |  |    |   (----`|  |  |  | |  .--.  ||  |__                  *
 *     |   __|  |   ___/  |  |     \   \    |  |  |  | |  |  |  ||   __|                 *
 *     |  |____ |  |      |  | .----)   |   |  `--'  | |  '--'  ||  |____                *
 *     |_______|| _|      |__| |_______/     \______/  |_______/ |_______|               *
 *****************************************************************************************
 *                                                                                       *
 * Star Wars: The Final Episode additions and changes from the Star Wars Reality code    *
 * copyright (c) 2025 /dev/null Industries - StygianRenegade                             *
 *                                                                                       *
 * Star Wars Reality Code Additions and changes from the Smaug Code copyright (c) 1997   *
 * by Sean Cooper                                                                        *
 *                                                                                       *
 * Starwars and Starwars Names copyright(c) Lucas Film Ltd.                              *
 *****************************************************************************************
 * Original SMAUG 1.4a written by Thoric (Derek Snider) with Altrag, Blodkai, Haus, Narn,*
 * Scryn, Swordbearer, Tricops, Gorog, Rennard, Grishnakh, Fireblade, and Nivek.         *
 *                                                                                       *
 * Original MERC 2.1 code by Hatchet, Furey, and Kahn.                                   *
 *                                                                                       *
 * Original DikuMUD code by: Hans Staerfeldt, Katja Nyboe, Tom Madsen, Michael Seifert,  *
 * and Sebastian Hammer.                                                                 *
 *****************************************************************************************
 *                              C++ Compatibility module                                 *
 *****************************************************************************************/

 #ifndef CPP_COMPAT_H
#define CPP_COMPAT_H

#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

#ifdef __cplusplus

#define STRING_LITERAL(s) const_cast<char*>(s)

inline char* safe_strdup(const char* str) {
    return str ? strdup(str) : NULL;
}

#define MUTABLE_CAPITALIZE(s) safe_strdup(capitalize(s))
#define STRALLOC_CAPITALIZE(s) STRALLOC(const_cast<char*>(capitalize(s)))

inline int safe_snprintf(char *buf, size_t size, const char *format, ...) {
    va_list args;
    va_start(args, format);
    int result = vsnprintf(buf, size, format, args);
    va_end(args);
    buf[size-1] = '\0';
    return result;
}

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

#define STRING_LITERAL(s) (s)
#define safe_strdup(str) strdup(str)
#define MUTABLE_CAPITALIZE(s) capitalize(s)
#define STRALLOC_CAPITALIZE(s) STRALLOC(capitalize(s))
#define safe_snprintf snprintf
#define SAFE_BUFFER_COPY(dest_buf, dest_size, src_buf) strncpy((dest_buf), (src_buf), (dest_size)-1); (dest_buf)[(dest_size)-1] = '\0'
#define SAFE_BUFFER_CAT(dest_buf, dest_size, src_buf) strncat((dest_buf), (src_buf), (dest_size)-strlen(dest_buf)-1); (dest_buf)[(dest_size)-1] = '\0'

#endif

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

#endif
