/*
 *  * Copyright (c) 2015, ip.access Ltd. All Rights Reserved.
 *   */

#include <common.h>
#include <command.h>
#include <environment.h>
#include <asm/byteorder.h>
#include <linux/compiler.h>
#include <malloc.h>
#include <errno.h>
#include "desc_constr.h"
#include "jobdesc.h"
#include "jr.h"
#include "desc.h"

#define LOCAL_OFF_NEXT_CMD 0x1
#define PROT_INFO_TK_EKT 0x00000300
#define PROT_INFO_BLACK_KEY 0x00000004

#define BLOB_EXTRA_LEN 48
#define BLACK_KEY_INITIALISED 0x01
#define PUB_MODULUS_INITIALISED 0x10
#define FIFOST_TYPE_PKHA_E       (0x13 << FIFOST_TYPE_SHIFT)
/*Siganture fields construct*/
#define SIGNATURE_HASH_4_BYTE 0x000C0000

static struct apk_ctx_t
{
    uint8_t black_key[268];
    uint8_t pub_modulus[256];
    uint8_t initialised;
} apk_ctx={{0}};

static uint8_t key_mod[]= {0x01,0x12,0x23,0x34,0x45,0x56,0x67,0x78,0x89,0x9a,0xab,0xbc,0xcd,0xde,0xef,0x10};


void sec_init_apk_ctx( const uint8_t* pub_mod, const uint8_t* black_key )
{
    if (!apk_ctx.initialised)
    {
        /*Initialise this only once*/
        memcpy(apk_ctx.pub_modulus,pub_mod,256);
        memcpy(apk_ctx.black_key,black_key,268);
        apk_ctx.initialised = 1;
    }

}

static void inline_cnstr_jobdesc_rng_gen(uint32_t *desc,uint32_t rn_size, uint8_t *rn)
{
    dma_addr_t dma_addr_out;

    dma_addr_out = virt_to_phys((void *)rn);

    init_job_desc(desc, 0);
    /*LOAD Class 1 data size register with number of random bytes to be generated*/
    append_load_imm_u32(desc, rn_size, LDST_SRCDST_WORD_DATASZ_REG|CLASS_1);

    /*OPERATION command to generate random number*/
    append_operation(desc, OP_TYPE_CLASS1_ALG|OP_ALG_ALGSEL_RNG);

    /* Get data from the Output Fifo to our pointer */
    append_fifo_store(desc, dma_addr_out, rn_size, FIFOST_TYPE_MESSAGE_DATA);

}


int sec_generate_random_number(uint32_t len, uint8_t *dst_addr)
{
    int ret = 0;
    u32 *desc;

    desc = calloc(MAX_CAAM_DESCSIZE,sizeof(int));
    if (!desc) {
        debug("Not enough memory for descriptor allocation\n");
        return -ENOMEM;
    }


    inline_cnstr_jobdesc_rng_gen(desc,len,dst_addr);

    ret = run_descriptor_jr(desc);

    if (ret)
        printf("Error in Random number generation %d\n", ret);

    free(desc);
    return ret;
}


static void inline_cnstr_jobdesc_priv_key_blob(uint32_t *desc,const uint8_t *priv_key,uint32_t key_size,uint8_t *priv_key_blob,uint8_t *black_key)
{
    dma_addr_t dma_addr_out;
    dma_addr_t dma_addr_in, dma_key_mod;
    u32 clrw_data = 0x20008000;
    dma_addr_t black_key_out;
    u32 out_size = key_size + BLOB_EXTRA_LEN;


    dma_addr_out = virt_to_phys((void *)priv_key_blob);
    dma_addr_in = virt_to_phys((void *)priv_key);
    black_key_out =  virt_to_phys((void *)black_key);
    dma_key_mod =  virt_to_phys((void *)key_mod);

    init_job_desc(desc,HDR_MAKE_TRUSTED|HDR_TRUSTED);

    /*SIGNATURE command */
    append_signature(desc,SIGNATURE_HASH_4_BYTE);

    append_key(desc,dma_addr_in,key_size,KEY_DEST_PKHA_E|CLASS_1);

    append_fifo_store(desc, black_key_out, key_size, FIFOST_TYPE_PKHA_E);

    append_jump(desc,JUMP_JSL|JUMP_COND_MATH_C|LOCAL_OFF_NEXT_CMD);

    append_load_imm_u32(desc, clrw_data, LDST_SRCDST_WORD_CLRW);

    append_seq_in_ptr_intlen(desc,black_key_out,key_size,0);

    append_signature(desc,SIGNATURE_HASH_4_BYTE);

    append_seq_out_ptr_intlen(desc,dma_addr_out,out_size,0);

    append_load(desc,dma_key_mod,sizeof(key_mod),LDST_SRCDST_BYTE_KEY|CLASS_2);

    append_operation(desc,OP_TYPE_ENCAP_PROTOCOL|OP_PCLID_BLOB|PROT_INFO_TK_EKT|PROT_INFO_BLACK_KEY);

    append_signature(desc,0x0);
    /*Add 8 words for signature*/
    desc[0] += 8;


}

int sec_gen_priv_key_blob(const uint8_t *priv_key,uint32_t key_size, uint8_t *black_blob_out,uint8_t *black_key_out)
{
    int ret = 0;
    u32 *desc;

    if ( !priv_key || !black_blob_out || !black_key_out)
    {
        debug("sec_gen_priv_key_blob Inavlid Input\n");
        return -ENOMEM;
    }

    desc = calloc(MAX_CAAM_DESCSIZE,sizeof(int));

    if (!desc) {
        debug("sec_gen_priv_key_blob memo for descriptor allocation failed\n");
        return -ENOMEM;
    } 


    inline_cnstr_jobdesc_priv_key_blob(desc,priv_key,key_size,black_blob_out,black_key_out);

    if (0 != (ret = run_descriptor_jr(desc)) )
    {
        debug("Signing priv_key_blob desc failed %8X\n",ret);
        goto end;
    }

    if (0 != (ret = run_descriptor_jr(desc)) )
    {
        debug("priv_key_blob desc failed %8X\n",ret);
        goto end;
    }

end:
    free(desc);
    return ret;
}


static void inline_cnstr_jobdesc_priv_expo(uint32_t *desc,const uint8_t *pub_mod,const uint8_t *in,uint32_t in_len, uint8_t *out)
{
    dma_addr_t dma_addr_in,dma_addr_out,black_key_dma,pub_mod_dma;

    dma_addr_out = virt_to_phys((void *)out);
    dma_addr_in = virt_to_phys((void *)in);
    black_key_dma = virt_to_phys((void *)(apk_ctx.black_key));
    pub_mod_dma = virt_to_phys((void *)(pub_mod));

    init_job_desc(desc,HDR_MAKE_TRUSTED|HDR_TRUSTED);

    /*SIGNATURE command */
    append_signature(desc,SIGNATURE_HASH_4_BYTE);

    append_key(desc,black_key_dma,in_len,KEY_DEST_PKHA_E|CLASS_1|KEY_ENC|KEY_NWB);

    append_signature(desc,SIGNATURE_HASH_4_BYTE);

    append_fifo_load(desc,pub_mod_dma,in_len, FIFOST_TYPE_PKHA_N|CLASS_1);

    append_signature(desc,SIGNATURE_HASH_4_BYTE);

    append_fifo_load(desc,dma_addr_in,in_len, FIFOST_TYPE_PKHA_A|CLASS_1);

    append_operation(desc,OP_TYPE_PK|OP_ALG_TYPE_CLASS1|OP_ALG_PKMODE_MOD_EXPO|0x00800000);

    append_signature(desc,SIGNATURE_HASH_4_BYTE);

    append_fifo_store(desc,dma_addr_out,in_len,FIFOST_TYPE_PKHA_B);

    append_signature(desc,0x0);
    /*Add 8 words for signature*/
    desc[0] += 8;


}

int sec_do_rsa_private(const uint8_t *pub_mod,const uint8_t *in,uint32_t in_len, uint8_t *out)
{
    int ret = 0;
    u32 *desc = NULL;
    const uint8_t *pub_mod_ptr = pub_mod;

    if( !in || !out || !in_len)
    {
        debug("sec_do_rsa_private: Invalid Input\n");
        ret = -EINVAL;
        goto end;
    }

    if( !apk_ctx.initialised)
    {
        debug("sec_do_rsa_private: apk_ctx not initialised\n");
        ret = -EFAULT;
        goto end;
    }
    else if ( !pub_mod_ptr )
    {
        pub_mod_ptr = (const uint8_t *) apk_ctx.pub_modulus;

    }

    desc = calloc(MAX_CAAM_DESCSIZE,sizeof(int));

    if (!desc) 
    {
        debug("sec_do_rsa_private No mem\n");
        ret = -ENOMEM;
        goto end;
    }


    inline_cnstr_jobdesc_priv_expo(desc,pub_mod,in,in_len,out);    

    if ( 0 != (ret = run_descriptor_jr(desc)) )
    {
        debug("Signing sec_do_rsa_private desc failed %8X\n",ret);
        goto end;
    }


    if ( 0 != (ret = run_descriptor_jr(desc)) )
    {
        debug("Run sec_do_rsa_private desc failed %8X\n",ret);
        goto end;
    }

end:
    if (desc)
        free(desc);
    return ret;
}


static void inline_cnstr_jobdesc_decrypt_key_blob(uint32_t *desc,const uint8_t *privkey_blob,uint32_t blob_length,uint8_t *black_key_out)
{
    dma_addr_t dma_addr_out;
    dma_addr_t dma_addr_in, dma_key_mod;
    u32 out_size = blob_length - BLOB_EXTRA_LEN; 

    dma_addr_out = virt_to_phys((void *)black_key_out);
    dma_addr_in = virt_to_phys((void *)privkey_blob);
    dma_key_mod =  virt_to_phys((void *)key_mod);

    init_job_desc(desc, HDR_MAKE_TRUSTED|HDR_TRUSTED );

    /*SIGNATURE command */
    append_signature(desc,SIGNATURE_HASH_4_BYTE);

    append_seq_in_ptr_intlen(desc,dma_addr_in,blob_length,0);

    append_signature(desc,SIGNATURE_HASH_4_BYTE);

    append_seq_out_ptr_intlen(desc,dma_addr_out,out_size,0);

    append_signature(desc,SIGNATURE_HASH_4_BYTE);

    append_load(desc,dma_key_mod,sizeof(key_mod),LDST_SRCDST_BYTE_KEY|CLASS_2);

    append_operation(desc,OP_TYPE_DECAP_PROTOCOL|OP_PCLID_BLOB|PROT_INFO_TK_EKT|PROT_INFO_BLACK_KEY);

    append_signature(desc,0x0);
    /*Add 8 words for signature*/
    desc[0] += 8;
}

int sec_init_apk_ctx_from_blob(const uint8_t *pub_modulus,const uint8_t *privkey_blob,uint32_t blob_length)
{

    int ret = 0;
    u32 *desc;
    uint8_t black_key[268];

    if ( !pub_modulus || !privkey_blob || !blob_length )
    {
        debug("sec_init_apk_ctx_from_blob: Invalid Input\n");
        return -EINVAL;
    }

    desc = calloc(MAX_CAAM_DESCSIZE,sizeof(int));

    if (!desc) {
        debug("sec_init_apk_ctx_from_blob: No mem\n");
        return -ENOMEM;
    }


    inline_cnstr_jobdesc_decrypt_key_blob(desc,privkey_blob,blob_length,black_key); 


    if ( 0 != (ret = run_descriptor_jr(desc)) )
    {
        debug("sec_init_apk_ctx_from_blob: Signing desc failed %8X\n",ret);
        goto end; 
    }

    if ( 0 != (ret = run_descriptor_jr(desc)) )
    {
        debug("sec_init_apk_ctx_from_blob: Run desc failed %8X\n",ret);
        goto end;
    }

    sec_init_apk_ctx(pub_modulus,black_key);
    ret = 0;

end:
    free(desc);
    return ret;

}


#if defined(CONFIG_CMD_SEC_GEN_RN)
int do_gen_random(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    u8 *dst_addr;
    u32 len;

    if (argc != 3)
        return CMD_RET_USAGE;

    dst_addr = (u8 *)simple_strtoul(argv[1], NULL, 16);
    len = (u32) simple_strtoul(argv[2], NULL, 16);

    return sec_generate_random_number(len,dst_addr);
}

U_BOOT_CMD(sec_gen_random,3,0,do_gen_random,
        "Excercise rng functions",
        "<dst_addr(hex_string)> <rand_num size in bytes(hex_string)>");
#endif


