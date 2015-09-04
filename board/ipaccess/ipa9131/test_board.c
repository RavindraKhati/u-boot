#include <common.h>
#include <linux/compiler.h>
#include <i2c.h>
#include "characterisation.h"

#if defined CONFIG_CMD_IPA9131_TEST_BOARD

static int is_test_board(void)
{
    uint8_t buf = 0;
    if ( 0 != i2c_read(CONFIG_CHARACTERISATION_EEPROM_ADDR,
                CONFIG_IPA9131_MISC_FLAGS_OFFSET,
                1,
                &buf,
                1) )
    {
        fprintf(stderr,"Error Reading test board bit\n");
        return -1;
    }
    return (int) (buf & 0x01);
}

static int set_test_board(void)
{
    uint8_t buf = 1;

    if ( 0 != i2c_write(CONFIG_CHARACTERISATION_EEPROM_ADDR,
                CONFIG_IPA9131_MISC_FLAGS_OFFSET,
                1,
                &buf,
                1) )
    {
        fprintf(stderr,"Error setting test board bit\n");
        return -1;
    }
    return 0;
}

static int clear_test_board(void)
{
    uint8_t buf = 0;

    if ( 0 != i2c_write(CONFIG_CHARACTERISATION_EEPROM_ADDR,
                CONFIG_IPA9131_MISC_FLAGS_OFFSET,
                1,
                &buf,
                1) )
    {
        fprintf(stderr,"Error clearing test board bit\n");
        return -1;
    }
    return 0;
}

int do_test_board(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    int ret = 0;
    if (argc != 2)
        return CMD_RET_USAGE;

    if ( !(characterisation_is_development_mode() || characterisation_is_test_mode()) )
    {
        fprintf(stderr,"Not a development board, command meant for dev boards only\n");
        return CMD_RET_FAILURE;
    }

    if (0 == strcmp(argv[1], "status"))
    {
        if ( -1 != (ret = is_test_board()))
        {
            (ret == 1)?printf("Configured as Test Board\n"):printf("Not configured as Test Board\n");
            return CMD_RET_SUCCESS;
        }
        else
        {
            return CMD_RET_FAILURE;
        }
    }
    else if (0 == strcmp(argv[1], "set"))
    {
        if ( 0 != set_test_board() )
            return CMD_RET_FAILURE;
        else
            return CMD_RET_SUCCESS;
    }
    else if (0 == strcmp(argv[1], "clear"))
    {
        if ( 0 != clear_test_board() )
            return CMD_RET_FAILURE;
        else
            return CMD_RET_SUCCESS;
    }

    return CMD_RET_USAGE;
}

U_BOOT_CMD(
        test_board, 2, 0, do_test_board,
        "Convert a dev board to test board vice-versa or displays it's status ",
        "set|clear|status"
        );
#endif
