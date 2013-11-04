#pragma once

#include "YCFile.h"

//----------------------------------------------------------------------------------------
//-- Text File Class ---------------------------------------------------------------------
//----------------------------------------------------------------------------------------

class YCStdioFile : public YCFile
{
public:

							YCStdioFile();
	virtual					~YCStdioFile();

	virtual BOOL			Open( LPCTSTR pszPathToFile, UINT uOpenFlags );
	virtual void			Close();

	virtual DWORD			Read( void* pvBuffer, DWORD dwReadSize );
	virtual DWORD			Write( const void* pvBuffer, DWORD dwWriteSize );

	virtual LPTSTR			ReadString( LPTSTR pszBuffer, DWORD dwBufferSize );
	virtual BOOL			ReadString( YCString& rfclsBuffer );
	virtual void			WriteString( LPCTSTR pszBuffer );

	virtual UINT64			Seek( INT64 offset, DWORD SeekMode );


private:

	FILE*					m_pStream;
};
