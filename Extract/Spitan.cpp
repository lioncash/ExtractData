#include "stdafx.h"
#include "Spitan.h"

bool CSpitan::Mount(CArcFile* pclArc)
{
	if (MountSound(pclArc))
		return true;
	if (MountGraphic1(pclArc))
		return true;
	if (MountGraphic2(pclArc))
		return true;
	if (MountGraphic3(pclArc))
		return true;

	return false;
}

// Function to get information from  Spitan bm0000, bv00**, k000* files.
bool CSpitan::MountSound(CArcFile* pclArc)
{
	if ((pclArc->GetArcName().Left(6) != _T("bm0000")) && (pclArc->GetArcName().Left(4) != _T("bv00")) && (pclArc->GetArcName().Left(4) != _T("k000")))
		return false;

	pclArc->Seek(4, FILE_BEGIN);

	for (int i = 1; ; i++)
	{
		BYTE header[16];
		pclArc->Read(header, sizeof(header));

		if (memcmp(header, "OggS", 4) == 0)
			break;
		if (memcmp(header, "\0\0\0\0", 4) == 0)
			break;

		// Get filename
		TCHAR szFileName[_MAX_FNAME];
		_stprintf(szFileName, _T("%s_%06d.ogg"), pclArc->GetArcName().GetString(), i);

		// Add to listview
		SFileInfo infFile;
		infFile.name = szFileName;
		infFile.sizeCmp = *(LPDWORD)&header[0];
		infFile.sizeOrg = infFile.sizeCmp;
		infFile.start = *(LPDWORD)&header[4];
		infFile.end = infFile.start + infFile.sizeCmp;
		pclArc->AddFileInfo(infFile);
	}

	return true;
}

// Function to get infomation from Spitan *.aif files
bool CSpitan::MountGraphic1(CArcFile* pclArc)
{
	if (pclArc->GetArcExten() != _T(".aif"))
		return false;
	if (memcmp(pclArc->GetHed(), "\x10\0\0\0", 4) != 0)
		return false;

	for (int i = 1; ; i++)
	{
		BYTE header[32];
		pclArc->Read(header, sizeof(header));

		if (memcmp(header, "臼NG", 4) == 0)
			break;

		// Get filename
		TCHAR szFileName[_MAX_FNAME];
		_stprintf(szFileName, _T("%s_%06d.png"), pclArc->GetArcName().GetString(), i);

		// Add to listview
		SFileInfo infFile;
		infFile.name = szFileName;
		infFile.start = *(LPDWORD)&header[4];
		infFile.sizeCmp = *(LPDWORD)&header[8];
		infFile.sizeOrg = infFile.sizeCmp;
		infFile.end = infFile.start + infFile.sizeCmp;
		pclArc->AddFileInfo(infFile);
	}

	return true;
}

// Function to get information from Spitan f0* files
bool CSpitan::MountGraphic2(CArcFile* pclArc)
{
	bool flag = false;
	for (int i = 0; i < 5; i++)
	{
		TCHAR szArcName[_MAX_FNAME];
		_stprintf(szArcName, _T("f%02d"), i);
		if (lstrcmp(pclArc->GetArcName(), szArcName) == 0)
			flag = true;
	}
	
	if (!flag)
		return false;

	// Get .field file information
	std::vector<FileInfo> FileInfoList;
	while (true)
	{
		BYTE header[8];
		pclArc->Read(header, sizeof(header));

		if (memcmp(header, ".field", 6) == 0)
			break;
		if (memcmp(header, "\0\0\0\0\0\0", 6) == 0)
			break;

		FileInfo file;
		file.size = *(LPDWORD)&header[0];
		file.start = *(LPDWORD)&header[4];
		FileInfoList.push_back(file);
	}

	for (int i = 0, ctFile = 1; i < (int)FileInfoList.size(); i++)
	{
		// Get file size and relative address of the PNG file
		FileInfo file[5];

		pclArc->Seek(FileInfoList[i].start + 24, FILE_BEGIN);
		for (int j = 0; j < 2; j++)
		{
			pclArc->Read(&file[j].start, 4);
			pclArc->Read(&file[j].size, 4);
		}

		pclArc->Read(&file[2].start, 4);
		// 3つ目のPNGファイルのファイルサイズは、3つ目のPNGヘッダの直前に書いてあるため、そこまで飛ぶ
		QWORD tmp_pos = pclArc->GetArcPointer();
		pclArc->Seek(FileInfoList[i].start + file[2].start, FILE_BEGIN);

		int count = 0;
		while (true)
		{
			// By counting the amount of times ReadFile is called, we can determine the relative address of the third header PNG
			count++;

			DWORD filesize;
			pclArc->Read(&filesize, 4);
			// PNGヘッダが来たら、この.fieldファイルの中のPNGファイル数は4つ(Usually 5)
			if (filesize == 0x474E5089)
			{
				file[2].size = 0;
				break;
			}
			// File size is greater than 30 bytes
			else if (filesize >= 30)
			{
				file[2].size = filesize;
				file[2].start += 4 * count;
				break;
			}
		}

		pclArc->Seek(tmp_pos, FILE_BEGIN);
		for (int j = 3; j < 5; j++)
		{
			pclArc->Read(&file[j].start, 4);
			pclArc->Read(&file[j].size, 4);
		}

		for (int j = 0; j < 5; j++)
		{
			// Skip it is the file has a size of 0
			if (file[j].size == 0)
				continue;

			// Get filename
			TCHAR szFileName[_MAX_FNAME];
			_stprintf(szFileName, _T("%s_%06d.png"), pclArc->GetArcName().GetString(), ctFile++);

			// Add to listview
			SFileInfo infFile;
			infFile.name = szFileName;
			infFile.sizeCmp = file[j].size;
			infFile.sizeOrg = infFile.sizeCmp;
			infFile.start = FileInfoList[i].start + file[j].start;
			infFile.end = infFile.start + infFile.sizeCmp;
			pclArc->AddFileInfo(infFile);

			pclArc->Seek(infFile.sizeCmp, FILE_CURRENT);
		}
	}

	return true;
}

// Function that gets information from Spitan b0*, c0*, IGNR**** files
bool CSpitan::MountGraphic3(CArcFile* pclArc)
{
	bool flag = false;
	
	if (pclArc->GetArcName().Left(4) == _T("IGNR"))
		flag = true;
	
	for (int i = 0; i < 5; i++)
	{
		TCHAR szArcName[_MAX_FNAME];
		_stprintf(szArcName, _T("b%02d"), i);
		
		if (pclArc->GetArcName() == szArcName)
			flag = true;
	}
	
	for (int i = 0; i < 2; i++)
	{
		TCHAR szArcName[_MAX_FNAME];
		_stprintf(szArcName, _T("c%02d"), i);
		
		if (pclArc->GetArcName() == szArcName)
			flag = true;
	}
	
	if (!flag)
		return false;

	std::vector<FileInfo> FileInfoList;

	// Get information from the NORI file
	while (true)
	{
		BYTE header[8];
		pclArc->Read(header, sizeof(header));

		if (memcmp(header, "NORI", 4) == 0)
			break;
		if (memcmp(header, "\0\0\0\0", 4) == 0)
			break;

		FileInfo file;
		file.size = *(LPDWORD)&header[0];
		file.start = *(LPDWORD)&header[4];
		FileInfoList.push_back(file);
	}

	for (size_t i = 0, ctFile = 1; i < FileInfoList.size(); i++)
	{
		// Get the number of PNG files in the NORI file
		DWORD ctPng;
		pclArc->Seek(FileInfoList[i].start + 0x4C, FILE_BEGIN);
		pclArc->Read(&ctPng, 4);

		pclArc->Seek(0x40 - 0x1C, FILE_CURRENT);

		for (DWORD j = 0; j < ctPng; j++)
		{
			pclArc->Seek(0x1C, FILE_CURRENT);

			// Get first and second file sizes
			DWORD filesize[2];
			for (int k = 0; k < 2; k++)
			{
				pclArc->Read(&filesize[k], 4);
			}

			for (int k = 0; k < 2; k++)
			{
				// Skip file if the filesize is 0
				if (filesize[k] == 0)
					continue;

				// Get filename
				TCHAR szFileName[_MAX_FNAME];
				_stprintf(szFileName, _T("%s_%06zu.png"), pclArc->GetArcName().GetString(), ctFile++);

				// Add file to listview
				SFileInfo infFile;
				infFile.name = szFileName;
				infFile.sizeCmp = filesize[k];
				infFile.sizeOrg = infFile.sizeCmp;
				infFile.start = pclArc->GetArcPointer();
				infFile.end = infFile.start + infFile.sizeCmp;
				pclArc->AddFileInfo(infFile);

				pclArc->Seek(infFile.sizeCmp, FILE_CURRENT);
			}
		}
	}

	return true;
}
