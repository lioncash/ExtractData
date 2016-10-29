#include "stdafx.h"
#include "SearchBase.h"

CSearchBase::CSearchBase()
{
	Init();
}

CSearchBase::~CSearchBase()
{
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

void CSearchBase::InitPattern(const void* pattern, DWORD size, DWORD num)
{
	memcpy(m_pattern[num].pattern, pattern, size);
	m_pattern[num].size = size;
}

// Enter the header
void CSearchBase::InitHed(const void* pattern, DWORD size)
{
	InitPattern(pattern, size, 0);
}
// Enter the footer
void CSearchBase::InitFot(const void* pattern, DWORD size)
{
	InitPattern(pattern, size, 1);
}

inline bool CSearchBase::CmpMem(LPCBYTE data, LPCBYTE pattern, DWORD size) const
{
	return std::equal(data, data + size, pattern, [](BYTE data_byte, BYTE pattern_byte) {
		return pattern_byte == '*' || data_byte == pattern_byte;
	});
}

bool CSearchBase::Search(LPCBYTE buf, DWORD dwSearchSize)
{
	for (int offset = 0; offset <= (int)dwSearchSize; offset++)
	{
		if (CmpHed(&buf[offset]))
		{
			SetOffset(offset);
			return true;
		}
	}

	return false;
}

bool CSearchBase::SearchFot(CArcFile* pclArc)
{
	DWORD FotSize = GetFotSize();
	while (true)
	{
		BYTE buf[SEARCH_BUFFER_SIZE];
		DWORD dwReadSize = pclArc->Read(buf, SEARCH_BUFFER_SIZE);
		
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
