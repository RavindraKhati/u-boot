/*
 *  * Copyright (c) 2015, ip.access Ltd. All Rights Reserved.
 *   */
#ifndef GEN_APK_CSR_H_20150611
#define GEN_APK_CSR_H_20150611

#include <common.h>


typedef struct crypt_buf_s
{
	uint32_t len;
	uint8_t  *buf;

}crypt_buf_t;

extern int get_key_pair_csr(crypt_buf_t *pub_key_buf,crypt_buf_t *csr_buf,crypt_buf_t *priv_key_blob);
extern int get_pub_key(const uint8_t *advert_buf,const uint8_t *privkey_enc_buf, uint32_t privkey_len,crypt_buf_t *pub_key_buf);
extern int get_csr( const uint8_t *pub_key_buf, uint32_t pub_key_len , crypt_buf_t *csr_buf);
extern int get_otpmk(u32 *otpmk,crypt_buf_t *otpmk_buf);
extern int get_dbg_rsp(u32 *dbg_rsp,crypt_buf_t *dbg_rsp_buf);
extern int get_signature( const uint8_t *pub_key_buf, uint32_t pub_key_len ,const uint8_t *hash,uint32_t hash_len, crypt_buf_t *sig_buf );
#endif
