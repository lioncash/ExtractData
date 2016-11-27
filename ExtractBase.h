#pragma once

class CArcFile;

class CExtractBase
{
public:
	virtual ~CExtractBase() = default;

	virtual bool Mount(CArcFile* archive) = 0;
	virtual bool Decode(CArcFile* archive);
	virtual bool Extract(CArcFile* archive);
};
