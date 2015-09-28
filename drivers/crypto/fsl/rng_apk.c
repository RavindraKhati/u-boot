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
#define SIGNATURE_HASH_2_BYTE 0x000A0000
#define SIGNATURE_HASH_3_BYTE 0x000B0000
#define BLACK_KEY_ADDR 0xFED00000

/*Sec memory registers*/
#define SMCJR0_ADDR         (CONFIG_SYS_IMMR + 0x310F4)
#define SMAPJR0_0_ADDR      (CONFIG_SYS_IMMR + 0x31104)
#define SMAG2JR0_0_ADDR     (CONFIG_SYS_IMMR + 0x31108)
#define SMAG1JR0_0_ADDR     (CONFIG_SYS_IMMR + 0x3110C)
#define SMAPJR0_1_ADDR      (CONFIG_SYS_IMMR + 0x31114)
#define SMAG2JR0_1_ADDR     (CONFIG_SYS_IMMR + 0x31118)
#define SMAG1JR0_1_ADDR     (CONFIG_SYS_IMMR + 0x3111C)
#define SMSTA_ADDR          (CONFIG_SYS_IMMR + 0x30FB4)

#define reg_in_be32(x) in_be32((const volatile unsigned __iomem *)(x))
#define reg_out_be32(x,y) out_be32((volatile unsigned __iomem *)(x),(y))


/*The same strucure will be used in kernel sec_jr module to recover back the
 * stored descriptors from memory, Any changes made here has to be propagated in sec_jr
 * module as well*/
static struct desc_store_s
{
	uint32_t desc_mod_xp[30];
};

static uint8_t key_mod[]= {0x01,0x12,0x23,0x34,0x45,0x56,0x67,0x78,0x89,0x9a,0xab,0xbc,0xcd,0xde,0xef,0x10};


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


static void inline_cnstr_jobdesc_priv_key_blob(uint32_t *desc,const uint8_t *priv_key,uint32_t key_size,uint8_t *priv_key_blob)
{
    dma_addr_t dma_addr_out;
    dma_addr_t dma_addr_in, dma_key_mod;
    u32 clrw_data = 0x20008000;
    dma_addr_t black_key_out;
    u32 out_size = key_size + BLOB_EXTRA_LEN;


    dma_addr_out = virt_to_phys((void *)priv_key_blob);
    dma_addr_in = virt_to_phys((void *)priv_key);
    black_key_out = BLACK_KEY_ADDR;
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

int sec_gen_priv_key_blob(const uint8_t *priv_key,uint32_t key_size, uint8_t *black_blob_out)
{
    int ret = 0;
    u32 *desc;

    if ( !priv_key || !black_blob_out)
    {
        debug("sec_gen_priv_key_blob Inavlid Input\n");
        return -ENOMEM;
    }

    desc = calloc(MAX_CAAM_DESCSIZE,sizeof(int));

    if (!desc) {
        debug("sec_gen_priv_key_blob memo for descriptor allocation failed\n");
        return -ENOMEM;
    } 


    inline_cnstr_jobdesc_priv_key_blob(desc,priv_key,key_size,black_blob_out);

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
    black_key_dma = BLACK_KEY_ADDR;
    pub_mod_dma = virt_to_phys((void *)(pub_mod));

    init_job_desc(desc,HDR_MAKE_TRUSTED|HDR_TRUSTED);

    /*SIGNATURE command */
    append_signature(desc,SIGNATURE_HASH_4_BYTE);

    append_key(desc,black_key_dma,in_len,KEY_DEST_PKHA_E|CLASS_1|KEY_ENC|KEY_NWB|KEY_EKT|KEY_TK);

    append_signature(desc,SIGNATURE_HASH_4_BYTE);

    append_fifo_load(desc,pub_mod_dma,in_len, FIFOST_TYPE_PKHA_N|CLASS_1);

    append_signature(desc,SIGNATURE_HASH_2_BYTE);
    /*input data lenght can vary in our case so exclude it from signing*/
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

    if( !pub_mod || !in || !out || !in_len)
    {
        debug("sec_do_rsa_private: Invalid Input\n");
        ret = -EINVAL;
        goto end;
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


static void inline_cnstr_jobdesc_decrypt_key_blob(uint32_t *desc,const uint8_t *privkey_blob,uint32_t blob_length)
{
    dma_addr_t dma_addr_out;
    dma_addr_t dma_addr_in, dma_key_mod;
    u32 out_size = blob_length - BLOB_EXTRA_LEN; 

    dma_addr_out = BLACK_KEY_ADDR;
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

int sec_init_apk_from_blob(const uint8_t *privkey_blob,uint32_t blob_length)
{

    int ret = 0;
    u32 *desc;


    if ( !privkey_blob || !blob_length )
    {
        debug("sec_init_apk_from_blob: Invalid Input\n");
        return -EINVAL;
    }

    desc = calloc(MAX_CAAM_DESCSIZE,sizeof(int));

    if (!desc) {
        debug("sec_init_apk_from_blob: No mem\n");
        return -ENOMEM;
    }


    inline_cnstr_jobdesc_decrypt_key_blob(desc,privkey_blob,blob_length); 


    if ( 0 != (ret = run_descriptor_jr(desc)) )
    {
        debug("sec_init_apk_from_blob: Signing desc failed %8X\n",ret);
        goto end; 
    }

    if ( 0 != (ret = run_descriptor_jr(desc)) )
    {
        debug("sec_init_apk_from_blob: Run desc failed %8X\n",ret);
        goto end;
    }

    ret = 0;

end:
    free(desc);
    return ret;

}

int gen_desc(uint32_t *dst_addr)
{
    int ret = 0;
    struct desc_store_s desc_store = {{0}};
    
    inline_cnstr_jobdesc_priv_expo(desc_store.desc_mod_xp,0x0,0x0,256,0x0);

    if ( 0 != (ret = run_descriptor_jr(desc_store.desc_mod_xp)) )
    {
        debug("Signing trusted desc (desc_mod_xp) failed %8X\n",ret);
        goto end;
    }


    memcpy(dst_addr,&desc_store,sizeof(desc_store));

end:
    return ret;


}

void sec_mem_init()
{


    if ( !(reg_in_be32(SMSTA_ADDR) & 0x2) )
    {
        udelay(1000000);
        if ( !(reg_in_be32(SMSTA_ADDR) & 0x2) )
        {
            set_sec_state_to_fail();
        }

    }

    /*Deallocate default partition*/
    reg_out_be32(SMCJR0_ADDR,0x03);

    /*Allocate partition 0*/
    reg_out_be32(SMAPJR0_0_ADDR,0xFFFFE000);
    /*Allocate page 0 to partition 0*/
    reg_out_be32(SMCJR0_ADDR,0x01);

    /*Allocate partition 1*/
    reg_out_be32(SMAPJR0_1_ADDR,0XFFFFE0FF);

    /*Allocate page 1,2,3 to partition 1*/
    reg_out_be32(SMCJR0_ADDR,0x00010101);
    reg_out_be32(SMCJR0_ADDR,0x00020101);
    reg_out_be32(SMCJR0_ADDR,0x00030101);
    /*Let sec engine and e500 core access these partitions*/
    reg_out_be32(SMAG2JR0_0_ADDR,0x00020001);
    reg_out_be32(SMAG1JR0_0_ADDR,0x00020001);
    reg_out_be32(SMAG2JR0_1_ADDR,0x00020001);
    reg_out_be32(SMAG1JR0_1_ADDR,0x00020001);
    /*Lock out everything*/
    reg_out_be32(SMAPJR0_0_ADDR,0xFFFFF000);
    reg_out_be32(SMAPJR0_1_ADDR,0XFFFFF0FF);


}

#if defined(CONFIG_CMD_SEC_GEN_TRUSTED_DESC)
int do_gen_trusted_desc(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    u32 *dst_addr;

    if (argc != 2)
        return CMD_RET_USAGE;

    dst_addr = (u32 *)simple_strtoul(argv[1], NULL, 16);

    if (0 != gen_desc(dst_addr))
        return CMD_RET_FAILURE;
    else
        return CMD_RET_SUCCESS;
}

U_BOOT_CMD(gen_trusted_desc,2,0,do_gen_trusted_desc,
        "Exercise gen_trusted_desc",
        "<dst_addr(hex_string)> ");
#endif


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
        "Exercise rng functions",
        "<dst_addr(hex_string)> <rand_num size in bytes(hex_string)>");
#endif


