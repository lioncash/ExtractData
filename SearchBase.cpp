#include "stdafx.h"
#include "SearchBase.h"

CSearchBase::CSearchBase()
{
	Init();
	//for (int i = 0; i < 2; i++)
	//    m_pattern[i].pattern = nullptr;
}

CSearchBase::~CSearchBase()
{
	//for (int i = 0; i < 2; i++)
	//{
	//    if (m_pattern[i].pattern != nullptr)
	//        delete[] m_pattern[i].pattern;
	//}
}

void CSearchBase::Init()
{
	m_offset = 0;
	m_ctFile = 1;
}

void CSearchBase::Init(SOption* pOption)
{
	Init();
	OnInit(pOption);
}

void CSearchBase::InitPattern(LPCVOID pattern, DWORD size, DWORD num)
{
	//m_pattern[num].pattern = new BYTE[size];
	memcpy(m_pattern[num].pattern, pattern, size);
	m_pattern[num].size = size;
}

// Enter the header
void CSearchBase::InitHed(LPCVOID pattern, DWORD size)
{
	InitPattern(pattern, size, 0);
}
// Enter the footer
void CSearchBase::InitFot(LPCVOID pattern, DWORD size)
{
	InitPattern(pattern, size, 1);
}

inline bool CSearchBase::CmpMem(LPCBYTE data, LPCBYTE pattern, DWORD size) const
{
	return std::equal(data, data + size, pattern, [](BYTE data_byte, BYTE pattern_byte) {
		return pattern_byte == '*' || data_byte == pattern_byte;
	});
}
/*
inline DWORD CSearchBase::CreateDecKey(LPBYTE buf)
{
	m_deckey = *(LPDWORD)&buf[0] ^ m_dwHeader;
	return (m_deckey);
}
*/
bool CSearchBase::Search(CArcFile* pclArc, LPCBYTE buf, DWORD dwReadSize, DWORD dwSearchSize)
{/*
	if (pclArc->GetOpt()->bEasyDecrypt == TRUE)
	{
		DWORD deckey = CreateDecKey(&buf[0]);
		for (int offset = 0; offset <= (int)dwSearchSize; offset++)
		{
			// Search while decoding
			for (int i = 0; i < GetHedSize; i += 4)
			DWORD deckey = *(LPDWORD)&buf[offset] ^ dwHeader;
			if (CmpHed(&buf[offset] ^ deckey))
			{
				SetOffset(offset);
				return true;
			}
		}
	}
	else
	{*/
		for (int offset = 0; offset <= (int)dwSearchSize; offset++)
		{
			if (CmpHed(&buf[offset]))
			{
				SetOffset(offset);
				return true;
			}
		}
//	}
	return false;
/*
	for (int i = 0; i <= (int)dwSearchSize; i++)
	{
		if (CmpHed(&buf[i]))
		{
			// dwReadSize - i back to first position, while moving through the header
			pclArc->Seek(-((int)dwReadSize-i), FILE_CURRENT);
			pclArc->GetProg()->UpdatePercent(i);
			return true;
		}
	}
	return false;
*/
}
/*
bool CSearchBase::Search(CArcFile* pclArc, LPBYTE buf, DWORD dwReadSize)
{
	LPBYTE hed2 = GetHed2();
	for (int i = 0; i < (int)dwReadSize; i++)
	{
		if (CmpHed(&buf[i]))
		{
			//if (hed2 != nullptr)
			{
			//  pclArc->Seek(-(BUFSIZE-i), FILE_CURRENT);
			//  if (CmpHed2(&buf[i]))
				{
					pclArc->Seek(-(BUFSIZE-i), FILE_CURRENT);
					pclArc->GetProg()->UpdatePercent(i);
					return true;
				}
			}
		}
	}
	return false;
}
*/
bool CSearchBase::SearchFot(CArcFile* pclArc)
{
	DWORD FotSize = GetFotSize();
	while (true)
	{
		BYTE buf[BUFSIZE];
		DWORD dwReadSize = pclArc->Read(buf, BUFSIZE);
		
		if (dwReadSize < FotSize)
		{
			pclArc->GetProg()->UpdatePercent(dwReadSize);
			break;
		}

		DWORD dwSearchSize = dwReadSize - FotSize;
		for (int i = 0; i <= (int)dwSearchSize; i++)
		{
			if (CmpFot(&buf[i]))
			{
				// dwReadSize - i -- Moves back to position found by i, has moved to the footer file and proceeds from the FotSize
				pclArc->Seek(-((int)dwReadSize-i - (int)FotSize), FILE_CURRENT);
				pclArc->GetProg()->UpdatePercent(i+FotSize);
				return true;
			}
		}
		pclArc->GetProg()->UpdatePercent(dwSearchSize);
		pclArc->Seek(-((int)FotSize-1), FILE_CURRENT);
	}
	return false;
}
