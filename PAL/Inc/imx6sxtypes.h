#ifndef _IMX6SXTYPES_H
#define _IMX6SXTYPES_H
#include <signal.h>  


/* program memory space abstraction */
#define FLASH_EXTERN(x) extern const x
#define FLASH_DECLARE(x)  const x
#define PGM_READ_BYTE(x) *(x)
#define PGM_READ_WORD(x) *(x)
#define PGM_READ_BLOCK(dst, src, len) memcpy((dst), (src), (len))
#define FUNC_PTR(x) void (*x)(union sigval v)



#endif
