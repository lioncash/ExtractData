#pragma once

class CSpitan final : public CExtractBase
{
public:
	BOOL Mount(CArcFile* pclArc) override;
	BOOL MountSound(CArcFile* pclArc);
	BOOL MountGraphic1(CArcFile* pclArc);
	BOOL MountGraphic2(CArcFile* pclArc);
	BOOL MountGraphic3(CArcFile* pclArc);

private:
	// Spitan File Info
	struct FileInfo
	{
		DWORD start;
		DWORD size;
		TCHAR dummy[20];
	};
};
