#include <common.h>
#include <compiler.h>
#include <asm/errno.h>
#include <asm/io.h>
#include <asm/immap_85xx.h>
#include "characterisation.h"


#define sec_out_be32(x,y) out_be32((volatile unsigned __iomem *)(x),(y))

#define SECMON_HPCOMR        (CONFIG_SYS_IMMR + 0x000e6004)



static void set_sec_state(void)
{
        if ( characterisation_is_specials_mode() )
        {
                /*Transition to soft fail state*/
                sec_out_be32(SECMON_HPCOMR,0x00000200);

        }

        /*Transition to appropriate state
         * for Specials causes: soft-fail->non-secure
         * for Prod/dev/test causes : Trusted-->secure*/
        sec_out_be32(SECMON_HPCOMR,0x00000001);

        /*Lock transition back to trusted state from secure state*/
        sec_out_be32(SECMON_HPCOMR,0x00000002);
}

int do_change_sec_state(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
        set_sec_state();
        return CMD_RET_SUCCESS;
}

U_BOOT_CMD(change_sec_state, 1, 0, do_change_sec_state,
                "Change security engine state",
                "change_sec_state"
          );

