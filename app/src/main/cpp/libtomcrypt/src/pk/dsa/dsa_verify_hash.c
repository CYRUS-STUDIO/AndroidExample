/* LibTomCrypt, modular cryptographic library -- Tom St Denis */
/* SPDX-License-Identifier: Unlicense */
#include "tomcrypt_private.h"

/**
   @file dsa_verify_hash.c
   DSA implementation, verify a signature, Tom St Denis
*/


#ifdef LTC_MDSA

/**
  Verify a DSA signature
  @param r        DSA "r" parameter
  @param s        DSA "s" parameter
  @param hash     The hash that was signed
  @param hashlen  The length of the hash that was signed
  @param stat     [out] The result of the signature verification, 1==valid, 0==invalid
  @param key      The corresponding public DSA key
  @return CRYPT_OK if successful (even if the signature is invalid)
*/
int dsa_verify_hash_raw(         void   *r,          void   *s,
                    const unsigned char *hash, unsigned long hashlen,
                                    int *stat, const dsa_key *key)
{
   void          *w, *v, *u1, *u2;
   int           err;

   LTC_ARGCHK(r    != NULL);
   LTC_ARGCHK(s    != NULL);
   LTC_ARGCHK(stat != NULL);
   LTC_ARGCHK(key  != NULL);

   /* default to invalid signature */
   *stat = 0;

   /* init our variables */
   if ((err = ltc_mp_init_multi(&w, &v, &u1, &u2, LTC_NULL)) != CRYPT_OK) {
      return err;
   }

   /* neither r or s can be null or >q*/
   if (ltc_mp_cmp_d(r, 0) != LTC_MP_GT || ltc_mp_cmp_d(s, 0) != LTC_MP_GT || ltc_mp_cmp(r, key->q) != LTC_MP_LT || ltc_mp_cmp(s, key->q) != LTC_MP_LT) {
      err = CRYPT_INVALID_PACKET;
      goto error;
   }

   /* FIPS 186-4 4.7: use leftmost min(bitlen(q), bitlen(hash)) bits of 'hash' */
   hashlen = MIN(hashlen, (unsigned long)(key->qord));

   /* w = 1/s mod q */
   if ((err = ltc_mp_invmod(s, key->q, w)) != CRYPT_OK)                                       { goto error; }

   /* u1 = m * w mod q */
   if ((err = ltc_mp_read_unsigned_bin(u1, hash, hashlen)) != CRYPT_OK)                       { goto error; }
   if ((err = ltc_mp_mulmod(u1, w, key->q, u1)) != CRYPT_OK)                                  { goto error; }

   /* u2 = r*w mod q */
   if ((err = ltc_mp_mulmod(r, w, key->q, u2)) != CRYPT_OK)                                   { goto error; }

   /* v = g^u1 * y^u2 mod p mod q */
   if ((err = ltc_mp_exptmod(key->g, u1, key->p, u1)) != CRYPT_OK)                            { goto error; }
   if ((err = ltc_mp_exptmod(key->y, u2, key->p, u2)) != CRYPT_OK)                            { goto error; }
   if ((err = ltc_mp_mulmod(u1, u2, key->p, v)) != CRYPT_OK)                                  { goto error; }
   if ((err = ltc_mp_mod(v, key->q, v)) != CRYPT_OK)                                          { goto error; }

   /* if r = v then we're set */
   if (ltc_mp_cmp(r, v) == LTC_MP_EQ) {
      *stat = 1;
   }

   err = CRYPT_OK;
error:
   ltc_mp_deinit_multi(w, v, u1, u2, LTC_NULL);
   return err;
}

/**
  Verify a DSA signature
  @param sig      The signature
  @param siglen   The length of the signature (octets)
  @param hash     The hash that was signed
  @param hashlen  The length of the hash that was signed
  @param stat     [out] The result of the signature verification, 1==valid, 0==invalid
  @param key      The corresponding public DSA key
  @return CRYPT_OK if successful (even if the signature is invalid)
*/
int dsa_verify_hash(const unsigned char *sig,        unsigned long  siglen,
                    const unsigned char *hash,       unsigned long  hashlen,
                          int           *stat, const dsa_key       *key)
{
   int    err;
   void   *r, *s;
   ltc_asn1_list sig_seq[2];
   unsigned long reallen = 0;

   LTC_ARGCHK(stat != NULL);
   *stat = 0; /* must be set before the first return */

   if ((err = ltc_mp_init_multi(&r, &s, LTC_NULL)) != CRYPT_OK) {
      return err;
   }

   LTC_SET_ASN1(sig_seq, 0, LTC_ASN1_INTEGER, r, 1UL);
   LTC_SET_ASN1(sig_seq, 1, LTC_ASN1_INTEGER, s, 1UL);

   err = der_decode_sequence_strict(sig, siglen, sig_seq, 2);
   if (err != CRYPT_OK) {
      goto LBL_ERR;
   }

   err = der_length_sequence(sig_seq, 2, &reallen);
   if (err != CRYPT_OK || reallen != siglen) {
      goto LBL_ERR;
   }

   /* do the op */
   err = dsa_verify_hash_raw(r, s, hash, hashlen, stat, key);

LBL_ERR:
   ltc_mp_deinit_multi(r, s, LTC_NULL);
   return err;
}

#endif

