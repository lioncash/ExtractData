#pragma once

//----------------------------------------------------------------------------------------
//-- File Class ----------------------------------------------------------------------
//----------------------------------------------------------------------------------------

class	YCFile
{
public:

    enum
    {
        modeCreate          = 0x00000001,
        modeNoTruncate      = 0x00000002,
        modeRead            = 0x00000004,
        modeReadWrite       = 0x00000008,
        modeWrite           = 0x00000010,
        modeNoInherit       = 0x00000020,
        shareDenyNone       = 0x00000040,
        shareDenyRead       = 0x00000080,
        shareDenyWrite      = 0x00000100,
        shareExclusive      = 0x00000200,
        shareCompat         = 0x00000400,
        typeText            = 0x00000800,
        typeBinary          = 0x00001000,
        osNoBuffer          = 0x00002000,
        osWriteThrough      = 0x00004000,
        osRandomAccess      = 0x00008000,
        osSequentialScan    = 0x00010000,
    };

    enum
    {
        begin               = 0,
        current             = 1,
        end	                = 2,
    };

                                            YCFile();
    virtual                                 ~YCFile();

    virtual BOOL                            Open( LPCTSTR pszPathToFile, UINT uOpenFlags );

    virtual void                            Close();

    virtual DWORD                           Read( void* pvBuffer, DWORD dwReadSize );
    virtual DWORD                           Write( const void* pvBuffer, DWORD dwWriteSize );

    virtual UINT64                          Seek( INT64 n64Offset, DWORD dwSeekMode );
    virtual UINT64                          SeekHed( INT64 n64Offset = 0 );
    virtual UINT64                          SeekEnd( INT64 n64Offset = 0 );
    virtual UINT64                          SeekCur( INT64 n64Offset );

    virtual UINT64                          GetPosition();
    virtual UINT64                          GetLength();

    virtual YCString                        GetFilePath();
    virtual YCString                        GetFileName();
    virtual YCString                        GetFileExt();


protected:

    YCString                                m_clsPathToFile;
    YCString                                m_clsFileName;
    YCString                                m_clsFileExt;


private:

    HANDLE                                  m_hFile;
};
