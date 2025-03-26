#ifndef SHA256_H_
#define SHA256_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct HASH_VTAB {
    void (*const init)(struct HASH_CTX *);

    void (*const update)(struct HASH_CTX *, const void *, int);

    const uint8_t *(*const final)(struct HASH_CTX *);

    const uint8_t *(*const hash)(const void *, int, uint8_t *);

    int size;
} HASH_VTAB;

typedef struct HASH_CTX {
    const HASH_VTAB *f;
    uint64_t count;
    uint8_t buf[64];
    uint32_t state[8];  // upto SHA2
} HASH_CTX;


typedef HASH_CTX SHA256_CTX;
void SHA256_init(SHA256_CTX *ctx);
void SHA256_update(SHA256_CTX *ctx, const void *data, int len);
const uint8_t *SHA256_final(SHA256_CTX *ctx);
// Convenience method. Returns digest address.
const uint8_t *SHA256_hash(const void *data, int len, uint8_t *digest);
#define SHA256_DIGEST_SIZE 32
#ifdef __cplusplus
}
#endif // __cplusplus
#endif  // SHA256_H_