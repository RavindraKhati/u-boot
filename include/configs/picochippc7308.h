/*****************************************************************************
 * $picoChipHeaderSubst$
 *****************************************************************************/

/*!
* \file picochippc7308.h
* \brief Configuration file for U-Boot on the PC7308 platform.
*
* Copyright (c) 2006-2011 Picochip Ltd
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
* All enquiries to support@picochip.com
*/

#ifndef __CONFIG_PC7308_H
#define __CONFIG_PC7308_H

#include <asm/arch/picoxcell.h>
#include <asm/arch/uart.h>
#include <asm/arch/gpio.h>
#include <asm/sizes.h>

/*-----------------------------------------------------------------------------
 * Platform Identification Stuff
 */
#define PICOCHIP "picochip"

/* Which hardware platform I am destined for */
#define PICOCHIP_PLATFORM "PC7308"

/* Specific version of this build */
#ifndef PICOCHIP_PLATFORM_VERSION
#define PICOCHIP_PLATFORM_VERSION "dummy_release"
#endif /* PICOCHIP_PLATFORM_VERSION */

#define CONFIG_IDENT_STRING " "PICOCHIP"-"PICOCHIP_PLATFORM_VERSION \
                            "-"PICOCHIP_PLATFORM

/*-----------------------------------------------------------------------------
 * High Level Configuration Options
 */
/* Running on Picochip PC3X2 Silicon */
#define CONFIG_PICOCHIP_PC3X2

/* Running on Picochip PC3X3 Silicon */
#define CONFIG_PICOCHIP_PC3X3

/* Running on Picochip PC3008 Silicon */
#define CONFIG_PICOCHIP_PC3008

/* Running on a Picochip PC7302 platform */
#define CONFIG_PICOCHIP_PC7308

/* Base address of the onchip SRAM */
#define PICOXCELL_ONCHIP_SRAM_BASE      (PICOXCELL_SRAM_BASE)
#define PICOXCELL_ONCHIP_SRAM_SIZE      (PICOXCELL_SRAM_SIZE)

/* ARM Sub-system peripherals are clocked at 200MHz */
#define PICOXCELL_AHB_CLOCK_FREQ        (200000000)

/* Don't use Interrupts */
#undef CONFIG_USE_IRQ

/* A time tick is 1 millisecond (this is NOT CPU tick) */
#define CONFIG_SYS_HZ               (1000)

/* Display board info */
#define CONFIG_DISPLAY_BOARDINFO

/* Display cpu info */
#define CONFIG_DISPLAY_CPUINFO

/* Build U-Boot to run at this address */
#define CONFIG_SYS_TEXT_BASE        0x06000000

/* Do not reboot the platform on a panic */
#define CONFIG_PANIC_HANG

/* Do not perform any low level initialisation
 *
 * Note: We will be running from ram and U-Boot
 *       will have been loaded by a first stage bootloader
 */
#define CONFIG_SKIP_LOWLEVEL_INIT

/*-----------------------------------------------------------------------
 * Stack Sizes
 *
 * The stack sizes are set up in arch/arm/lib/board.c using the
 * settings below
 */
#define CONFIG_STACKSIZE	(SZ_256K) 	    /* regular stack */
#ifdef CONFIG_USE_IRQ
#define CONFIG_STACKSIZE_IRQ	(SZ_4K)             /* IRQ stack */
#define CONFIG_STACKSIZE_FIQ	(SZ_4K)    	    /* FIQ stack */
#endif /* CONFIG_USE_IRQ */

/*-----------------------------------------------------------------------
 * Initial Stack Pointer
 *
 * Note: This is only used before U-Boot relocation takes place.
 *       The global data is stored above this address, whilst the stack
 *       is below this address. After relocation the stack is automatically
 *       moved to the top of the available sdram.
 */
#define CONFIG_SYS_INIT_SP_ADDR (0x00001000)

/*-----------------------------------------------------------------------------
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN   (SZ_1M)

/*-----------------------------------------------------------------------------
 * Linux Kernel Stuff
 */
/* Allow passing of command line args (bootargs) to the linux kernel*/
#define CONFIG_CMDLINE_TAG          1
#define CONFIG_SETUP_MEMORY_TAGS    1

/*-----------------------------------------------------------------------------
 * DDR2 RAM Memory Map
 *
 * We have 1 linear addressable RAM bank
 *
 * Note: CONFIG_SYS_SDRAM_BASE must start at 0
 */
#define CONFIG_SYS_SDRAM_BASE	(0x00000000)
#define CONFIG_NR_DRAM_BANKS    (1)
#define PHYS_SDRAM_1		(PICOXCELL_DDRBANK_BASE)
#define PHYS_SDRAM_1_SIZE	(SZ_128M)

/*-----------------------------------------------------------------------------
 * SPI Flash Memory Stuff
 */
#define CONFIG_DW_SPI

/* Include generic support for SPI Flash memory devices */
#define CONFIG_SPI_FLASH

/* Include support for SPI Flash memory devices from ST Micro/Numonyx */
#define CONFIG_SPI_FLASH_STMICRO

/*-----------------------------------------------------------------------------
 * NAND Flash Memory Stuff
 */
#define CONFIG_NAND_DENALI

#define CONFIG_SYS_NAND_BASE        (PICOXCELL_EBI_CS2_BASE)
#define CONFIG_SYS_NAND_MAX_CHIPS   (1)
#define CONFIG_SYS_MAX_NAND_DEVICE  (1)

/* Define Flash memory sector size */
#define NAND_FLASH_SECTOR_SIZE      (SZ_128K)

/* Needed for nand_util.c */
#define CONFIG_SYS_64BIT_VSPRINTF

/* No flash memory in the system */
#define CONFIG_SYS_NO_FLASH

/*-----------------------------------------------------------------------------
 * U-Boot Environment Stuff
 */

/* NAND Flash memory map
 *
 *  Block 0-7   Reserved
 *  Block 8-11  U-Boot image
 *  Block 12-15 Reserved
 *  Block 16-19 Redundant U-Boot image
 *  Block 20-23 Reserved
 *  Block 24    U-Boot env variables
 *  Block 25    Redundant U-Boot env variables
 *  Block 26-27 Reserved
 *  Block 28    Linux kernel...
 *
 */
#ifdef CONFIG_NAND_DENALI
#define CONFIG_ENV_IS_IN_NAND

/* Offset address of environment variables */
#define CONFIG_ENV_OFFSET           (NAND_FLASH_SECTOR_SIZE * 24)

/* Size of environment variables */
#define CONFIG_ENV_SIZE             (NAND_FLASH_SECTOR_SIZE)

/* Offset address of the redundant envinronment variables */
#define CONFIG_ENV_OFFSET_REDUND    (NAND_FLASH_SECTOR_SIZE * 25)

/* Turn off wite protection for vendor parameters */
#define CONFIG_ENV_OVERWRITE
#else
#define CONFIG_ENV_IS_NOWHERE
#define CONFIG_ENV_SIZE             (NAND_FLASH_SECTOR_SIZE)
#endif

/*-----------------------------------------------------------------------------
 * Timer Stuff
 */
#define CONFIG_SYS_TIMERBASE    (PICOXCELL_TIMER_BASE)

/*-----------------------------------------------------------------------------
 * Ethernet Stuff
 */
#define CONFIG_DW_EMAC
#define CONFIG_PHY_ADDR         (1)
#define CONFIG_NET_MULTI

/*-----------------------------------------------------------------------------
 * Serial Port Stuff
 */
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE (-4)
#define CONFIG_SYS_NS16550_CLK      (PICOXCELL_AHB_CLOCK_FREQ)
#define CONFIG_SYS_NS16550_COM1     (PICOXCELL_UART1_BASE)

/* Our uart registers need 32 bit access */
#define CONFIG_SYS_NS16550_MEM32

/* Console on Uart #0 */
#define CONFIG_CONS_INDEX	    (1)

/* Table of supported baud rates */
#define CONFIG_SYS_BAUDRATE_TABLE   { 9600, 19200,  \
                                      38400, 57600, \
                                      115200, 230400 }

/*-----------------------------------------------------------------------------
 * U-Boot Memory Test (mtest command) Stuff
 */
/* Default start address for memory test */
#define CONFIG_SYS_MEMTEST_START    (PICOXCELL_ONCHIP_SRAM_BASE)

/* Default end address for memory test */
#define CONFIG_SYS_MEMTEST_END  (CONFIG_SYS_MEMTEST_START + \
                                 PICOXCELL_ONCHIP_SRAM_SIZE - 1)

/* Define this to use the super duper memory test */
#define CONFIG_SYS_ALT_MEMTEST

/* Use Uart #1 scratch pad reg */
#define CONFIG_SYS_MEMTEST_SCRATCH  (PICOXCELL_UART1_BASE + \
                                     UART_SCRATCH_REG_OFFSET)

/*-----------------------------------------------------------------------------
 * U-Boot Supported Commands
 */
#include "config_cmd_default.h"

/* Include 'ping' command */
#define CONFIG_CMD_PING

/* Include commands for SPI Flash memory */
#define CONFIG_CMD_SF

#ifdef CONFIG_NAND_DENALI
/* Include commands for NAND Flash Memory */
#define CONFIG_CMD_NAND
#endif

/* Turn off a bunch of default commands */
#undef CONFIG_CMD_BOOTD
#undef CONFIG_CMD_CONSOLE
#undef CONFIG_CMD_ECHO
#undef CONFIG_CMD_EDITENV
#undef CONFIG_CMD_FPGA
#undef CONFIG_CMD_ITEST
#undef CONFIG_CMD_IMLS
#undef CONFIG_CMD_LOADB
#undef CONFIG_CMD_LOADS
#undef CONFIG_CMD_NFS
#undef CONFIG_CMD_SETGETDCR
#undef CONFIG_CMD_SOURCE
#undef CONFIG_CMD_XIMG

#ifdef CONFIG_SYS_NO_FLASH
#undef CONFIG_CMD_FLASH
#endif /* CONFIG_SYS_NO_FLASH */

/* Use the HUSH parser */
#define CONFIG_SYS_HUSH_PARSER

#ifdef  CONFIG_SYS_HUSH_PARSER
/* This defines the secondary prompt string */
#define CONFIG_SYS_PROMPT_HUSH_PS2 "> "
#endif /* CONFIG_SYS_HUSH_PARSER */

/* Enable command line editing and history */
#define CONFIG_CMDLINE_EDITING

#ifdef CONFIG_CMD_NAND
/* Enable command line MTD partitioning */
#define CONFIG_CMD_MTDPARTS
#define CONFIG_MTD_DEVICE
#endif

/*-----------------------------------------------------------------------------
 * Miscellaneous Configurable Options...
 */
/* Use 'long' help messages */
#define CONFIG_SYS_LONGHELP

/* Monitor Command Prompt */
#define CONFIG_SYS_PROMPT   "=> "

/* Console I/O Buffer Size*/
#define CONFIG_SYS_CBSIZE   (SZ_1K)

/* Print buffer size */
#define CONFIG_SYS_PBSIZE   (CONFIG_SYS_CBSIZE + \
                             sizeof(CONFIG_SYS_PROMPT) + 16)

/* Maximum number of command args */
#define CONFIG_SYS_MAXARGS  (16)

/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE (CONFIG_SYS_CBSIZE)

/* Default load address for tftp, bootm and friends */
#define CONFIG_SYS_LOAD_ADDR    0x00200000
#define CONFIG_LOADADDR         CONFIG_SYS_LOAD_ADDR

/* Default command line mtd partitioning */
#define MTD_PARTITION_DEFAULT   "nand0,0"

#define MTDIDS_DEFAULT          "nand0=gpio-nand"

#define MTDPARTS_DEFAULT	"mtdparts=gpio-nand:"\
                                "128K(NandFirstStage0),"\
                                "128K(NandFirstStage1),"\
                                "128K(NandFirstStage2),"\
                                "128K(NandFirstStage3),"\
                                "1M@1M(Boot),"\
				"1M(RedundantBoot),"\
				"128K(BootEnvironment),"\
                                "128K(RedundantBootEnv),"\
                                "8M@0x380000(KernelA),"\
                                "80M(FileSystemA),"\
                                "8M(Configuration),"\
                                "8M(KernelB),"\
                                "80M(FileSystemB)"

/* Timeout delay (mS) used for checking for the reset button
 * being held down at boot time.
 */
#define CONFIG_SYS_RESET_DELAY (10000)

/* Which arm gpio pin the reset button is wired to */
#define CONFIG_SYS_RESET_PIN   (GPIO_BIT_6)

/* The check for factory reset request is contained in the
 * misc_init_r function
 */
#define CONFIG_MISC_INIT_R

/*-----------------------------------------------------------------------
 * Environment Configuration
 */

#if defined(CONFIG_DW_EMAC)

/* Picochip OUI
 *
 * Note: The MAC Address defined by 'CONFIG_ETHADDR' is based on
 * Picochip's OUI,see http://standards.ieee.org/regauth/oui/index.shtml
 * for more information. It will need to be modified for each and every
 * individual hardware platform.
 */
#define CONFIG_ETHADDR          00:15:E1:00:00:00

/* Picochip default for testing, will need noodling by users */
#define CONFIG_IPADDR           172.17.16.248

#define CONFIG_HOSTNAME	        picopc7308
#define CONFIG_ROOTPATH	        /var/nfspc7308
#define CONFIG_BOOTFILE	        uImage-pc7308

#define CONFIG_SERVERIP         172.17.7.100
#define CONFIG_GATEWAYIP        172.17.0.1
#define CONFIG_NETMASK          255.255.0.0

/* Enable the MII utility commands */
#undef CONFIG_CMD_MII

#endif /* CONFIG_DW_EMAC */

/* This is the offset from the start of NAND Flash
 * to where the Linux kernel can be found.
 */
#define CONFIG_NAND_KERNEL_OFFSET  0x00380000

/* Time in seconds before autoboot, -1 disables auto-boot */
#define CONFIG_BOOTDELAY        5

/* The boot command will set bootargs */
#undef  CONFIG_BOOTARGS

/* Default console baud rate */
#define CONFIG_BAUDRATE	        115200

/* Unless specified here we'll just rely on the kernel default */
#define OTHERBOOTARGS

/* Define the UBIFS root filesystem partition for NAND Flash */
#define NAND_UBIFS_ROOT         5

/* Define CONFIG_BOOTCOMMAND as */
#define CONFIG_BOOTCOMMAND  "run nand_ubifs"

#define	CONFIG_EXTRA_ENV_SETTINGS				            \
   "othbootargs=" __stringify (OTHERBOOTARGS) "\0"                               \
   "netdev=eth0\0"                                                          \
   "consoledev=ttyS0\0"                                                     \
   "kernel_nand_offset=" __stringify(CONFIG_NAND_KERNEL_OFFSET) "\0"             \
   "nand_ubifs_root=" __stringify(NAND_UBIFS_ROOT) "\0"		            \
   "nand_ubifs=run nand_ubifs_args; nboot $loadaddr 0 "                     \
   "$kernel_nand_offset; bootm $loadaddr\0"                                 \
   "fixed_nfs=run nfs_args; tftp; bootm\0"				    \
   "nand_ubifs_args=setenv bootargs root=ubi0:rootfs rw rootfstype=ubifs "  \
   "ubi.mtd=$nand_ubifs_root,2048 "                                         \
   "ip=$ipaddr:$serverip:$gatewayip:$netmask:$hostname:$netdev:any "        \
   "console=$consoledev,$baudrate $mtdparts $othbootargs;\0"                \
   "nfs_args=setenv bootargs root=/dev/nfs rw nfsroot=$serverip:$rootpath " \
   "ip=$ipaddr:$serverip:$gatewayip:$netmask:$hostname:$netdev:any "        \
   "console=$consoledev,$baudrate $mtdparts $othbootargs;\0"                \
   "partition=" MTD_PARTITION_DEFAULT "\0"                                  \
   "mtdids=" MTDIDS_DEFAULT "\0"                                            \
   "mtdparts=" MTDPARTS_DEFAULT "\0"

#endif /* __CONFIG_PC7308_H */
