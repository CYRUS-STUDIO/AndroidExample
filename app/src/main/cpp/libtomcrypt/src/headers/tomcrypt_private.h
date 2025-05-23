/* LibTomCrypt, modular cryptographic library -- Tom St Denis */
/* SPDX-License-Identifier: Unlicense */

#include "tomcrypt.h"
#include <stdarg.h>

#ifndef TOMCRYPT_PRIVATE_H_
#define TOMCRYPT_PRIVATE_H_

/*
 * Internal Macros
 */
/* Static assertion */
#define LTC_STATIC_ASSERT(msg, cond) typedef char ltc_static_assert_##msg[(cond) ? 1 : -1];

#define LTC_PAD_MASK       (0xF000U)

/* only real 64bit, not ILP32 */
#if defined(ENDIAN_64BITWORD) && !defined(ENDIAN_64BITWORD_ILP32)
   #define CONSTPTR(n) CONST64(n)
#else
   #define CONSTPTR(n) n ## uL
#endif

LTC_STATIC_ASSERT(correct_CONSTPTR_size, sizeof(CONSTPTR(1)) == sizeof(void*))

/* Poor-man's `uintptr_t` since we can't use stdint.h
 * c.f. https://github.com/DCIT/perl-CryptX/issues/95#issuecomment-1745280962 */
typedef size_t ltc_uintptr;

LTC_STATIC_ASSERT(correct_ltc_uintptr_size, sizeof(ltc_uintptr) == sizeof(void*))

/* Aligns a `unsigned char` buffer `buf` to `n` bytes and returns that aligned address.
 * Make sure that the buffer that is passed is huge enough.
 */
#define LTC_ALIGN_BUF(buf, n) ((void*)((ltc_uintptr)&((unsigned char*)(buf))[n - 1] & (~(CONSTPTR(n) - CONSTPTR(1)))))

#define LTC_OID_MAX_STRLEN 256

/* `NULL` as defined by the standard is not guaranteed to be of a pointer
 * type. In order to make sure that in vararg API's a pointer type is used,
 * define our own version and use that one internally.
 */
#ifndef LTC_NULL
   #define LTC_NULL ((void *)0)
#endif

/*
 * Internal Enums
 */

enum ltc_oid_id {
   LTC_OID_UNDEF,
   LTC_OID_RSA,
   LTC_OID_DSA,
   LTC_OID_EC,
   LTC_OID_EC_PRIMEF,
   LTC_OID_X25519,
   LTC_OID_ED25519,
   LTC_OID_DH,
   LTC_OID_NUM
};

/*
 * Internal Types
 */

typedef struct {
  int size;
  const char *name, *base, *prime;
} ltc_dh_set_type;


struct password {
   /* usually a `char*` but could also contain binary data
    * so use a `void*` + length to be on the safe side.
    */
   void *pw;
   unsigned long l;
};

typedef int (*fn_kdf_t)(const struct password *pwd,
                        const unsigned char *salt,  unsigned long salt_len,
                              int iteration_count,  int hash_idx,
                              unsigned char *out,   unsigned long *outlen);

#if defined(LTC_PBES)
typedef struct {
   /* KDF */
   fn_kdf_t kdf;
   /* Hash or HMAC */
   const char* h;
   /* cipher */
   const char* c;
   unsigned long keylen;
   /* not used for pbkdf2 */
   unsigned long blocklen;
} pbes_properties;

typedef struct
{
   pbes_properties type;
   struct password pw;
   ltc_asn1_list *enc_data;
   ltc_asn1_list *salt;
   ltc_asn1_list *iv;
   unsigned long iterations;
   /* only used for RC2 */
   unsigned long key_bits;
} pbes_arg;
#endif

typedef struct {
   const pbes_properties *data;
   const char *oid;
} oid_to_pbes;

/*
 * Internal functions
 */


/* tomcrypt_cipher.h */

void blowfish_enc(ulong32 *data, unsigned long blocks, const symmetric_key *skey);
int blowfish_expand(const unsigned char *key, int keylen,
                    const unsigned char *data, int datalen,
                    symmetric_key *skey);
int blowfish_setup_with_data(const unsigned char *key, int keylen,
                             const unsigned char *data, int datalen,
                             symmetric_key *skey);

/* tomcrypt_hash.h */

/* a simple macro for making hash "process" functions */
#define HASH_PROCESS(func_name, compress_name, state_var, block_size)                       \
int func_name (hash_state * md, const unsigned char *in, unsigned long inlen)               \
{                                                                                           \
    unsigned long n;                                                                        \
    int           err;                                                                      \
    LTC_ARGCHK(md != NULL);                                                                 \
    LTC_ARGCHK(in != NULL);                                                                 \
    if (md-> state_var .curlen > sizeof(md-> state_var .buf)) {                             \
       return CRYPT_INVALID_ARG;                                                            \
    }                                                                                       \
    if (((md-> state_var .length + inlen * 8) < md-> state_var .length)                     \
          || ((inlen * 8) < inlen)) {                                                       \
      return CRYPT_HASH_OVERFLOW;                                                           \
    }                                                                                       \
    while (inlen > 0) {                                                                     \
        if (md-> state_var .curlen == 0 && inlen >= block_size) {                           \
           if ((err = compress_name (md, in)) != CRYPT_OK) {                                \
              return err;                                                                   \
           }                                                                                \
           md-> state_var .length += block_size * 8;                                        \
           in             += block_size;                                                    \
           inlen          -= block_size;                                                    \
        } else {                                                                            \
           n = MIN(inlen, (block_size - md-> state_var .curlen));                           \
           XMEMCPY(md-> state_var .buf + md-> state_var.curlen, in, (size_t)n);             \
           md-> state_var .curlen += n;                                                     \
           in             += n;                                                             \
           inlen          -= n;                                                             \
           if (md-> state_var .curlen == block_size) {                                      \
              if ((err = compress_name (md, md-> state_var .buf)) != CRYPT_OK) {            \
                 return err;                                                                \
              }                                                                             \
              md-> state_var .length += 8*block_size;                                       \
              md-> state_var .curlen = 0;                                                   \
           }                                                                                \
       }                                                                                    \
    }                                                                                       \
    return CRYPT_OK;                                                                        \
}


/* tomcrypt_mac.h */

int ocb3_int_ntz(unsigned long x);
void ocb3_int_xor_blocks(unsigned char *out, const unsigned char *block_a, const unsigned char *block_b, unsigned long block_len);

int omac_vprocess(omac_state *omac, const unsigned char *in,  unsigned long inlen, va_list args);

/* tomcrypt_math.h */

#if !defined(DESC_DEF_ONLY)

#define LTC_MP_DIGIT_BIT                 ltc_mp.bits_per_digit

/* some handy macros */
#define ltc_mp_init(a)                   ltc_mp.init(a)
#define ltc_mp_clear(a)                  ltc_mp.deinit(a)
#define ltc_mp_init_copy(a, b)           ltc_mp.init_copy(a, b)

#define ltc_mp_neg(a, b)                 ltc_mp.neg(a, b)
#define ltc_mp_copy(a, b)                ltc_mp.copy(a, b)

#define ltc_mp_set(a, b)                 ltc_mp.set_int(a, b)
#define ltc_mp_set_int(a, b)             ltc_mp.set_int(a, b)
#define ltc_mp_get_int(a)                ltc_mp.get_int(a)
#define ltc_mp_get_digit(a, n)           ltc_mp.get_digit(a, n)
#define ltc_mp_get_digit_count(a)        ltc_mp.get_digit_count(a)
#define ltc_mp_cmp(a, b)                 ltc_mp.compare(a, b)
#define ltc_mp_cmp_d(a, b)               ltc_mp.compare_d(a, b)
#define ltc_mp_count_bits(a)             ltc_mp.count_bits(a)
#define ltc_mp_cnt_lsb(a)                ltc_mp.count_lsb_bits(a)
#define ltc_mp_2expt(a, b)               ltc_mp.twoexpt(a, b)

#define ltc_mp_read_radix(a, b, c)       ltc_mp.read_radix(a, b, c)
#define ltc_mp_toradix(a, b, c)          ltc_mp.write_radix(a, b, c)
#define ltc_mp_unsigned_bin_size(a)      ltc_mp.unsigned_size(a)
#define ltc_mp_to_unsigned_bin(a, b)     ltc_mp.unsigned_write(a, b)
#define ltc_mp_read_unsigned_bin(a, b, c) ltc_mp.unsigned_read(a, b, c)

#define ltc_mp_add(a, b, c)              ltc_mp.add(a, b, c)
#define ltc_mp_add_d(a, b, c)            ltc_mp.addi(a, b, c)
#define ltc_mp_sub(a, b, c)              ltc_mp.sub(a, b, c)
#define ltc_mp_sub_d(a, b, c)            ltc_mp.subi(a, b, c)
#define ltc_mp_mul(a, b, c)              ltc_mp.mul(a, b, c)
#define ltc_mp_mul_d(a, b, c)            ltc_mp.muli(a, b, c)
#define ltc_mp_sqr(a, b)                 ltc_mp.sqr(a, b)
#define ltc_mp_sqrtmod_prime(a, b, c)    ltc_mp.sqrtmod_prime(a, b, c)
#define ltc_mp_div(a, b, c, d)           ltc_mp.mpdiv(a, b, c, d)
#define ltc_mp_div_2(a, b)               ltc_mp.div_2(a, b)
#define ltc_mp_mod(a, b, c)              ltc_mp.mpdiv(a, b, NULL, c)
#define ltc_mp_mod_d(a, b, c)            ltc_mp.modi(a, b, c)
#define ltc_mp_gcd(a, b, c)              ltc_mp.gcd(a, b, c)
#define ltc_mp_lcm(a, b, c)              ltc_mp.lcm(a, b, c)

#define ltc_mp_addmod(a, b, c, d)        ltc_mp.addmod(a, b, c, d)
#define ltc_mp_submod(a, b, c, d)        ltc_mp.submod(a, b, c, d)
#define ltc_mp_mulmod(a, b, c, d)        ltc_mp.mulmod(a, b, c, d)
#define ltc_mp_sqrmod(a, b, c)           ltc_mp.sqrmod(a, b, c)
#define ltc_mp_invmod(a, b, c)           ltc_mp.invmod(a, b, c)

#define ltc_mp_montgomery_setup(a, b)    ltc_mp.montgomery_setup(a, b)
#define ltc_mp_montgomery_normalization(a, b) ltc_mp.montgomery_normalization(a, b)
#define ltc_mp_montgomery_reduce(a, b, c)   ltc_mp.montgomery_reduce(a, b, c)
#define ltc_mp_montgomery_free(a)        ltc_mp.montgomery_deinit(a)

#define ltc_mp_exptmod(a,b,c,d)          ltc_mp.exptmod(a,b,c,d)
#define ltc_mp_prime_is_prime(a, b, c)   ltc_mp.isprime(a, b, c)

#define ltc_mp_iszero(a)                 (ltc_mp_cmp_d(a, 0) == LTC_MP_EQ ? LTC_MP_YES : LTC_MP_NO)
#define ltc_mp_isodd(a)                  (ltc_mp_get_digit_count(a) > 0 ? (ltc_mp_get_digit(a, 0) & 1 ? LTC_MP_YES : LTC_MP_NO) : LTC_MP_NO)
#define ltc_mp_exch(a, b)                do { void *ABC__tmp = a; a = b; b = ABC__tmp; } while(0)

#define ltc_mp_tohex(a, b)               ltc_mp_toradix(a, b, 16)

#define ltc_mp_rand(a, b)                ltc_mp.rand(a, b)

#endif


/* tomcrypt_misc.h */

typedef enum {
   /** Use `\r\n` as line separator */
   BASE64_PEM_CRLF = 1,
   /** Create output with 72 chars line length */
   BASE64_PEM_SSH = 2,
} base64_pem_flags;

int base64_encode_pem(const unsigned char *in,  unsigned long inlen,
                                     char *out, unsigned long *outlen,
                            unsigned int  flags);

/* PEM related */

#ifdef LTC_PEM
enum cipher_mode {
   cm_modes =           0x00ff,
   cm_flags =           0xff00,
   /* Flags */
   cm_openssh =         0x0100,
   cm_1bit =            0x0200,
   cm_8bit =            0x0400,
   /* Modes */
   cm_none =            0x0000,
   cm_cbc =             0x0001,
   cm_cfb =             0x0002,
   cm_ctr =             0x0003,
   cm_ofb =             0x0004,
   cm_stream =          0x0005,
   cm_gcm =             0x0006,
   cm_cfb1 =            cm_cfb | cm_1bit,
   cm_cfb8 =            cm_cfb | cm_8bit,
   cm_stream_openssh =  cm_stream | cm_openssh,
};

struct blockcipher_info {
   const char *name;
   const char *algo;
   unsigned long keylen;
   enum cipher_mode mode;
   /* should use `MAXBLOCKSIZE` here, but all supported
    * blockciphers require max 16 bytes IV */
   char iv[16 * 2 + 1];
};

struct str {
   char *p;
   unsigned long len;
};

#define SET_STR(n, s) n.p = s, n.len = XSTRLEN(s)
#define SET_CSTR(n, s) n.p = (char*)s, n.len = (sizeof s) - 1
#define COPY_STR(n, s, l) do { XMEMCPY(n.p, s, l); n.len = l; } while(0)
#define RESET_STR(n) do { n.p = NULL; n.len = 0; } while(0)

enum more_headers {
   no,
   yes,
   maybe
};

enum pem_flags {
   pf_encrypted = 0x01u,
   pf_pkcs8 = 0x02u,
   pf_public = 0x04u,
   pf_x509 = 0x08u,
   pf_encrypted_pkcs8 = pf_encrypted | pf_pkcs8,
};

struct pem_header_id {
   struct str start, end;
   enum more_headers has_more_headers;
   enum pem_flags flags;
   enum ltc_pka_id pka;
   int (*decrypt)(void *, unsigned long *, void *);
};

struct pem_headers {
   const struct pem_header_id *id;
   int encrypted;
   struct blockcipher_info info;
   struct password *pw;
};

struct bufp {
   /* `end` points to one byte after the last
    * element of the allocated buffer
    */
   char *start, *work, *end;
};

#define SET_BUFP(n, d, l) n.start = (char*)d, n.work = (char*)d, n.end = (char*)d + l + 1

struct get_char {
   int (*get)(struct get_char*);
   union {
#ifndef LTC_NO_FILE
      FILE *f;
#endif /* LTC_NO_FILE */
      struct bufp buf;
   } data;
   struct str unget_buf;
   char unget_buf_[LTC_PEM_DECODE_BUFSZ];
};
#endif

/* others */

void copy_or_zeromem(const unsigned char* src, unsigned char* dest, unsigned long len, int coz);
void password_free(struct password *pw, const struct password_ctx *ctx);

#if defined(LTC_PBES)
int pbes_decrypt(const pbes_arg  *arg, unsigned char *dec_data, unsigned long *dec_size);

int pbes1_extract(const ltc_asn1_list *s, pbes_arg *res);
int pbes2_extract(const ltc_asn1_list *s, pbes_arg *res);
#endif

#ifdef LTC_PEM
int pem_decrypt(unsigned char *data, unsigned long *datalen,
                unsigned char *key,  unsigned long keylen,
                unsigned char *iv,   unsigned long ivlen,
                unsigned char *tag,  unsigned long taglen,
                const struct blockcipher_info *info,
                enum padding_type padding);
#ifndef LTC_NO_FILE
int pem_get_char_from_file(struct get_char *g);
#endif /* LTC_NO_FILE */
int pem_get_char_from_buf(struct get_char *g);
int pem_read(void *pem, unsigned long *w, struct pem_headers *hdr, struct get_char *g);
#endif

/* tomcrypt_pk.h */

int rand_bn_bits(void *N, int bits, prng_state *prng, int wprng);
int rand_bn_upto(void *N, void *limit, prng_state *prng, int wprng);

int pk_get_oid(enum ltc_oid_id id, const char **st);
int pk_get_pka_id(enum ltc_oid_id id, enum ltc_pka_id *pka);
int pk_get_oid_id(enum ltc_pka_id pka, enum ltc_oid_id *oid);
#ifdef LTC_DER
int pk_get_oid_from_asn1(const ltc_asn1_list *oid, enum ltc_oid_id *id);
#endif
int pk_oid_str_to_num(const char *OID, unsigned long *oid, unsigned long *oidlen);
int pk_oid_num_to_str(const unsigned long *oid, unsigned long oidlen, char *OID, unsigned long *outlen);

int pk_oid_cmp_with_ulong(const char *o1, const unsigned long *o2, unsigned long o2size);

/* ---- DH Routines ---- */
#ifdef LTC_MRSA
int rsa_init(rsa_key *key);
void rsa_shrink_key(rsa_key *key);
int rsa_make_key_bn_e(prng_state *prng, int wprng, int size, void *e,
                      rsa_key *key); /* used by op-tee */
int rsa_import_pkcs1(const unsigned char *in, unsigned long inlen, rsa_key *key);
int rsa_import_pkcs8_asn1(ltc_asn1_list *alg_id, ltc_asn1_list *priv_key, rsa_key *key);
#endif /* LTC_MRSA */

/* ---- DH Routines ---- */
#ifdef LTC_MDH
extern const ltc_dh_set_type ltc_dh_sets[];

int dh_init(dh_key *key);
int dh_check_pubkey(const dh_key *key);
int dh_import_pkcs8_asn1(ltc_asn1_list *alg_id, ltc_asn1_list *priv_key, dh_key *key);
#endif /* LTC_MDH */

/* ---- ECC Routines ---- */
#ifdef LTC_MECC
int ecc_set_curve_from_mpis(void *a, void *b, void *prime, void *order, void *gx, void *gy, unsigned long cofactor, ecc_key *key);
int ecc_copy_curve(const ecc_key *srckey, ecc_key *key);
int ecc_set_curve_by_size(int size, ecc_key *key);
int ecc_import_subject_public_key_info(const unsigned char *in, unsigned long inlen, ecc_key *key);
#ifdef LTC_DER
int ecc_import_pkcs8_asn1(ltc_asn1_list *alg_id, ltc_asn1_list *priv_key, ecc_key *key);
#endif
int ecc_import_with_curve(const unsigned char *in, unsigned long inlen, int type, ecc_key *key);
int ecc_import_with_oid(const unsigned char *in, unsigned long inlen, unsigned long *oid, unsigned long oid_len, int type, ecc_key *key);

int ecc_sign_hash_internal(const unsigned char *in,  unsigned long inlen,
                           void *r, void *s, prng_state *prng, int wprng,
                           int *recid, const ecc_key *key);

int ecc_verify_hash_internal(void *r, void *s,
                             const unsigned char *hash, unsigned long hashlen,
                             int *stat, const ecc_key *key);

#ifdef LTC_SSH
int ecc_ssh_ecdsa_encode_name(char *buffer, unsigned long *buflen, const ecc_key *key);
#endif

/* low level functions */
ecc_point *ltc_ecc_new_point(void);
void       ltc_ecc_del_point(ecc_point *p);
int        ltc_ecc_set_point_xyz(ltc_mp_digit x, ltc_mp_digit y, ltc_mp_digit z, ecc_point *p);
int        ltc_ecc_copy_point(const ecc_point *src, ecc_point *dst);
int        ltc_ecc_is_point(const ltc_ecc_dp *dp, void *x, void *y);
int        ltc_ecc_is_point_at_infinity(const ecc_point *P, const void *modulus, int *retval);
int        ltc_ecc_import_point(const unsigned char *in, unsigned long inlen, void *prime, void *a, void *b, void *x, void *y);
int        ltc_ecc_export_point(unsigned char *out, unsigned long *outlen, void *x, void *y, unsigned long size, int compressed);
int        ltc_ecc_verify_key(const ecc_key *key);

/* point ops (mp == montgomery digit) */
#if !defined(LTC_MECC_ACCEL) || defined(LTM_DESC) || defined(GMP_DESC)
/* R = 2P */
int ltc_ecc_projective_dbl_point(const ecc_point *P, ecc_point *R,
                                 const void *ma, const void *modulus, void *mp);

/* R = P + Q */
int ltc_ecc_projective_add_point(const ecc_point *P, const ecc_point *Q, ecc_point *R,
                                 const void *ma, const void *modulus, void *mp);
#endif

#if defined(LTC_MECC_FP)
/* optimized point multiplication using fixed point cache (HAC algorithm 14.117) */
int ltc_ecc_fp_mulmod(void *k, ecc_point *G, ecc_point *R, void *a, void *modulus, int map);

/* functions for saving/loading/freeing/adding to fixed point cache */
int ltc_ecc_fp_save_state(unsigned char **out, unsigned long *outlen);
int ltc_ecc_fp_restore_state(unsigned char *in, unsigned long inlen);
void ltc_ecc_fp_free(void);
int ltc_ecc_fp_add_point(ecc_point *g, void *modulus, int lock);

/* lock/unlock all points currently in fixed point cache */
void ltc_ecc_fp_tablelock(int lock);
#endif

/* R = kG */
int ltc_ecc_mulmod(const void *k, const ecc_point *G, ecc_point *R,
                   const void *a, const void *modulus, int map);

#ifdef LTC_ECC_SHAMIR
/* kA*A + kB*B = C */
int ltc_ecc_mul2add(const ecc_point *A, void *kA,
                    const ecc_point *B, void *kB,
                          ecc_point *C,
                         const void *ma,
                         const void *modulus);

#ifdef LTC_MECC_FP
/* Shamir's trick with optimized point multiplication using fixed point cache */
int ltc_ecc_fp_mul2add(const ecc_point *A, void *kA,
                       const ecc_point *B, void *kB,
                             ecc_point *C,
                            const void *ma,
                            const void *modulus);
#endif

#endif


/* map P to affine from projective */
int ltc_ecc_map(ecc_point *P, const void *modulus, void *mp);
#endif /* LTC_MECC */

#ifdef LTC_MDSA
int dsa_int_init(dsa_key *key);
int dsa_int_validate(const dsa_key *key, int *stat);
int dsa_int_validate_xy(const dsa_key *key, int *stat);
int dsa_int_validate_pqg(const dsa_key *key, int *stat);
int dsa_int_validate_primes(const dsa_key *key, int *stat);
int dsa_import_pkcs1(const unsigned char *in, unsigned long inlen, dsa_key *key);
int dsa_import_pkcs8_asn1(ltc_asn1_list *alg_id, ltc_asn1_list *priv_key, dsa_key *key);
#endif /* LTC_MDSA */


#ifdef LTC_CURVE25519

int tweetnacl_crypto_sign(
  unsigned char *sm,unsigned long long *smlen,
  const unsigned char *m,unsigned long long mlen,
  const unsigned char *sk,const unsigned char *pk,
  const unsigned char *ctx,unsigned long long cs);
int tweetnacl_crypto_sign_open(
  int *stat,
  unsigned char *m,unsigned long long *mlen,
  const unsigned char *sm,unsigned long long smlen,
  const unsigned char *ctx, unsigned long long cs,
  const unsigned char *pk);
int tweetnacl_crypto_sign_keypair(prng_state *prng, int wprng, unsigned char *pk,unsigned char *sk);
int tweetnacl_crypto_sk_to_pk(unsigned char *pk, const unsigned char *sk);
int tweetnacl_crypto_scalarmult(unsigned char *q, const unsigned char *n, const unsigned char *p);
int tweetnacl_crypto_scalarmult_base(unsigned char *q,const unsigned char *n);
int tweetnacl_crypto_ph(unsigned char *out, const unsigned char *msg, unsigned long long msglen);

int ed25519_import_pkcs8_asn1(ltc_asn1_list  *alg_id, ltc_asn1_list *priv_key,
                              curve25519_key *key);
int x25519_import_pkcs8_asn1(ltc_asn1_list  *alg_id, ltc_asn1_list *priv_key,
                             curve25519_key *key);

int ec25519_import_pkcs8_asn1(ltc_asn1_list *alg_id, ltc_asn1_list *priv_key,
                              enum ltc_oid_id id,
                              curve25519_key *key);
int ec25519_import_pkcs8(const unsigned char *in, unsigned long inlen,
                         const password_ctx  *pw_ctx,
                         enum ltc_oid_id id,
                         curve25519_key *key);
int ec25519_export(       unsigned char *out, unsigned long *outlen,
                                    int  which,
                   const curve25519_key *key);
int ec25519_crypto_ctx(      unsigned char *out, unsigned long *outlen,
                             unsigned char flag,
                       const unsigned char *ctx, unsigned long  ctxlen);
#endif /* LTC_CURVE25519 */

#ifdef LTC_DER

#define LTC_ASN1_IS_TYPE(e, t) (((e) != NULL) && ((e)->type == (t)))

/* DER handling */
int der_decode_custom_type_ex(const unsigned char *in, unsigned long  inlen,
                           ltc_asn1_list *root,
                           ltc_asn1_list *list,     unsigned long  outlen, unsigned int flags);

int der_encode_asn1_identifier(const ltc_asn1_list *id, unsigned char *out, unsigned long *outlen);
int der_decode_asn1_identifier(const unsigned char *in, unsigned long *inlen, ltc_asn1_list *id);
int der_length_asn1_identifier(const ltc_asn1_list *id, unsigned long *idlen);

int der_encode_asn1_length(unsigned long len, unsigned char* out, unsigned long* outlen);
int der_decode_asn1_length(const unsigned char *in, unsigned long *inlen, unsigned long *outlen);
int der_length_asn1_length(unsigned long len, unsigned long *outlen);

int der_length_sequence_ex(const ltc_asn1_list *list, unsigned long inlen,
                           unsigned long *outlen, unsigned long *payloadlen);

typedef struct {
   ltc_asn1_type t;
   ltc_asn1_list **pp;
} der_flexi_check;

#define LTC_SET_DER_FLEXI_CHECK(list, index, Type, P)    \
   do {                                         \
      int LTC_SDFC_temp##__LINE__ = (index);   \
      list[LTC_SDFC_temp##__LINE__].t = Type;  \
      list[LTC_SDFC_temp##__LINE__].pp = P;    \
   } while (0)


extern const ltc_asn1_type  der_asn1_tag_to_type_map[];
extern const unsigned long  der_asn1_tag_to_type_map_sz;

extern const int der_asn1_type_to_identifier_map[];
extern const unsigned long der_asn1_type_to_identifier_map_sz;

int der_flexi_sequence_cmp(const ltc_asn1_list *flexi, der_flexi_check *check);

int der_decode_sequence_multi_ex(const unsigned char *in, unsigned long inlen, unsigned int flags, ...)
                                 LTC_NULL_TERMINATED;

int der_teletex_char_encode(int c);
int der_teletex_value_decode(int v);

int der_utf8_valid_char(const wchar_t c);

typedef int (*public_key_decode_cb)(const unsigned char *in, unsigned long inlen, void *ctx);

int x509_decode_public_key_from_certificate(const unsigned char *in, unsigned long inlen,
                                            enum ltc_oid_id algorithm, ltc_asn1_type param_type,
                                            ltc_asn1_list* parameters, unsigned long *parameters_len,
                                            public_key_decode_cb callback, void *ctx);
int x509_decode_spki(const unsigned char *in, unsigned long inlen, ltc_asn1_list **out, ltc_asn1_list **spki);

/* SUBJECT PUBLIC KEY INFO */
int x509_encode_subject_public_key_info(unsigned char *out, unsigned long *outlen,
        enum ltc_oid_id algorithm, const void* public_key, unsigned long public_key_len,
        ltc_asn1_type parameters_type, ltc_asn1_list* parameters, unsigned long parameters_len);

int x509_decode_subject_public_key_info(const unsigned char *in, unsigned long inlen,
        enum ltc_oid_id algorithm, void *public_key, unsigned long *public_key_len,
        ltc_asn1_type parameters_type, ltc_asn1_list* parameters, unsigned long *parameters_len);

int pk_oid_cmp_with_asn1(const char *o1, const ltc_asn1_list *o2);

#endif /* LTC_DER */

/* tomcrypt_pkcs.h */

#ifdef LTC_PKCS_8

/* Public-Key Cryptography Standards (PKCS) #8:
 * Private-Key Information Syntax Specification Version 1.2
 * https://tools.ietf.org/html/rfc5208
 *
 * PrivateKeyInfo ::= SEQUENCE {
 *      version                   Version,
 *      privateKeyAlgorithm       PrivateKeyAlgorithmIdentifier,
 *      privateKey                PrivateKey,
 *      attributes           [0]  IMPLICIT Attributes OPTIONAL }
 * where:
 * - Version ::= INTEGER
 * - PrivateKeyAlgorithmIdentifier ::= AlgorithmIdentifier
 * - PrivateKey ::= OCTET STRING
 * - Attributes ::= SET OF Attribute
 *
 * EncryptedPrivateKeyInfo ::= SEQUENCE {
 *        encryptionAlgorithm  EncryptionAlgorithmIdentifier,
 *        encryptedData        EncryptedData }
 * where:
 * - EncryptionAlgorithmIdentifier ::= AlgorithmIdentifier
 * - EncryptedData ::= OCTET STRING
 */

int pkcs8_decode_flexi(const unsigned char  *in,  unsigned long inlen,
                       const password_ctx   *pw_ctx,
                             ltc_asn1_list **decoded_list);

int pkcs8_get_children(const ltc_asn1_list *decoded_list, enum ltc_oid_id *pka,
                        ltc_asn1_list **alg_id, ltc_asn1_list **priv_key);

#endif  /* LTC_PKCS_8 */


#ifdef LTC_PKCS_12

int pkcs12_utf8_to_utf16(const unsigned char *in,  unsigned long  inlen,
                               unsigned char *out, unsigned long *outlen);

int pkcs12_kdf(               int   hash_id,
               const unsigned char *pw,         unsigned long pwlen,
               const unsigned char *salt,       unsigned long saltlen,
                     unsigned int   iterations, unsigned char purpose,
                     unsigned char *out,        unsigned long outlen);

#endif  /* LTC_PKCS_12 */

/* tomcrypt_prng.h */

#define LTC_PRNG_EXPORT(which) \
int which ## _export(unsigned char *out, unsigned long *outlen, prng_state *prng)      \
{                                                                                      \
   unsigned long len = which ## _desc.export_size;                                     \
                                                                                       \
   LTC_ARGCHK(prng   != NULL);                                                         \
   LTC_ARGCHK(out    != NULL);                                                         \
   LTC_ARGCHK(outlen != NULL);                                                         \
                                                                                       \
   if (*outlen < len) {                                                                \
      *outlen = len;                                                                   \
      return CRYPT_BUFFER_OVERFLOW;                                                    \
   }                                                                                   \
                                                                                       \
   if (which ## _read(out, len, prng) != len) {                                        \
      return CRYPT_ERROR_READPRNG;                                                     \
   }                                                                                   \
                                                                                       \
   *outlen = len;                                                                      \
   return CRYPT_OK;                                                                    \
}

/* extract a byte portably */
#ifdef _MSC_VER
   #define LTC_BYTE(x, n) ((unsigned char)((x) >> (8 * (n))))
#else
   #define LTC_BYTE(x, n) (((x) >> (8 * (n))) & 255)
#endif

/*
 * On Windows, choose whether to use CryptGenRandom() [older Windows versions]
 * or BCryptGenRandom() [newer Windows versions].
 * If CryptGenRandom() is desired, define LTC_NO_WIN32_BCRYPT when building.
 */
#if defined(_MSC_VER) && defined(_WIN32_WINNT) && _WIN32_WINNT >= 0x0600
   #if !defined(LTC_NO_WIN32_BCRYPT)
      #define LTC_WIN32_BCRYPT
   #endif
#endif

#endif /* TOMCRYPT_PRIVATE_H_ */
