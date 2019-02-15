#pragma once

/// Local Memory Class
class YCLocalMemory final
{
public:
	YCLocalMemory();
	~YCLocalMemory();

	bool Alloc(u32 flags, u32 bytes);
	bool Free();

	void* Lock();
	bool Unlock();

	u32 GetSize() const;

	HLOCAL& GetHandle();
	void* GetPtr() const;

protected:
	HLOCAL m_memory_handle = nullptr;
	void* m_memory_ptr = nullptr;
};
