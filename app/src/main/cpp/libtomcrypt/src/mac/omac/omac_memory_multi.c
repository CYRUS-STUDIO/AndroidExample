/* LibTomCrypt, modular cryptographic library -- Tom St Denis */
/* SPDX-License-Identifier: Unlicense */
#include "tomcrypt_private.h"

/**
  @file omac_memory_multi.c
  OMAC1 support, process multiple blocks of memory, Tom St Denis
*/

#ifdef LTC_OMAC

static LTC_INLINE int s_omac_vprocess(omac_state *omac, const unsigned char *in,  unsigned long inlen, va_list args)
{
   const unsigned char * curptr = in;
   unsigned long curlen = inlen;
   int err;
   for (;;) {
      /* process buf */
      if ((err = omac_process(omac, curptr, curlen)) != CRYPT_OK) {
         return err;
      }
      /* step to next */
      curptr = va_arg(args, const unsigned char*);
      if (curptr == NULL) {
         break;
      }
      curlen = va_arg(args, unsigned long);
   }
   return CRYPT_OK;
}

int omac_vprocess(omac_state *omac, const unsigned char *in,  unsigned long inlen, va_list args)
{
   return s_omac_vprocess(omac, in, inlen, args);
}

/**
   OMAC multiple blocks of memory
   @param cipher    The index of the desired cipher
   @param key       The secret key
   @param keylen    The length of the secret key (octets)
   @param out       [out] The destination of the authentication tag
   @param outlen    [in/out]  The max size and resulting size of the authentication tag (octets)
   @param in        The data to send through OMAC
   @param inlen     The length of the data to send through OMAC (octets)
   @param ...       tuples of (data,len) pairs to OMAC, terminated with a (NULL,x) (x=don't care)
   @return CRYPT_OK if successful
*/
int omac_memory_multi(int cipher,
                const unsigned char *key, unsigned long keylen,
                      unsigned char *out, unsigned long *outlen,
                const unsigned char *in,  unsigned long inlen, ...)
{
   int                  err;
   omac_state          *omac;
   va_list              args;

   LTC_ARGCHK(key    != NULL);
   LTC_ARGCHK(in     != NULL);
   LTC_ARGCHK(out    != NULL);
   LTC_ARGCHK(outlen != NULL);

   /* allocate ram for omac state */
   omac = XMALLOC(sizeof(omac_state));
   if (omac == NULL) {
      return CRYPT_MEM;
   }

   /* omac process the message */
   if ((err = omac_init(omac, cipher, key, keylen)) != CRYPT_OK) {
      goto LBL_ERR;
   }
   va_start(args, inlen);
   if ((err = s_omac_vprocess(omac, in, inlen, args)) != CRYPT_OK) {
      goto LBL_ERR;
   }
   err = omac_done(omac, out, outlen);
LBL_ERR:
#ifdef LTC_CLEAN_STACK
   zeromem(omac, sizeof(omac_state));
#endif
   XFREE(omac);
   va_end(args);
   return err;
}

#endif
