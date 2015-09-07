#pragma once

class CNitro final : public CExtractBase
{
public:
	BOOL Mount(CArcFile* pclArc) override;
	BOOL Decode(CArcFile* pclArc) override;

private:
	BOOL MountPak1(CArcFile* pclArc);
	BOOL MountPak2(CArcFile* pclArc);
	BOOL MountPak3(CArcFile* pclArc);
	BOOL MountPak4(CArcFile* pclArc);
	BOOL MountPK2(CArcFile* pclArc);
	BOOL MountN3Pk(CArcFile* pclArc);
	BOOL MountPck(CArcFile* pclArc);
	BOOL MountNpp(CArcFile* pclArc);
	BOOL MountNpa(CArcFile* pclArc);

	BOOL DecodePak1(CArcFile* pclArc);
	BOOL DecodePak3(CArcFile* pclArc);
	BOOL DecodePak4(CArcFile* pclArc);
	BOOL DecodePK2(CArcFile* pclArc);
	BOOL DecodeN3Pk(CArcFile* pclArc);
	BOOL DecodePck(CArcFile* pclArc);
	BOOL DecodeNpa(CArcFile* pclArc);

	void DecryptPak3(LPBYTE data, DWORD size, DWORD offset, SFileInfo* pInfFile);
	void DecryptPak4(LPBYTE data, DWORD size, DWORD offset, SFileInfo* pInfFile);
	void DecryptN3Pk(LPBYTE data, DWORD size, DWORD offset, SFileInfo* pInfFile, BYTE& byKey);
	void DecryptNpa(LPBYTE data, DWORD size, DWORD offset, SFileInfo* pInfFile);
};
