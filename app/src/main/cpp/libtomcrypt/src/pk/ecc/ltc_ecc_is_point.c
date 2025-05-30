/* LibTomCrypt, modular cryptographic library -- Tom St Denis */
/* SPDX-License-Identifier: Unlicense */

#include "tomcrypt_private.h"

#ifdef LTC_MECC

/** Returns whether [x,y] is a point on curve defined by dp
  @param dp     curve parameters
  @param x      x point coordinate
  @param y      y point coordinate
  @return CRYPT_OK if valid
*/

int ltc_ecc_is_point(const ltc_ecc_dp *dp, void *x, void *y)
{
  void *prime, *a, *b, *t1, *t2;
  int err;

  prime = dp->prime;
  b     = dp->B;
  a     = dp->A;

  if ((err = ltc_mp_init_multi(&t1, &t2, LTC_NULL)) != CRYPT_OK)  return err;

  /* compute y^2 */
  if ((err = ltc_mp_sqr(y, t1)) != CRYPT_OK)                  goto cleanup;

  /* compute x^3 */
  if ((err = ltc_mp_sqr(x, t2)) != CRYPT_OK)                  goto cleanup;
  if ((err = ltc_mp_mod(t2, prime, t2)) != CRYPT_OK)          goto cleanup;
  if ((err = ltc_mp_mul(x, t2, t2)) != CRYPT_OK)              goto cleanup;

  /* compute y^2 - x^3 */
  if ((err = ltc_mp_sub(t1, t2, t1)) != CRYPT_OK)             goto cleanup;

  /* compute y^2 - x^3 - a*x */
  if ((err = ltc_mp_submod(prime, a, prime, t2)) != CRYPT_OK) goto cleanup;
  if ((err = ltc_mp_mulmod(t2, x, prime, t2)) != CRYPT_OK)    goto cleanup;
  if ((err = ltc_mp_addmod(t1, t2, prime, t1)) != CRYPT_OK)   goto cleanup;

  /* adjust range (0, prime) */
  while (ltc_mp_cmp_d(t1, 0) == LTC_MP_LT) {
     if ((err = ltc_mp_add(t1, prime, t1)) != CRYPT_OK)       goto cleanup;
  }
  while (ltc_mp_cmp(t1, prime) != LTC_MP_LT) {
     if ((err = ltc_mp_sub(t1, prime, t1)) != CRYPT_OK)       goto cleanup;
  }

  /* compare to b */
  if (ltc_mp_cmp(t1, b) != LTC_MP_EQ) {
     err = CRYPT_INVALID_PACKET;
  } else {
     err = CRYPT_OK;
  }

cleanup:
  ltc_mp_deinit_multi(t1, t2, LTC_NULL);
  return err;
}

#endif
