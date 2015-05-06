#include <common.h>
#include <compiler.h>
#include <fuse.h>
#include <asm/errno.h>
#include <asm/io.h>
#include <asm/immap_85xx.h>

#define IPA9131_LOADER_REVOCATION_MAX 12
#define IPA9131_APPLICATION_REVOCATION_MAX 28

#define SFP_INGR_ADDRESS	(CONFIG_SYS_IMMR + 0x000e7020)
#define SFP_DESSR_ADDRESS	(CONFIG_SYS_IMMR + 0x000e7024)
#define SFP_SFPCR_ADDRESS	(CONFIG_SYS_IMMR + 0x000e7028)
#define SFP_OSPR_ADDRESS	(CONFIG_SYS_IMMR + 0x000e7040)
#define SFP_FSWPR_ADDRESS	(CONFIG_SYS_IMMR + 0x000e7044)
#define SFP_DPR_ADDRESS		(CONFIG_SYS_IMMR + 0x000e7048)
#define SFP_DCVR0_ADDRESS	(CONFIG_SYS_IMMR + 0x000e704c)
#define SFP_DCVR1_ADDRESS	(CONFIG_SYS_IMMR + 0x000e7050)
#define SFP_DRVR0_ADDRESS	(CONFIG_SYS_IMMR + 0x000e7054)
#define SFP_DRVR1_ADDRESS	(CONFIG_SYS_IMMR + 0x000e7058)
#define SFP_OTPMKR0_ADDRESS	(CONFIG_SYS_IMMR + 0x000e705c)
#define SFP_OTPMKR1_ADDRESS	(CONFIG_SYS_IMMR + 0x000e7060)
#define SFP_OTPMKR2_ADDRESS	(CONFIG_SYS_IMMR + 0x000e7064)
#define SFP_OTPMKR3_ADDRESS	(CONFIG_SYS_IMMR + 0x000e7068)
#define SFP_OTPMKR4_ADDRESS	(CONFIG_SYS_IMMR + 0x000e706c)
#define SFP_OTPMKR5_ADDRESS	(CONFIG_SYS_IMMR + 0x000e7070)
#define SFP_OTPMKR6_ADDRESS	(CONFIG_SYS_IMMR + 0x000e7074)
#define SFP_OTPMKR7_ADDRESS	(CONFIG_SYS_IMMR + 0x000e7078)
#define SFP_SRKHR0_ADDRESS	(CONFIG_SYS_IMMR + 0x000e707c)
#define SFP_SRKHR1_ADDRESS	(CONFIG_SYS_IMMR + 0x000e7080)
#define SFP_SRKHR2_ADDRESS	(CONFIG_SYS_IMMR + 0x000e7084)
#define SFP_SRKHR3_ADDRESS	(CONFIG_SYS_IMMR + 0x000e7088)
#define SFP_SRKHR4_ADDRESS	(CONFIG_SYS_IMMR + 0x000e708c)
#define SFP_SRKHR5_ADDRESS	(CONFIG_SYS_IMMR + 0x000e7090)
#define SFP_SRKHR6_ADDRESS	(CONFIG_SYS_IMMR + 0x000e7094)
#define SFP_SRKHR7_ADDRESS	(CONFIG_SYS_IMMR + 0x000e7098)
#define SFP_OUIDR_ADDRESS	(CONFIG_SYS_IMMR + 0x000e709c)
#define SFP_OVPR_ADDRESS	(CONFIG_SYS_IMMR + 0x000e70a4)
#define SFP_OSCR_ADDRESS	(CONFIG_SYS_IMMR + 0x000e70a8)
#define SFP_ONSEC_ADDRESS	(CONFIG_SYS_IMMR + 0x000e70ac)
#define SFP_FUIDR_ADDRESS	(CONFIG_SYS_IMMR + 0x000e70b0)
#define SFP_FSCR_ADDRESS	(CONFIG_SYS_IMMR + 0x000e70b8)
#define SFP_IBRConfig_ADDRESS	(CONFIG_SYS_IMMR + 0x000e70bc)

#define SFP_LOWEST_ADDRESS	SFP_DESSR_ADDRESS
#define SFP_HIGHEST_ADDRESS	SFP_IBRConfig_ADDRESS


#define fuse_in_be32(x)	in_be32((const volatile unsigned __iomem *)(x))
#define fuse_out_be32(x,y) out_be32((volatile unsigned __iomem *)(x),(y))




int ipa9131_fuse_init(void)
{
	fuse_out_be32(SFP_INGR_ADDRESS, 0x1);
	return 0;
}


int ipa9131_fuse_read_eid(u64 * eid)
{
	u64 r0;
	u64 r1;

	if (!eid)
		return -EINVAL;

	r0 = fuse_in_be32(SFP_DCVR0_ADDRESS);
	r1 = fuse_in_be32(SFP_DCVR1_ADDRESS);

	*eid = ((r0 << 32) & 0xffffffff00000000) |
		(r1 & 0x00000000ffffffff);
	return 0;
}


int ipa9131_fuse_read_security_profile(u8 * p, u8 * d, u8 * s)
{
	u32 r;

	if (!p || !d || !s)
		return -EINVAL;

	r = fuse_in_be32(SFP_OUIDR_ADDRESS);

	*p = (r & 0x01) ? 1 : 0;
	*d = (r & 0x02) ? 1 : 0;
	*s = (r & 0x04) ? 1 : 0;
	return 0;
}


int ipa9131_fuses_are_write_protected(void)
{
	return (fuse_in_be32(SFP_OSPR_ADDRESS) & 0x01) ? 1 : 0;
}


int ipa9131_fuse_should_be_silent(void)
{
	if (!ipa9131_fuses_are_write_protected())
		return 0;

	return (fuse_in_be32(SFP_OUIDR_ADDRESS) & 0x01) ? 1 : 0;
}


int ipa9131_fuse_read_loader_revocation_value(u16 * r)
{
	u32 r0;

	if (!r)
		return -EINVAL;

	r0 = fuse_in_be32(SFP_OUIDR_ADDRESS);
	r0 >>= 4;
	*r = r0 & 0xfff;
	return 0;
}


int ipa9131_fuse_read_application_revocation_value(u32 * r)
{
	u32 r0;
	u32 r1;
	u32 r2;

	if (!r)
		return -EINVAL;

	r0 = fuse_in_be32(SFP_OUIDR_ADDRESS);
	r1 = fuse_in_be32(SFP_DCVR0_ADDRESS);
	r2 = fuse_in_be32(SFP_DCVR1_ADDRESS);
	*r = (((r0 >> 16) & 0x0000ffff) | ((r1 << 16) & 0x00ff0000) | ((r2 >>  4) & 0x0f000000)) & 0x0fffffff;
	return 0;
}


static u32 determine_fused_bit_count(u32 N, u32 M)
{
	u32 i;

	for (i = N; i > 0; --i)
	{
		if (M & (1U << (i - 1)))
		{
			return i;
		}
	}

	return 0;
}


u16 ipa9131_fuse_read_loader_revocation(void)
{
	u16 constructed;

	if (0 != ipa9131_fuse_read_loader_revocation_value(&constructed))
		return IPA9131_LOADER_REVOCATION_MAX;

	return determine_fused_bit_count(IPA9131_LOADER_REVOCATION_MAX, constructed);
}


u32 ipa9131_fuse_read_application_revocation(void)
{
	u32 constructed;

	if (0 != ipa9131_fuse_read_application_revocation_value(&constructed))
		return IPA9131_APPLICATION_REVOCATION_MAX;

	return determine_fused_bit_count(IPA9131_APPLICATION_REVOCATION_MAX, constructed);
}


int ipa9131_is_unfused(void)
{
	if (fuse_in_be32(SFP_OUIDR_ADDRESS) ||
	    fuse_in_be32(SFP_DCVR0_ADDRESS) ||
	    fuse_in_be32(SFP_DCVR1_ADDRESS) ||
	    fuse_in_be32(SFP_SRKHR0_ADDRESS) ||
	    fuse_in_be32(SFP_SRKHR1_ADDRESS) ||
	    fuse_in_be32(SFP_SRKHR2_ADDRESS) ||
	    fuse_in_be32(SFP_SRKHR3_ADDRESS) ||
	    fuse_in_be32(SFP_SRKHR4_ADDRESS) ||
	    fuse_in_be32(SFP_SRKHR5_ADDRESS) ||
	    fuse_in_be32(SFP_SRKHR6_ADDRESS) ||
	    fuse_in_be32(SFP_SRKHR7_ADDRESS))
		return 0;

	return 1;
}



#if defined(CONFIG_CMD_IPA9131_FUSE)
int do_ipa9131_fuse(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	if (argc != 2)
		return CMD_RET_USAGE;

	if (0 == strcmp(argv[1], "init")) {
		if (0 != ipa9131_fuse_init()) {
			return CMD_RET_FAILURE;
		}

		printf("init OK\n");
		return CMD_RET_SUCCESS;
	}

	if (0 == strcmp(argv[1], "eid")) {
		u64 eid;
		u8 oui0;
		u8 oui1;
		u8 oui2;
		u32 serial;

		if (0 != ipa9131_fuse_read_eid(&eid)) {
			return CMD_RET_FAILURE;
		}

		oui0 = (eid >> 56) & 0xff;
		oui1 = (eid >> 48) & 0xff;
		oui2 = (eid >> 40) & 0xff;
		serial = eid & 0x00000000ffffffff;

		printf("EID: %02X%02X%02X-%010u\n", oui0, oui1, oui2, serial);
		return CMD_RET_SUCCESS;
	}

	if (0 == strcmp(argv[1], "security")) {
		u8 p;
		u8 d;
		u8 s;

		if (0 != ipa9131_fuse_read_security_profile(&p, &d, &s)) {
			return CMD_RET_FAILURE;
		}

		printf("security profile: prod <%u>, dev <%u>, special <%u>\n", p, d, s);
		return CMD_RET_SUCCESS;
	}

	if (0 == strcmp(argv[1], "ldr_revo")) {
		printf("loader revocation: %u\n", ipa9131_fuse_read_loader_revocation());
		return CMD_RET_SUCCESS;
	}

	if (0 == strcmp(argv[1], "app_revo")) {
		printf("application revocation: %u\n", ipa9131_fuse_read_application_revocation());
		return CMD_RET_SUCCESS;
	}

	return CMD_RET_USAGE;
}

U_BOOT_CMD(ipa9131_fuse, 2, 0, do_ipa9131_fuse,
	"Excercise ipa9131 fuse functions",
	" init|eid|security|ldr_revo|app_revo"
);
#endif
