#pragma once

class CYuris : public CExtractBase
{
public:
	BOOL Mount(CArcFile* pclArc);
	BOOL Decode(CArcFile* pclArc);

protected:
	BOOL MountYPF(CArcFile* pclArc);
	BOOL MountYMV(CArcFile* pclArc);

	BOOL DecodeYMV(CArcFile* pclArc);
};