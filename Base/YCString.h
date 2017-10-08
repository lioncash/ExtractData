#pragma once

//----------------------------------------------------------------------------------------
//-- Basic String Class ------------------------------------------------------------------
//----------------------------------------------------------------------------------------

class YCBaseString
{
public:
	int GetBaseTypeLength(const char* src) const;
	int GetBaseTypeLength(const char* src, int length) const;
	int GetBaseTypeLength(const wchar_t* src) const;
	int GetBaseTypeLength(const wchar_t* src, int length) const;

	int ConvertToBaseType(wchar_t* dst, int dst_length, const char* src, int src_length = -1) const;
	int ConvertToBaseType(char* dst, int dst_length, const wchar_t* src, int src_length = -1) const;
};

//----------------------------------------------------------------------------------------
//-- String Class Template ---------------------------------------------------------------
//----------------------------------------------------------------------------------------

// 可変長引数に渡すため、仮想関数を作ってはいけない
// 仮想関数を作ると「__vfptr」メンバ変数の存在により可変長引数に渡せなくなる

template<class TYPE>
class YCStringT : protected YCBaseString
{
private:
	TYPE* m_ptString;

	static constexpr size_t YCSTRINGT_BUFFERSIZE_ALIGNMENT = 64; // Buffer alignment
	static constexpr size_t YCSTRINGT_OFFSET_BUFFERSIZE = 16;
	static constexpr size_t YCSTRINGT_OFFSET_LENGTH = 12;

public:
	// Constructor
	YCStringT();
	YCStringT(const char* pszSrc);
	YCStringT(const char* pszSrc, int nCount);
	YCStringT(const char& rfcSrc);
	YCStringT(const wchar_t* pwszSrc);
	YCStringT(const wchar_t* pwszSrc, int nCount);
	YCStringT(const wchar_t& rfwcSrc);
	YCStringT(const YCStringT<TYPE>& rfclsSrc);
	YCStringT(const YCStringT<TYPE>& rfclsSrc, int nCount);

	// Destructor
	~YCStringT(); // No longer a virtual function.

	// Assignment
	BOOL LoadString(UINT uID);

	void Copy(const char* pszSrc);
	void Copy(const char* pszSrc, int nLength);
	void Copy(const char& rfcSrc);
	void Copy(const wchar_t* pwszSrc);
	void Copy(const wchar_t* pwszSrc, int nLength);
	void Copy(const wchar_t& rfwcSrc);

	// Adding
	void Append(const char* pszAppend);
	void Append(const char* pszAppend, int nLength);
	void Append(const char& rfcAppend);
	void Append(const wchar_t* pwszAppend);
	void Append(const wchar_t* pwszAppend, int nLength);
	void Append(const wchar_t& rfwcAppend);

	// Format data
	void Format(const TYPE* pszFormat, ...);
	void AppendFormat(const TYPE* pszFormat, ...);

	// Insert
	int Insert(int nIndex, const TYPE* pszInsert);
	int Insert(int nIndex, const TYPE& rfcInsert);

	// Deleting
	void Empty();

	int Delete(int nIndex, int nCount = 1);

	int Remove(const TYPE* pszRemove);
	int Remove(const TYPE& rfcRemove);

	// Replacing
	int Replace(const TYPE* pszOld, const TYPE* pszNew);
	int Replace(const TYPE& rfcOld, const TYPE& rfcNew);

	// Uppercase - Lowercase
	YCStringT<TYPE>& MakeLower();
	YCStringT<TYPE>& MakeUpper();

	// Comparison
	int Compare(const char* pszSrc) const;
	int Compare(const char& rfcSrc) const;
	int Compare(const wchar_t* pwszSrc) const;
	int Compare(const wchar_t& pwwcSrc) const;

	int CompareNoCase(const char* pszSrc) const;
	int CompareNoCase(const char& rfcSrc) const;
	int CompareNoCase(const wchar_t* pwszSrc) const;
	int CompareNoCase(const wchar_t& rfwcSrc) const;

	// Searching
	int Find(const TYPE* pszSub, int nStart = 0) const;
	int Find(const TYPE& rfcSub, int nStart = 0) const;

	int ReverseFind(const TYPE* pszSub) const;
	int ReverseFind(const TYPE& rfcSub) const;

	// Extraction
	YCStringT<TYPE> Left(int nCount) const;

	// Gets a string buffer
	TYPE* GetBuffer(int nBufferSize);

	// Gets a string buffer pointer
	inline const TYPE* GetString() const;

	// Gets the length of the string
	inline int GetLength() const;

	// Determine if a string is empty
	bool IsEmpty() const;

	// Path-related
	void RenameExtension(const TYPE* ptSrc);

	YCStringT<TYPE> GetFileName() const;
	int             GetFileNameIndex() const;
	YCStringT<TYPE> GetFileTitle() const;
	YCStringT<TYPE> GetFileExt() const;
	int             GetFileExtIndex() const;
	YCStringT<TYPE> GetDirPath() const;

	// Assignment operator
	YCStringT<TYPE>& operator=(const char* pszSrc);
	YCStringT<TYPE>& operator=(const char& rfcSrc);
	YCStringT<TYPE>& operator=(const wchar_t* pwszSrc);
	YCStringT<TYPE>& operator=(const wchar_t& rfwcSrc);
	YCStringT<TYPE>& operator=(const YCStringT<TYPE>& rfclsSrc);

	// + Operator
	YCStringT<TYPE> operator+(const char* pszAppend) const;
	YCStringT<TYPE> operator+(const char& rfcAppend) const;
	YCStringT<TYPE> operator+(const wchar_t* pwszAppend) const;
	YCStringT<TYPE> operator+(const wchar_t& rfwcAppend) const;
	YCStringT<TYPE> operator+(const YCStringT<TYPE>& rfclsAppend) const;

	// + Operator (External functions)
	template<class TYPE>
	friend YCStringT<TYPE> operator+(const char* pszSrc, const YCStringT<TYPE>& rfclsAppend);

	template<class TYPE>
	friend YCStringT<TYPE> operator+(const char& rfcSrc, const YCStringT<TYPE>& rfclsAppend);

	template<class TYPE>
	friend YCStringT<TYPE> operator+(const wchar_t* pwszSrc, const YCStringT<TYPE>& rfclsAppend);

	template<class TYPE>
	friend YCStringT<TYPE> operator+(const wchar_t& rfwcSrc, const YCStringT<TYPE>& rfclsAppend);

	// += Operator
	YCStringT<TYPE>& operator+=(const char* pszAppend);
	YCStringT<TYPE>& operator+=(const char& rfcAppend);
	YCStringT<TYPE>& operator+=(const wchar_t* pwszAppend);
	YCStringT<TYPE>& operator+=(const wchar_t& rfwcAppend);
	YCStringT<TYPE>& operator+=(const YCStringT<TYPE>& rfclsAppend);

	// == Operator
	bool operator==(const char* pszSrc) const;
	bool operator==(const char& rfcSrc) const;
	bool operator==(const wchar_t* pwszSrc) const;
	bool operator==(const wchar_t& rfwcSrc) const;
	bool operator==(const YCStringT<TYPE>& rfclsSrc) const;

	// == Operator(External functions)
	template<class TYPE>
	friend bool operator==(const char* pszSrc, const YCStringT<TYPE>& rfclsSrc);

	template<class TYPE>
	friend bool operator==(const char& rfcSrc, const YCStringT<TYPE>& rfclsSrc);

	template<class TYPE>
	friend bool operator==(const wchar_t* pwszSrc, const YCStringT<TYPE>& rfclsSrc);

	template<class TYPE>
	friend bool operator==(const wchar_t& rfwcSrc, const YCStringT<TYPE>& rfclsSrc);

	// != Operator
	bool operator!=(const char* pszSrc) const;
	bool operator!=(const char& rfcSrc) const;
	bool operator!=(const wchar_t* pwszSrc) const;
	bool operator!=(const wchar_t& rfwcSrc) const;
	bool operator!=(const YCStringT<TYPE>& rfclsSrc) const;

	// != Operator (External functions)
	template<class TYPE>
	friend bool operator!=(const char* pszSrc, const YCStringT<TYPE>& rfclsSrc);

	template<class TYPE>
	friend bool operator!=(const char& rfcSrc, const YCStringT<TYPE>& rfclsSrc);

	template<class TYPE>
	friend bool operator!=(const wchar_t* pwszSrc, const YCStringT<TYPE>& rfclsSrc);

	template<class TYPE>
	friend bool operator!=(const wchar_t& rfwcSrc, const YCStringT<TYPE>& rfclsSrc);

	// < Operator
	bool operator<(const char* pszSrc) const;
	bool operator<(const wchar_t* pwszSrc) const;
	bool operator<(const YCStringT<TYPE>& rfclsSrc) const;

	// < Operator(External functions)
	template<class TYPE>
	friend bool operator<(const char* pszSrc, const YCStringT<TYPE>& rfclsSrc);

	template<class TYPE>
	friend bool operator<(const wchar_t* pwszSrc, const YCStringT<TYPE>& rfclsSrc);

	// <= Operator
	bool operator<=(const char* pszSrc) const;
	bool operator<=(const wchar_t* pwszSrc) const;
	bool operator<=(const YCStringT<TYPE>& rfclsSrc) const;

	// <= Operator (External functions)
	template<class TYPE>
	friend bool operator<=(const char* pszSrc, const YCStringT<TYPE>& rfclsSrc);

	template<class TYPE>
	friend bool operator<=(const wchar_t* pwszSrc, const YCStringT<TYPE>& rfclsSrc);

	// > Operator
	bool operator>(const char* pszSrc) const;
	bool operator>(const wchar_t* pwszSrc) const;
	bool operator>(const YCStringT<TYPE>& rfclsSrc) const;

	// > Operator(External functions)
	template<class TYPE>
	friend bool operator>(const char* pszSrc, const YCStringT<TYPE>& rfclsSrc);

	template<class TYPE>
	friend bool operator>(const wchar_t* pwszSrc, const YCStringT<TYPE>& rfclsSrc);

	// >= Operator
	bool operator>=(const char* pszSrc) const;
	bool operator>=(const wchar_t* pwszSrc) const;
	bool operator>=(const YCStringT<TYPE>& rfclsSrc) const;

	// >= Operator(External functions)
	template<class TYPE>
	friend bool operator>=(const char* pszSrc, const YCStringT<TYPE>& rfclsSrc);

	template<class TYPE>
	friend bool operator>=(const wchar_t* pwszSrc, const YCStringT<TYPE>& rfclsSrc);

	// [] Operator
	inline TYPE&       operator[](int nPos);
	inline const TYPE& operator[](int nPos) const;

	// const TYPE* Operator
	inline operator const TYPE*() const;


protected:

	void Alloc(int nBufferSize);
	void Free();

	void ExtendBuffer(int nBufferSize);

	void SetBufferSize(int nBufferSize);
	int GetBufferSize();

	void SetLength(int nLength);
};

using YCString  = YCStringT<TCHAR>;
using YCStringA = YCStringT<char>;
using YCStringW = YCStringT<wchar_t>;

/// Constructor
template<class TYPE>
YCStringT<TYPE>::YCStringT()
{
	m_ptString = nullptr;

	Empty();
}

/// Constructor
///
/// @param pszSrc Source string
///
template<class TYPE>
YCStringT<TYPE>::YCStringT(const char* pszSrc)
{
	m_ptString = nullptr;

	Copy(pszSrc);
}

/// Constructor
///
/// @param pszSrc Source string
/// @param nCount Number of characters to copy
///
template<class TYPE>
YCStringT<TYPE>::YCStringT(const char* pszSrc, int nCount)
{
	m_ptString = nullptr;

	Copy(pszSrc, nCount);
}

/// Constructor
///
/// @param rfcSrc Source string
///
template<class TYPE>
YCStringT<TYPE>::YCStringT(const char& rfcSrc)
{
	m_ptString = nullptr;

	Copy(rfcSrc);
}

/// Constructor
///
/// @param pwszSrc Source string
///
template<class TYPE>
YCStringT<TYPE>::YCStringT(const wchar_t* pwszSrc)
{
	m_ptString = nullptr;

	Copy(pwszSrc);
}

/// Constructor
///
/// @param pwszSrc Source string
/// @param nCount  Number of characters to copy
///
template<class TYPE>
YCStringT<TYPE>::YCStringT(const wchar_t* pwszSrc, int nCount)
{
	m_ptString = nullptr;

	Copy(pwszSrc, nCount);
}

/// Constructor
///
/// @param rfwcSrc Source string
///
template<class TYPE> YCStringT<TYPE>::YCStringT(const wchar_t& rfwcSrc)
{
	m_ptString = nullptr;

	Copy(rfwcSrc);
}

/// Copy Constructor
///
/// @param rfclsSrc Source string
///
template<class TYPE>
YCStringT<TYPE>::YCStringT(const YCStringT<TYPE>& rfclsSrc)
{
	m_ptString = nullptr;

	Copy(rfclsSrc);
}

/// Destructor
template<class TYPE>
YCStringT<TYPE>::~YCStringT()
{
	Free();
}

/// Reads a string resource
///
/// @param uID Resource ID
///
template<class TYPE>
BOOL YCStringT<TYPE>::LoadString(UINT uID)
{
	return ::LoadString(::GetModuleHandle(nullptr), uID, GetBuffer(1024), 1024);
}

/// Assignment of a string
///
/// @param pszSrc Source string
///
template<class TYPE>
void YCStringT<TYPE>::Copy(const char* pszSrc)
{
	TYPE* ptString;
	int   nSrcLength = 0;

	switch (sizeof(TYPE))
	{
	case 1: // char type
		nSrcLength = lstrlenA(pszSrc);
		ptString = GetBuffer((nSrcLength + 1));
		lstrcpyA((char*)ptString, pszSrc);
		break;

	case 2: // wchar_t type
		nSrcLength = GetBaseTypeLength(pszSrc);
		ptString = GetBuffer((nSrcLength + 1));
		ConvertToBaseType((wchar_t*)ptString, GetBufferSize(), pszSrc);
		break;
	}

	SetLength(nSrcLength);
}

/// Assignment of a string
///
/// @param pszSrc Source string
/// @param nCount Number of characters to copy
///
template<class TYPE>
void YCStringT<TYPE>::Copy(const char* pszSrc, int nCount)
{
	TYPE* ptString;
	int   nSrcLength = 0;

	switch (sizeof(TYPE))
	{
	case 1: // char type
		nSrcLength = nCount;
		ptString = GetBuffer((nSrcLength + 1));
		lstrcpynA((char*)ptString, pszSrc, (nCount + 1));
		break;

	case 2: // wchar_t type
		nSrcLength = GetBaseTypeLength( pszSrc, nCount );
		ptString = GetBuffer( (nSrcLength + 1) );
		ConvertToBaseType((wchar_t*)ptString, GetBufferSize(), pszSrc, nCount);
		break;
	}

	SetLength(nSrcLength);
}

/// Assignment of a string
///
/// @param rfcSrc Source string
///
template<class TYPE> 
void YCStringT<TYPE>::Copy(const char& rfcSrc)
{
	char szSrc[2];

	szSrc[0] = rfcSrc;
	szSrc[1] = '\0';

	Copy(szSrc);
}

/// Assignment of a string
///
/// @param pwszSrc Source string
///
template<class TYPE>
void YCStringT<TYPE>::Copy(const wchar_t* pwszSrc)
{
	TYPE* ptString;
	int   nSrcLength = 0;

	switch( sizeof(TYPE) )
	{
	case 1: // char type
		nSrcLength = GetBaseTypeLength(pwszSrc);
		ptString = GetBuffer((nSrcLength + 1));
		ConvertToBaseType((char*)ptString, GetBufferSize(), pwszSrc);
		break;

	case 2: // wchar_t type
		nSrcLength = lstrlenW(pwszSrc);
		ptString = GetBuffer((nSrcLength + 1));
		lstrcpyW((wchar_t*)ptString, pwszSrc);
		break;
	}

	SetLength(nSrcLength);
}

/// Assignment of a string.
///
/// @param pwszSrc Source string
/// @param nCount Number of characters to copy
///
template<class TYPE>
void YCStringT<TYPE>::Copy(const wchar_t* pwszSrc, int nCount)
{
	TYPE* ptString;
	int   nSrcLength = 0;

	switch (sizeof(TYPE))
	{
	case 1: // char type
		nSrcLength = GetBaseTypeLength(pwszSrc, nCount);
		ptString = GetBuffer((nSrcLength + 1));
		ConvertToBaseType((char*)ptString, GetBufferSize(), pwszSrc, nCount);
		break;

	case 2: // wchar_t type
		nSrcLength = nCount;
		ptString = GetBuffer((nSrcLength + 1));
		lstrcpynW((wchar_t*)ptString, (wchar_t*)pwszSrc, (nCount + 1));
		break;
	}

	SetLength(nSrcLength);
}

/// Assignment of a character
///
/// @param rfwcSrc Source character
///
template<class TYPE>
void YCStringT<TYPE>::Copy(const wchar_t& rfwcSrc)
{
	wchar_t wszSrc[2];

	wszSrc[0] = rfwcSrc;
	wszSrc[1] = L'\0';

	Copy(wszSrc);
}

/// Appending a string
///
/// @param pszAppend String to append
///
template<class TYPE>
void YCStringT<TYPE>::Append(const char* pszAppend)
{
	int nStringLength = GetLength();
	int nNewLength = nStringLength;

	switch (sizeof(TYPE))
	{
	case 1: // char type
		nNewLength += lstrlenA(pszAppend);
		// Expand the buffer
		ExtendBuffer((nNewLength + 1));
		// Append the string
		lstrcpyA((char*)&m_ptString[nStringLength], pszAppend);
		break;

	case 2: // wchar_t type
		nNewLength += GetBaseTypeLength(pszAppend);
		// Expand the buffer
		ExtendBuffer((nNewLength + 1));
		// Append the string
		ConvertToBaseType((wchar_t*)&m_ptString[nStringLength], (GetBufferSize() - nStringLength), pszAppend);
		break;
	}

	SetLength( nNewLength );
}

/// Appending a string
///
/// @param pszAppend String to append
/// @param nCount    Number of characters to append
///
template<class TYPE>
void YCStringT<TYPE>::Append(const char* pszAppend, int nCount)
{
	int nStringLength = GetLength();
	int nNewLength = nStringLength;

	switch (sizeof(TYPE))
	{
	case 1: // char type
		nNewLength += nCount;
		// Expand the buffer
		ExtendBuffer((nNewLength + 1));
		// Append the character(s)
		lstrcpynA((char*)&m_ptString[nStringLength], pszAppend, (nCount + 1));
		break;

	case 2: // wchar_t type
		nNewLength += GetBaseTypeLength(pszAppend, nCount);
		// Expand the buffer
		ExtendBuffer((nNewLength + 1));
		// Append the character(s)
		ConvertToBaseType((wchar_t*)&m_ptString[nStringLength], (GetBufferSize() - nStringLength), pszAppend, nCount);
		break;
	}

	SetLength(nNewLength);
}

/// Appending a character
///
/// @param rfcAppend Character to append
///
template<class TYPE>
void YCStringT<TYPE>::Append(const char& rfcAppend)
{
	char szAppend[2];

	szAppend[0] = rfcAppend;
	szAppend[1] = '\0';

	Append( szAppend );
}

/// Appending a string
///
/// @param pwszAppend String to append
///
template<class TYPE>
void YCStringT<TYPE>::Append(const wchar_t* pwszAppend)
{
	int nStringLength = GetLength();
	int nNewLength = nStringLength;

	switch (sizeof(TYPE))
	{
	case 1: // char type
		nNewLength += GetBaseTypeLength(pwszAppend);
		// Expand the buffer
		ExtendBuffer((nNewLength + 1));
		// Append the string
		ConvertToBaseType((char*)&m_ptString[nStringLength], (GetBufferSize() - nStringLength), pwszAppend);
		break;

	case 2: // wchar_t type
		nNewLength += lstrlenW(pwszAppend);
		// Expand the buffer
		ExtendBuffer((nNewLength + 1));
		// Append the string
		lstrcpyW((wchar_t*)&m_ptString[nStringLength], pwszAppend);
		break;
	}

	SetLength(nNewLength);
}

/// Appending a string
///
/// @param pwszAppend String to append
/// @param nCount     Number of characters to append
///
template<class TYPE>
void YCStringT<TYPE>::Append(const wchar_t* pwszAppend, int nCount)
{
	int nStringLength = GetLength();
	int nNewLength = nStringLength;

	switch (sizeof(TYPE))
	{
	case 1: // char type
		nNewLength += GetBaseTypeLength(pwszAppend, nCount);
		// Expand the buffer
		ExtendBuffer((nNewLength + 1));
		// Append the string
		ConvertToBaseType((char*)&m_ptString[nStringLength], (GetBufferSize() - nStringLength), pwszAppend, nCount);
		break;

	case 2: // wchar_t type
		nNewLength += nCount;
		// Expand the buffer
		ExtendBuffer((nNewLength + 1));
		// Append the character(s)
		lstrcpynW((wchar_t*)&m_ptString[nStringLength], pwszAppend, (nCount + 1));
		break;
	}

	SetLength(nNewLength);
}

/// Appending a character
///
/// @param rfwcAppend String to append
///
template<class TYPE>
void YCStringT<TYPE>::Append(const wchar_t& rfwcAppend)
{
	wchar_t wszAppend[2];

	wszAppend[0] = rfwcAppend;
	wszAppend[1] = L'\0';

	Append(wszAppend);
}

/// Formatting settings
///
/// @param pszFormat Formatted data
///
template<class TYPE> void YCStringT<TYPE>::Format(const TYPE* pszFormat, ...)
{
	va_list vaArgPtr;

	va_start(vaArgPtr, pszFormat);

	std::vector<TYPE> clmBuffer(1024);

	while (true)
	{
		va_list vaWork = vaArgPtr;

		if (_vsntprintf(clmBuffer.data(), (clmBuffer.size() - 1), pszFormat, vaWork) == -1)
		{
			clmBuffer.resize(clmBuffer.size() * 2);
		}
		else
		{
			break;
		}
	}

	va_end(vaArgPtr);

	Copy(clmBuffer.data());
}

/// Formatting settings
///
/// @param pszFormat Formatted data
///
template<class TYPE>
void YCStringT<TYPE>::AppendFormat(const TYPE* pszFormat, ...)
{
	va_list vaArgPtr;

	va_start(vaArgPtr, pszFormat);

	std::vector<TYPE> clmBuffer(1024);

	while (true)
	{
		va_list vaWork = vaArgPtr;

		if (_vsntprintf(clmBuffer.data(), (clmBuffer.size() - 1), pszFormat, vaWork) == -1)
		{
			clmBuffer.resize(clmBuffer.size() * 2);
		}
		else
		{
			break;
		}
	}

	va_end(vaArgPtr);

	Append(clmBuffer.data());
}


/// Insertion
///
/// @param nIndex    Insertion offset
/// @param pszInsert String to insert
///
/// @return The length of the string after insertion
///
template<class TYPE>
int YCStringT<TYPE>::Insert(int nIndex, const TYPE* pszInsert)
{
	if (nIndex < 0)
	{
		// Insert at the beginning

		nIndex = 0;
	}
	else if (nIndex >= GetLength())
	{
		// Concatenation

		Append(pszInsert);

		return GetLength();
	}

	// Insertion processing

	int nStringLength = GetLength();
	int nNewLength = nStringLength;
	int nInsertLength = 0;
	int nShiftLength = 0;

	switch (sizeof(TYPE))
	{
	case 1: // char type
		nInsertLength = lstrlenA((const char*)pszInsert);
		nShiftLength = lstrlenA((const char*)&m_ptString[nIndex]);
		break;

	case 2: // wchar_t type
		nInsertLength = lstrlenW((const wchar_t*)pszInsert);
		nShiftLength = lstrlenW((const wchar_t*)&m_ptString[nIndex]);
		break;
	}

	// Expand the buffer
	nNewLength += nInsertLength;
	ExtendBuffer((nNewLength + 1));

	// Create a free spot to insert the string
	memmove(&m_ptString[nIndex + nInsertLength], &m_ptString[nIndex], (sizeof(TYPE) * (nShiftLength + 1)));

	// Insert the string
	memcpy(&m_ptString[nIndex], pszInsert, (sizeof(TYPE) * nInsertLength));

	SetLength(nNewLength);

	return GetLength();
}

/// Character insertion
///
/// @param nIndex    Insertion offset
/// @param rfcInsert Character to insert
///
/// @return The length of the string after insertion
///
template<class TYPE>
int YCStringT<TYPE>::Insert(int nIndex, const TYPE& rfcInsert)
{
	TYPE szInsert[2];

	szInsert[0] = rfcInsert;
	szInsert[1] = 0;

	return	Insert(nIndex, szInsert);
}

/// Clears the string
template<class TYPE>
void YCStringT<TYPE>::Empty()
{
	switch (sizeof(TYPE))
	{
	case 1: // char type
		Copy("");
		break;
	case 2: // wchar_t type
		Copy(L"");
		break;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Deletes the string
///
/// @param nIndex Offset to being deletion at
/// @param nCount Number of characters to delete
///
/// @return The length of the string after deletion
///
template<class TYPE>
int YCStringT<TYPE>::Delete(int nIndex, int nCount)
{
	int nStringLength = GetLength();
	int nNewLength = nStringLength;

	if (nIndex < 0)
	{
		// Delete from the beginning
		nIndex = 0;
	}
	else if (nIndex >= nStringLength)
	{
		// Don't delete anything
		return GetLength();
	}

	if (nCount <= 0)
	{
		// Don't delete anything
		return GetLength();
	}

	if ((nIndex + nCount) >= nStringLength)
	{
		// Offset and # of characters to delete is larger than the whole string length.

		m_ptString[nIndex] = 0;

		SetLength(nIndex);

		return GetLength();
	}

	// Deletion processing

	int nShiftLength = (GetLength() - (nIndex + nCount));

	// Delete the string
	// Overwriting will be packed to the left side of the string.

	memmove(&m_ptString[nIndex], &m_ptString[nIndex + nCount], (sizeof(TYPE) * (nShiftLength + 1)));

	nNewLength -= nCount;

	SetLength(nNewLength);

	return GetLength();
}

/// String deletion
///
/// @param pszRemove String occurrence to remove
///
/// @return Number of times the given string was removed from the string.
///
template<class TYPE>
int YCStringT<TYPE>::Remove(const TYPE* pszRemove)
{
	int nIndex = 0;
	int nCount = 0;
	int nRemoveLength = 0;

	switch (sizeof(TYPE))
	{
	case 1: // char type
		nRemoveLength = lstrlenA(pszRemove);
		break;

	case 2: // wchar_t type
		nRemoveLength = lstrlenW(pszRemove);
		break;
	}

	if (nRemoveLength <= 0)
	{
		// Don't delete anything
		return nCount;
	}

	while (true)
	{
		nIndex = Find(pszRemove, nIndex);

		// Lines to delete are present
		if (nIndex >= 0)
		{
			Delete(nIndex, nRemoveLength);

			nCount++;
		}
		else // Nothing present
		{
			break;
		}
	}

	return nCount;
}

/// Character deletion
///
/// @param rfcRemove Character occurrence to delete
///
/// @return  Number of times the given character was removed from the string.
///
template<class TYPE>
int YCStringT<TYPE>::Remove(const TYPE& rfcRemove)
{
	int nIndex = 0;
	int nCount = 0;
	int nRemoveLength = 1;

	while (true)
	{
		nIndex = Find(rfcRemove, nIndex);

		// Found characters to remove
		if (nIndex >= 0)
		{
			Delete(nIndex, nRemoveLength);

			nCount++;
		}
		else // Nothing present to remove
		{
			break;
		}
	}

	return nCount;
}

/// Replacing
///
/// @param pszOld String to replace
/// @param pszNew String to replace with
///
/// @return Number of times replacements occurred
///
template<class TYPE>
int YCStringT<TYPE>::Replace(const TYPE* pszOld, const TYPE* pszNew)
{
	int nIndex = 0;
	int nCount = 0;
	int nStringLength = GetLength();
	int nNewStringLength = nStringLength;
	int nOldLength = 0;
	int nNewLength = 0;

	switch (sizeof(TYPE))
	{
	case 1: // char type
		nOldLength = lstrlenA((const char*)pszOld);
		nNewLength = lstrlenA((const char*)pszNew);
		break;

	case 2: // wchar_t type
		nOldLength = lstrlenW((const wchar_t*)pszOld);
		nNewLength = lstrlenW((const wchar_t*)pszNew);
		break;
	}

	while (true)
	{
		nIndex = Find(pszOld, nIndex);

		if (nIndex >= 0)
		{
			// Target string to replace is present

			if (nNewLength == nOldLength)
			{
				// Length of the string cannot change, even after replacement.
				memcpy(&m_ptString[nIndex], pszNew, (sizeof(TYPE) * nNewLength));
			}
			else
			{
				// String length is increased/decreased after replacement

				nNewStringLength += (nNewLength - nOldLength);

				ExtendBuffer((nNewStringLength + 1));

				memmove(&m_ptString[nIndex + nNewLength], &m_ptString[nIndex + nOldLength], (sizeof(TYPE) * ((nStringLength + 1) - (nIndex + nOldLength))));

				memcpy(&m_ptString[nIndex], pszNew, (sizeof(TYPE) * nNewLength));

				nStringLength = nNewStringLength;
			}

			nCount++;
			nIndex += nNewLength;
		}
		else
		{
			// Couldn't find any target string to replace
			break;
		}
	}

	SetLength(nStringLength);

	return nCount;
}

/// Replacing
///
/// @param rfcOld Character to replace
/// @param rfcNew Character to replace with
///
/// @return Number of times replacement occurred
///
template<class TYPE>
int YCStringT<TYPE>::Replace(const TYPE& rfcOld, const TYPE& rfcNew)
{
	int nIndex = 0;
	int nCount = 0;
	int nNewIndex = GetLength();

	while (true)
	{
		nIndex = Find(rfcOld, nIndex);

		if (nIndex >= 0)
		{
			// Target character to replace exists
			m_ptString[nIndex] = rfcNew;

			nCount++;
		}
		else
		{
			// Target character to replace was not present
			break;
		}
	}

	return nCount;
}

/// Lowercasing
template<class TYPE>
YCStringT<TYPE>& YCStringT<TYPE>::MakeLower()
{
	::CharLower(m_ptString);
	return *this;
}

/// Uppercasing
template<class TYPE>
YCStringT<TYPE>& YCStringT<TYPE>::MakeUpper()
{
	::CharUpper(m_ptString);
	return *this;
}

/// Comparison
///
/// @param pszSrc Comparison string
///
/// @return 0               Both equal
///         Negative value  pszSrc is smaller
///         Positive value  pszSrc is larger
///
template<class TYPE>
int YCStringT<TYPE>::Compare(const char* pszSrc) const
{
	int   nReturn = 0;
	TYPE* ptWork;
	int   nWork;

	switch (sizeof(TYPE))
	{
	case 1: // char type
		nReturn = lstrcmpA( (const char*) m_ptString, pszSrc );
		break;

	case 2: // wchar_t type
		nWork = GetBaseTypeLength( pszSrc );
		ptWork = new TYPE[nWork];
		ConvertToBaseType( (wchar_t*) ptWork, nWork, pszSrc );
		nReturn = lstrcmpW( (const wchar_t*) m_ptString, (const wchar_t*) ptWork );
		delete[] ptWork;
		break;
	}

	return nReturn;
}

/// Comparison
///
/// @param rfcSrc Comparison character
///
/// @return 0               Both are equal
///         Negative value  rfcSrc is smaller
///         Positive value  rfcSrc is larger
///
template<class TYPE>
int YCStringT<TYPE>::Compare(const char& rfcSrc) const
{
	char szSrc[2];

	szSrc[0] = rfcSrc;
	szSrc[1] = '\0';

	return Compare(szSrc);
}

/// Comparison
///
/// @param pwszSrc Comparison string
///
/// @return 0               Both are equal
///         Negative value  pszSrc is smaller
///         Positive value  pszSrc is larger
///
template<class TYPE>
int YCStringT<TYPE>::Compare(const wchar_t* pwszSrc) const
{
	int   nReturn = 0;
	TYPE* ptWork;
	int   nWork;

	switch (sizeof(TYPE))
	{
	case 1: // char type
		nWork = GetBaseTypeLength(pwszSrc);
		ptWork = new TYPE[nWork];
		ConvertToBaseType((char*)ptWork, nWork, pwszSrc);
		nReturn = lstrcmpA((const char*)m_ptString, (const char*)ptWork);
		delete[] ptWork;
		break;

	case 2: // wchar_t type
		nReturn = lstrcmpW((const wchar_t*)m_ptString, pwszSrc);
		break;
	}

	return nReturn;
}

/// Comparison
///
/// @param rfwcSrc Comparison character
///
/// @return 0               Both are equal
///         Negative value  rfwcSrc is smaller
///         Positive value  rfwcSrc is larger
///
template<class TYPE>
int YCStringT<TYPE>::Compare(const wchar_t& rfwcSrc) const
{
	wchar_t wszSrc[2];

	wszSrc[0] = rfwcSrc;
	wszSrc[1] = L'\0';

	return Compare( wszSrc );
}

/// Comparison
///
/// @param pszSrc Comparison string
///
/// @return 0               Both are equal
///         Negative value  pszSrc is smaller
///         Positive value  pszSrc is larger
///
/// @remark Casing of the characters are ignored.
///
template<class TYPE>
int YCStringT<TYPE>::CompareNoCase(const char* pszSrc) const
{
	int   nReturn = 0;
	TYPE* ptWork;
	int   nWork;

	switch (sizeof(TYPE))
	{
	case 1: // char type
		nReturn = lstrcmpiA((const char*)m_ptString, pszSrc);
		break;

	case 2: // wchar_t type
		nWork = GetBaseTypeLength(pszSrc);
		ptWork = new TYPE[nWork];
		ConvertToBaseType((wchar_t*)ptWork, nWork, pszSrc);
		nReturn = lstrcmpiW((const wchar_t*)m_ptString, (const wchar_t*)ptWork);
		delete[] ptWork;
		break;
	}

	return nReturn;
}

/// Comparison
///
/// @return 0              Both are equal
///        Negative value  rfcSrc is smaller
///        Positive value  rfcSrc is larger
///
/// @remark Casing of the character is ignored.
///
template<class TYPE>
int YCStringT<TYPE>::CompareNoCase(const char& rfcSrc) const
{
	char szSrc[2];

	szSrc[0] = rfcSrc;
	szSrc[1] = '\0';

	return CompareNoCase(szSrc);
}

/// Comparison
///
/// @param pwszSrc Comparison string
///
/// @return 0               Both are equal
///         Negative value  pszSrc is smaller
///         Positive value  pszSrc is larger
///
/// @remark Casing of the characters are ignored.
///
template<class TYPE>
int YCStringT<TYPE>::CompareNoCase(const wchar_t* pwszSrc) const
{
	int   nReturn = 0;
	TYPE* ptWork;
	int   nWork;

	switch (sizeof(TYPE))
	{
	case 1: // char type
		nWork = GetBaseTypeLength(pwszSrc);
		ptWork = new TYPE[nWork];
		ConvertToBaseType((char*)ptWork, nWork, pwszSrc);
		nReturn = lstrcmpiA((const char*)m_ptString, (const char*)ptWork);
		delete[] ptWork;
		break;

	case 2: // wchar_t type
		nReturn = lstrcmpiW((const wchar_t*)m_ptString, pwszSrc);
		break;
	}

	return nReturn;
}

/// Comparison
///
/// @param rfwcSrc Comparison character
///
/// @return 0               Both are equal
///         Negative value  rfwcSrc is smaller
///         Positive value  rfwcSrc is larger
///
/// @remark Casing of the characters is ignored
///
template<class TYPE>
int YCStringT<TYPE>::CompareNoCase(const wchar_t& rfwcSrc) const
{
	wchar_t wszSrc[2];

	wszSrc[0] = rfwcSrc;
	wszSrc[1] = L'\0';

	return CompareNoCase(wszSrc);
}

//////////////////////////////////////////////////////////////////////////////////////////
//  Forward search
//
//  return The index at which the string was found at
//
//  Remarks: -1 is returned if a matching string could not be found

template<class TYPE>
int YCStringT<TYPE>::Find(const TYPE* pszSub, int nStart) const
{
	INT_PTR npResult = -1;

	if (nStart < 0)
	{
		// Search from the beginning of the string

		nStart = 0;
	}
	else if (nStart >= GetLength())
	{
		// Search from the end of the string

		return (int) npResult;
	}

	switch (sizeof(TYPE))
	{
		case 1: // char type
		{
			const u8* found = _mbsstr((const u8*)&m_ptString[nStart], (const u8*)pszSub);

			if (found != nullptr)
			{
				// Target string is present
				npResult = (found - (u8*) m_ptString);
			}
		}
		break;

		case 2: // wchar_t type
		{
			const wchar_t* pwszFound = wcsstr((const wchar_t*)&m_ptString[nStart], (const wchar_t*)pszSub);

			if (pwszFound != nullptr)
			{
				//  Target string is present
				npResult = (pwszFound - (wchar_t*) m_ptString);
			}
		}
		break;
	}

	return (int) npResult;
}

//////////////////////////////////////////////////////////////////////////////////////////
//  Forward search

template<class TYPE> int YCStringT<TYPE>::Find(const TYPE& rfcSub, int nStart) const
{
	INT_PTR npResult = -1;

	if (nStart < 0)
	{
		// Search from the beginning of the string
		nStart = 0;
	}
	else if (nStart >= GetLength())
	{
		// Search from the end of the string
		return (int) npResult;
	}

	switch (sizeof(TYPE))
	{
		case 1: // char type
		{
			const u8* found = _mbschr((const u8*)&m_ptString[nStart], rfcSub);

			if (found != nullptr)
			{
				// Target string is present
				npResult = (found - (u8*) m_ptString);
			}
		}
		break;

		case 2: // wchar_t type
		{
			const wchar_t* pwszFound = wcschr((const wchar_t*)&m_ptString[nStart], rfcSub);

			if (pwszFound != nullptr)
			{
				// Target string is present
				npResult = (pwszFound - (wchar_t*) m_ptString);
			}
		}
		break;
	}

	return (int) npResult;
}

//////////////////////////////////////////////////////////////////////////////////////////
//  Backward search
//
//  return  The index at which the beginning of the string was found.
//
//  Remarks: For multi-byte characters, this is pretty slow, so only use it when necessary.

template<class TYPE>
int YCStringT<TYPE>::ReverseFind(const TYPE* pszSub) const
{
	INT_PTR npResult = -1;

	// Getting the length of the search string
	int nSubLength = 0;

	switch (sizeof(TYPE))
	{
	case 1: // char type
		nSubLength = lstrlenA((const char*)pszSub);
		break;

	case 2: // wchar_t type
		nSubLength = lstrlenW((const wchar_t*)pszSub);
		break;
	}

	if (nSubLength <= 0)
	{
		// No search string
		return (int) npResult;
	}

	// Search from the end of the string
	int nStart = (GetLength() - nSubLength);
	int nIndex;

	switch (sizeof(TYPE))
	{
	case 1: // char type

		char* pszWork = (char*) &m_ptString[nStart];

		while (true)
		{
			if (lstrcmpA(pszWork, (const char*)pszSub) == 0)
			{
				// Strings are equal
				npResult = (pszWork - (char*) m_ptString);
				break;
			}

			if (pszWork == (char*)m_ptString)
				break;

			pszWork = ::CharPrevA((char*)m_ptString, pszWork);
		}
		break;

	case 2: // wchar_t type
		for (nIndex = nStart; nIndex >= 0; nIndex--)
		{
			if (lstrcmpW((wchar_t*)&m_ptString[nIndex], (const wchar_t*)pszSub) == 0)
			{
				// Strings are equal
				npResult = nIndex;
				break;
			}
		}

		break;
	}

	return (int) npResult;
}

//////////////////////////////////////////////////////////////////////////////////////////
//  Backward search

template<class TYPE>
int YCStringT<TYPE>::ReverseFind(const TYPE& rfcSub) const
{
	INT_PTR npResult = -1;

	switch (sizeof(TYPE))
	{
		case 1: // char type
		{
			const u8* found = _mbsrchr((const u8*)m_ptString, rfcSub);

			if (found != nullptr)
			{
				// Target string is present
				npResult = (found - (u8*) m_ptString);
			}
		}
		break;

		case 2: // wchar_t type
		{
			// wchar_t type

			const wchar_t* pwszFound = wcsrchr((wchar_t*)m_ptString, rfcSub);

			if (pwszFound != nullptr)
			{
				// Target string is present
				npResult = (pwszFound - (wchar_t*) m_ptString);
			}
		}
		break;
	}

	return (int) npResult;
}

//////////////////////////////////////////////////////////////////////////////////////////
//  Extracts the left part of the string
//
//  return  Extracted character string

template<class TYPE>
YCStringT<TYPE> YCStringT<TYPE>::Left(int nCount) const
{
	YCStringT<TYPE> clsResult = *this;

	clsResult.Delete(nCount, clsResult.GetLength());

	return clsResult;
}

///  Gets the string buffer
template<class TYPE> TYPE* YCStringT<TYPE>::GetBuffer(int nBufferSize)
{
	if (nBufferSize > GetBufferSize())
	{
		// Desired buffer size is larger than the current buffer's size
		Free();

		// Allocate memory
		Alloc(nBufferSize);
	}

	return m_ptString;
}

///  Gets the string length
template<class TYPE>
int YCStringT<TYPE>::GetLength() const
{
	if (m_ptString == nullptr)
		return 0;

	return *(int*) (m_ptString - (YCSTRINGT_OFFSET_LENGTH / sizeof(TYPE)));
}

//////////////////////////////////////////////////////////////////////////////////////////
//  Gets the string

template<class TYPE>
const TYPE* YCStringT<TYPE>::GetString() const
{
	return m_ptString;
}

//////////////////////////////////////////////////////////////////////////////////////////
//  Whether or not the string is empty

template<class TYPE>
bool YCStringT<TYPE>::IsEmpty() const
{
	bool bReturn = false;

	switch (sizeof(TYPE))
	{
	case 1: // char type
		bReturn = (*this == "");
		break;

	case 2: // wchar_t type
		bReturn = (*this == L"");
		break;
	}

	return bReturn;
}

/// Changes the extension of a file (for when using YCStrings to deal with files)
template<class TYPE>
void YCStringT<TYPE>::RenameExtension(const TYPE* ptRenameExt)
{
	int nFileNameIndex = GetFileNameIndex();
	int nFileExtIndex = GetFileExtIndex();

	// Found an extension
	if (nFileExtIndex >= nFileNameIndex)
	{
		int nNewStringLength = GetLength();

		switch (sizeof(TYPE))
		{
		case 1: // char type
			nNewStringLength += lstrlenA((const char*)ptRenameExt) - lstrlenA((const char*)&m_ptString[nFileExtIndex]);
			ExtendBuffer((nNewStringLength + 1));
			lstrcpyA((char*)&m_ptString[nFileExtIndex], (const char*)ptRenameExt);
			break;

		case 2: // wchar_t type
			nNewStringLength += lstrlenA((const char*)ptRenameExt) - lstrlenA((const char*)&m_ptString[nFileExtIndex]);
			ExtendBuffer((nNewStringLength + 1));
			lstrcpyW((wchar_t*)&m_ptString[nFileExtIndex], (const wchar_t*)ptRenameExt);
			break;
		}

		SetLength( nNewStringLength );
	}
}

/// Gets the name of a file (when using YCStrings to deal with files)
template<class TYPE>
YCStringT<TYPE> YCStringT<TYPE>::GetFileName() const
{
	int nIndex = GetFileNameIndex();

	return &m_ptString[nIndex];
}

/// Gets the index in the string where the name of a file begins
template<class TYPE> int YCStringT<TYPE>::GetFileNameIndex() const
{
	int nFound = -1;
	int nIndex = 0;

	switch (sizeof(TYPE))
	{
	case 1: // char type
		nFound = ReverseFind('\\');
		break;

	case 2: // wchar_t type
		nFound = ReverseFind(L'\\');
		break;
	}

	if (nFound >= 0)
	{
		// Found a file name
		nIndex = (nFound + 1);
	}
	else
	{
		// Whole string happens to be the file name
		nIndex = 0;
	}

	return nIndex;
}

/// Gets the name of a file (When using YCStrings to deal with file names)
template<class TYPE>
YCStringT<TYPE> YCStringT<TYPE>::GetFileTitle() const
{
	YCStringT<TYPE> clsFileTitle = GetFileName();
	TYPE            szWork[1];

	switch (sizeof(TYPE))
	{
	case 1: // char type
		szWork[0] = '\0';
		break;

	case 2: // wchar_t type
		szWork[0] = L'\0';
		break;
	}

	clsFileTitle.RenameExtension(szWork);

	return clsFileTitle;
}

/// Gets the file extension (When dealing with files, etc)
template<class TYPE>
YCStringT<TYPE> YCStringT<TYPE>::GetFileExt() const
{
	int nIndex = GetFileExtIndex();

	return &m_ptString[nIndex];
}

/// Gets the index of a file extension (for when dealing with file names)
template<class TYPE>
int YCStringT<TYPE>::GetFileExtIndex() const
{
	int nFound = -1;
	int nIndex = 0;

	switch (sizeof(TYPE))
	{
	case 1: // char type
		nFound = ReverseFind('.');
		break;

	case 2: // wchar_t type
		nFound = ReverseFind(L'.');
		break;
	}

	if (nFound >= 0)
	{
		// Extension found
		nIndex = nFound;
	}
	else
	{
		// There is no extension
		nIndex = GetLength();
	}

	return nIndex;
}

/// Gets a folder path
template<class TYPE> YCStringT<TYPE> YCStringT<TYPE>::GetDirPath() const
{
	int nTargetIndex = 0;
	int nFileNameIndex = GetFileNameIndex();
	int nDriveIndex = -1;

	if (nFileNameIndex >= 1)
	{
		// Folder name found
		nTargetIndex = (nFileNameIndex - 1);
	}

	switch (sizeof(TYPE))
	{
	case 1: // char type
		nDriveIndex = ReverseFind(':');
		break;

	case 2: // wchar_t type
		nDriveIndex = ReverseFind(L':');
		break;
	}

	if (nDriveIndex >= 0)
	{
		// Drive name is included

		if (nTargetIndex == (nDriveIndex + 1))
		{
			// '\\' immediately after ':'
			nTargetIndex = nFileNameIndex;
		}
		else
		{
			// Don't delete anything prior to ':'
			nTargetIndex = (nTargetIndex < nDriveIndex) ? (nDriveIndex + 1) : nTargetIndex;
		}
	}

	return Left(nTargetIndex);
}

/// Assignment operator
template<class TYPE>
YCStringT<TYPE>& YCStringT<TYPE>::operator=(const char* pszSrc)
{
	Copy(pszSrc);
	return *this;
}


/// Assignment operator
template<class TYPE>
YCStringT<TYPE>& YCStringT<TYPE>::operator=(const char& rfcSrc)
{
	Copy(rfcSrc);
	return *this;
}

/// Assignment operator
template<class TYPE>
YCStringT<TYPE>& YCStringT<TYPE>::operator=(const wchar_t* pwszSrc)
{
	Copy(pwszSrc);
	return *this;
}

/// Assignment operator
template<class TYPE>
YCStringT<TYPE>& YCStringT<TYPE>::operator=(const wchar_t& rfwcSrc)
{
	Copy(rfwcSrc);
	return *this;
}

/// Assignment operator
template<class TYPE>
YCStringT<TYPE>& YCStringT<TYPE>::operator=(const YCStringT<TYPE>& rfclsSrc)
{
	Copy(rfclsSrc);
	return *this;
}

//////////////////////////////////////////////////////////////////////////////////////////
//  + Operator

template<class TYPE>
YCStringT<TYPE> YCStringT<TYPE>::operator+(const char* pszAppend) const
{
	YCStringT<TYPE> clsResult = *this;
	clsResult.Append(pszAppend);
	return clsResult;
}

//////////////////////////////////////////////////////////////////////////////////////////
//  + Operator

template<class TYPE>
YCStringT<TYPE> YCStringT<TYPE>::operator+(const char& rfcAppend) const
{
	YCStringT<TYPE> clsResult = *this;
	clsResult.Append(rfcAppend);
	return clsResult;
}

//////////////////////////////////////////////////////////////////////////////////////////
//  + Operator

template<class TYPE>
YCStringT<TYPE> YCStringT<TYPE>::operator+(const wchar_t* pwszAppend) const
{
	YCStringT<TYPE> clsResult = *this;
	clsResult.Append(pwszAppend);
	return clsResult;
}

//////////////////////////////////////////////////////////////////////////////////////////
//  + Operator

template<class TYPE>
YCStringT<TYPE> YCStringT<TYPE>::operator+(const wchar_t& rfwcAppend) const
{
	YCStringT<TYPE> clsResult = *this;
	clsResult.Append(rfwcAppend);
	return clsResult;
}

//////////////////////////////////////////////////////////////////////////////////////////
//  + Operator

template<class TYPE>
YCStringT<TYPE> YCStringT<TYPE>::operator+(const YCStringT<TYPE>&	rfclsAppend) const
{
	YCStringT<TYPE> clsResult = *this;
	clsResult.Append(rfclsAppend);
	return clsResult;
}

//////////////////////////////////////////////////////////////////////////////////////////
//  + Operator (External function)

template<class TYPE>
YCStringT<TYPE> operator+(const char* pszSrc, const YCStringT<TYPE>& rfclsAppend)
{
	YCStringT<TYPE> clsResult = pszSrc;
	clsResult.Append(rfclsAppend);
	return clsResult;
}

//////////////////////////////////////////////////////////////////////////////////////////
//  + Operator (External function)

template<class TYPE>
YCStringT<TYPE> operator+(const char& rfcSrc, const YCStringT<TYPE>& rfclsAppend)
{
	YCStringT<TYPE> clsResult = rfcSrc;
	clsResult.Append( rfclsAppend );
	return clsResult;
}

//////////////////////////////////////////////////////////////////////////////////////////
//  + Operator (External function)

template<class TYPE>
YCStringT<TYPE> operator+(const wchar_t* pwszSrc, const YCStringT<TYPE>& rfclsAppend)
{
	YCStringT<TYPE> clsResult = pwszSrc;
	clsResult.Append( rfclsAppend );
	return clsResult;
}

//////////////////////////////////////////////////////////////////////////////////////////
// + Operator (External function)

template<class TYPE>
YCStringT<TYPE> operator+(const wchar_t& rfwcSrc, const YCStringT<TYPE>& rfclsAppend)
{
	YCStringT<TYPE> clsResult = rfwcSrc;
	clsResult.Append(rfclsAppend);
	return clsResult;
}

//////////////////////////////////////////////////////////////////////////////////////////
//  += Operator

template<class TYPE>
YCStringT<TYPE>& YCStringT<TYPE>::operator+=(const char* pszAppend)
{
	Append(pszAppend);
	return *this;
}

//////////////////////////////////////////////////////////////////////////////////////////
//  += Operator

template<class TYPE>
YCStringT<TYPE>& YCStringT<TYPE>::operator+=(const char& rfcAppend)
{
	Append(rfcAppend);
	return *this;
}

//////////////////////////////////////////////////////////////////////////////////////////
//  += Operator

template<class TYPE>
YCStringT<TYPE>& YCStringT<TYPE>::operator+=(const wchar_t* pwszAppend)
{
	Append(pwszAppend);
	return *this;
}

//////////////////////////////////////////////////////////////////////////////////////////
//  += Operator

template<class TYPE>
YCStringT<TYPE>& YCStringT<TYPE>::operator+=(const wchar_t& rfwcAppend)
{
	Append(rfwcAppend);
	return *this;
}

//////////////////////////////////////////////////////////////////////////////////////////
//  += Operator

template<class TYPE>
YCStringT<TYPE>& YCStringT<TYPE>::operator+=(const YCStringT<TYPE>& rfclsAppend)
{
	Append(rfclsAppend);
	return *this;
}

//////////////////////////////////////////////////////////////////////////////////////////
//  == Operator

template<class TYPE>
bool YCStringT<TYPE>::operator==(const char* pszSrc) const
{
	return (Compare(pszSrc) == 0);
}

//////////////////////////////////////////////////////////////////////////////////////////
//  == Operator

template<class TYPE>
bool YCStringT<TYPE>::operator==(const char& rfcSrc) const
{
	return (Compare(rfcSrc) == 0);
}

//////////////////////////////////////////////////////////////////////////////////////////
//  == Operator

template<class TYPE>
bool YCStringT<TYPE>::operator==(const wchar_t* pwszSrc) const
{
	return (Compare(pwszSrc) == 0);
}

//////////////////////////////////////////////////////////////////////////////////////////
// == Operator

template<class TYPE>
bool YCStringT<TYPE>::operator==(const wchar_t& rfwcSrc) const
{
	return (Compare(rfwcSrc) == 0);
}

//////////////////////////////////////////////////////////////////////////////////////////
//  == Operator

template<class TYPE>
bool YCStringT<TYPE>::operator==(const YCStringT<TYPE>& rfclsSrc) const
{
	return (Compare(rfclsSrc) == 0);
}

//////////////////////////////////////////////////////////////////////////////////////////
//  != Operator

template<class TYPE>
bool YCStringT<TYPE>::operator!=(const char* pszSrc) const
{
	return (Compare(pszSrc) != 0);
}

//////////////////////////////////////////////////////////////////////////////////////////
//  != Operator

template<class TYPE>
bool YCStringT<TYPE>::operator!=(const char& rfcSrc) const
{
	return (Compare(rfcSrc) != 0);
}

//////////////////////////////////////////////////////////////////////////////////////////
//  != Operator
template<class TYPE>
bool YCStringT<TYPE>::operator!=(const wchar_t* pwszSrc) const
{
	return (Compare(pwszSrc) != 0);
}

//////////////////////////////////////////////////////////////////////////////////////////
// != Operator

template<class TYPE>
bool YCStringT<TYPE>::operator!=(const wchar_t& rfwcSrc) const
{
	return (Compare(rfwcSrc) != 0);
}

//////////////////////////////////////////////////////////////////////////////////////////
//  != Operator

template<class TYPE>
bool YCStringT<TYPE>::operator!=(const YCStringT<TYPE>& rfclsSrc) const
{
	return (Compare(rfclsSrc) != 0);
}

//////////////////////////////////////////////////////////////////////////////////////////
//  < Operator
//
//  return  TRUE    pszSrc is smaller
//          FALSE   pszSrc is larger

template<class TYPE>
bool YCStringT<TYPE>::operator<(const char* pszSrc) const
{
	return (Compare(pszSrc) < 0);
}

//////////////////////////////////////////////////////////////////////////////////////////
//  < Operator
//
//  return  TRUE    pwszSrc is smaller
//          FALSE   pwszSrc is larger

template<class TYPE>
bool YCStringT<TYPE>::operator<(const wchar_t* pwszSrc) const
{
	return (Compare(pwszSrc) < 0);
}

//////////////////////////////////////////////////////////////////////////////////////////
//  < Operator
//
//  return  TRUE    rfclsSrc is smaller
//          FALSE   rfclsSrc is larger

template<class TYPE>
bool YCStringT<TYPE>::operator<(const YCStringT<TYPE>& rfclsSrc) const
{
	return (Compare(rfclsSrc) < 0);
}

//////////////////////////////////////////////////////////////////////////////////////////
//  < Operator (External function)
//
//  return  TRUE    rfclsSrc is smaller
//          FALSE   rfclsSrc is larger

template<class TYPE>
bool operator<(const char* pszSrc, const YCStringT<TYPE>& rfclsSrc)
{
	return (rfclsSrc.Compare(pszSrc) >= 0);
}

//////////////////////////////////////////////////////////////////////////////////////////
//  < Operator (External function)
//
//  return  TRUE    rfclsSrc is smaller
//          FALSE   rfclsSrc is larger

template<class TYPE>
bool operator<(const wchar_t* pwszSrc, const YCStringT<TYPE>& rfclsSrc)
{
	return (rfclsSrc.Compare(pwszSrc) >= 0);
}

//////////////////////////////////////////////////////////////////////////////////////////
//  <= Operator
//
//  return  TRUE    pszSrc is smaller or equal
//          FALSE   pszSrc is larger

template<class TYPE>
bool YCStringT<TYPE>::operator<=(const char* pszSrc) const
{
	return (Compare( pszSrc ) <= 0);
}

//////////////////////////////////////////////////////////////////////////////////////////
//  <= Operator
//
//  return  TRUE    pwszSrc is smaller or equal
//          FALSE   pwszSrc is larger

template<class TYPE>
bool YCStringT<TYPE>::operator<=(const wchar_t* pwszSrc) const
{
	return (Compare(pwszSrc) <= 0);
}

//////////////////////////////////////////////////////////////////////////////////////////
//  <= Operator
//
//  return  TRUE    rfclsSrc is smaller or equal
//          FALSE   rfclsSrc is larger

template<class TYPE>
bool YCStringT<TYPE>::operator<=(const YCStringT<TYPE>& rfclsSrc) const
{
	return (Compare(rfclsSrc) <= 0);
}

//////////////////////////////////////////////////////////////////////////////////////////
//  <= Operator (External function)
//
//  return  TRUE    rfclsSrc is smaller or equal
//          FALSE   rfclsSrc is larger

template<class TYPE>
bool operator<=(const char* pszSrc, const YCStringT<TYPE>& rfclsSrc)
{
	return (rfclsSrc.Compare(pszSrc) > 0);
}

//////////////////////////////////////////////////////////////////////////////////////////
//  <= Operator (External function)
//
//  return  TRUE    rfclsSrc is smaller or equal
//          FALSE   rfclsSrc is larger

template<class TYPE>
bool operator<=(const wchar_t* pwszSrc, const YCStringT<TYPE>& rfclsSrc)
{
	return (rfclsSrc.Compare(pwszSrc) > 0);
}

//////////////////////////////////////////////////////////////////////////////////////////
//  > Operator
//
//  return  TRUE    pszSrc is larger
//          FALSE   pszSrc is smaller

template<class TYPE>
bool YCStringT<TYPE>::operator>(const char* pszSrc) const
{
	return (Compare(pszSrc) > 0);
}

//////////////////////////////////////////////////////////////////////////////////////////
//  > Operator
//
//  return  TRUE    pwszSrc is larger
//          FALSE   pwszSrc is smaller

template<class TYPE>
bool YCStringT<TYPE>::operator>(const wchar_t* pwszSrc) const
{
	return (Compare(pwszSrc) > 0);
}

//////////////////////////////////////////////////////////////////////////////////////////
//  > Operator
//
//  return  TRUE    rfclsSrc is larger
//          FALSE   rfclsSrc is smaller

template<class TYPE>
bool YCStringT<TYPE>::operator>(const YCStringT<TYPE>& rfclsSrc) const
{
	return (Compare(rfclsSrc) > 0);
}

//////////////////////////////////////////////////////////////////////////////////////////
//  > Operator (External function)
//
//  return  TRUE    rfclsSrc is larger
//          FALSE   rfclsSrc is smaller

template<class TYPE>
bool operator>(const char* pszSrc, const YCStringT<TYPE>& rfclsSrc)
{
	return (rfclsSrc.Compare( pszSrc ) <= 0);
}

//////////////////////////////////////////////////////////////////////////////////////////
//  > Operator (External function)
//
//  return  TRUE    rfclsSrc is larger
//          FALSE   rfclsSrc is smaller

template<class TYPE>
bool operator>(const wchar_t* pwszSrc, const YCStringT<TYPE>& rfclsSrc)
{
	return (rfclsSrc.Compare(pwszSrc) <= 0);
}

//////////////////////////////////////////////////////////////////////////////////////////
//  >= Operator
//
//  return  TRUE    pszSrc is smaller
//          FALSE   pszSrc is larger

template<class TYPE>
bool YCStringT<TYPE>::operator>=(const char* pszSrc) const
{
	return (Compare( pszSrc ) >= 0);
}

//////////////////////////////////////////////////////////////////////////////////////////
//  >= Operator
//
//  return  TRUE    pwszSrc is smaller
//          FALSE   pwszSrc is larger

template<class TYPE>
bool YCStringT<TYPE>::operator>=(const wchar_t* pwszSrc) const
{
	return (Compare(pwszSrc) >= 0);
}

//////////////////////////////////////////////////////////////////////////////////////////
//  >= Operator
//
//  return  TRUE    rfclsSrc is smaller
//          FALSE   rfclsSrc is larger

template<class TYPE>
bool YCStringT<TYPE>::operator>=(const YCStringT<TYPE>& rfclsSrc) const
{
	return (Compare(rfclsSrc) >= 0);
}

//////////////////////////////////////////////////////////////////////////////////////////
//  >= Operator (External function)
//
//  return  TRUE    rfclsSrc is smaller
//          FALSE   rfclsSrc is larger

template<class TYPE>
bool operator>=(const char* pszSrc, const YCStringT<TYPE>&	rfclsSrc)
{
	return (rfclsSrc.Compare(pszSrc) < 0);
}

//////////////////////////////////////////////////////////////////////////////////////////
//  >= Operator (External function)
//
//  return  TRUE    rfclsSrc is smaller
//          FALSE   rfclsSrc is larger

template<class TYPE>
bool operator>=(const wchar_t* pwszSrc, const YCStringT<TYPE>& rfclsSrc)
{
	return (rfclsSrc.Compare(pwszSrc) < 0);
}

//////////////////////////////////////////////////////////////////////////////////////////
//  Element accessing
//
//  Remarks: Faster if you don't want to do anything in regards to bounds checking.
//           Though it still has a chance of failing.

template<class TYPE>
TYPE& YCStringT<TYPE>::operator[](int nPos)
{
	return m_ptString[nPos];
}

//////////////////////////////////////////////////////////////////////////////////////////
//  Element referencing
//
//  Remarks: Faster if you don't want to do anything in regards to bounds checking.
//           Though it still has a chance of failing.

template<class TYPE>
const TYPE& YCStringT<TYPE>::operator[](int nPos) const
{
	return m_ptString[nPos];
}

//////////////////////////////////////////////////////////////////////////////////////////
//  Gets the string

template<class TYPE>
YCStringT<TYPE>::operator const TYPE*() const
{
	return GetString();
}

//////////////////////////////////////////////////////////////////////////////////////////
//  Memory allocation

template<class TYPE>
void YCStringT<TYPE>::Alloc(int nBufferSize)
{
	// Alignment

	int nPaddingSize = (YCSTRINGT_BUFFERSIZE_ALIGNMENT - (nBufferSize % YCSTRINGT_BUFFERSIZE_ALIGNMENT));

	if (nPaddingSize < YCSTRINGT_BUFFERSIZE_ALIGNMENT)
	{
		nBufferSize += nPaddingSize;
	}

	// Ensure the string buffer has a sufficient size
	int nDataSize = (16 / sizeof(TYPE));

	m_ptString = new TYPE[nDataSize + nBufferSize];
	m_ptString += nDataSize;

	// Store the buffer size
	SetBufferSize(nBufferSize);
}

//////////////////////////////////////////////////////////////////////////////////////////
//  Freeing memory

template<class TYPE>
void YCStringT<TYPE>::Free()
{
	if (m_ptString != nullptr)
	{
		m_ptString -= (16 / sizeof(TYPE));
		delete[] m_ptString;
		m_ptString = nullptr;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
//  Buffer extending

template<class TYPE>
void YCStringT<TYPE>::ExtendBuffer(int nBufferSize)
{
	if (nBufferSize < GetBufferSize())
	{
		// No need to extend the buffer
		return;
	}

	TYPE* ptString = m_ptString;

	switch (sizeof(TYPE))
	{
	case 1: // char type
		Alloc(nBufferSize);
		lstrcpyA((char*)m_ptString, (const char*)ptString);
		break;

	case 2: // wchar_t type
		Alloc(nBufferSize);
		lstrcpyW((wchar_t*)m_ptString, (const wchar_t*)ptString);
		break;
	}

	// Release the previous buffer
	delete[] (ptString - (16 / sizeof(TYPE)));
}

//////////////////////////////////////////////////////////////////////////////////////////
//  Sets the buffer size for a string

template<class TYPE>
void YCStringT<TYPE>::SetBufferSize(int nBufferSize)
{
	*(int*) (m_ptString - (YCSTRINGT_OFFSET_BUFFERSIZE / sizeof(TYPE))) = nBufferSize;
}

//////////////////////////////////////////////////////////////////////////////////////////
//  Gets the buffer size of the string
//
//  Returns: String buffer size

template<class TYPE>
int YCStringT<TYPE>::GetBufferSize()
{
	if (m_ptString == nullptr)
		return 0;

	return *(int*) (m_ptString - (YCSTRINGT_OFFSET_BUFFERSIZE / sizeof(TYPE)));
}

//////////////////////////////////////////////////////////////////////////////////////////
//  Sets the length of a string

template<class TYPE>
void YCStringT<TYPE>::SetLength(int nLength)
{
	*(int*) (m_ptString - (YCSTRINGT_OFFSET_LENGTH / sizeof(TYPE))) = nLength;
}
