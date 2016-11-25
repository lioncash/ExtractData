#include "StdAfx.h"
#include "EntisGLS.h"

/// Mount
///
/// @param archive Archive
///
bool CEntisGLS::Mount(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".noa"))
		return false;

	if (memcmp(archive->GetHeader(), "Entis\x1A\0\0", 8) != 0)
		return false;

//	if( memcmp( archive->GetHed(), ""


	//  Read 64 bytes - Header check
	//  Read 16 bytes - DirEntry
	//  Read 4 bytes - Obtain number of entries

	//  Read 8 bytes - Get original file size
	//  Read 4 bytes…Unknown
	//  Read 4 bytes…Gets file type(10 00 00 80 is a file、10 00 00 00 is a folder)
	//  Read 8 bytes…Unknown
	//  Read 8 bytes…Unknown
	//  Read 4 bytes…Unknown
	//  Read 4 bytes long and get file name
	//  Reading of the filename length segment gets the file name

	//  Read 8 bytes…Gets original file size
	//  Read 4 bytes…Unknown
	//  Read 4 bytes…Gets filetype
	//  Read 8 bytes…Unknown
	//  Read 8 bytes…Unknown
	//  Read 4 bytes…Unknown
	//  Read 4 bytes - Gets file name
	//  Gets the file name by reading its segment

	return false;
}

/// Decode
///
/// @param archive Archive
///
bool CEntisGLS::Decode(CArcFile* archive)
{
	return false;
}
