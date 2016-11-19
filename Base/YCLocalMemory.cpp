#include "StdAfx.h"
#include "YCLocalMemory.h"

/// Constructor
YCLocalMemory::YCLocalMemory()
{
	m_hMemory = nullptr;
	m_pvMemory = nullptr;
}

/// Destructor
YCLocalMemory::~YCLocalMemory()
{
	// Free memory

	Free();
}

/// Allocation of memory
///
/// @param uFlags Flags
/// @param uBytes Bytes to allocate
///
bool YCLocalMemory::Alloc(UINT uFlags, UINT uBytes)
{
	m_hMemory = ::LocalAlloc(uFlags, uBytes);

	if (m_hMemory == nullptr)
		throw std::bad_alloc();

	return (m_hMemory != nullptr);
}

/// Release Memory
bool YCLocalMemory::Free()
{
	Unlock();

	if (m_hMemory != nullptr)
	{
		if (::LocalFree(m_hMemory) != nullptr)
			return false;

		m_hMemory = nullptr;
	}

	return true;
}

/// Lock Memory
void* YCLocalMemory::Lock()
{
	m_pvMemory = ::LocalLock(m_hMemory);

	return m_pvMemory;
}

/// Unlock Memory
bool YCLocalMemory::Unlock()
{
	if (m_pvMemory != nullptr)
	{
		while (::LocalUnlock(m_hMemory));
		m_pvMemory = nullptr;
	}

	return true;
}

/// Get Memory Size
UINT YCLocalMemory::GetSize() const
{
	if (m_hMemory == nullptr)
		return 0;

	return ::LocalSize(m_hMemory);
}

/// Get Memory Handle
HLOCAL& YCLocalMemory::GetHandle()
{
	return m_hMemory;
}

/// Get Memory Pointer
void* YCLocalMemory::GetPtr() const
{
	return m_pvMemory;
}
