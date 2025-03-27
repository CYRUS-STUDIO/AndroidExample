#ifndef SHA1_H
#define SHA1_H
#include <stdlib.h>
#include <string.h>
#include <asm/types.h>

#define os_memcpy	memcpy
#define os_memset	memset
#define os_memcmp	memcmp
#define os_strlen	strlen

#define MAX_SHA1_LEN	32
#define SHA1_MAC_LEN	20

typedef __u8 u8;
typedef __u32 u32;

struct SHA1Context {
    u32 state[5];            // 5个32位的变量存储当前的哈希状态
    u32 count[2];            // 消息长度，以64位存储（count[0]为低32位，count[1]为高32位）
    unsigned char buffer[64]; // 512位的数据缓冲区
};
typedef struct SHA1Context SHA1_CTX;

void SHA1Init(SHA1_CTX *context);
void SHA1Update(SHA1_CTX *context, const unsigned char *data, u32 len);
void SHA1Final(unsigned char digest[20], SHA1_CTX *context);


void SHA1Init2(SHA1_CTX *context);


void SHA1Init3(SHA1_CTX *context);
void SHA1Update3(SHA1_CTX *context, const unsigned char *data, u32 len);
void SHA1Final3(unsigned char digest[20], SHA1_CTX *context);


#endif /* SHA1_H */