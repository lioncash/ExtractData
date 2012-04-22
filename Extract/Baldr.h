
#pragma	once

class	CBaldr : public CExtractBase
{
public:

	struct	SPACFileInfo
	{
		char				szFileName[64];
		DWORD				dwOffset;
		DWORD				dwFileSize;
		DWORD				dwCompFileSize;
	};

	BOOL									Mount( CArcFile* pclArc );
};
