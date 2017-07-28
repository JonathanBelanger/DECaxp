typedef struct
{
	u32		pageSize;
	u16		byteOffsetBits;
	u16		levelSizeBits;
	u16		vaBitsMin;
	u16		vaBitsMax;
	u16		paBitsMax;
	u16		res;	/* Quadword align */
} AXP_VA_OPTIONS;

AXP_VA_OPTIONS axpVaOptions[] =
{
	{8192,	13, 10, 43, 43, 45},
	{16384,	14, 11, 43, 47, 46},
	{32768,	15, 12, 43, 51, 47},
	{65536,	16, 13, 44, 55, 48}
};