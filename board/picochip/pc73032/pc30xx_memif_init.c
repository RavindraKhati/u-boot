/*!
 * \file memif_initialise.c
 * \brief Memory interface setup for PC30xx devices.
 *
 * Copyright (c) 2010-2012 Picochip Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * All enquiries to support@picochip.com
 */

/* Includes ---------------------------------------------------------------- */
#include <common.h>
#include <asm/arch/picoxcell.h>
#include <asm/arch/utilities.h>
#include <asm/arch/axi2cfg.h>
#include <asm/arch/pc30xx_mem_arm.h>
#include <asm/arch/pc30xx_mem_shd.h>

/* Macros ------------------------------------------------------------------ */
#define MEMIF_SHD_AEID      (0x0068)
/*
 * Timing Parameters from Spreadsheet MS-111258-RP
 *
 * These #define values have been obtained via a cut 'n' paste from
 * the spreadsheet
 */
#define DTUAWDT_VALUE (0x0000000B)
#define MCFG_VALUE (0x00040021)
#define TAL_VALUE (0x00000000)
#define TCKE_VALUE (0x00000003)
#define TCKESR_VALUE (0x00000004)
#define TCKSRE_VALUE (0x00000006)
#define TCKSRX_VALUE (0x00000006)
#define TCL_VALUE (0x00000006)
#define TCWL_VALUE (0x00000005)
#define TDPD_VALUE (0x00000000)
#define TDQS_VALUE (0x00000001)
#define TEXSR_VALUE (0x00000200)
#define TINIT_VALUE (0x000000C8)
#define TMOD_VALUE (0x0000000C)
#define TMRD_VALUE (0x00000004)
#define TMRR_VALUE (0x00000001)
#define TOGCNT100N_VALUE (0x00000036)
#define TOGCNT1U_VALUE (0x00000215)
#define TRAS_VALUE (0x00000014)
#define TRC_VALUE (0x0000001C)
#define TRCD_VALUE (0x00000008)
#define TREFI_VALUE (0x0000004E)
#define TRFC_VALUE (0x0000003B)
#define TRP_VALUE (0x00000008)
#define TRRD_VALUE (0x00000004)
#define TRSTH_VALUE (0x00000000)
#define TRSTL_VALUE (0x00000036)
#define TRTP_VALUE (0x00000004)
#define TRTW_VALUE (0x00000002)
#define TWR_VALUE (0x00000008)
#define TWTR_VALUE (0x00000004)
#define TXP_VALUE (0x00000004)
#define TXPDLL_VALUE (0x0000000D)
#define TZQCL_VALUE (0x00000200)
#define TZQCS_VALUE (0x00000040)
#define TZQCSI_VALUE (0x00000064)

#define ACIOCCR_VALUE (0xB0400812)
#define DCR_VALUE (0x0000000B)
#define DTPR0_VALUE (0x38948890)
#define DTPR1_VALUE (0x093B00C0)
#define DTPR2_VALUE (0x1001B600)
#define DXCCR_VALUE (0x00000F71)
#define MR0_VALUE (0x00000820)
#define MR1_VALUE (0x00000006)
#define MR2_VALUE (0x00000000)
#define MR3_VALUE (0x00000000)
#define PTR0_VALUE (0x0022AA5B)
#define PTR1_VALUE (0x06041104)
#define PTR2_VALUE (0x042DA072)

#define SDRAM_AXI_CONFIG_VALUE (0x0000000A)
#define SDRAM_SETUP_VALUE (0x00000005)


/* Drift Compensation Disable - not in spreadsheet as not a timing parameter */
/* but value advised by Mindspeed silicon team */
#define PGCR_VALUE (0x01842E08)


/* Constants --------------------------------------------------------------- */

/* Types ------------------------------------------------------------------- */

/* Prototypes--------------------------------------------------------------- */
/*!
 * \brief Read a 32 bit value from the 'apb' on memif shared
 *
 * \param base The block base address
 * \param addr The register address offset
 * \param data Store the read value
 */
static void memif_shd_apb_read (unsigned int base, unsigned short addr,
				unsigned int *data);

/*!
 * \brief Poll for a value from the 'apb' on memif shared
 *
 * \param base The block base address
 * \param addr The register address offset
 * \param mask The data bits we are interested in
 * \param data The data bit value we are interested in
 */
static void memif_shd_apb_poll (unsigned int base, unsigned short addr,
				unsigned int mask, unsigned int data);

/*!
 * \brief Write a 32 bit value to the 'apb' on memif shared
 *
 * \param base The block base address
 * \param addr The register address offset
 * \param data The value to write
 */
static void memif_shd_apb_write (unsigned int base, unsigned short addr,
				 unsigned int data);

/*!
 * \brief Read a 32 bit value from the 'apb' in the arm subsystem
 *
 * \param base The block base address
 * \param addr The register address offset
 * \param data Store the read value
 */
static void memif_arm_apb_read (unsigned int base, unsigned int addr,
				unsigned int *data);

/*!
 * \brief Poll for a value from the 'apb' in the arm subsystem
 *
 * \param base The block base address
 * \param addr The register address offset
 * \param mask The data bits we are interested in
 * \param data The data bit value we are interested in
 */
static void memif_arm_apb_poll (unsigned int base, unsigned int addr,
				unsigned int mask, unsigned int data);

/*!
 * \brief Write a 32 bit value to the 'apb' in the arm subsystem
 *
 * \param base The block base address
 * \param addr The register address offset
 * \param data The value to write
 */
static void memif_arm_apb_write (unsigned int base, unsigned int addr,
				 unsigned int data);

/* Functions -------------------------------------------------------------- */

/* ------------------------------------------------------------------------ */
/*  PC30xx MEMIF_SHD initialisation routines                                */
/* ------------------------------------------------------------------------ */
static void memif_shd_apb_read (unsigned int base, unsigned short addr,
				unsigned int *data)
{

	unsigned short read_value_upr = 0;
	unsigned short read_value_lwr = 0;

	axi2cfg_config_read (MEMIF_SHD_AEID, (base + (addr >> 1) + 1),
			     &read_value_upr, 1);
	axi2cfg_config_read (MEMIF_SHD_AEID, (base + (addr >> 1)),
			     &read_value_lwr, 1);

	*data = (read_value_upr << 16) | read_value_lwr;
}

static void memif_shd_apb_poll (unsigned int base, unsigned short addr,
				unsigned int mask, unsigned int data)
{
	unsigned int read_value;

	do {
		memif_shd_apb_read (base, addr, &read_value);
	} while ((read_value & mask) != data);
}

static void memif_shd_apb_write (unsigned int base, unsigned short addr,
				 unsigned int data)
{

	unsigned short value_upr;
	unsigned short value_lwr;

	value_upr = data >> 16;
	value_lwr = data & 0xFFFF;

	axi2cfg_config_write (MEMIF_SHD_AEID, (base + (addr >> 1)), &value_lwr,
			      1);
	axi2cfg_config_write (MEMIF_SHD_AEID, (base + (addr >> 1) + 1),
			      &value_upr, 1);
}

void pc30xx_memif_shared_init (void)
{
	unsigned short value;

	picoxcell_clk_enable (PICOXCELL_MEMIF_SHD_SDRAM_CLOCK);

	/* Arbitration algorithm setup */

	/* wake up */
	value = 0x0000;
	axi2cfg_config_write (MEMIF_SHD_AEID, CBFM_SLEEPREG_ADDR, &value, 1);

	/* configure the sdram arbitration for all banks */
	value = SDRAM_ARB_DATA;
	axi2cfg_config_write (MEMIF_SHD_AEID,
			      MEMIF_SHD_REGS_SDRAM_ARB_G0_S0_S1_OFFSET,
			      &value, 1);
	axi2cfg_config_write (MEMIF_SHD_AEID,
			      MEMIF_SHD_REGS_SDRAM_ARB_G0_S2_S3_OFFSET,
			      &value, 1);
	axi2cfg_config_write (MEMIF_SHD_AEID,
			      MEMIF_SHD_REGS_SDRAM_ARB_G1_S0_S1_OFFSET,
			      &value, 1);
	axi2cfg_config_write (MEMIF_SHD_AEID,
			      MEMIF_SHD_REGS_SDRAM_ARB_G1_S2_S3_OFFSET,
			      &value, 1);

	value = SDRAM_ARB_2_VALID_GRPS;
	axi2cfg_config_write (MEMIF_SHD_AEID,
			      MEMIF_SHD_REGS_SDRAM_VALID_GROUPS_OFFSET,
			      &value, 1);

	/* configure the sdram_setup and sdram_axi_config registers */
	value = SDRAM_SETUP_VALUE;
	axi2cfg_config_write (MEMIF_SHD_AEID,
			      MEMIF_SHD_REGS_SDRAM_SETUP_OFFSET, &value, 1);
	value = SDRAM_AXI_CONFIG_VALUE;
	axi2cfg_config_write (MEMIF_SHD_AEID,
			      MEMIF_SHD_REGS_SDRAM_AXI_CONFIG_OFFSET, &value, 1);

	/* Bank opportunistic mode disabled,
	 * Bank row pre-charge mode enabled
	 */
	value = 0x2;
	axi2cfg_config_write (MEMIF_SHD_AEID,
			      MEMIF_SHD_REGS_SDRAM_ARB_CFG_OFFSET, &value, 1);

	/* hit the go button on the sdram arbitration */
	value = SDRAM_CFG_DONE_DATA;
	axi2cfg_config_write (MEMIF_SHD_AEID,
			      MEMIF_SHD_REGS_SDRAM_CFG_DONE_OFFSET, &value, 1);

	/* PCTL and PUBL setup */
	memif_shd_apb_write (MEMIF_SHD_PCTL_BASE, PCTL_DTUAWDT_OFFSET,
                             DTUAWDT_VALUE);

	/* 1. uPCTL Memory Timing Registers configuration */
	memif_shd_apb_write (MEMIF_SHD_PCTL_BASE, PCTL_TOGCNT1U_OFFSET,
			     TOGCNT1U_VALUE);
	memif_shd_apb_write (MEMIF_SHD_PCTL_BASE, PCTL_TINIT_OFFSET,
			     TINIT_VALUE);
	memif_shd_apb_write (MEMIF_SHD_PCTL_BASE, PCTL_TRSTH_OFFSET,
			     TRSTH_VALUE);
	memif_shd_apb_write (MEMIF_SHD_PCTL_BASE, PCTL_TOGCNT100N_OFFSET,
			     TOGCNT100N_VALUE);
	memif_shd_apb_write (MEMIF_SHD_PCTL_BASE, PCTL_TZQCL_OFFSET,
                             TZQCL_VALUE);
	memif_shd_apb_write (MEMIF_SHD_PCTL_BASE, PCTL_TZQCS_OFFSET,
                             TZQCS_VALUE);
	memif_shd_apb_write (MEMIF_SHD_PCTL_BASE, PCTL_TZQCSI_OFFSET,
                             TZQCSI_VALUE);
        memif_shd_apb_write (MEMIF_SHD_PUBL_BASE, PUBL_PTR0_OFFSET,
                             PTR0_VALUE);
        memif_shd_apb_write (MEMIF_SHD_PUBL_BASE, PUBL_PTR1_OFFSET,
                             PTR1_VALUE);
        memif_shd_apb_write (MEMIF_SHD_PUBL_BASE, PUBL_PTR2_OFFSET,
                             PTR2_VALUE);

        /* 2. PHY initialisation start */
        memif_shd_apb_write (MEMIF_SHD_PUBL_BASE, PUBL_PGCR_OFFSET,
                             PGCR_VALUE);
	memif_shd_apb_write (MEMIF_SHD_PUBL_BASE, PUBL_PIR_OFFSET,
			     PUBL_PIR_CLRSR);
	memif_shd_apb_write (MEMIF_SHD_PUBL_BASE, PUBL_PIR_OFFSET,
			     (PUBL_PIR_INIT | PUBL_PIR_DLLSRST |
			      PUBL_PIR_DLLLOCK));

	/* 3. Wait for PHY initialisation following reset */
	memif_shd_apb_poll (MEMIF_SHD_PUBL_BASE, PUBL_PGSR_OFFSET,
			    0x0001, 0x0001);

	/* 4. DFI initialisation status, wait for dfi_init_complete */
	memif_shd_apb_poll (MEMIF_SHD_PCTL_BASE, PCTL_DFISTSTAT0_OFFSET,
			    0x0001, 0x0001);

	/* 5. uPTCL MCFG register configuration */
        memif_shd_apb_write (MEMIF_SHD_PCTL_BASE, PCTL_MCFG_OFFSET,
			     MCFG_VALUE);

        /* 6. uPCTL Power-up sequence */
	memif_shd_apb_write (MEMIF_SHD_PCTL_BASE, PCTL_POWCTL_OFFSET, 0x0001);
	memif_shd_apb_poll (MEMIF_SHD_PCTL_BASE, PCTL_POWSTAT_OFFSET,
			    0x0001, 0x0001);

	/* 7. uPCTL timing register setup */
	memif_shd_apb_write (MEMIF_SHD_PCTL_BASE, PCTL_TREFI_OFFSET,
			     TREFI_VALUE);
        memif_shd_apb_write (MEMIF_SHD_PCTL_BASE, PCTL_TMRD_OFFSET,
			     TMRD_VALUE);
        memif_shd_apb_write (MEMIF_SHD_PCTL_BASE, PCTL_TRFC_OFFSET,
			     TRFC_VALUE);
        memif_shd_apb_write (MEMIF_SHD_PCTL_BASE, PCTL_TRP_OFFSET, TRP_VALUE);
        memif_shd_apb_write (MEMIF_SHD_PCTL_BASE, PCTL_TCL_OFFSET, TCL_VALUE);
        memif_shd_apb_write (MEMIF_SHD_PCTL_BASE, PCTL_TCWL_OFFSET,
			     TCWL_VALUE);
        memif_shd_apb_write (MEMIF_SHD_PCTL_BASE, PCTL_TRAS_OFFSET,
			     TRAS_VALUE);
        memif_shd_apb_write (MEMIF_SHD_PCTL_BASE, PCTL_TRC_OFFSET, TRC_VALUE);
        memif_shd_apb_write (MEMIF_SHD_PCTL_BASE, PCTL_TRCD_OFFSET,
			     TRCD_VALUE);
        memif_shd_apb_write (MEMIF_SHD_PCTL_BASE, PCTL_TRTP_OFFSET,
			     TRTP_VALUE);
	memif_shd_apb_write (MEMIF_SHD_PCTL_BASE, PCTL_TWR_OFFSET, TWR_VALUE);
        memif_shd_apb_write (MEMIF_SHD_PCTL_BASE,
			     PCTL_DFIPHYWRLAT_OFFSET, (TCWL_VALUE - 1));
        memif_shd_apb_write (MEMIF_SHD_PCTL_BASE,
			     PCTL_DFITRDDATEN_OFFSET, (TCL_VALUE - 2));
	memif_shd_apb_write (MEMIF_SHD_PCTL_BASE, PCTL_TAL_OFFSET, TAL_VALUE);
	memif_shd_apb_write (MEMIF_SHD_PCTL_BASE, PCTL_TCKE_OFFSET,
                             TCKE_VALUE);
	memif_shd_apb_write (MEMIF_SHD_PCTL_BASE, PCTL_TCKESR_OFFSET,
                             TCKESR_VALUE);
	memif_shd_apb_write (MEMIF_SHD_PCTL_BASE, PCTL_TCKSRE_OFFSET,
                             TCKSRE_VALUE);
	memif_shd_apb_write (MEMIF_SHD_PCTL_BASE, PCTL_TCKSRX_OFFSET,
                             TCKSRX_VALUE);
	memif_shd_apb_write (MEMIF_SHD_PCTL_BASE, PCTL_TDPD_OFFSET,
                             TDPD_VALUE);
	memif_shd_apb_write (MEMIF_SHD_PCTL_BASE, PCTL_TDQS_OFFSET,
                             TDQS_VALUE);
	memif_shd_apb_write (MEMIF_SHD_PCTL_BASE, PCTL_TEXSR_OFFSET,
                             TEXSR_VALUE);
	memif_shd_apb_write (MEMIF_SHD_PCTL_BASE, PCTL_TMOD_OFFSET,
                             TMOD_VALUE);
	memif_shd_apb_write (MEMIF_SHD_PCTL_BASE, PCTL_TMRR_OFFSET,
                             TMRR_VALUE);
	memif_shd_apb_write (MEMIF_SHD_PCTL_BASE, PCTL_TRRD_OFFSET,
                             TRRD_VALUE);
	memif_shd_apb_write (MEMIF_SHD_PCTL_BASE, PCTL_TRSTL_OFFSET,
                             TRSTL_VALUE);
	memif_shd_apb_write (MEMIF_SHD_PCTL_BASE, PCTL_TRTW_OFFSET,
                             TRTW_VALUE);
	memif_shd_apb_write (MEMIF_SHD_PCTL_BASE, PCTL_TWTR_OFFSET,
                             TWTR_VALUE);
        memif_shd_apb_write (MEMIF_SHD_PCTL_BASE, PCTL_TXP_OFFSET, TXP_VALUE);
	memif_shd_apb_write (MEMIF_SHD_PCTL_BASE, PCTL_TXPDLL_OFFSET,
                             TXPDLL_VALUE);

        /* 8. DRAM initialisation via PUBL */
        memif_shd_apb_write (MEMIF_SHD_PUBL_BASE, PUBL_ACIOCCR_OFFSET, 
                             ACIOCCR_VALUE);
        memif_shd_apb_write (MEMIF_SHD_PUBL_BASE, PUBL_DXCCR_OFFSET, 
                             DXCCR_VALUE);

        memif_shd_apb_write (MEMIF_SHD_PUBL_BASE, PUBL_DCR_OFFSET, DCR_VALUE);

        memif_shd_apb_write (MEMIF_SHD_PUBL_BASE, PUBL_DTPR0_OFFSET,
                             DTPR0_VALUE);
        memif_shd_apb_write (MEMIF_SHD_PUBL_BASE, PUBL_DTPR1_OFFSET,
                             DTPR1_VALUE);
        memif_shd_apb_write (MEMIF_SHD_PUBL_BASE, PUBL_DTPR2_OFFSET,
                             DTPR2_VALUE);

        memif_shd_apb_write (MEMIF_SHD_PUBL_BASE, PUBL_MR2_OFFSET, MR2_VALUE);
        memif_shd_apb_write (MEMIF_SHD_PUBL_BASE, PUBL_MR3_OFFSET, MR3_VALUE);
        memif_shd_apb_write (MEMIF_SHD_PUBL_BASE, PUBL_MR1_OFFSET, MR1_VALUE);
        memif_shd_apb_write (MEMIF_SHD_PUBL_BASE, PUBL_MR0_OFFSET, MR0_VALUE);

	/* PHY timing register */
	memif_shd_apb_write (MEMIF_SHD_PUBL_BASE, PUBL_PIR_OFFSET,
			     PUBL_PIR_CLRSR);

	memif_shd_apb_write (MEMIF_SHD_PUBL_BASE, PUBL_PIR_OFFSET,
			     (PUBL_PIR_INIT |
			      PUBL_PIR_ITMSRST |
			      PUBL_PIR_DRAMRST |
			      PUBL_PIR_DRAMINIT | PUBL_PIR_QSTRN));

	memif_shd_apb_poll (MEMIF_SHD_PUBL_BASE, PUBL_PGSR_OFFSET,
			    0x0001, 0x0001);

	/* 9. Move uPCTL to Config state */
	memif_shd_apb_write (MEMIF_SHD_PCTL_BASE, PCTL_SCTL_OFFSET, 0x0001);
	memif_shd_apb_poll (MEMIF_SHD_PCTL_BASE, PCTL_STAT_OFFSET,
			    0x0007, 0x0001);

	/* 10. Further uPCTL configuration */

	/* 11. Wait for uPCTL initialisation to complete */
	memif_shd_apb_write (MEMIF_SHD_PCTL_BASE, PCTL_CMDSTATEN_OFFSET,
			     0x0001);
	memif_shd_apb_poll (MEMIF_SHD_PCTL_BASE, PCTL_CMDSTAT_OFFSET,
			    0x0001, 0x0001);

	/* 12. PUBL training */

	/* 13. Move uPCTL to Access state */
	memif_shd_apb_write (MEMIF_SHD_PCTL_BASE, PCTL_SCTL_OFFSET, 0x0002);
	memif_shd_apb_poll (MEMIF_SHD_PCTL_BASE, PCTL_STAT_OFFSET,
			    0x0007, 0x0003);
}

/* ------------------------------------------------------------------------ */
/*  PC30xx MEMIF_ARM initialisation routines                                */
/* ------------------------------------------------------------------------ */

static void memif_arm_apb_read (unsigned int base, unsigned int addr,
				unsigned int *data)
{
	*data = *(volatile unsigned int *)(base + addr);
}

static void memif_arm_apb_poll (unsigned int base, unsigned int addr,
				unsigned int mask, unsigned int data)
{
	unsigned int read_value;

	do {
		memif_arm_apb_read (base, addr, &read_value);
	} while ((read_value & mask) != data);
}

static void memif_arm_apb_write (unsigned int base,
				 unsigned int addr, unsigned int data)
{
	*(volatile unsigned int *)(base + addr) = data;
}

void pc30xx_memif_arm_init (void)
{
	/* PCTL MCFG Register */
	unsigned int tfaw_cfg = 1;	/* 2=t_faw=5*t_rrd */
	unsigned int ddr3_en = 1;	/* 0=DDR2, 1=DDR3 */
	unsigned int mem_bl = 1;	/* 0=BL4, 1=BL8 */

	/* PUBL DCR Register */
	unsigned int ddrmd = 3;
	unsigned int ddr8bnk = 1;

	/* PCTL TIMING Registers */
	unsigned int togcnt1u = 534;
	unsigned int t_init = 200;
	unsigned int t_rsth = 500;
	unsigned int togcnt100n = 53;
	unsigned int t_refi = 70;
	unsigned int t_mrd = 4;
	unsigned int t_rfc = 105;
	unsigned int t_rp = 9;
	unsigned int prea_extra = 2;
	unsigned int t_cl = 8;
	unsigned int t_cwl = 6;
	unsigned int t_ras = 20;
	unsigned int t_rc = 29;
	unsigned int t_rcd = t_cl;
	unsigned int t_rtp = 4;
	unsigned int t_wr = 8;

	/* cas latency */
	unsigned int mrd_ddr3_cl = 4;

	/* write recovery */
	unsigned int mrd_ddr3_wr = 4;
	unsigned int mr2_ddr3_cwl = 1;

	if (!is_memif_arm_usable_on_pc30xx ()) {
		return;
	}

	/* Make sure the memory interface is being clocked */
	picoxcell_clk_enable (PICOXCELL_MEMIF_ARM_CLOCK);

	/* MCTL setup */
	memif_arm_apb_write (MEMIF_ARM_PCTL_BASE,
			     MCTL_DCFG_OFFSET,
			     (MCTL_DCFG_IO_WIDTH_16BIT <<
			      MCTL_DCFG_IO_WIDTH_IDX) |
			     (MCTL_DCFG_DRAM_DENSITY_1Gb <<
			      MCTL_DCFG_DRAM_DENSITY_IDX) |
			     (MCTL_DCFG_DRAM_TYPE_13x10 <<
			      MCTL_DCFG_DRAM_TYPE_IDX) |
			     (MCTL_DCFG_ADDR_MAP_BRC << MCTL_DCFG_ADDR_MAP_IDX)
	    );

	memif_arm_apb_write (MEMIF_ARM_PCTL_BASE,
			     MCTL_PCFG_0_OFFSET,
			     (0x1 << MCTL_PCFG_BP_RD_EN_IDX) |
			     (0x1 << MCTL_PCFG_BP_WR_EN_IDX)
	    );

	memif_arm_apb_write (MEMIF_ARM_PCTL_BASE,
			     MCTL_PCFG_1_OFFSET,
			     (0x1 << MCTL_PCFG_BP_RD_EN_IDX) |
			     (0x1 << MCTL_PCFG_BP_WR_EN_IDX)
	    );

	memif_arm_apb_write (MEMIF_ARM_PCTL_BASE,
			     MCTL_PCFG_2_OFFSET,
			     (0x1 << MCTL_PCFG_BP_RD_EN_IDX) |
			     (0x1 << MCTL_PCFG_BP_WR_EN_IDX)
	    );

	memif_arm_apb_write (MEMIF_ARM_PCTL_BASE,
			     MCTL_PCFG_3_OFFSET,
			     (0x1 << MCTL_PCFG_BP_RD_EN_IDX) |
			     (0x1 << MCTL_PCFG_BP_WR_EN_IDX)
	    );

	/* PCTL and PUBL setup */

	/* Set up the row, bank, col configuration for the attached sdram */
	/* rank = 1
	 * row addr = 13 bits
	 * col addr = 10 bits
	 * banks = 8
	 */
	memif_arm_apb_write (MEMIF_ARM_PCTL_BASE, PCTL_DTUAWDT_OFFSET, 0x0B);

	/* 1. uPCTL timer reg configuration */
	memif_arm_apb_write (MEMIF_ARM_PCTL_BASE,
			     PCTL_TOGCNT1U_OFFSET, togcnt1u);
	memif_arm_apb_write (MEMIF_ARM_PCTL_BASE, PCTL_TINIT_OFFSET, t_init);
	memif_arm_apb_write (MEMIF_ARM_PCTL_BASE, PCTL_TRSTH_OFFSET, t_rsth);
	memif_arm_apb_write (MEMIF_ARM_PCTL_BASE,
			     PCTL_TOGCNT100N_OFFSET, togcnt100n);

	/* 2. PHY initialisation start
	 * - not required, PUBL initiates automatically following reset
	 */
	memif_arm_apb_write (MEMIF_ARM_PUBL_BASE, PUBL_PIR_OFFSET,
			     PUBL_PIR_CLRSR);

	memif_arm_apb_write (MEMIF_ARM_PUBL_BASE, PUBL_PIR_OFFSET,
			     (PUBL_PIR_INIT | PUBL_PIR_DLLSRST |
			      PUBL_PIR_DLLLOCK));

	/* 3. Wait for PHY initialisation following reset
	 * (DLL lock and Zo calibration)
	 */
	memif_arm_apb_poll (MEMIF_ARM_PUBL_BASE,
			    PUBL_PGSR_OFFSET, 0x0001, 0x0001);

	/* 4. DFI initialisation status, wait for dfi_init_complete */
	memif_arm_apb_poll (MEMIF_ARM_PCTL_BASE,
			    PCTL_DFISTSTAT0_OFFSET, 0x0001, 0x0001);

	/* 5. uPTCL MCFG register configuration */
	memif_arm_apb_write (MEMIF_ARM_PCTL_BASE,
			     PCTL_MCFG_OFFSET,
			     (tfaw_cfg << 18) | (ddr3_en << 5) | (mem_bl << 0));

	/* 6. uPCTL Power-up sequence */
	memif_arm_apb_write (MEMIF_ARM_PCTL_BASE, PCTL_POWCTL_OFFSET, 0x0001);
	memif_arm_apb_poll (MEMIF_ARM_PCTL_BASE,
			    PCTL_POWSTAT_OFFSET, 0x0001, 0x0001);

	/* 7. uPCTL timing register setup */
	memif_arm_apb_write (MEMIF_ARM_PCTL_BASE, PCTL_TREFI_OFFSET, t_refi);
	memif_arm_apb_write (MEMIF_ARM_PCTL_BASE, PCTL_TMRD_OFFSET, t_mrd);
	memif_arm_apb_write (MEMIF_ARM_PCTL_BASE, PCTL_TRFC_OFFSET, t_rfc);
	memif_arm_apb_write (MEMIF_ARM_PCTL_BASE,
			     PCTL_TRP_OFFSET, (t_rp << 0) | (prea_extra << 16));
	memif_arm_apb_write (MEMIF_ARM_PCTL_BASE, PCTL_TCL_OFFSET, t_cl);
	memif_arm_apb_write (MEMIF_ARM_PCTL_BASE, PCTL_TCWL_OFFSET, t_cwl);
	memif_arm_apb_write (MEMIF_ARM_PCTL_BASE, PCTL_TRAS_OFFSET, t_ras);
	memif_arm_apb_write (MEMIF_ARM_PCTL_BASE, PCTL_TRC_OFFSET, t_rc);
	memif_arm_apb_write (MEMIF_ARM_PCTL_BASE, PCTL_TRCD_OFFSET, t_rcd);
	memif_arm_apb_write (MEMIF_ARM_PCTL_BASE, PCTL_TRTP_OFFSET, t_rtp);
	memif_arm_apb_write (MEMIF_ARM_PCTL_BASE, PCTL_TWR_OFFSET, t_wr);
	memif_arm_apb_write (MEMIF_ARM_PCTL_BASE,
			     PCTL_DFIPHYWRLAT_OFFSET, (t_cwl - 1));
	memif_arm_apb_write (MEMIF_ARM_PCTL_BASE,
			     PCTL_DFITRDDATEN_OFFSET, (t_cl - 2));

	/* 8. DRAM initialisation via PUBL */
	/* DRAM configuration register */
	memif_arm_apb_write (MEMIF_ARM_PUBL_BASE,
			     PUBL_DCR_OFFSET, (ddrmd << 0) | (ddr8bnk << 3));

	/* DRAM mode register 0 (DDR3) */
	memif_arm_apb_write (MEMIF_ARM_PUBL_BASE, PUBL_MR0_OFFSET, (mrd_ddr3_cl << 4) |	/* burst length */
			     (mrd_ddr3_wr << 9));	/* write recover */

	/* DRAM mode register 1 */
	memif_arm_apb_write (MEMIF_ARM_PUBL_BASE, PUBL_MR1_OFFSET, 0x0000);

	/* DRAM mode register 2 */
	memif_arm_apb_write (MEMIF_ARM_PUBL_BASE, PUBL_MR2_OFFSET,
			     (mr2_ddr3_cwl << 3));

	/* PHY timing register */
	memif_arm_apb_write (MEMIF_ARM_PUBL_BASE, PUBL_PIR_OFFSET,
			     PUBL_PIR_CLRSR);

	memif_arm_apb_write (MEMIF_ARM_PUBL_BASE, PUBL_PIR_OFFSET,
			     (PUBL_PIR_INIT |
			      PUBL_PIR_ITMSRST |
			      PUBL_PIR_DRAMRST |
			      PUBL_PIR_DRAMINIT | PUBL_PIR_QSTRN));

	memif_arm_apb_poll (MEMIF_ARM_PUBL_BASE, PUBL_PGSR_OFFSET, 0x0001,
			    0x0001);

	/* 9. Move uPCTL to Config state */
	memif_arm_apb_write (MEMIF_ARM_PCTL_BASE, PCTL_SCTL_OFFSET, 0x0001);
	memif_arm_apb_poll (MEMIF_ARM_PCTL_BASE,
			    PCTL_STAT_OFFSET, 0x0007, 0x0001);

	/* 10. Further uPCTL configuration */

	/* 11. Wait for uPCTL initialisation to complete */
	memif_arm_apb_write (MEMIF_ARM_PCTL_BASE,
			     PCTL_CMDSTATEN_OFFSET, 0x0001);
	memif_arm_apb_poll (MEMIF_ARM_PCTL_BASE,
			    PCTL_CMDSTAT_OFFSET, 0x0001, 0x0001);

	/* 12. PUBL training */

	/* 13. Move uPCTL to Access state */
	memif_arm_apb_write (MEMIF_ARM_PCTL_BASE, PCTL_SCTL_OFFSET, 0x0002);
	memif_arm_apb_poll (MEMIF_ARM_PCTL_BASE,
			    PCTL_STAT_OFFSET, 0x0007, 0x0003);
}
