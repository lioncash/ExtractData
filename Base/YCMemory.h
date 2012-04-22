
#pragma	once

//#define	YCMemory	std::valarray

//----------------------------------------------------------------------------------------
//-- メモリ管理クラス --------------------------------------------------------------------
//----------------------------------------------------------------------------------------

// std::vectorやstd::valarrayをバッファとして利用している部分を置き換える

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
//	コンストラクタ

template<class TYPE> YCMemory<TYPE>::YCMemory()
{
	m_ptMemory = NULL;
	m_dwMemoryCount = 0;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	コンストラクタ

template<class TYPE> YCMemory<TYPE>::YCMemory(
	DWORD				dwCount							// 要素数
	)
{
	alloc( dwCount );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	コンストラクタ

template<class TYPE> YCMemory<TYPE>::YCMemory(
	DWORD				dwCount,						// 要素数
	const TYPE&			rftInitValue					// 初期値
	)
{
	alloc( dwCount );

	fill( rftInitValue );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	コピーコンストラクタ

template<class TYPE> YCMemory<TYPE>::YCMemory(
	const YCMemory<TYPE>&	rfclmSrc					// コピー元メモリクラス
	)
{
	alloc( rfclmSrc.size() );

	memcpy( m_ptMemory, &rfclmSrc[0], sizeof(TYPE) * rfclmSrc.size() );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	デストラクタ

template<class TYPE> YCMemory<TYPE>::~YCMemory()
{
	clear();
}

//////////////////////////////////////////////////////////////////////////////////////////
//	メモリサイズの変更

template<class TYPE> void YCMemory<TYPE>::resize(
	DWORD				dwCount							// 要素数
	)
{
	clear();

	alloc( dwCount );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	メモリサイズの変更

template<class TYPE> void YCMemory<TYPE>::resize(
	DWORD				dwCount,						// 要素数
	const TYPE&			rftInitValue					// 初期値
	)
{
	clear();

	alloc( dwCount );

	fill( rftInitValue );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	メモリの解放

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
//	要素数の取得

template<class TYPE> DWORD YCMemory<TYPE>::size() const
{
	return	m_dwMemoryCount;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	要素の参照
//
//	備考	範囲外の参照に対して「std::out_of_range」例外を投げる

template<class TYPE> TYPE YCMemory<TYPE>::at(
	DWORD				dwPos							// 参照位置
	) const
{
	if( dwPos >= m_dwMemorySize )
	{
		// 範囲外の参照

		throw	std::out_of_range;
	}

	return	m_ptMemory[dwPos];
}

//////////////////////////////////////////////////////////////////////////////////////////
//	要素にアクセス
//
//	備考	範囲外の参照に対して何もしないため高速だが、落ちる危険性があるので注意

template<class TYPE> TYPE& YCMemory<TYPE>::operator[](
	DWORD				dwPos							// 参照位置
	)
{
	return	m_ptMemory[dwPos];
}

//////////////////////////////////////////////////////////////////////////////////////////
//	要素の参照
//
//	備考	範囲外の参照に対して何もしないため高速だが、落ちる危険性があるので注意

template<class TYPE> const TYPE& YCMemory<TYPE>::operator[](
	DWORD				dwPos							// 参照位置
	) const
{
	return	m_ptMemory[dwPos];
}

//////////////////////////////////////////////////////////////////////////////////////////
//	コピー

template<class TYPE> YCMemory<TYPE>& YCMemory<TYPE>::operator=(
	const YCMemory<TYPE>&	rfclmSrc					// コピー元メモリクラス
	)
{
	resize( rfclmSrc.size() );

	memcpy( m_ptMemory, &rfclmSrc[0], rfclmSrc.size() );

	return	*this;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	メモリの確保

template<class TYPE> void YCMemory<TYPE>::alloc(
	DWORD				dwCount							// 要素数
	)
{
	if( dwCount == 0 )
	{
		// 0バイトのメモリ確保の対処

		dwCount = 1;
	}

	m_ptMemory = new TYPE[dwCount];
	m_dwMemoryCount = dwCount;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	メモリを埋める

template<class TYPE> void YCMemory<TYPE>::fill(
	const TYPE&			rftValue						// 値
	)
{
	for( DWORD i = 0 ; i < size() ; i++ )
	{
		m_ptMemory[i] = rftValue;
	}
}
