#include "stdafx.h"
#include "Common.h"
#include "FileMap.h"

CFileMap::CFileMap()
{
	m_hFile = INVALID_HANDLE_VALUE;
	m_hFileMap = INVALID_HANDLE_VALUE;
	m_lpFileMapBase = NULL;
	m_lpFileMap = NULL;
}

CFileMap::~CFileMap()
{
	Close();
}

LPVOID CFileMap::Open(HANDLE hFile, DWORD Mode)
{
	// Create a file mapping object.
	if (Mode == FILE_READ)
		m_hFileMap = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
	else if (Mode == FILE_WRITE)
		m_hFileMap = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0, 0, NULL);

	if (m_hFileMap == INVALID_HANDLE_VALUE)
	{
		CError error;
		error.Message(GetForegroundWindow(), _T("File mapping error."));
		return NULL;
	}

	// Get start address of the map view.
	if (Mode == FILE_READ)
		m_lpFileMapBase = MapViewOfFile(m_hFileMap, FILE_MAP_READ, 0, 0, 0);
	else if (Mode == FILE_WRITE)
		m_lpFileMapBase = MapViewOfFile(m_hFileMap, FILE_MAP_WRITE, 0, 0, 0);

	if (m_lpFileMapBase == NULL)
	{
		CError error;
		error.Message(GetForegroundWindow(), _T("File mapping error."));
		return NULL;
	}
	m_lpFileMap = m_lpFileMapBase;

	return m_lpFileMapBase;
}

void CFileMap::Close()
{
	if (m_lpFileMapBase != NULL)
		UnmapViewOfFile(m_lpFileMapBase);

	if (m_hFileMap != INVALID_HANDLE_VALUE)
		CloseHandle(m_hFileMap);

	if (m_hFile != INVALID_HANDLE_VALUE)
		CloseHandle(m_hFile);
}

