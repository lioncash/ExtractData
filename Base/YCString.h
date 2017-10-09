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
	YCStringT(const char* src);
	YCStringT(const char* src, int count);
	YCStringT(const char& src);
	YCStringT(const wchar_t* src);
	YCStringT(const wchar_t* src, int count);
	YCStringT(const wchar_t& src);
	YCStringT(const YCStringT<TYPE>& src);
	YCStringT(const YCStringT<TYPE>& src, int count);

	// Destructor
	~YCStringT(); // No longer a virtual function.

	// Assignment
	BOOL LoadString(UINT id);

	void Copy(const char* src);
	void Copy(const char* src, int length);
	void Copy(const char& src);
	void Copy(const wchar_t* src);
	void Copy(const wchar_t* src, int length);
	void Copy(const wchar_t& src);

	// Adding
	void Append(const char* append);
	void Append(const char* append, int length);
	void Append(const char& append);
	void Append(const wchar_t* append);
	void Append(const wchar_t* append, int length);
	void Append(const wchar_t& append);

	// Format data
	void Format(const TYPE* format, ...);
	void AppendFormat(const TYPE* format, ...);

	// Insert
	int Insert(int index, const TYPE* insert);
	int Insert(int index, const TYPE& insert);

	// Deleting
	void Clear();

	int Delete(int index, int count = 1);

	int Remove(const TYPE* remove);
	int Remove(const TYPE& remove);

	// Replacing
	int Replace(const TYPE* old_str, const TYPE* new_str);
	int Replace(const TYPE& old_char, const TYPE& new_char);

	// Uppercase - Lowercase
	YCStringT<TYPE>& MakeLower();
	YCStringT<TYPE>& MakeUpper();

	// Comparison
	int Compare(const char* src) const;
	int Compare(const char& src) const;
	int Compare(const wchar_t* src) const;
	int Compare(const wchar_t& src) const;

	int CompareNoCase(const char* src) const;
	int CompareNoCase(const char& src) const;
	int CompareNoCase(const wchar_t* src) const;
	int CompareNoCase(const wchar_t& src) const;

	// Searching
	int Find(const TYPE* sub, int start = 0) const;
	int Find(const TYPE& sub, int start = 0) const;

	int ReverseFind(const TYPE* sub) const;
	int ReverseFind(const TYPE& sub) const;

	// Extraction
	YCStringT<TYPE> Left(int count) const;

	// Gets a string buffer
	TYPE* GetBuffer(int buffer_size);

	// Gets a string buffer pointer
	inline const TYPE* GetString() const;

	// Gets the length of the string
	inline int GetLength() const;

	// Determine if a string is empty
	bool IsEmpty() const;

	// Path-related
	void RenameExtension(const TYPE* src);

	YCStringT<TYPE> GetFileName() const;
	int             GetFileNameIndex() const;
	YCStringT<TYPE> GetFileTitle() const;
	YCStringT<TYPE> GetFileExt() const;
	int             GetFileExtIndex() const;
	YCStringT<TYPE> GetDirPath() const;

	// Assignment operator
	YCStringT<TYPE>& operator=(const char* other);
	YCStringT<TYPE>& operator=(const char& other);
	YCStringT<TYPE>& operator=(const wchar_t* other);
	YCStringT<TYPE>& operator=(const wchar_t& other);
	YCStringT<TYPE>& operator=(const YCStringT<TYPE>& other);

	// + Operator
	YCStringT<TYPE> operator+(const char* append) const;
	YCStringT<TYPE> operator+(const char& append) const;
	YCStringT<TYPE> operator+(const wchar_t* append) const;
	YCStringT<TYPE> operator+(const wchar_t& append) const;
	YCStringT<TYPE> operator+(const YCStringT<TYPE>& append) const;

	// + Operator (External functions)
	template<class TYPE>
	friend YCStringT<TYPE> operator+(const char* lhs, const YCStringT<TYPE>& rhs);

	template<class TYPE>
	friend YCStringT<TYPE> operator+(const char& lhs, const YCStringT<TYPE>& rhs);

	template<class TYPE>
	friend YCStringT<TYPE> operator+(const wchar_t* lhs, const YCStringT<TYPE>& rhs);

	template<class TYPE>
	friend YCStringT<TYPE> operator+(const wchar_t& lhs, const YCStringT<TYPE>& rhs);

	// += Operator
	YCStringT<TYPE>& operator+=(const char* other);
	YCStringT<TYPE>& operator+=(const char& other);
	YCStringT<TYPE>& operator+=(const wchar_t* other);
	YCStringT<TYPE>& operator+=(const wchar_t& other);
	YCStringT<TYPE>& operator+=(const YCStringT<TYPE>& other);

	// == Operator
	bool operator==(const char* other) const;
	bool operator==(const char& other) const;
	bool operator==(const wchar_t* other) const;
	bool operator==(const wchar_t& other) const;
	bool operator==(const YCStringT<TYPE>& other) const;

	// == Operator(External functions)
	template<class TYPE>
	friend bool operator==(const char* lhs, const YCStringT<TYPE>& rhs);

	template<class TYPE>
	friend bool operator==(const char& lhs, const YCStringT<TYPE>& rhs);

	template<class TYPE>
	friend bool operator==(const wchar_t* lhs, const YCStringT<TYPE>& rhs);

	template<class TYPE>
	friend bool operator==(const wchar_t& lhs, const YCStringT<TYPE>& rhs);

	// != Operator
	bool operator!=(const char* other) const;
	bool operator!=(const char& other) const;
	bool operator!=(const wchar_t* other) const;
	bool operator!=(const wchar_t& other) const;
	bool operator!=(const YCStringT<TYPE>& other) const;

	// != Operator (External functions)
	template<class TYPE>
	friend bool operator!=(const char* lhs, const YCStringT<TYPE>& rhs);

	template<class TYPE>
	friend bool operator!=(const char& lhs, const YCStringT<TYPE>& rhs);

	template<class TYPE>
	friend bool operator!=(const wchar_t* lhs, const YCStringT<TYPE>& rhs);

	template<class TYPE>
	friend bool operator!=(const wchar_t& lhs, const YCStringT<TYPE>& rhs);

	// < Operator
	bool operator<(const char* other) const;
	bool operator<(const wchar_t* other) const;
	bool operator<(const YCStringT<TYPE>& other) const;

	// < Operator(External functions)
	template<class TYPE>
	friend bool operator<(const char* lhs, const YCStringT<TYPE>& rhs);

	template<class TYPE>
	friend bool operator<(const wchar_t* lhs, const YCStringT<TYPE>& rhs);

	// <= Operator
	bool operator<=(const char* other) const;
	bool operator<=(const wchar_t* other) const;
	bool operator<=(const YCStringT<TYPE>& other) const;

	// <= Operator (External functions)
	template<class TYPE>
	friend bool operator<=(const char* lhs, const YCStringT<TYPE>& rhs);

	template<class TYPE>
	friend bool operator<=(const wchar_t* lhs, const YCStringT<TYPE>& rhs);

	// > Operator
	bool operator>(const char* other) const;
	bool operator>(const wchar_t* other) const;
	bool operator>(const YCStringT<TYPE>& other) const;

	// > Operator(External functions)
	template<class TYPE>
	friend bool operator>(const char* lhs, const YCStringT<TYPE>& rhs);

	template<class TYPE>
	friend bool operator>(const wchar_t* lhs, const YCStringT<TYPE>& rhs);

	// >= Operator
	bool operator>=(const char* other) const;
	bool operator>=(const wchar_t* other) const;
	bool operator>=(const YCStringT<TYPE>& other) const;

	// >= Operator(External functions)
	template<class TYPE>
	friend bool operator>=(const char* lhs, const YCStringT<TYPE>& rhs);

	template<class TYPE>
	friend bool operator>=(const wchar_t* lhs, const YCStringT<TYPE>& rhs);

	// [] Operator
	inline TYPE&       operator[](int pos);
	inline const TYPE& operator[](int pos) const;

	// const TYPE* Operator
	inline operator const TYPE*() const;


protected:

	void Alloc(int buffer_size);
	void Free();

	void ExtendBuffer(int buffer_size);

	void SetBufferSize(int buffer_size);
	int GetBufferSize();

	void SetLength(int length);
};

using YCString  = YCStringT<TCHAR>;
using YCStringA = YCStringT<char>;
using YCStringW = YCStringT<wchar_t>;

/// Constructor
template<class TYPE>
YCStringT<TYPE>::YCStringT()
{
	m_ptString = nullptr;

	Clear();
}

/// Constructor
///
/// @param src Source string
///
template<class TYPE>
YCStringT<TYPE>::YCStringT(const char* src)
{
	m_ptString = nullptr;

	Copy(src);
}

/// Constructor
///
/// @param src   Source string
/// @param count Number of characters to copy
///
template<class TYPE>
YCStringT<TYPE>::YCStringT(const char* src, int count)
{
	m_ptString = nullptr;

	Copy(src, count);
}

/// Constructor
///
/// @param src Source string
///
template<class TYPE>
YCStringT<TYPE>::YCStringT(const char& src)
{
	m_ptString = nullptr;

	Copy(src);
}

/// Constructor
///
/// @param src Source string
///
template<class TYPE>
YCStringT<TYPE>::YCStringT(const wchar_t* src)
{
	m_ptString = nullptr;

	Copy(src);
}

/// Constructor
///
/// @param src   Source string
/// @param count Number of characters to copy
///
template<class TYPE>
YCStringT<TYPE>::YCStringT(const wchar_t* src, int count)
{
	m_ptString = nullptr;

	Copy(src, count);
}

/// Constructor
///
/// @param src Source string
///
template<class TYPE> YCStringT<TYPE>::YCStringT(const wchar_t& src)
{
	m_ptString = nullptr;

	Copy(src);
}

/// Copy Constructor
///
/// @param src Source string
///
template<class TYPE>
YCStringT<TYPE>::YCStringT(const YCStringT<TYPE>& src)
{
	m_ptString = nullptr;

	Copy(src);
}

/// Destructor
template<class TYPE>
YCStringT<TYPE>::~YCStringT()
{
	Free();
}

/// Reads a string resource
///
/// @param id Resource ID
///
template<class TYPE>
BOOL YCStringT<TYPE>::LoadString(UINT id)
{
	return ::LoadString(::GetModuleHandle(nullptr), id, GetBuffer(1024), 1024);
}

/// Assignment of a string
///
/// @param src Source string
///
template<class TYPE>
void YCStringT<TYPE>::Copy(const char* src)
{
	TYPE* string;
	int   src_length = 0;

	switch (sizeof(TYPE))
	{
	case 1: // char type
		src_length = lstrlenA(src);
		string = GetBuffer(src_length + 1);
		lstrcpyA((char*)string, src);
		break;

	case 2: // wchar_t type
		src_length = GetBaseTypeLength(src);
		string = GetBuffer(src_length + 1);
		ConvertToBaseType((wchar_t*)string, GetBufferSize(), src);
		break;
	}

	SetLength(src_length);
}

/// Assignment of a string
///
/// @param src   Source string
/// @param count Number of characters to copy
///
template<class TYPE>
void YCStringT<TYPE>::Copy(const char* src, int count)
{
	TYPE* string;
	int   src_length = 0;

	switch (sizeof(TYPE))
	{
	case 1: // char type
		src_length = count;
		string = GetBuffer(src_length + 1);
		lstrcpynA((char*)string, src, count + 1);
		break;

	case 2: // wchar_t type
		src_length = GetBaseTypeLength(src, count);
		string = GetBuffer(src_length + 1);
		ConvertToBaseType((wchar_t*)string, GetBufferSize(), src, count);
		break;
	}

	SetLength(src_length);
}

/// Assignment of a character
///
/// @param src Source character
///
template<class TYPE> 
void YCStringT<TYPE>::Copy(const char& src)
{
	char buffer[2];

	buffer[0] = src;
	buffer[1] = '\0';

	Copy(buffer);
}

/// Assignment of a string
///
/// @param src Source string
///
template<class TYPE>
void YCStringT<TYPE>::Copy(const wchar_t* src)
{
	TYPE* string;
	int   src_length = 0;

	switch (sizeof(TYPE))
	{
	case 1: // char type
		src_length = GetBaseTypeLength(src);
		string = GetBuffer(src_length + 1);
		ConvertToBaseType((char*)string, GetBufferSize(), src);
		break;

	case 2: // wchar_t type
		src_length = lstrlenW(src);
		string = GetBuffer(src_length + 1);
		lstrcpyW((wchar_t*)string, src);
		break;
	}

	SetLength(src_length);
}

/// Assignment of a string.
///
/// @param src   Source string
/// @param count Number of characters to copy
///
template<class TYPE>
void YCStringT<TYPE>::Copy(const wchar_t* src, int count)
{
	TYPE* string;
	int   src_length = 0;

	switch (sizeof(TYPE))
	{
	case 1: // char type
		src_length = GetBaseTypeLength(src, count);
		string = GetBuffer(src_length + 1);
		ConvertToBaseType((char*)string, GetBufferSize(), src, count);
		break;

	case 2: // wchar_t type
		src_length = count;
		string = GetBuffer(src_length + 1);
		lstrcpynW((wchar_t*)string, (wchar_t*)src, count + 1);
		break;
	}

	SetLength(src_length);
}

/// Assignment of a character
///
/// @param src Source character
///
template<class TYPE>
void YCStringT<TYPE>::Copy(const wchar_t& src)
{
	wchar_t buffer[2];

	buffer[0] = src;
	buffer[1] = L'\0';

	Copy(buffer);
}

/// Appending a string
///
/// @param append String to append
///
template<class TYPE>
void YCStringT<TYPE>::Append(const char* append)
{
	const int string_length = GetLength();
	int new_length = string_length;

	switch (sizeof(TYPE))
	{
	case 1: // char type
		new_length += lstrlenA(append);
		// Expand the buffer
		ExtendBuffer(new_length + 1);
		// Append the string
		lstrcpyA((char*)&m_ptString[string_length], append);
		break;

	case 2: // wchar_t type
		new_length += GetBaseTypeLength(append);
		// Expand the buffer
		ExtendBuffer(new_length + 1);
		// Append the string
		ConvertToBaseType((wchar_t*)&m_ptString[string_length], GetBufferSize() - string_length, append);
		break;
	}

	SetLength(new_length);
}

/// Appending a string
///
/// @param append String to append
/// @param count  Number of characters to append
///
template<class TYPE>
void YCStringT<TYPE>::Append(const char* append, int count)
{
	const int string_length = GetLength();
	int new_length = string_length;

	switch (sizeof(TYPE))
	{
	case 1: // char type
		new_length += count;
		// Expand the buffer
		ExtendBuffer(new_length + 1);
		// Append the character(s)
		lstrcpynA((char*)&m_ptString[string_length], append, count + 1);
		break;

	case 2: // wchar_t type
		new_length += GetBaseTypeLength(append, count);
		// Expand the buffer
		ExtendBuffer(new_length + 1);
		// Append the character(s)
		ConvertToBaseType((wchar_t*)&m_ptString[string_length], GetBufferSize() - string_length, append, count);
		break;
	}

	SetLength(new_length);
}

/// Appending a character
///
/// @param append Character to append
///
template<class TYPE>
void YCStringT<TYPE>::Append(const char& append)
{
	char buffer[2];

	buffer[0] = append;
	buffer[1] = '\0';

	Append(buffer);
}

/// Appending a string
///
/// @param append String to append
///
template<class TYPE>
void YCStringT<TYPE>::Append(const wchar_t* append)
{
	const int string_length = GetLength();
	int new_length = string_length;

	switch (sizeof(TYPE))
	{
	case 1: // char type
		new_length += GetBaseTypeLength(append);
		// Expand the buffer
		ExtendBuffer(new_length + 1);
		// Append the string
		ConvertToBaseType((char*)&m_ptString[string_length], (GetBufferSize() - string_length), append);
		break;

	case 2: // wchar_t type
		new_length += lstrlenW(append);
		// Expand the buffer
		ExtendBuffer(new_length + 1);
		// Append the string
		lstrcpyW((wchar_t*)&m_ptString[string_length], append);
		break;
	}

	SetLength(new_length);
}

/// Appending a string
///
/// @param append String to append
/// @param count  Number of characters to append
///
template<class TYPE>
void YCStringT<TYPE>::Append(const wchar_t* append, int count)
{
	const int string_length = GetLength();
	int new_length = string_length;

	switch (sizeof(TYPE))
	{
	case 1: // char type
		new_length += GetBaseTypeLength(append, count);
		// Expand the buffer
		ExtendBuffer(new_length + 1);
		// Append the string
		ConvertToBaseType((char*)&m_ptString[string_length], GetBufferSize() - string_length, append, count);
		break;

	case 2: // wchar_t type
		new_length += count;
		// Expand the buffer
		ExtendBuffer(new_length + 1);
		// Append the character(s)
		lstrcpynW((wchar_t*)&m_ptString[string_length], append, count + 1);
		break;
	}

	SetLength(new_length);
}

/// Appending a character
///
/// @param append Character to append
///
template<class TYPE>
void YCStringT<TYPE>::Append(const wchar_t& append)
{
	wchar_t buffer[2];

	buffer[0] = append;
	buffer[1] = L'\0';

	Append(buffer);
}

/// Formatting settings
///
/// @param format Formatted data
///
template<class TYPE> void YCStringT<TYPE>::Format(const TYPE* format, ...)
{
	va_list arg_ptr;
	va_start(arg_ptr, format);

	std::vector<TYPE> buffer(1024);

	while (true)
	{
		va_list work = arg_ptr;

		if (_vsntprintf(buffer.data(), (buffer.size() - 1), format, work) == -1)
		{
			buffer.resize(buffer.size() * 2);
		}
		else
		{
			break;
		}
	}

	va_end(arg_ptr);

	Copy(buffer.data());
}

/// Formatting settings
///
/// @param format Formatted data
///
template<class TYPE>
void YCStringT<TYPE>::AppendFormat(const TYPE* format, ...)
{
	va_list arg_ptr;
	va_start(arg_ptr, format);

	std::vector<TYPE> buffer(1024);

	while (true)
	{
		va_list work = arg_ptr;

		if (_vsntprintf(buffer.data(), (buffer.size() - 1), format, work) == -1)
		{
			buffer.resize(buffer.size() * 2);
		}
		else
		{
			break;
		}
	}

	va_end(arg_ptr);

	Append(buffer.data());
}


/// Insertion
///
/// @param index  Insertion offset
/// @param insert String to insert
///
/// @return The length of the string after insertion
///
template<class TYPE>
int YCStringT<TYPE>::Insert(int index, const TYPE* insert)
{
	if (index < 0)
	{
		// Insert at the beginning
		index = 0;
	}
	else if (index >= GetLength())
	{
		// Concatenation
		Append(insert);
		return GetLength();
	}

	// Insertion processing

	const int string_length = GetLength();
	int new_length = string_length;
	int insert_length = 0;
	int shift_length = 0;

	switch (sizeof(TYPE))
	{
	case 1: // char type
		insert_length = lstrlenA((const char*)insert);
		shift_length = lstrlenA((const char*)&m_ptString[index]);
		break;

	case 2: // wchar_t type
		insert_length = lstrlenW((const wchar_t*)insert);
		shift_length = lstrlenW((const wchar_t*)&m_ptString[index]);
		break;
	}

	// Expand the buffer
	new_length += insert_length;
	ExtendBuffer(new_length + 1);

	// Create a free spot to insert the string
	memmove(&m_ptString[index + insert_length], &m_ptString[index], sizeof(TYPE) * (shift_length + 1));

	// Insert the string
	memcpy(&m_ptString[index], insert, sizeof(TYPE) * insert_length);

	SetLength(new_length);

	return GetLength();
}

/// Character insertion
///
/// @param index  Insertion offset
/// @param insert Character to insert
///
/// @return The length of the string after insertion
///
template<class TYPE>
int YCStringT<TYPE>::Insert(int index, const TYPE& insert)
{
	TYPE buffer[2];

	buffer[0] = insert;
	buffer[1] = 0;

	return	Insert(index, buffer);
}

/// Clears the string
template<class TYPE>
void YCStringT<TYPE>::Clear()
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

/// Deletes the string
///
/// @param index Offset to being deletion at
/// @param count Number of characters to delete
///
/// @return The length of the string after deletion
///
template<class TYPE>
int YCStringT<TYPE>::Delete(int index, int count)
{
	const int string_length = GetLength();
	int new_length = string_length;

	if (index < 0)
	{
		// Delete from the beginning
		index = 0;
	}
	else if (index >= string_length)
	{
		// Don't delete anything
		return GetLength();
	}

	if (count <= 0)
	{
		// Don't delete anything
		return GetLength();
	}

	if (index + count >= string_length)
	{
		// Offset and # of characters to delete is larger than the whole string length.
		m_ptString[index] = 0;

		SetLength(index);

		return GetLength();
	}

	// Deletion processing
	const int shift_length = GetLength() - (index + count);

	// Delete the string
	// Overwriting will be packed to the left side of the string.
	memmove(&m_ptString[index], &m_ptString[index + count], sizeof(TYPE) * (shift_length + 1));

	new_length -= count;

	SetLength(new_length);

	return GetLength();
}

/// String deletion
///
/// @param remove String occurrence to remove
///
/// @return Number of times the given string was removed from the string.
///
template<class TYPE>
int YCStringT<TYPE>::Remove(const TYPE* remove)
{
	int index = 0;
	int count = 0;
	int remove_length = 0;

	switch (sizeof(TYPE))
	{
	case 1: // char type
		remove_length = lstrlenA(remove);
		break;

	case 2: // wchar_t type
		remove_length = lstrlenW(remove);
		break;
	}

	if (remove_length <= 0)
	{
		// Don't delete anything
		return count;
	}

	while (true)
	{
		index = Find(remove, index);

		// Lines to delete are present
		if (index >= 0)
		{
			Delete(index, remove_length);

			count++;
		}
		else // Nothing present
		{
			break;
		}
	}

	return count;
}

/// Character deletion
///
/// @param remove Character occurrence to delete
///
/// @return  Number of times the given character was removed from the string.
///
template<class TYPE>
int YCStringT<TYPE>::Remove(const TYPE& remove)
{
	int index = 0;
	int count = 0;
	const int remove_length = 1;

	while (true)
	{
		index = Find(remove, index);

		// Found characters to remove
		if (index >= 0)
		{
			Delete(index, remove_length);
			count++;
		}
		else // Nothing present to remove
		{
			break;
		}
	}

	return count;
}

/// Replacing
///
/// @param old_str String to replace
/// @param new_str String to replace with
///
/// @return Number of times replacements occurred
///
template<class TYPE>
int YCStringT<TYPE>::Replace(const TYPE* old_str, const TYPE* new_str)
{
	int index = 0;
	int count = 0;
	int string_length = GetLength();
	int new_string_length = string_length;
	int old_length = 0;
	int new_length = 0;

	switch (sizeof(TYPE))
	{
	case 1: // char type
		old_length = lstrlenA((const char*)old_str);
		new_length = lstrlenA((const char*)new_str);
		break;

	case 2: // wchar_t type
		old_length = lstrlenW((const wchar_t*)old_str);
		new_length = lstrlenW((const wchar_t*)new_str);
		break;
	}

	while (true)
	{
		index = Find(old_str, index);

		if (index >= 0)
		{
			// Target string to replace is present
			if (new_length == old_length)
			{
				// Length of the string cannot change, even after replacement.
				memcpy(&m_ptString[index], new_str, (sizeof(TYPE) * new_length));
			}
			else
			{
				// String length is increased/decreased after replacement
				new_string_length += new_length - old_length;

				ExtendBuffer(new_string_length + 1);

				memmove(&m_ptString[index + new_length], &m_ptString[index + old_length], sizeof(TYPE) * ((string_length + 1) - (index + old_length)));

				memcpy(&m_ptString[index], new_str, sizeof(TYPE) * new_length);

				string_length = new_string_length;
			}

			count++;
			index += new_length;
		}
		else
		{
			// Couldn't find any target string to replace
			break;
		}
	}

	SetLength(string_length);
	return count;
}

/// Replacing
///
/// @param old_char Character to replace
/// @param new_char Character to replace with
///
/// @return Number of times replacement occurred
///
template<class TYPE>
int YCStringT<TYPE>::Replace(const TYPE& old_char, const TYPE& new_char)
{
	int index = 0;
	int count = 0;

	while (true)
	{
		index = Find(old_char, index);

		if (index >= 0)
		{
			// Target character to replace exists
			m_ptString[index] = new_char;
			count++;
		}
		else
		{
			// Target character to replace was not present
			break;
		}
	}

	return count;
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
/// @param src Comparison string
///
/// @return 0               Both equal
///         Negative value  src is smaller
///         Positive value  src is larger
///
template<class TYPE>
int YCStringT<TYPE>::Compare(const char* src) const
{
	int return_val = 0;

	switch (sizeof(TYPE))
	{
	case 1: // char type
		return_val = lstrcmpA((const char*)m_ptString, src);
		break;

	case 2: // wchar_t type
	{
		const int work_length = GetBaseTypeLength(src);
		TYPE* work = new TYPE[work_length];
		ConvertToBaseType((wchar_t*)work, work_length, src);
		return_val = lstrcmpW((const wchar_t*)m_ptString, (const wchar_t*)work);
		delete[] work;
		break;
	}
	}

	return return_val;
}

/// Comparison
///
/// @param src Comparison character
///
/// @return 0               Both are equal
///         Negative value  src is smaller
///         Positive value  src is larger
///
template<class TYPE>
int YCStringT<TYPE>::Compare(const char& src) const
{
	char buffer[2];

	buffer[0] = src;
	buffer[1] = '\0';

	return Compare(buffer);
}

/// Comparison
///
/// @param src Comparison string
///
/// @return 0               Both are equal
///         Negative value  src is smaller
///         Positive value  src is larger
///
template<class TYPE>
int YCStringT<TYPE>::Compare(const wchar_t* src) const
{
	int return_val = 0;

	switch (sizeof(TYPE))
	{
	case 1: // char type
	{
		const int work_length = GetBaseTypeLength(src);
		TYPE* work = new TYPE[work_length];
		ConvertToBaseType((char*)work, work_length, src);
		return_val = lstrcmpA((const char*)m_ptString, (const char*)work);
		delete[] work;
		break;
	}

	case 2: // wchar_t type
		return_val = lstrcmpW((const wchar_t*)m_ptString, src);
		break;
	}

	return return_val;
}

/// Comparison
///
/// @param src Comparison character
///
/// @return 0               Both are equal
///         Negative value  src is smaller
///         Positive value  src is larger
///
template<class TYPE>
int YCStringT<TYPE>::Compare(const wchar_t& src) const
{
	wchar_t buffer[2];

	buffer[0] = src;
	buffer[1] = L'\0';

	return Compare(buffer);
}

/// Comparison
///
/// @param src Comparison string
///
/// @return 0               Both are equal
///         Negative value  src is smaller
///         Positive value  src is larger
///
/// @remark Casing of the characters are ignored.
///
template<class TYPE>
int YCStringT<TYPE>::CompareNoCase(const char* src) const
{
	int return_val = 0;

	switch (sizeof(TYPE))
	{
	case 1: // char type
		return_val = lstrcmpiA((const char*)m_ptString, src);
		break;

	case 2: // wchar_t type
	{
		const int work_length = GetBaseTypeLength(src);
		TYPE* work = new TYPE[work_length];
		ConvertToBaseType((wchar_t*)work, work_length, src);
		return_val = lstrcmpiW((const wchar_t*)m_ptString, (const wchar_t*)work);
		delete[] work;
		break;
	}
	}

	return return_val;
}

/// Comparison
///
/// @return 0              Both are equal
///        Negative value  src is smaller
///        Positive value  src is larger
///
/// @remark Casing of the character is ignored.
///
template<class TYPE>
int YCStringT<TYPE>::CompareNoCase(const char& src) const
{
	char buffer[2];

	buffer[0] = src;
	buffer[1] = '\0';

	return CompareNoCase(buffer);
}

/// Comparison
///
/// @param src Comparison string
///
/// @return 0               Both are equal
///         Negative value  src is smaller
///         Positive value  src is larger
///
/// @remark Casing of the characters are ignored.
///
template<class TYPE>
int YCStringT<TYPE>::CompareNoCase(const wchar_t* src) const
{
	int return_val = 0;

	switch (sizeof(TYPE))
	{
	case 1: // char type
	{
		const int work_length = GetBaseTypeLength(src);
		TYPE* work = new TYPE[work_length];
		ConvertToBaseType((char*)work, work_length, src);
		return_val = lstrcmpiA((const char*)m_ptString, (const char*)work);
		delete[] work;
		break;
	}

	case 2: // wchar_t type
		return_val = lstrcmpiW((const wchar_t*)m_ptString, src);
		break;
	}

	return return_val;
}

/// Comparison
///
/// @param src Comparison character
///
/// @return 0               Both are equal
///         Negative value  src is smaller
///         Positive value  src is larger
///
/// @remark Casing of the characters is ignored
///
template<class TYPE>
int YCStringT<TYPE>::CompareNoCase(const wchar_t& src) const
{
	wchar_t buffer[2];

	buffer[0] = src;
	buffer[1] = L'\0';

	return CompareNoCase(buffer);
}

/// Forward search
///
/// @param sub   The substring to search for.
/// @param start The index to begin searching at.
///
/// @return The index at which the string was found at.
///
/// @remark -1 is returned if a matching string could not be found
///
template<class TYPE>
int YCStringT<TYPE>::Find(const TYPE* sub, int start) const
{
	INT_PTR result = -1;

	if (start < 0)
	{
		// Search from the beginning of the string
		start = 0;
	}
	else if (start >= GetLength())
	{
		// Search from the end of the string
		return (int) result;
	}

	switch (sizeof(TYPE))
	{
		case 1: // char type
		{
			const u8* found = _mbsstr((const u8*)&m_ptString[start], (const u8*)sub);

			if (found != nullptr)
			{
				// Target string is present
				result = (found - (u8*) m_ptString);
			}
		}
		break;

		case 2: // wchar_t type
		{
			const wchar_t* found = wcsstr((const wchar_t*)&m_ptString[start], (const wchar_t*)sub);

			if (found != nullptr)
			{
				//  Target string is present
				result = found - (wchar_t*) m_ptString;
			}
		}
		break;
	}

	return (int) result;
}

/// Forward search
///
/// @param sub   The character to search for.
/// @param start The index to begin searching from.
///
/// @return The index at which the character was found.
///
template<class TYPE> int YCStringT<TYPE>::Find(const TYPE& sub, int start) const
{
	INT_PTR result = -1;

	if (start < 0)
	{
		// Search from the beginning of the string
		start = 0;
	}
	else if (start >= GetLength())
	{
		// Search from the end of the string
		return (int) result;
	}

	switch (sizeof(TYPE))
	{
		case 1: // char type
		{
			const u8* found = _mbschr((const u8*)&m_ptString[start], sub);

			if (found != nullptr)
			{
				// Target string is present
				result = found - (u8*) m_ptString;
			}
		}
		break;

		case 2: // wchar_t type
		{
			const wchar_t* found = wcschr((const wchar_t*)&m_ptString[start], sub);

			if (found != nullptr)
			{
				// Target string is present
				result = found - (wchar_t*) m_ptString;
			}
		}
		break;
	}

	return (int) result;
}

/// Backward search
///
/// @param sub The character to search for.
///
/// @return The index at which the beginning of the string was found.
///
/// @remark For multi-byte characters, this is pretty slow, so only use it when necessary.
///
template<class TYPE>
int YCStringT<TYPE>::ReverseFind(const TYPE* sub) const
{
	INT_PTR result = -1;

	// Getting the length of the search string
	int sub_length = 0;

	switch (sizeof(TYPE))
	{
	case 1: // char type
		sub_length = lstrlenA((const char*)sub);
		break;

	case 2: // wchar_t type
		sub_length = lstrlenW((const wchar_t*)sub);
		break;
	}

	if (sub_length <= 0)
	{
		// No search string
		return (int) result;
	}

	// Search from the end of the string
	const int start = GetLength() - sub_length;

	switch (sizeof(TYPE))
	{
	case 1: // char type
		const char* work = (const char*) &m_ptString[start];

		while (true)
		{
			if (lstrcmpA(work, (const char*)sub) == 0)
			{
				// Strings are equal
				result = work - (const char*) m_ptString;
				break;
			}

			if (work == (const char*)m_ptString)
				break;

			work = ::CharPrevA((const char*)m_ptString, work);
		}
		break;

	case 2: // wchar_t type
		for (int index = start; index >= 0; index--)
		{
			if (lstrcmpW((const wchar_t*)&m_ptString[index], (const wchar_t*)sub) == 0)
			{
				// Strings are equal
				result = index;
				break;
			}
		}

		break;
	}

	return (int) result;
}

/// Backward search
///
/// @param sub The character to search for.
///
/// @return The index at which the character was found.
///
template<class TYPE>
int YCStringT<TYPE>::ReverseFind(const TYPE& sub) const
{
	INT_PTR result = -1;

	switch (sizeof(TYPE))
	{
		case 1: // char type
		{
			const u8* found = _mbsrchr((const u8*)m_ptString, sub);

			if (found != nullptr)
			{
				// Target string is present
				result = found - (u8*) m_ptString;
			}
		}
		break;

		case 2: // wchar_t type
		{
			// wchar_t type
			const wchar_t* found = wcsrchr((wchar_t*)m_ptString, sub);

			if (found != nullptr)
			{
				// Target string is present
				result = found - (wchar_t*) m_ptString;
			}
		}
		break;
	}

	return (int) result;
}

/// Extracts the left part of the string
///
/// @return Extracted character string
///
template<class TYPE>
YCStringT<TYPE> YCStringT<TYPE>::Left(int count) const
{
	YCStringT<TYPE> result = *this;

	result.Delete(count, result.GetLength());

	return result;
}

/// Gets the string buffer
template<class TYPE> TYPE* YCStringT<TYPE>::GetBuffer(int buffer_size)
{
	if (buffer_size > GetBufferSize())
	{
		// Desired buffer size is larger than the current buffer's size
		Free();

		// Allocate memory
		Alloc(buffer_size);
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

/// Gets the the underlying character buffer for this string
///
/// @return The underlying character buffer
///
template<class TYPE>
const TYPE* YCStringT<TYPE>::GetString() const
{
	return m_ptString;
}


/// Whether or not the string is empty
///
/// @retval true if the string is empty.
/// @retval false if the string is not empty.
///
template<class TYPE>
bool YCStringT<TYPE>::IsEmpty() const
{
	bool result = false;

	switch (sizeof(TYPE))
	{
	case 1: // char type
		result = (*this == "");
		break;

	case 2: // wchar_t type
		result = (*this == L"");
		break;
	}

	return result;
}

/// Changes the extension of a file (for when using YCStrings to deal with files)
template<class TYPE>
void YCStringT<TYPE>::RenameExtension(const TYPE* new_extension)
{
	const int file_name_index = GetFileNameIndex();
	const int file_ext_index = GetFileExtIndex();

	// Found an extension
	if (file_ext_index >= file_name_index)
	{
		int new_string_length = GetLength();

		switch (sizeof(TYPE))
		{
		case 1: // char type
			new_string_length += lstrlenA((const char*)new_extension) - lstrlenA((const char*)&m_ptString[file_ext_index]);
			ExtendBuffer(new_string_length + 1);
			lstrcpyA((char*)&m_ptString[file_ext_index], (const char*)new_extension);
			break;

		case 2: // wchar_t type
			new_string_length += lstrlenA((const char*)new_extension) - lstrlenA((const char*)&m_ptString[file_ext_index]);
			ExtendBuffer(new_string_length + 1);
			lstrcpyW((wchar_t*)&m_ptString[file_ext_index], (const wchar_t*)new_extension);
			break;
		}

		SetLength(new_string_length);
	}
}

/// Gets the name of a file (when using YCStrings to deal with files)
template<class TYPE>
YCStringT<TYPE> YCStringT<TYPE>::GetFileName() const
{
	const int index = GetFileNameIndex();

	return &m_ptString[index];
}

/// Gets the index in the string where the name of a file begins
template<class TYPE> int YCStringT<TYPE>::GetFileNameIndex() const
{
	int found = -1;
	int index = 0;

	switch (sizeof(TYPE))
	{
	case 1: // char type
		found = ReverseFind('\\');
		break;

	case 2: // wchar_t type
		found = ReverseFind(L'\\');
		break;
	}

	if (found >= 0)
	{
		// Found a file name
		index = (found + 1);
	}
	else
	{
		// Whole string happens to be the file name
		index = 0;
	}

	return index;
}

/// Gets the name of a file (When using YCStrings to deal with file names)
template<class TYPE>
YCStringT<TYPE> YCStringT<TYPE>::GetFileTitle() const
{
	YCStringT<TYPE> file_title = GetFileName();
	TYPE            buffer[1];

	switch (sizeof(TYPE))
	{
	case 1: // char type
		buffer[0] = '\0';
		break;

	case 2: // wchar_t type
		buffer[0] = L'\0';
		break;
	}

	file_title.RenameExtension(buffer);

	return file_title;
}

/// Gets the file extension (When dealing with files, etc)
template<class TYPE>
YCStringT<TYPE> YCStringT<TYPE>::GetFileExt() const
{
	const int index = GetFileExtIndex();

	return &m_ptString[index];
}

/// Gets the index of a file extension (for when dealing with file names)
template<class TYPE>
int YCStringT<TYPE>::GetFileExtIndex() const
{
	int found = -1;
	int index = 0;

	switch (sizeof(TYPE))
	{
	case 1: // char type
		found = ReverseFind('.');
		break;

	case 2: // wchar_t type
		found = ReverseFind(L'.');
		break;
	}

	if (found >= 0)
	{
		// Extension found
		index = found;
	}
	else
	{
		// There is no extension
		index = GetLength();
	}

	return index;
}

/// Gets a folder path
template<class TYPE> YCStringT<TYPE> YCStringT<TYPE>::GetDirPath() const
{
	const int file_name_index = GetFileNameIndex();
	int target_index = 0;
	int drive_index = -1;

	if (file_name_index >= 1)
	{
		// Folder name found
		target_index = (file_name_index - 1);
	}

	switch (sizeof(TYPE))
	{
	case 1: // char type
		drive_index = ReverseFind(':');
		break;

	case 2: // wchar_t type
		drive_index = ReverseFind(L':');
		break;
	}

	if (drive_index >= 0)
	{
		// Drive name is included

		if (target_index == (drive_index + 1))
		{
			// '\\' immediately after ':'
			target_index = file_name_index;
		}
		else
		{
			// Don't delete anything prior to ':'
			target_index = (target_index < drive_index) ? (drive_index + 1) : target_index;
		}
	}

	return Left(target_index);
}

/// Assignment operator
template<class TYPE>
YCStringT<TYPE>& YCStringT<TYPE>::operator=(const char* other)
{
	Copy(other);
	return *this;
}


/// Assignment operator
template<class TYPE>
YCStringT<TYPE>& YCStringT<TYPE>::operator=(const char& other)
{
	Copy(other);
	return *this;
}

/// Assignment operator
template<class TYPE>
YCStringT<TYPE>& YCStringT<TYPE>::operator=(const wchar_t* other)
{
	Copy(other);
	return *this;
}

/// Assignment operator
template<class TYPE>
YCStringT<TYPE>& YCStringT<TYPE>::operator=(const wchar_t& other)
{
	Copy(other);
	return *this;
}

/// Assignment operator
template<class TYPE>
YCStringT<TYPE>& YCStringT<TYPE>::operator=(const YCStringT<TYPE>& other)
{
	Copy(other);
	return *this;
}

/// + Operator
template<class TYPE>
YCStringT<TYPE> YCStringT<TYPE>::operator+(const char* append) const
{
	YCStringT<TYPE> result = *this;
	result.Append(append);
	return result;
}

/// + Operator
template<class TYPE>
YCStringT<TYPE> YCStringT<TYPE>::operator+(const char& append) const
{
	YCStringT<TYPE> result = *this;
	result.Append(append);
	return result;
}

/// + Operator
template<class TYPE>
YCStringT<TYPE> YCStringT<TYPE>::operator+(const wchar_t* append) const
{
	YCStringT<TYPE> result = *this;
	result.Append(append);
	return result;
}

/// + Operator
template<class TYPE>
YCStringT<TYPE> YCStringT<TYPE>::operator+(const wchar_t& append) const
{
	YCStringT<TYPE> result = *this;
	result.Append(append);
	return result;
}

/// + Operator
template<class TYPE>
YCStringT<TYPE> YCStringT<TYPE>::operator+(const YCStringT<TYPE>& append) const
{
	YCStringT<TYPE> result = *this;
	result.Append(append);
	return result;
}

/// + Operator (External function)
template<class TYPE>
YCStringT<TYPE> operator+(const char* lhs, const YCStringT<TYPE>& rhs)
{
	YCStringT<TYPE> result = lhs;
	result.Append(rhs);
	return result;
}

/// + Operator (External function)
template<class TYPE>
YCStringT<TYPE> operator+(const char& lhs, const YCStringT<TYPE>& rhs)
{
	YCStringT<TYPE> result = lhs;
	result.Append(rhs);
	return result;
}

/// + Operator (External function)
template<class TYPE>
YCStringT<TYPE> operator+(const wchar_t* lhs, const YCStringT<TYPE>& rhs)
{
	YCStringT<TYPE> result = lhs;
	result.Append(rhs);
	return result;
}

/// + Operator (External function)
template<class TYPE>
YCStringT<TYPE> operator+(const wchar_t& lhs, const YCStringT<TYPE>& rhs)
{
	YCStringT<TYPE> result = lhs;
	result.Append(rhs);
	return result;
}

/// += Operator
template<class TYPE>
YCStringT<TYPE>& YCStringT<TYPE>::operator+=(const char* other)
{
	Append(other);
	return *this;
}

/// += Operator
template<class TYPE>
YCStringT<TYPE>& YCStringT<TYPE>::operator+=(const char& other)
{
	Append(other);
	return *this;
}

/// += Operator
template<class TYPE>
YCStringT<TYPE>& YCStringT<TYPE>::operator+=(const wchar_t* other)
{
	Append(other);
	return *this;
}

/// += Operator
template<class TYPE>
YCStringT<TYPE>& YCStringT<TYPE>::operator+=(const wchar_t& other)
{
	Append(other);
	return *this;
}

/// += Operator
template<class TYPE>
YCStringT<TYPE>& YCStringT<TYPE>::operator+=(const YCStringT<TYPE>& other)
{
	Append(other);
	return *this;
}

/// == Operator
template<class TYPE>
bool YCStringT<TYPE>::operator==(const char* other) const
{
	return Compare(other) == 0;
}

/// == Operator
template<class TYPE>
bool YCStringT<TYPE>::operator==(const char& other) const
{
	return Compare(other) == 0;
}

/// == Operator
template<class TYPE>
bool YCStringT<TYPE>::operator==(const wchar_t* other) const
{
	return Compare(other) == 0;
}

/// == Operator
template<class TYPE>
bool YCStringT<TYPE>::operator==(const wchar_t& other) const
{
	return Compare(other) == 0;
}

/// == Operator
template<class TYPE>
bool YCStringT<TYPE>::operator==(const YCStringT<TYPE>& other) const
{
	return Compare(other) == 0;
}

/// != Operator
template<class TYPE>
bool YCStringT<TYPE>::operator!=(const char* other) const
{
	return Compare(other) != 0;
}

/// != Operator
template<class TYPE>
bool YCStringT<TYPE>::operator!=(const char& other) const
{
	return Compare(other) != 0;
}

/// != Operator
template<class TYPE>
bool YCStringT<TYPE>::operator!=(const wchar_t* other) const
{
	return Compare(other) != 0;
}

/// != Operator
template<class TYPE>
bool YCStringT<TYPE>::operator!=(const wchar_t& other) const
{
	return Compare(other) != 0;
}

/// != Operator
template<class TYPE>
bool YCStringT<TYPE>::operator!=(const YCStringT<TYPE>& other) const
{
	return Compare(other) != 0;
}

/// < Operator
///
/// @retval true if other is smaller.
/// @retval false if other is larger.
///
template<class TYPE>
bool YCStringT<TYPE>::operator<(const char* other) const
{
	return Compare(other) < 0;
}

/// < Operator
///
/// @retval true if other is smaller.
/// @retval false if other is larger.
///
template<class TYPE>
bool YCStringT<TYPE>::operator<(const wchar_t* other) const
{
	return Compare(other) < 0;
}

/// < Operator
///
/// @retval true if other is smaller.
/// @retval false if other is larger.
///
template<class TYPE>
bool YCStringT<TYPE>::operator<(const YCStringT<TYPE>& other) const
{
	return Compare(other) < 0;
}

/// < Operator (External function)
///
/// @retval true if rhs is smaller.
/// @retval false if rhs is larger.
///
template<class TYPE>
bool operator<(const char* lhs, const YCStringT<TYPE>& rhs)
{
	return rhs.Compare(lhs) >= 0;
}

/// < Operator (External function)
///
/// @retval true if rhs is smaller.
/// @retval false if rhs is larger.
///
template<class TYPE>
bool operator<(const wchar_t* lhs, const YCStringT<TYPE>& rhs)
{
	return rhs.Compare(lhs) >= 0;
}

/// <= Operator
///
/// @retval true if other is smaller or equal.
/// @retval false if other is larger.
///
template<class TYPE>
bool YCStringT<TYPE>::operator<=(const char* other) const
{
	return Compare(other) <= 0;
}

/// <= Operator
///
/// @retval true if other is smaller or equal.
/// @retval false if other is larger.
///
template<class TYPE>
bool YCStringT<TYPE>::operator<=(const wchar_t* other) const
{
	return Compare(other) <= 0;
}

/// <= Operator
///
/// @retval true if other is smaller or equal.
/// @retval false if other is larger.
///
template<class TYPE>
bool YCStringT<TYPE>::operator<=(const YCStringT<TYPE>& other) const
{
	return Compare(other) <= 0;
}

/// <= Operator (External function)
///
/// @retval true if rhs is smaller or equal.
/// @retval false if rhs is larger.
///
template<class TYPE>
bool operator<=(const char* lhs, const YCStringT<TYPE>& rhs)
{
	return rhs.Compare(lhs) > 0;
}

/// <= Operator (External function)
///
/// @retval true if rhs is smaller or equal.
/// @retval false if rhs is larger.
///
template<class TYPE>
bool operator<=(const wchar_t* lhs, const YCStringT<TYPE>& rhs)
{
	return rhs.Compare(lhs) > 0;
}

/// > Operator
///
/// @retval true if other is larger.
/// @retval false if other is smaller.
///
template<class TYPE>
bool YCStringT<TYPE>::operator>(const char* other) const
{
	return Compare(other) > 0;
}

/// > Operator
///
/// @retval true if other is larger.
/// @retval false if other is smaller.
///
template<class TYPE>
bool YCStringT<TYPE>::operator>(const wchar_t* other) const
{
	return Compare(other) > 0;
}

/// > Operator
///
/// @retval true if other is larger.
/// @retval false if other is smaller.
///
template<class TYPE>
bool YCStringT<TYPE>::operator>(const YCStringT<TYPE>& other) const
{
	return Compare(other) > 0;
}

/// > Operator (External function)
///
/// @retval true if rhs is larger.
/// @retval false if rhs is smaller.
///
template<class TYPE>
bool operator>(const char* lhs , const YCStringT<TYPE>& rhs)
{
	return rhs.Compare(lhs) <= 0;
}

/// > Operator (External function)
///
/// @retval true if rhs is larger.
/// @retval false if rhs is smaller.
///
template<class TYPE>
bool operator>(const wchar_t* lhs, const YCStringT<TYPE>& rhs)
{
	return rhs.Compare(lhs) <= 0;
}

/// >= Operator
///
/// @retval true if other is smaller.
/// @retval false if other is larger.
///
template<class TYPE>
bool YCStringT<TYPE>::operator>=(const char* other) const
{
	return Compare(other) >= 0;
}

/// >= Operator
///
/// @retval true if other is smaller.
/// @retval false if other is larger.
///
template<class TYPE>
bool YCStringT<TYPE>::operator>=(const wchar_t* other) const
{
	return Compare(other) >= 0;
}

/// >= Operator
///
/// @retval true if other is smaller.
/// @retval false if other is larger.
///
template<class TYPE>
bool YCStringT<TYPE>::operator>=(const YCStringT<TYPE>& other) const
{
	return Compare(other) >= 0;
}

/// >= Operator (External function)
///
/// @retval true if rhs is smaller.
/// @retval false if rhs is larger.
///
template<class TYPE>
bool operator>=(const char* lhs, const YCStringT<TYPE>& rhs)
{
	return rhs.Compare(lhs) < 0;
}

/// >= Operator (External function)
///
/// @retval true if rhs is smaller.
/// @retval false if rhs is larger.
///
template<class TYPE>
bool operator>=(const wchar_t* lhs, const YCStringT<TYPE>& rhs)
{
	return rhs.Compare(lhs) < 0;
}

/// Element accessing
///
/// @remark Faster if you don't want to do anything in regards to bounds checking.
///         Though it still has a chance of failing.
///
template<class TYPE>
TYPE& YCStringT<TYPE>::operator[](int pos)
{
	return m_ptString[pos];
}

/// Element referencing
///
/// @remark Faster if you don't want to do anything in regards to bounds checking.
///         Though it still has a chance of failing.
///
template<class TYPE>
const TYPE& YCStringT<TYPE>::operator[](int pos) const
{
	return m_ptString[pos];
}

/// Implicit string conversion operator
template<class TYPE>
YCStringT<TYPE>::operator const TYPE*() const
{
	return GetString();
}

/// Memory allocation
template<class TYPE>
void YCStringT<TYPE>::Alloc(int buffer_size)
{
	// Alignment
	const int padding_size = YCSTRINGT_BUFFERSIZE_ALIGNMENT - (buffer_size % YCSTRINGT_BUFFERSIZE_ALIGNMENT);

	if (padding_size < YCSTRINGT_BUFFERSIZE_ALIGNMENT)
	{
		buffer_size += padding_size;
	}

	// Ensure the string buffer has a sufficient size
	const int data_size = (16 / sizeof(TYPE));

	m_ptString = new TYPE[data_size + buffer_size];
	m_ptString += data_size;

	// Store the buffer size
	SetBufferSize(buffer_size);
}

/// Freeing memory
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

/// Buffer extending
template<class TYPE>
void YCStringT<TYPE>::ExtendBuffer(int buffer_size)
{
	if (buffer_size < GetBufferSize())
	{
		// No need to extend the buffer
		return;
	}

	const TYPE* string = m_ptString;

	switch (sizeof(TYPE))
	{
	case 1: // char type
		Alloc(buffer_size);
		lstrcpyA((char*)m_ptString, (const char*)string);
		break;

	case 2: // wchar_t type
		Alloc(buffer_size);
		lstrcpyW((wchar_t*)m_ptString, (const wchar_t*)string);
		break;
	}

	// Release the previous buffer
	delete[] (string - (16 / sizeof(TYPE)));
}

/// Sets the buffer size for a string
template<class TYPE>
void YCStringT<TYPE>::SetBufferSize(int buffer_size)
{
	*(int*) (m_ptString - (YCSTRINGT_OFFSET_BUFFERSIZE / sizeof(TYPE))) = buffer_size;
}

/// Gets the buffer size of the string
///
/// @returns String buffer size
///
template<class TYPE>
int YCStringT<TYPE>::GetBufferSize()
{
	if (m_ptString == nullptr)
		return 0;

	return *(int*) (m_ptString - (YCSTRINGT_OFFSET_BUFFERSIZE / sizeof(TYPE)));
}

/// Sets the length of a string
template<class TYPE>
void YCStringT<TYPE>::SetLength(int length)
{
	*(int*) (m_ptString - (YCSTRINGT_OFFSET_LENGTH / sizeof(TYPE))) = length;
}
