#ifndef _MB_ERR_CODE_H_
#define _MB_ERR_CODE_H_

#define ERR_BASE 0x10000000
#define NEXT_ERR 8

#define MAKE_ERR_CODE(e) (ERR_BASE + e)
#endif