#pragma once

class CSpitan final : public CExtractBase
{
public:
	bool Mount(CArcFile* pclArc) override;

private:
	// Spitan File Info
	struct FileInfo
	{
		DWORD start;
		DWORD size;
		TCHAR dummy[20];
	};

	bool MountSound(CArcFile* pclArc);
	bool MountGraphic1(CArcFile* pclArc);
	bool MountGraphic2(CArcFile* pclArc);
	bool MountGraphic3(CArcFile* pclArc);
};
