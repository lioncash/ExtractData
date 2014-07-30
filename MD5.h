#pragma once

struct SMD5
{
	char    szABCD[32];
	DWORD   adwABCD[4];
};

class CMD5
{
public:

	CMD5();

	SMD5    Calculate(LPCTSTR pszPathToFile);
	SMD5    Calculate(const void* pvSrc, DWORD dwSrcSize, const DWORD* pdwInitialize = NULL, BOOL bAlignment = FALSE);

	DWORD   CalculatePadding(DWORD dwSize);
	void    AppendPadding(void* pvSrc, DWORD dwSrcSize, DWORD dwPadding);


protected:

	DWORD   CalculateSub1(DWORD dwX, DWORD dwY, DWORD dwZ);
	DWORD   CalculateSub2(DWORD dwX, DWORD dwY, DWORD dwZ);
	DWORD   CalculateSub3(DWORD dwX, DWORD dwY, DWORD dwZ);
	DWORD   CalculateSub4(DWORD dwX, DWORD dwY, DWORD dwZ);
	void    CalculateSub5(DWORD& dwA, DWORD dwB, DWORD dwC, DWORD dwD, DWORD dwK, DWORD dwS, DWORD dwI);
	void    CalculateSub6(DWORD& dwA, DWORD dwB, DWORD dwC, DWORD dwD, DWORD dwK, DWORD dwS, DWORD dwI);
	void    CalculateSub7(DWORD& dwA, DWORD dwB, DWORD dwC, DWORD dwD, DWORD dwK, DWORD dwS, DWORD dwI);
	void    CalculateSub8(DWORD& dwA, DWORD dwB, DWORD dwC, DWORD dwD, DWORD dwK, DWORD dwS, DWORD dwI);

	DWORD   RotateLeft(DWORD dwA, DWORD dwS);
	void    ValueToStr(LPSTR pszDstOfMD5, DWORD dwMD5);
	void    MD5ToStrings(LPSTR pszDstOfMD5, const DWORD* pdwMD5);


private:

	int     m_anTable[65];
	SMD5    m_stmd5Value;
	DWORD   m_adwX[16];
};
