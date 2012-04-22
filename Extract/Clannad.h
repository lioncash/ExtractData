#pragma once

class CClannad : public CExtractBase {
public:
	BOOL Mount(CArcFile* pclArc);
	BOOL Decode(CArcFile* pclArc);
};
