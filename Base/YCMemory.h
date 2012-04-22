#pragma	once

//#define	YCMemory	std::valarray

//----------------------------------------------------------------------------------------
//-- Memory Management Class -------------------------------------------------------------
//----------------------------------------------------------------------------------------

// Replace the part that is used as the buffer std :: vector and std :: valarray

template<class TYPE>
class	YCMemory
{
public:

											YCMemory();
											YCMemory( DWORD dwCount );
											YCMemory( DWORD dwCount, const TYPE& rftInitValue );
											YCMemory( const YCMemory<TYPE>& rfclmSrc );
											~YCMemory();

	void									resize( DWORD dwCount );
	void									resize( DWORD dwCount, const TYPE& rftInitValue );
	void									clear();

	inline	DWORD							size() const;
	TYPE									at( DWORD dwPos ) const;

	inline	TYPE&							operator[]( DWORD dwPos );
	inline	const TYPE&						operator[]( DWORD dwPos ) const;
	inline	YCMemory<TYPE>&					operator=( const YCMemory<TYPE>& rfclmSrc );


protected:

	void									alloc( DWORD dwCount );
	inline	void							fill( const TYPE& rftValue );

private:

	TYPE*									m_ptMemory;
	DWORD									m_dwMemoryCount;
};

//////////////////////////////////////////////////////////////////////////////////////////
//	Constructor

template<class TYPE> YCMemory<TYPE>::YCMemory()
{
	m_ptMemory = NULL;
	m_dwMemoryCount = 0;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	Constructor

template<class TYPE> YCMemory<TYPE>::YCMemory(
	DWORD				dwCount							// Element count
	)
{
	alloc( dwCount );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	Constructor

template<class TYPE> YCMemory<TYPE>::YCMemory(
	DWORD				dwCount,						// Element count
	const TYPE&			rftInitValue					// Initial Value
	)
{
	alloc( dwCount );

	fill( rftInitValue );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	Copy Constructor

template<class TYPE> YCMemory<TYPE>::YCMemory(
	const YCMemory<TYPE>&	rfclmSrc					// Copy source memory class
	)
{
	alloc( rfclmSrc.size() );

	memcpy( m_ptMemory, &rfclmSrc[0], sizeof(TYPE) * rfclmSrc.size() );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	Destructor

template<class TYPE> YCMemory<TYPE>::~YCMemory()
{
	clear();
}

//////////////////////////////////////////////////////////////////////////////////////////
//	Change Memory Size

template<class TYPE> void YCMemory<TYPE>::resize(
	DWORD				dwCount							// Element Count
	)
{
	clear();

	alloc( dwCount );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	Change Memory Size

template<class TYPE> void YCMemory<TYPE>::resize(
	DWORD				dwCount,						// Element Count
	const TYPE&			rftInitValue					// Initial Value
	)
{
	clear();

	alloc( dwCount );

	fill( rftInitValue );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	Release Memory

template<class TYPE> void YCMemory<TYPE>::clear()
{
	if( m_ptMemory != NULL )
	{
		delete[]	m_ptMemory;
		m_ptMemory = NULL;
		m_dwMemoryCount = 0;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
//	Acquisition of the number of elements

template<class TYPE> DWORD YCMemory<TYPE>::size() const
{
	return	m_dwMemoryCount;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	Element Reference
//
//	Remark: Throw an exception "std :: out_of_range" out of range for the reference

template<class TYPE> TYPE YCMemory<TYPE>::at(
	DWORD				dwPos							// Reference Position
	) const
{
	if( dwPos >= m_dwMemorySize )
	{
		// Outside the reference range.
        
		throw	std::out_of_range;
	}

	return	m_ptMemory[dwPos];
}

//////////////////////////////////////////////////////////////////////////////////////////
//	Access Element
//
//	Remark: Fast, but because it does not do anything out of range for the reference, note there is a risk of this failing

template<class TYPE> TYPE& YCMemory<TYPE>::operator[](
	DWORD				dwPos							// Reference Position
	)
{
	return	m_ptMemory[dwPos];
}

//////////////////////////////////////////////////////////////////////////////////////////
//	Element Reference
//
//	Remark: Fast, but because it does not do anything out of range for the reference, note there is a risk of this failing

template<class TYPE> const TYPE& YCMemory<TYPE>::operator[](
	DWORD				dwPos							// Reference Position
	) const
{
	return	m_ptMemory[dwPos];
}

//////////////////////////////////////////////////////////////////////////////////////////
//	Copy

template<class TYPE> YCMemory<TYPE>& YCMemory<TYPE>::operator=(
	const YCMemory<TYPE>&	rfclmSrc					// Copy source memory class
	)
{
	resize( rfclmSrc.size() );

	memcpy( m_ptMemory, &rfclmSrc[0], rfclmSrc.size() );

	return	*this;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	Memory Allocation

template<class TYPE> void YCMemory<TYPE>::alloc(
	DWORD				dwCount							// Element Count
	)
{
	if( dwCount == 0 )
	{
		// Address of memory allocation of 0 bytes

		dwCount = 1;
	}

	m_ptMemory = new TYPE[dwCount];
	m_dwMemoryCount = dwCount;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	Fill Memory

template<class TYPE> void YCMemory<TYPE>::fill(
	const TYPE&			rftValue						// value
	)
{
	for( DWORD i = 0 ; i < size() ; i++ )
	{
		m_ptMemory[i] = rftValue;
	}
}
