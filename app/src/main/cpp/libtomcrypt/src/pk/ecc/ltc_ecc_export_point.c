/* LibTomCrypt, modular cryptographic library -- Tom St Denis */
/* SPDX-License-Identifier: Unlicense */

#include "tomcrypt_private.h"

#ifdef LTC_MECC

int ltc_ecc_export_point(unsigned char *out, unsigned long *outlen, void *x, void *y, unsigned long size, int compressed)
{
   int err;
   unsigned char buf[ECC_BUF_SIZE];
   unsigned long xsize, ysize;

   if (size > sizeof(buf)) return CRYPT_BUFFER_OVERFLOW;
   if ((xsize = ltc_mp_unsigned_bin_size(x)) > size) return CRYPT_BUFFER_OVERFLOW;
   if ((ysize = ltc_mp_unsigned_bin_size(y)) > size) return CRYPT_BUFFER_OVERFLOW;

   if(compressed) {
      if (*outlen < (1 + size)) {
         *outlen = 1 + size;
         return CRYPT_BUFFER_OVERFLOW;
      }
      /* store first byte */
      out[0] = ltc_mp_isodd(y) ? 0x03 : 0x02;
      /* pad and store x */
      zeromem(buf, sizeof(buf));
      if ((err = ltc_mp_to_unsigned_bin(x, buf + (size - xsize))) != CRYPT_OK) return err;
      XMEMCPY(out+1, buf, size);
      /* adjust outlen */
      *outlen = 1 + size;
   }
   else {
      if (*outlen < (1 + 2*size)) {
         *outlen = 1 + 2*size;
         return CRYPT_BUFFER_OVERFLOW;
      }
      /* store byte 0x04 */
      out[0] = 0x04;
      /* pad and store x */
      zeromem(buf, sizeof(buf));
      if ((err = ltc_mp_to_unsigned_bin(x, buf + (size - xsize))) != CRYPT_OK) return err;
      XMEMCPY(out+1, buf, size);
      /* pad and store y */
      zeromem(buf, sizeof(buf));
      if ((err = ltc_mp_to_unsigned_bin(y, buf + (size - ysize))) != CRYPT_OK) return err;
      XMEMCPY(out+1+size, buf, size);
      /* adjust outlen */
      *outlen = 1 + 2*size;
   }
   return CRYPT_OK;
}

#endif
