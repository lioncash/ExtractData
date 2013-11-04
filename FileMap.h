#pragma once

#include "File.h"

class CFileMap
{
protected:
	HANDLE m_hFile;
	HANDLE m_hFileMap;
	LPVOID m_lpFileMapBase;
	LPVOID m_lpFileMap;

public:
	CFileMap();
	virtual ~CFileMap();

	virtual LPVOID Open(HANDLE hFile, DWORD Mode);
	virtual void Close();
};