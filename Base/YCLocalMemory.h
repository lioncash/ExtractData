
#pragma	once

//----------------------------------------------------------------------------------------
//-- ローカルメモリクラス ----------------------------------------------------------------
//----------------------------------------------------------------------------------------

class	YCLocalMemory
{
public:

											YCLocalMemory();
	virtual									~YCLocalMemory();

	BOOL									Alloc( UINT uFlags, UINT uBytes );
	BOOL									Free();

	void*									Lock();
	BOOL									Unlock();

	UINT									GetSize();

	HLOCAL&									GetHandle();
	void*									GetPtr();


protected:

	HLOCAL									m_hMemory;
	void*									m_pvMemory;
};
