/*****************************************************************************
 * $picoChipHeaderSubst$
 *****************************************************************************/

/*!
* \file picochippc7302.h
* \brief Configuration file for U-Boot on the PC7302 platform.
*
* Copyright (c) 2006-2011 Picochip Ltd
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
* All enquiries to support@picochip.com
*/

#ifndef __CONFIG_PC7302_H
#define __CONFIG_PC7302_H

#include <asm/arch/picoxcell.h>
#include <asm/arch/uart.h>
#include <asm/sizes.h>

/*-----------------------------------------------------------------------------
 * Platform Identification Stuff
 */
#define PICOCHIP "picochip"

/* Which hardware platform I am destined for */
#define PICOCHIP_PLATFORM "PC7302"

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

/* Running on a Picochip PC7302 platform */
#define CONFIG_PICOCHIP_PC7302

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
 * U-Boot Environment Stuff
 *
 * Note: See picochippc7302nor.h and pichippc7302nand.h
 *       for more information.
 */
#if !defined(CONFIG_RUN_FROM_RAM) && !defined(CONFIG_CMD_NAND) && \
    !defined(CONFIG_ENV_SIZE)

/* We are running from ram, therefore no environment */
#define CONFIG_ENV_IS_NOWHERE

/* Size of environment variables */
#define CONFIG_ENV_SIZE		(SZ_2K)

#endif /* CONFIG_RUN_FROM_RAM && !defined(CONFIG_CMD_NAMD) &&
	  !defined(CONFIG_ENV_SIZE) */

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
#define CONFIG_SYS_NS16550_CLK      (3686400)
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

#ifdef CONFIG_ENV_IS_NOWHERE
#undef CONFIG_CMD_SAVEENV
#endif /* CONFIG_ENV_IS_NOWHERE */

/* Use the HUSH parser */
#define CONFIG_SYS_HUSH_PARSER

#ifdef  CONFIG_SYS_HUSH_PARSER
/* This defines the secondary prompt string */
#define CONFIG_SYS_PROMPT_HUSH_PS2 "> "
#endif /* CONFIG_SYS_HUSH_PARSER */

/* Enable command line editing and history */
#define CONFIG_CMDLINE_EDITING

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
#define CONFIG_IPADDR           172.17.13.248

#define CONFIG_HOSTNAME	        picopc7302
#define CONFIG_ROOTPATH	        /var/nfspc7302
#define CONFIG_BOOTFILE	        uImage-pc7302

#define CONFIG_SERVERIP         172.17.7.100
#define CONFIG_GATEWAYIP        172.17.0.1
#define CONFIG_NETMASK          255.255.0.0

/* Enable the MII utility commands */
#undef CONFIG_CMD_MII

#endif /* CONFIG_DW_EMAC */

/* This is the address in NOR Flash where the Linux kernel
 * can be found.
 */
#define CONFIG_FLASH_KERNEL_BASE   0x40060000

/* This is the offset from the start of NAND Flash
 * to where the Linux kernel can be found.
 */
#define CONFIG_NAND_KERNEL_OFFSET  0x00180000

/* Time in seconds before autoboot, -1 disables auto-boot */
#define CONFIG_BOOTDELAY        5

/* The boot command will set bootargs */
#undef  CONFIG_BOOTARGS

/* Default console baud rate */
#define CONFIG_BAUDRATE	        115200

/* Unless specified here we'll just rely on the kernel default */
#define OTHERBOOTARGS

/* Define the JFFS2 root filesystem partition (NOR Flash) */
#define NOR_JFFS2_ROOT          /dev/mtdblock3

/* Define the JFFS2 root filesystem partition (NAND Flash) */
#define NAND_JFFS2_ROOT         /dev/mtdblock5

/* Define the UBIFS root filesystem partition (NOR Flash) */
#define NOR_UBIFS_ROOT          3

/* Define the UBIFS root filesystem partition for NAND Flash */
#define NAND_UBIFS_ROOT         5

#define	CONFIG_EXTRA_ENV_SETTINGS				            \
   "othbootargs=" __stringify (OTHERBOOTARGS) "\0"                               \
   "netdev=eth0\0"                                                          \
   "consoledev=ttyS0\0"                                                     \
   "kernel_flash_addr=" __stringify(CONFIG_FLASH_KERNEL_BASE) "\0"		    \
   "kernel_nand_offset=" __stringify(CONFIG_NAND_KERNEL_OFFSET) "\0"             \
   "nor_jffs2_root=" __stringify(NOR_JFFS2_ROOT) "\0"		            \
   "nand_jffs2_root=" __stringify(NAND_JFFS2_ROOT) "\0"		            \
   "nor_ubifs_root=" __stringify(NOR_UBIFS_ROOT) "\0"		            \
   "nand_ubifs_root=" __stringify(NAND_UBIFS_ROOT) "\0"		            \
   "flash_jffs2=run jffs2_args; bootm $kernel_flash_addr\0"		    \
   "flash_ubifs=run ubifs_args; bootm $kernel_flash_addr\0"		    \
   "nand_jffs2=run nand_jffs2_args; nboot $loadaddr 0 "                     \
   "$kernel_nand_offset; bootm $loadaddr\0"                                 \
   "nand_ubifs=run nand_ubifs_args; nboot $loadaddr 0 "                     \
   "$kernel_nand_offset; bootm $loadaddr\0"                                 \
   "fixed_nfs=run nfs_args; tftp; bootm\0"				    \
   "jffs2_args=setenv bootargs root=$nor_jffs2_root rw rootfstype=jffs2 "   \
   "ip=$ipaddr:$serverip:$gatewayip:$netmask:$hostname:$netdev:any "        \
   "console=$consoledev,$baudrate $othbootargs;\0"                          \
   "ubifs_args=setenv bootargs root=ubi0:rootfs rw rootfstype=ubifs "       \
   "ubi.mtd=$nor_ubifs_root "                                               \
   "ip=$ipaddr:$serverip:$gatewayip:$netmask:$hostname:$netdev:any "        \
   "console=$consoledev,$baudrate $othbootargs;\0"                          \
   "nand_jffs2_args=setenv bootargs root=$nand_jffs2_root rw "              \
   "rootfstype=jffs2 "                                                      \
   "ip=$ipaddr:$serverip:$gatewayip:$netmask:$hostname:$netdev:any "        \
   "console=$consoledev,$baudrate $othbootargs;\0"                          \
   "nand_ubifs_args=setenv bootargs root=ubi0:rootfs rw rootfstype=ubifs "  \
   "ubi.mtd=$nand_ubifs_root,2048 "                                         \
   "ip=$ipaddr:$serverip:$gatewayip:$netmask:$hostname:$netdev:any "        \
   "console=$consoledev,$baudrate $othbootargs;\0"                          \
   "nfs_args=setenv bootargs root=/dev/nfs rw nfsroot=$serverip:$rootpath " \
   "ip=$ipaddr:$serverip:$gatewayip:$netmask:$hostname:$netdev:any "        \
   "console=$consoledev,$baudrate $othbootargs;"                            \

/* Define CONFIG_BOOTCOMMAND as
 * "run nand_ubifs" for boot from NAND flash with ubifs filesystem
 * "run nand_jffs2" for boot from NAND flash with jffs2 filesystem
 * "run flash_ubifs" for boot from NOR flash with ubifs filesystem
 * "run flash_jffs2" for boot from NOR flash with jffs2 filesystem
 * "run fixed_nfs" for standard NFS with fixed IP address.
 */
#if defined(CONFIG_CMD_NAND)
    #if defined(CONFIG_USE_UBIFS)
        #define CONFIG_BOOTCOMMAND  "run nand_ubifs"
    #else
        #define CONFIG_BOOTCOMMAND  "run nand_jffs2"
    #endif
#else /* !defined(CONFIG_CMD_NAND) */
    #if defined(CONFIG_USE_UBIFS)
        #define CONFIG_BOOTCOMMAND  "run flash_ubifs"
    #else
        #define CONFIG_BOOTCOMMAND  "run flash_jffs2"
    #endif
#endif

#endif /* __CONFIG_PC7302_H */
