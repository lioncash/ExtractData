#pragma once

#include <stdexcept>

//#define YCMemory std::valarray

//----------------------------------------------------------------------------------------
//-- Memory Management Class -------------------------------------------------------------
//----------------------------------------------------------------------------------------

// Replace the part that is used as the buffer std :: vector and std :: valarray

template<class TYPE>
class YCMemory
{
public:
	YCMemory();
	explicit YCMemory(size_t count);
	YCMemory(size_t count, const TYPE& init_value);
	YCMemory(const YCMemory<TYPE>& src);
	~YCMemory();

	void resize(size_t count);
	void resize(size_t count, const TYPE& init_value);
	void clear();

	size_t size() const;
	TYPE at(size_t pos) const;

	TYPE* data();
	const TYPE* data() const;

	TYPE& operator[](size_t pos);
	const TYPE& operator[](size_t pos) const;
	YCMemory<TYPE>& operator=(const YCMemory<TYPE>& src);

protected:
	void alloc(size_t count);
	void fill(const TYPE& value);

private:
	TYPE* m_memory = nullptr;
	size_t m_size = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Constructor

template<class TYPE>
YCMemory<TYPE>::YCMemory()
{
}

//////////////////////////////////////////////////////////////////////////////////////////
// Constructor

template<class TYPE>
YCMemory<TYPE>::YCMemory(size_t count)
{
	alloc(count);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Constructor

template<class TYPE>
YCMemory<TYPE>::YCMemory(size_t count, const TYPE& init_value)
{
	alloc(count);

	fill(init_value);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Copy Constructor

template<class TYPE>
YCMemory<TYPE>::YCMemory(const YCMemory<TYPE>& src)
{
	alloc(src.size());

	memcpy(m_memory, &src[0], sizeof(TYPE)* src.size());
}

//////////////////////////////////////////////////////////////////////////////////////////
// Destructor

template<class TYPE>
YCMemory<TYPE>::~YCMemory()
{
	clear();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Change Memory Size

template<class TYPE>
void YCMemory<TYPE>::resize(size_t count)
{
	clear();

	alloc(count);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Change Memory Size

template<class TYPE>
void YCMemory<TYPE>::resize(size_t count, const TYPE& init_value)
{
	clear();

	alloc(count);

	fill(init_value);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Release Memory

template<class TYPE> void YCMemory<TYPE>::clear()
{
	if (m_memory != nullptr)
	{
		delete[] m_memory;
		m_memory = nullptr;
		m_size = 0;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
// Acquisition of the number of elements

template<class TYPE>
size_t YCMemory<TYPE>::size() const
{
	return m_size;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Element Reference
//
// Remark: Throw an exception "std :: out_of_range" out of range for the reference

template<class TYPE>
TYPE YCMemory<TYPE>::at(size_t pos) const
{
	if (pos >= m_size)
	{
		// Outside the reference range.

		throw std::out_of_range("dwPos is outsize the memory range.");
	}

	return m_memory[pos];
}

//////////////////////////////////////////////////////////////////////////////////////////
// Data Access
//

template<class TYPE>
TYPE* YCMemory<TYPE>::data()
{
	return m_memory;
}

template<class TYPE>
const TYPE* YCMemory<TYPE>::data() const
{
	return m_memory;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Element Reference
//
// Remark: Throw an exception "std :: out_of_range" out of range for the reference

//////////////////////////////////////////////////////////////////////////////////////////
// Access Element
//
// Remark: Fast, but because it does not do anything out of range for the reference, note there is a risk of this failing

template<class TYPE>
TYPE& YCMemory<TYPE>::operator[](size_t pos)
{
	return m_memory[pos];
}

//////////////////////////////////////////////////////////////////////////////////////////
// Element Reference
//
// Remark: Fast, but because it does not do anything out of range for the reference, note there is a risk of this failing

template<class TYPE>
const TYPE& YCMemory<TYPE>::operator[](size_t pos) const
{
	return m_memory[pos];
}

//////////////////////////////////////////////////////////////////////////////////////////
// Copy

template<class TYPE>
YCMemory<TYPE>& YCMemory<TYPE>::operator=(const YCMemory<TYPE>& src)
{
	resize(src.size());

	memcpy(m_memory, &src[0], src.size());

	return *this;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Memory Allocation

template<class TYPE>
void YCMemory<TYPE>::alloc(size_t count)
{
	if (count == 0)
	{
		// Address of memory allocation of 0 bytes

		count = 1;
	}

	m_memory = new TYPE[count];
	m_size = count;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Fill Memory

template<class TYPE>
void YCMemory<TYPE>::fill(const TYPE& value)
{
	std::fill(m_memory, m_memory + m_size, value);
}
