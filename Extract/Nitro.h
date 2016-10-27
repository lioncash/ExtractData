#pragma once

class CNitro final : public CExtractBase
{
public:
	bool Mount(CArcFile* pclArc) override;
	bool Decode(CArcFile* pclArc) override;

private:
	bool MountPak1(CArcFile* pclArc);
	bool MountPak2(CArcFile* pclArc);
	bool MountPak3(CArcFile* pclArc);
	bool MountPak4(CArcFile* pclArc);
	bool MountPK2(CArcFile* pclArc);
	bool MountN3Pk(CArcFile* pclArc);
	bool MountPck(CArcFile* pclArc);
	bool MountNpp(CArcFile* pclArc);
	bool MountNpa(CArcFile* pclArc);

	bool DecodePak1(CArcFile* pclArc);
	bool DecodePak3(CArcFile* pclArc);
	bool DecodePak4(CArcFile* pclArc);
	bool DecodePK2(CArcFile* pclArc);
	bool DecodeN3Pk(CArcFile* pclArc);
	bool DecodePck(CArcFile* pclArc);
	bool DecodeNpa(CArcFile* pclArc);

	void DecryptPak3(LPBYTE data, DWORD size, DWORD offset, const SFileInfo* file_info);
	void DecryptPak4(LPBYTE data, DWORD size, DWORD offset, const SFileInfo* file_info);
	void DecryptN3Pk(LPBYTE data, DWORD size, DWORD offset, const SFileInfo* file_info, BYTE& byKey);
	void DecryptNpa(LPBYTE data, DWORD size, DWORD offset, const SFileInfo* file_info);
};
