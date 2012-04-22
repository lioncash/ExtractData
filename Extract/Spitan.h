#pragma once

class CSpitan : public CExtractBase {
protected:
	// Spitan File Info
	struct FileInfo {
		DWORD start;
		DWORD size;
		TCHAR dummy[20];
	};

public:
	BOOL Mount(CArcFile* pclArc);
	BOOL MountSound(CArcFile* pclArc);
	BOOL MountGraphic1(CArcFile* pclArc);
	BOOL MountGraphic2(CArcFile* pclArc);
	BOOL MountGraphic3(CArcFile* pclArc);
	BOOL MountGraphic4(CArcFile* pclArc);
};
