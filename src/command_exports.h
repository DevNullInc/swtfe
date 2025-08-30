#ifndef _COMMAND_EXPORTS_H_
#define _COMMAND_EXPORTS_H_

/*
 * This header ensures command functions are exported with C linkage
 * to prevent name mangling and allow dynamic lookup.
 */

#include "mud.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Just add a few critical commands to test the fix */
DECLARE_DO_FUN(do_say);
DECLARE_DO_FUN(do_emote);
DECLARE_DO_FUN(do_gtell);
DECLARE_DO_FUN(do_accelerate);
DECLARE_DO_FUN(do_addpilot);
DECLARE_DO_FUN(do_auction);

#ifdef __cplusplus
}
#endif

#endif /* _COMMAND_EXPORTS_H_ */
