/* LibTomCrypt, modular cryptographic library -- Tom St Denis */
/* SPDX-License-Identifier: Unlicense */
#include "tomcrypt_private.h"

/**
  @file siv.c
  RFC 5297  SIV - Synthetic Initialization Vector, Steffen Jaeckel
*/

#ifdef LTC_SIV_MODE

/* RFC 5297 - Chapter 7 - Security Considerations
 *
 * [...] S2V must not be
 * passed more than 127 components.  Since SIV includes the plaintext as
 * a component to S2V, that limits the number of components of
 * associated data that can be safely passed to SIV to 126.
 */
static const unsigned long s_siv_max_aad_components = 126;

static LTC_INLINE void s_siv_dbl(unsigned char *inout)
{
   int y, mask, msb, len;

   /* setup the system */
   mask = 0x87;
   len = 16;

   /* if msb(L * u^(x+1)) = 0 then just shift, otherwise shift and xor constant mask */
   msb = inout[0] >> 7;

   /* shift left */
   for (y = 0; y < (len - 1); y++) {
      inout[y] = ((inout[y] << 1) | (inout[y + 1] >> 7)) & 255;
   }
   inout[len - 1] = ((inout[len - 1] << 1) ^ (msb ? mask : 0)) & 255;
}

static LTC_INLINE int s_siv_S2V_one(int cipher,
                    const unsigned char *key,    unsigned long keylen,
                          unsigned char *V,      unsigned long *Vlen)
{
   /* if n = 0 then
    *   return V = AES-CMAC(K, <one>)
    */
   unsigned char zero_or_one[16] = {0};
   zero_or_one[0] = 1;
   return omac_memory(cipher, key, keylen, zero_or_one, sizeof(zero_or_one), V, Vlen);
}

typedef struct siv_omac_ctx_t {
   omac_state omac;
   int cipher;
} siv_omac_ctx_t;

static LTC_INLINE int s_siv_ctx_init(int cipher,
                     const unsigned char *key,    unsigned long keylen,
                          siv_omac_ctx_t *ctx)
{
   ctx->cipher = cipher;
   return omac_init(&ctx->omac, cipher, key, keylen);
}

static LTC_INLINE int s_siv_omac_memory(siv_omac_ctx_t *ctx,
                                   const unsigned char *in,  unsigned long inlen,
                                         unsigned char *out, unsigned long *outlen)
{
   int err;
   omac_state omac = ctx->omac;
   if ((err = omac_process(&omac, in, inlen)) != CRYPT_OK) {
      return err;
   }
   err = omac_done(&omac, out, outlen);
   zeromem(&omac, sizeof(omac));
   return err;
}

static LTC_INLINE int s_siv_S2V_zero(siv_omac_ctx_t *ctx,
                                      unsigned char *D,      unsigned long *Dlen)
{
   /* D = AES-CMAC(K, <zero>) */
   const unsigned char zero_or_one[16] = {0};
   return s_siv_omac_memory(ctx, zero_or_one, sizeof(zero_or_one), D, Dlen);
}

static LTC_INLINE int s_siv_S2V_dbl_xor_cmac(siv_omac_ctx_t *ctx,
                                        const unsigned char *aad, unsigned long aadlen,
                                              unsigned char *D,   unsigned long Dlen)
{
   /* for i = 1 to n-1 do
    *   D = dbl(D) xor AES-CMAC(K, Si)
    * done
    */
   int err;
   unsigned char TMP[16];
   unsigned long i, TMPlen = sizeof(TMP);
   s_siv_dbl(D);
   if ((err = s_siv_omac_memory(ctx, aad, aadlen, TMP, &TMPlen)) != CRYPT_OK) {
      return err;
   }
   for (i = 0; i < Dlen; ++i) {
      D[i] ^= TMP[i];
   }
   return err;
}

static LTC_INLINE int s_siv_omac_memory_multi(siv_omac_ctx_t *ctx,
                                               unsigned char *out, unsigned long *outlen,
                                         const unsigned char *in,  unsigned long inlen,
                                                              ...)
{
   int err;
   va_list args;
   omac_state omac = ctx->omac;
   va_start(args, inlen);

   if ((err = omac_vprocess(&omac, in, inlen, args)) != CRYPT_OK) {
      return err;
   }
   err = omac_done(&omac, out, outlen);
   zeromem(&omac, sizeof(omac));
   return err;
}

static LTC_INLINE int s_siv_S2V_T(siv_omac_ctx_t *ctx,
                             const unsigned char *in,     unsigned long inlen,
                                   unsigned char *D,
                                   unsigned char *V,      unsigned long *Vlen)
{
   int err;
   unsigned long i;
   unsigned char T[16];

   /* if len(Sn) >= 128 then
    *   T = Sn xorend D
    * else
    *   T = dbl(D) xor pad(Sn)
    * fi
    */
   if (inlen >= 16) {
      XMEMCPY(T, &in[inlen - 16], 16);
      for(i = 0; i < 16; ++i) {
         T[i] ^= D[i];
      }
      err = s_siv_omac_memory_multi(ctx, V, Vlen, in, inlen - 16, T, 16uL, NULL);
   } else {
      s_siv_dbl(D);
      XMEMCPY(T, in, inlen);
      T[inlen] = 0x80;
      for (i = inlen + 1; i < 16; ++i) {
         T[i] = 0x0;
      }
      for(i = 0; i < 16; ++i) {
         T[i] ^= D[i];
      }

      err = s_siv_omac_memory(ctx, T, 16, V, Vlen);
   }
   return err;
}

static int s_siv_S2V(int cipher,
    const unsigned char *key,    unsigned long keylen,
    const unsigned char **ad,    unsigned long *adlen,
    const unsigned char *in,     unsigned long inlen,
          unsigned char *V,      unsigned long *Vlen)
{
   int err;
   unsigned char D[16];
   unsigned long Dlen = sizeof(D), n = 0;
   siv_omac_ctx_t ctx;

   if(ad == NULL || adlen == NULL || ad[0] == NULL || adlen[0] == 0) {
      err = s_siv_S2V_one(cipher, key, keylen, V, Vlen);
   } else {
      if ((err = s_siv_ctx_init(cipher, key, keylen, &ctx)) != CRYPT_OK) {
         return err;
      }
      Dlen = sizeof(D);
      if ((err = s_siv_S2V_zero(&ctx, D, &Dlen)) != CRYPT_OK) {
         return err;
      }

      while(ad[n] != NULL && adlen[n] != 0) {
         if (n >= s_siv_max_aad_components) {
            return CRYPT_INPUT_TOO_LONG;
         }
         if ((err = s_siv_S2V_dbl_xor_cmac(&ctx, ad[n], adlen[n], D, Dlen)) != CRYPT_OK) {
            return err;
         }
         n++;
      }

      err = s_siv_S2V_T(&ctx, in, inlen, D, V, Vlen);
   }

   return err;
}

static LTC_INLINE void s_siv_bitand(const unsigned char* V, unsigned char* Q)
{
   int n;
   XMEMSET(Q, 0xff, 16);
   Q[8] = Q[12] = 0x7f;
   for (n = 0; n < 16; ++n) {
      Q[n] &= V[n];
   }
}

static LTC_INLINE int s_ctr_crypt_memory(int   cipher,
                          const unsigned char *IV,
                          const unsigned char *key,           int keylen,
                          const unsigned char *in,
                                unsigned char *out, unsigned long len)
{
   int err;
   symmetric_CTR ctr;

   if ((err = ctr_start(cipher, IV, key, keylen, 0, CTR_COUNTER_BIG_ENDIAN | 16, &ctr)) != CRYPT_OK) {
      goto out;
   }
   if ((err = ctr_encrypt(in, out, len, &ctr)) != CRYPT_OK) {
      goto out;
   }
   if ((err = ctr_done(&ctr)) != CRYPT_OK) {
      goto out;
   }

out:
   zeromem(&ctr, sizeof(ctr));
   return err;
}

typedef struct {
   unsigned char Q[16], V[16];
} siv_state;

/**
   SIV encrypt

   @param cipher     The index of the cipher desired
   @param key        The secret key to use
   @param keylen     The length of the secret key (octets)
   @param ad         An array of Associated Data pointers (must be NULL terminated)
   @param adlen      An array with the lengths of the Associated Data
   @param pt         The plaintext
   @param ptlen      The length of the plaintext
   @param ct         The ciphertext
   @param ctlen      [in/out] The length of the ciphertext
   @return CRYPT_OK if successful
*/
int siv_encrypt_memory(                int  cipher,
                       const unsigned char *key,    unsigned long  keylen,
                       const unsigned char *ad[],   unsigned long  adlen[],
                       const unsigned char *pt,     unsigned long  ptlen,
                             unsigned char *ct,     unsigned long *ctlen)
{
   int err;
   const unsigned char *K1, *K2;
   unsigned long Vlen;
   siv_state siv;

   LTC_ARGCHK(key    != NULL);
   LTC_ARGCHK(ad     != NULL);
   LTC_ARGCHK(adlen  != NULL);
   LTC_ARGCHK(pt     != NULL);
   LTC_ARGCHK(ct     != NULL);
   LTC_ARGCHK(ctlen  != NULL);

   if (ptlen + 16 < ptlen) {
      return CRYPT_OVERFLOW;
   }
   if (*ctlen < ptlen + 16) {
      *ctlen = ptlen + 16;
      return CRYPT_BUFFER_OVERFLOW;
   }
   if ((err = cipher_is_valid(cipher)) != CRYPT_OK) {
      return err;
   }


   K1 = key;
   K2 = &key[keylen/2];

   Vlen = sizeof(siv.V);
   err = s_siv_S2V(cipher, K1, keylen/2, ad, adlen, pt, ptlen, siv.V, &Vlen);
#ifdef LTC_CLEAN_STACK
   burn_stack(3 * 16 + 7 * sizeof(unsigned long) + 1 * sizeof(void*));
#endif
   if (err != CRYPT_OK) {
      return err;
   }

   s_siv_bitand(siv.V, siv.Q);
   XMEMCPY(ct, siv.V, 16);
   ct += 16;

   if ((err = s_ctr_crypt_memory(cipher, siv.Q, K2, keylen/2, pt, ct, ptlen)) != CRYPT_OK) {
      zeromem(ct, ptlen + 16);
      goto out;
   }
   *ctlen = ptlen + 16;

out:
#ifdef LTC_CLEAN_STACK
   zeromem(&siv, sizeof(siv));
#endif

   return err;
}

/**
   SIV decrypt

   @param cipher     The index of the cipher desired
   @param key        The secret key to use
   @param keylen     The length of the secret key (octets)
   @param ad         An array of Associated Data pointers (must be NULL terminated)
   @param adlen      An array with the lengths of the Associated Data
   @param ct         The ciphertext
   @param ctlen      The length of the ciphertext
   @param pt         The plaintext
   @param ptlen      [in/out] The length of the plaintext
   @return CRYPT_OK if successful
*/
int siv_decrypt_memory(                int  cipher,
                       const unsigned char *key,    unsigned long  keylen,
                       const unsigned char *ad[],   unsigned long  adlen[],
                       const unsigned char *ct,     unsigned long  ctlen,
                             unsigned char *pt,     unsigned long *ptlen)
{
   int err;
   unsigned char *pt_work;
   const unsigned char *K1, *K2, *ct_work;
   unsigned long Vlen;
   siv_state siv;

   LTC_ARGCHK(key    != NULL);
   LTC_ARGCHK(ad     != NULL);
   LTC_ARGCHK(adlen  != NULL);
   LTC_ARGCHK(ct     != NULL);
   LTC_ARGCHK(pt     != NULL);
   LTC_ARGCHK(ptlen  != NULL);

   if (ctlen < 16) {
      return CRYPT_INVALID_ARG;
   }
   if (*ptlen < (ctlen - 16)) {
      *ptlen = ctlen - 16;
      return CRYPT_BUFFER_OVERFLOW;
   }

   if ((err = cipher_is_valid(cipher)) != CRYPT_OK) {
      return err;
   }
   *ptlen = ctlen - 16;
   pt_work = XMALLOC(*ptlen);
   if (pt_work == NULL) {
      return CRYPT_MEM;
   }

   K1 = key;
   K2 = &key[keylen/2];

   ct_work = ct;
   s_siv_bitand(ct_work, siv.Q);
   ct_work += 16;

   if ((err = s_ctr_crypt_memory(cipher, siv.Q, K2, keylen/2, ct_work, pt_work, *ptlen)) != CRYPT_OK) {
      goto out;
   }
   Vlen = sizeof(siv.V);
   if ((err = s_siv_S2V(cipher, K1, keylen/2, ad, adlen, pt_work, *ptlen, siv.V, &Vlen)) != CRYPT_OK) {
      goto out;
   }

   err = XMEM_NEQ(siv.V, ct, Vlen);
   copy_or_zeromem(pt_work, pt, *ptlen, err);
out:
#ifdef LTC_CLEAN_STACK
   zeromem(&siv, sizeof(siv));
#endif
   zeromem(pt_work, *ptlen);
   XFREE(pt_work);

   return err;
}

/**
  Process an entire SIV packet in one call.

  @param cipher            The index of the cipher desired
  @param direction         Encrypt or Decrypt mode (LTC_ENCRYPT or LTC_DECRYPT)
  @param key               The secret key to use
  @param keylen            The length of the secret key (octets)
  @param in                The input
  @param inlen             The length of the input
  @param out               The output
  @param outlen            [in/out]  The max size and resulting size of the output
  @remark <...> is of the form <pointer, length> (void*, unsigned long) and contains the Associated Data
  @return CRYPT_OK on success
 */
int siv_memory(                int  cipher,           int  direction,
               const unsigned char *key,    unsigned long  keylen,
               const unsigned char *in,     unsigned long  inlen,
                     unsigned char *out,    unsigned long *outlen,
                                   ...)
{
   int err;
   va_list args;
   siv_state siv;
   unsigned char D[16], *in_buf = NULL, *out_work;
   const unsigned char *aad, *K1, *K2, *in_work;
   unsigned long n = 0, aadlen, Dlen = sizeof(D), Vlen = sizeof(siv.V), in_work_len;

   LTC_ARGCHK(key    != NULL);
   LTC_ARGCHK(in     != NULL);
   LTC_ARGCHK(out    != NULL);
   LTC_ARGCHK(outlen != NULL);

   if ((err = cipher_is_valid(cipher)) != CRYPT_OK) {
      return err;
   }
   if (direction == LTC_ENCRYPT && *outlen < inlen + 16) {
      *outlen = inlen + 16;
      return CRYPT_BUFFER_OVERFLOW;
   } else if (direction == LTC_DECRYPT && (inlen < 16 || *outlen < inlen - 16)) {
      *outlen = inlen - 16;
      return CRYPT_BUFFER_OVERFLOW;
   }

   K1 = key;
   K2 = &key[keylen/2];
   in_work = in;
   in_work_len = inlen;
   out_work = out;

   if (direction == LTC_DECRYPT) {
      in_work_len -= 16;
      in_buf = XMALLOC(in_work_len);
      if (in_buf == NULL)
         return CRYPT_MEM;
      s_siv_bitand(in_work, siv.Q);
      in_work += 16;

      if ((err = s_ctr_crypt_memory(cipher, siv.Q, K2, keylen/2, in_work, in_buf, in_work_len)) != CRYPT_OK) {
         goto err_out;
      }
      in_work = in_buf;
   }

   va_start(args, outlen);
   aad = va_arg(args, const unsigned char*);
   aadlen = aad ? va_arg(args, unsigned long) : 0;
   if (aad == NULL || aadlen == 0) {
      if ((err = s_siv_S2V_one(cipher, K1, keylen/2, siv.V, &Vlen)) != CRYPT_OK) {
         goto err_out;
      }
   } else {
      siv_omac_ctx_t ctx;
      if ((err = s_siv_ctx_init(cipher, K1, keylen/2, &ctx)) != CRYPT_OK) {
         goto err_out;
      }
      if ((err = s_siv_S2V_zero(&ctx, D, &Dlen)) != CRYPT_OK) {
         goto err_out;
      }

      do {
         if (n >= s_siv_max_aad_components) {
            err = CRYPT_INPUT_TOO_LONG;
            goto err_out;
         }
         if ((err = s_siv_S2V_dbl_xor_cmac(&ctx, aad, aadlen, D, Dlen)) != CRYPT_OK) {
            goto err_out;
         }
         aad = va_arg(args, const unsigned char*);
         if (aad == NULL)
            break;
         aadlen = va_arg(args, unsigned long);
         n++;
      } while (aadlen);

      if ((err = s_siv_S2V_T(&ctx, in_work, in_work_len, D, siv.V, &Vlen)) != CRYPT_OK) {
         goto err_out;
      }
   }

   if (direction == LTC_DECRYPT) {
      err = XMEM_NEQ(siv.V, in, Vlen);
      copy_or_zeromem(in_work, out, in_work_len, err);
      *outlen = in_work_len;
   } else {
      s_siv_bitand(siv.V, siv.Q);
      XMEMCPY(out_work, siv.V, 16);
      out_work += 16;

      if ((err = s_ctr_crypt_memory(cipher, siv.Q, K2, keylen/2, in, out_work, inlen)) != CRYPT_OK) {
         zeromem(out, inlen + 16);
         goto err_out;
      }
      *outlen = inlen + 16;
   }
err_out:
   if (in_buf) {
      zeromem(in_buf, in_work_len);
      XFREE(in_buf);
   }
   va_end(args);
#ifdef LTC_CLEAN_STACK
   zeromem(D, sizeof(D));
   zeromem(&siv, sizeof(siv));
#endif
   return err;
}

int siv_test(void)
{
#ifndef LTC_TEST
   return CRYPT_NOP;
#else
   /*
    * RFC5297 - A.1.  Deterministic Authenticated Encryption Example
    */
   const unsigned char Key_A1[] =
      { 0xff, 0xfe, 0xfd, 0xfc, 0xfb, 0xfa, 0xf9, 0xf8,
        0xf7, 0xf6, 0xf5, 0xf4, 0xf3, 0xf2, 0xf1, 0xf0,
        0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7,
        0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff };
   const unsigned char AD_A1[] =
      { 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
        0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
        0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27 };
   const unsigned char Plaintext_A1[] =
      { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
        0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee };
   const unsigned char output_A1[] =
      { 0x85, 0x63, 0x2d, 0x07, 0xc6, 0xe8, 0xf3, 0x7f,
        0x95, 0x0a, 0xcd, 0x32, 0x0a, 0x2e, 0xcc, 0x93,
        0x40, 0xc0, 0x2b, 0x96, 0x90, 0xc4, 0xdc, 0x04,
        0xda, 0xef, 0x7f, 0x6a, 0xfe, 0x5c };
   const unsigned char *ad_A1[] =
      { AD_A1, NULL };
   unsigned long adlen_A1[] =
      { sizeof(AD_A1), 0 };

   const unsigned char Key_A2[] =
      { 0x7f, 0x7e, 0x7d, 0x7c, 0x7b, 0x7a, 0x79, 0x78,
        0x77, 0x76, 0x75, 0x74, 0x73, 0x72, 0x71, 0x70,
        0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
        0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f };
   const unsigned char AD1_A2[] =
      { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
        0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff,
        0xde, 0xad, 0xda, 0xda, 0xde, 0xad, 0xda, 0xda,
        0xff, 0xee, 0xdd, 0xcc, 0xbb, 0xaa, 0x99, 0x88,
        0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x00 };
   const unsigned char AD2_A2[] =
      { 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80,
        0x90, 0xa0 };
   const unsigned char AD3_A2[] =
      { 0x09, 0xf9, 0x11, 0x02, 0x9d, 0x74, 0xe3, 0x5b,
        0xd8, 0x41, 0x56, 0xc5, 0x63, 0x56, 0x88, 0xc0 };
   const unsigned char Plaintext_A2[] =
      { 0x74, 0x68, 0x69, 0x73, 0x20, 0x69, 0x73, 0x20,
        0x73, 0x6f, 0x6d, 0x65, 0x20, 0x70, 0x6c, 0x61,
        0x69, 0x6e, 0x74, 0x65, 0x78, 0x74, 0x20, 0x74,
        0x6f, 0x20, 0x65, 0x6e, 0x63, 0x72, 0x79, 0x70,
        0x74, 0x20, 0x75, 0x73, 0x69, 0x6e, 0x67, 0x20,
        0x53, 0x49, 0x56, 0x2d, 0x41, 0x45, 0x53 };
   const unsigned char output_A2[] =
      { 0x7b, 0xdb, 0x6e, 0x3b, 0x43, 0x26, 0x67, 0xeb,
        0x06, 0xf4, 0xd1, 0x4b, 0xff, 0x2f, 0xbd, 0x0f,
        0xcb, 0x90, 0x0f, 0x2f, 0xdd, 0xbe, 0x40, 0x43,
        0x26, 0x60, 0x19, 0x65, 0xc8, 0x89, 0xbf, 0x17,
        0xdb, 0xa7, 0x7c, 0xeb, 0x09, 0x4f, 0xa6, 0x63,
        0xb7, 0xa3, 0xf7, 0x48, 0xba, 0x8a, 0xf8, 0x29,
        0xea, 0x64, 0xad, 0x54, 0x4a, 0x27, 0x2e, 0x9c,
        0x48, 0x5b, 0x62, 0xa3, 0xfd, 0x5c, 0x0d };
   const unsigned char *ad_A2[] =
      { AD1_A2, AD2_A2, AD3_A2, NULL };
   unsigned long adlen_A2[] =
      { sizeof(AD1_A2), sizeof(AD2_A2), sizeof(AD3_A2), 0 };

#define PL_PAIR(n) n, sizeof(n)
   struct {
      const unsigned char* Key;
            unsigned long  Keylen;
      const unsigned char* Plaintext;
            unsigned long  Plaintextlen;
      const          void* ADs;
                     void* ADlens;
      const unsigned char* output;
            unsigned long  outputlen;
      const          char* name;
   } siv_tests[] = {
     { PL_PAIR(Key_A1), PL_PAIR(Plaintext_A1), &ad_A1, &adlen_A1, PL_PAIR(output_A1), "RFC5297 - A.1.  Deterministic Authenticated Encryption Example" },
     { PL_PAIR(Key_A2), PL_PAIR(Plaintext_A2), &ad_A2, &adlen_A2, PL_PAIR(output_A2), "RFC5297 - A.2.  Nonce-Based Authenticated Encryption Example" }
   };
#undef PL_PAIR

   int err, cipher;
   unsigned n;
   unsigned long buflen, tmplen;
   unsigned char buf[MAX(sizeof(output_A1), sizeof(output_A2))];
   const unsigned long niter = 1000;
   unsigned char *tmpe, *tmpd;
   const unsigned long tmpmax = 16 + niter * 16;

   cipher = find_cipher("aes");

   for (n = 0; n < sizeof(siv_tests)/sizeof(siv_tests[0]); ++n) {
      buflen = sizeof(buf);
      if ((err = siv_encrypt_memory(cipher,
                             siv_tests[n].Key, siv_tests[n].Keylen,
                             (const unsigned char **)siv_tests[n].ADs, siv_tests[n].ADlens,
                             siv_tests[n].Plaintext, siv_tests[n].Plaintextlen,
                             buf, &buflen)) != CRYPT_OK) {
         return err;
      }
      if (compare_testvector(buf, buflen, siv_tests[n].output, siv_tests[n].outputlen, siv_tests[n].name, n) != 0) {
         return CRYPT_FAIL_TESTVECTOR;
      }
      buflen = sizeof(buf);
      if ((err = siv_decrypt_memory(cipher,
                             siv_tests[n].Key, siv_tests[n].Keylen,
                             (const unsigned char **)siv_tests[n].ADs, siv_tests[n].ADlens,
                             siv_tests[n].output, siv_tests[n].outputlen,
                             buf, &buflen)) != CRYPT_OK) {
         return err;
      }
      if (compare_testvector(buf, buflen, siv_tests[n].Plaintext, siv_tests[n].Plaintextlen, siv_tests[n].name, n + 0x1000) != 0) {
         return CRYPT_FAIL_TESTVECTOR;
      }
   }

   /* Testcase 0x2 */
   buflen = sizeof(buf);
   if ((err = siv_memory(cipher, LTC_ENCRYPT,
                         siv_tests[0].Key, siv_tests[0].Keylen,
                         siv_tests[0].Plaintext, siv_tests[0].Plaintextlen,
                         buf, &buflen,
                         AD_A1, sizeof(AD_A1),
                         NULL)) != CRYPT_OK) {
      return err;
   }
   if (compare_testvector(buf, buflen, siv_tests[0].output, siv_tests[0].outputlen, siv_tests[0].name, n) != 0) {
      return CRYPT_FAIL_TESTVECTOR;
   }
   /* Testcase 0x1002 */
   buflen = sizeof(buf);
   if ((err = siv_memory(cipher, LTC_DECRYPT,
                         siv_tests[0].Key, siv_tests[0].Keylen,
                         siv_tests[0].output, siv_tests[0].outputlen,
                         buf, &buflen,
                         AD_A1, sizeof(AD_A1),
                         NULL)) != CRYPT_OK) {
      return err;
   }
   if (compare_testvector(buf, buflen, siv_tests[0].Plaintext, siv_tests[0].Plaintextlen, siv_tests[0].name, n + 0x1000) != 0) {
      return CRYPT_FAIL_TESTVECTOR;
   }

   n++;

   /* Testcase 0x3 */
   buflen = sizeof(buf);
   if ((err = siv_memory(cipher, LTC_ENCRYPT,
                         siv_tests[1].Key, siv_tests[1].Keylen,
                         siv_tests[1].Plaintext, siv_tests[1].Plaintextlen,
                         buf, &buflen,
                         ad_A2[0], adlen_A2[0],
                         ad_A2[1], adlen_A2[1],
                         ad_A2[2], adlen_A2[2],
                         NULL)) != CRYPT_OK) {
      return err;
   }
   if (compare_testvector(buf, buflen, siv_tests[1].output, siv_tests[1].outputlen, siv_tests[1].name, n) != 0) {
      return CRYPT_FAIL_TESTVECTOR;
   }
   /* Testcase 0x1003 */
   buflen = sizeof(buf);
   if ((err = siv_memory(cipher, LTC_DECRYPT,
                         siv_tests[1].Key, siv_tests[1].Keylen,
                         siv_tests[1].output, siv_tests[1].outputlen,
                         buf, &buflen,
                         ad_A2[0], adlen_A2[0],
                         ad_A2[1], adlen_A2[1],
                         ad_A2[2], adlen_A2[2],
                         NULL)) != CRYPT_OK) {
      return err;
   }
   if (compare_testvector(buf, buflen, siv_tests[1].Plaintext, siv_tests[1].Plaintextlen, siv_tests[1].name, n + 0x1000) != 0) {
      return CRYPT_FAIL_TESTVECTOR;
   }

   tmpe = XCALLOC(1, tmpmax);
   if (tmpe == NULL) {
      return CRYPT_MEM;
   }
   tmpd = XCALLOC(1, tmpmax);
   if (tmpd == NULL) {
      err = CRYPT_MEM;
      goto out_tmpd;
   }
   tmplen = 16;
   for (n = 0; n < niter; ++n) {
      buflen = tmpmax;
      if ((err = siv_memory(cipher, LTC_ENCRYPT,
                            siv_tests[0].Key, siv_tests[0].Keylen,
                            tmpe, tmplen,
                            tmpe, &buflen,
                            NULL)) != CRYPT_OK) {
         goto out;
      }
      tmplen = buflen;
   }
   if (compare_testvector(&buflen, sizeof(buflen), &tmpmax, sizeof(tmpmax), "Multiple encrypt length", -(int)niter)) {
      err = CRYPT_FAIL_TESTVECTOR;
      goto out;
   }
   XMEMCPY(tmpd, tmpe, buflen);
   for (n = 0; n < niter; ++n) {
      buflen = tmpmax;
      if ((err = siv_memory(cipher, LTC_DECRYPT,
                            siv_tests[0].Key, siv_tests[0].Keylen,
                            tmpd, tmplen,
                            tmpd, &buflen,
                            NULL)) != CRYPT_OK) {
         goto out;
      }
      tmplen = buflen;
   }
   if (compare_testvector(tmpd, tmplen, tmpe, tmplen, "Multi decrypt", niter + 0x2000)) {
      err = CRYPT_FAIL_TESTVECTOR;
   }

out:
   XFREE(tmpd);
out_tmpd:
   XFREE(tmpe);

   return err;
#endif
}

#endif
