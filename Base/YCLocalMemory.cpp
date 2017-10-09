#include "StdAfx.h"
#include "YCLocalMemory.h"

YCLocalMemory::YCLocalMemory()
{
}

YCLocalMemory::~YCLocalMemory()
{
	// Free memory
	Free();
}

/// Allocation of memory
///
/// @param flags Flags
/// @param bytes Bytes to allocate
///
bool YCLocalMemory::Alloc(u32 flags, u32 bytes)
{
	m_memory_handle = ::LocalAlloc(flags, bytes);

	if (m_memory_handle == nullptr)
		throw std::bad_alloc();

	return m_memory_handle != nullptr;
}

/// Release Memory
bool YCLocalMemory::Free()
{
	Unlock();

	if (m_memory_handle != nullptr)
	{
		if (::LocalFree(m_memory_handle) != nullptr)
			return false;

		m_memory_handle = nullptr;
	}

	return true;
}

/// Lock Memory
void* YCLocalMemory::Lock()
{
	m_memory_ptr = ::LocalLock(m_memory_handle);

	return m_memory_ptr;
}

/// Unlock Memory
bool YCLocalMemory::Unlock()
{
	if (m_memory_ptr != nullptr)
	{
		while (::LocalUnlock(m_memory_handle));
		m_memory_ptr = nullptr;
	}

	return true;
}

/// Get Memory Size
u32 YCLocalMemory::GetSize() const
{
	if (m_memory_handle == nullptr)
		return 0;

	return ::LocalSize(m_memory_handle);
}

/// Get Memory Handle
HLOCAL& YCLocalMemory::GetHandle()
{
	return m_memory_handle;
}

/// Get Memory Pointer
void* YCLocalMemory::GetPtr() const
{
	return m_memory_ptr;
}
