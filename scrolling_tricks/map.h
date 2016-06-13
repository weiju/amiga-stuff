struct RawMap
{
	WORD	mapwidth;
	WORD	mapheight;
	WORD	maplayers;
	WORD	blockwidth;
	WORD	blockheight;
	BYTE	bytesperblock;
	BYTE	transparentblock;
	UBYTE	data[1];
};

