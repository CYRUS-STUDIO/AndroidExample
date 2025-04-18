/* LibTomCrypt, modular cryptographic library -- Tom St Denis */
/* SPDX-License-Identifier: Unlicense */

#include "tomcrypt_private.h"

#ifdef LTC_MECC

int ltc_ecc_import_point(const unsigned char *in, unsigned long inlen, void *prime, void *a, void *b, void *x, void *y)
{
   int err;
   unsigned long size;
   void *t1, *t2;

   /* init key + temporary numbers */
   if (ltc_mp_init_multi(&t1, &t2, LTC_NULL) != CRYPT_OK) {
      return CRYPT_MEM;
   }

   size = ltc_mp_unsigned_bin_size(prime);

   if (in[0] == 0x04 && (inlen&1) && ((inlen-1)>>1) == size) {
      /* read uncompressed point */
      /* load x */
      if ((err = ltc_mp_read_unsigned_bin(x, in+1, size)) != CRYPT_OK)                       { goto cleanup; }
      /* load y */
      if ((err = ltc_mp_read_unsigned_bin(y, in+1+size, size)) != CRYPT_OK)                  { goto cleanup; }
   }
   else if ((in[0] == 0x02 || in[0] == 0x03) && (inlen-1) == size && ltc_mp.sqrtmod_prime != NULL) {
      /* read compressed point - BEWARE: requires sqrtmod_prime */
      /* load x */
      if ((err = ltc_mp_read_unsigned_bin(x, in+1, size)) != CRYPT_OK)                       { goto cleanup; }
      /* compute x^3 */
      if ((err = ltc_mp_sqr(x, t1)) != CRYPT_OK)                                             { goto cleanup; }
      if ((err = ltc_mp_mulmod(t1, x, prime, t1)) != CRYPT_OK)                               { goto cleanup; }
      /* compute x^3 + a*x */
      if ((err = ltc_mp_mulmod(a, x, prime, t2)) != CRYPT_OK)                                { goto cleanup; }
      if ((err = ltc_mp_add(t1, t2, t1)) != CRYPT_OK)                                        { goto cleanup; }
      /* compute x^3 + a*x + b */
      if ((err = ltc_mp_add(t1, b, t1)) != CRYPT_OK)                                         { goto cleanup; }
      /* compute sqrt(x^3 + a*x + b) */
      if ((err = ltc_mp_sqrtmod_prime(t1, prime, t2)) != CRYPT_OK)                           { goto cleanup; }
      /* adjust y */
      if ((ltc_mp_isodd(t2) && in[0] == 0x03) || (!ltc_mp_isodd(t2) && in[0] == 0x02)) {
         if ((err = ltc_mp_mod(t2, prime, y)) != CRYPT_OK)                                   { goto cleanup; }
      }
      else {
         if ((err = ltc_mp_submod(prime, t2, prime, y)) != CRYPT_OK)                         { goto cleanup; }
      }
   }
   else {
      err = CRYPT_INVALID_PACKET;
      goto cleanup;
   }

   err = CRYPT_OK;
cleanup:
   ltc_mp_deinit_multi(t1, t2, LTC_NULL);
   return err;
}

#endif
