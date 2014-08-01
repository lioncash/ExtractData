#pragma once

//----------------------------------------------------------------------------------------
//-- Local Memory Class ------------------------------------------------------------------
//----------------------------------------------------------------------------------------

class YCLocalMemory
{
public:
	YCLocalMemory();
	virtual ~YCLocalMemory();

	bool Alloc(UINT uFlags, UINT uBytes);
	bool Free();

	void* Lock();
	bool Unlock();

	UINT GetSize();

	HLOCAL& GetHandle();
	void* GetPtr();

protected:
	HLOCAL m_hMemory;
	void* m_pvMemory;
};
