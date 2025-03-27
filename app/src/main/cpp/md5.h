#ifndef __MD5_INCLUDE__

/* typedef a 32-bit type */
#ifdef _LP64
typedef unsigned int UINT4;
typedef int          INT4;
#else
typedef unsigned long UINT4;
typedef long          INT4;
#endif
#define _UINT4_T

/* Data structure for MD5 (Message-Digest) computation */
typedef struct {
    UINT4 i[2];                   /* number of _bits_ handled mod 2^64 */
    UINT4 buf[4];                                    /* scratch buffer */
    unsigned char in[64];                              /* input buffer */
    unsigned char digest[16];     /* actual digest after MD5Final call */
} MD5_CTX;

void MD5_Init (MD5_CTX *mdContext);
void MD5_Update (MD5_CTX *mdContext, const unsigned char *inBuf, unsigned int inLen);
void MD5_Final (unsigned char hash[], MD5_CTX *mdContext);

#define __MD5_INCLUDE__
#endif /* __MD5_INCLUDE__ */