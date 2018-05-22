#ifndef __MD2_H__
#define __MD2_H__

#define	UINT64 unsigned long long
#define	UINT32 unsigned int
#define	BYTE unsigned char

static const UINT64 MAXNUM64 = 0xffffffffffffffff;
static const UINT32 MAXNUM = 0xffffffff;

typedef unsigned int word32;

void CreateHashMD2(const BYTE *input, size_t length, BYTE *out);

#endif //__MD2_H__
