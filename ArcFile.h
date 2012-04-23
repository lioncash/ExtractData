#pragma once

#include "ProgressBar.h"
#include "MD5.h"

class CArcFile
{
public:

                                            CArcFile();
    virtual                                 ~CArcFile();

    BOOL                                    Mount();
    BOOL                                    Decode();
    BOOL                                    Extract();

    // Archive file manipulation

    BOOL                                    Open( LPCTSTR pszPathToArc );
    void                                    Close();

    DWORD                                   Read( void* pvBuffer, DWORD dwReadSize );
    BYTE*                                   ReadHed();

    UINT64                                  Seek( INT64 n64Offset, DWORD dwSeekMode );
    UINT64                                  SeekHed( INT64 n64Offset = 0 );
    UINT64                                  SeekEnd( INT64 n64Offset = 0 );
    UINT64                                  SeekCur( INT64 n64Offset );

    UINT64                                  GetArcPointer();
    UINT64                                  GetArcSize();

    // Output file operations

    YCString                                CreateFileName( LPCTSTR pszReFileExt = NULL );

    BOOL                                    OpenFile( LPCTSTR pszReFileExt = NULL );
    BOOL                                    OpenScriptFile();
    void                                    CloseFile();

    DWORD                                   WriteFile( const void* pvBuffer, DWORD dwWriteSize, DWORD dwSizeOrg = 0xFFFFFFFF );
    void                                    ReadWrite();
    void                                    ReadWrite(DWORD FileSize);

    // Simple decoding

    DWORD                                   InitDecrypt();
    DWORD                                   InitDecrypt( const void* pvData );
    DWORD                                   InitDecryptForText( const void* pvText, DWORD dwTextSize );
    void                                    Decrypt( void* pvBuffer, DWORD dwSize );

    // File information to be added to the list in the archive

    void                                    AddFileInfo( SFileInfo& infFile );
    void                                    AddFileInfo( SFileInfo& infFile, DWORD& ctFile, LPCTSTR pFileExt );

    YCString                                SetFileFormat( const YCString& sFilePath );
    YCString                                SetCommaFormat( DWORD dwSize );

    // Initialization processes performed prior to decoding/mounting

    void SetFileInfo(DWORD num)                 { m_pInfFile = &(*m_pEnt)[num]; m_dwArcsID = m_pInfFile->arcsID; m_dwInfFileNum = num; }
    void SetArcID(DWORD dwArcID)                { m_dwArcID = dwArcID; }
    void SetEnt(std::vector<SFileInfo>& rEnt)   { m_pEnt = &rEnt; m_nStartEnt = rEnt.size(); }
    void SetProg(CProgBar& prog)                { m_pProg = &prog; }
    void SetOpt(SOption* pOption)               { m_pOption = pOption; }
    void SetSaveDir(LPCTSTR pSaveDir)           { m_pSaveDir = pSaveDir; }

    // TRUE if file is supported(Closed immerdiately if the file is not supported)

    void SetState(BOOL bState) { m_bState = bState; }

    // Split files support
    
    void        SetFirstArc()               { m_dwArcsID = 0; }
    void        SetNextArc()                { m_dwArcsID++; }
    HANDLE      GetArcHandle()              { return (m_hArcs[m_dwArcsID]); }
    void        SetArcsID( DWORD dwArcsID ) { m_dwArcsID = dwArcsID; }
    DWORD       GetArcsID()	                { return (m_dwArcsID); }
    size_t      GetArcCount()               { return (m_hArcs.size()); }
    YCString&   GetArcPath()                { return (m_pclArcPaths[m_dwArcsID]); }
    YCString&   GetArcName()                { return (m_pclArcNames[m_dwArcsID]); }
    YCString&   GetArcExten()               { return (m_pclArcExtens[m_dwArcsID]); }

    // Returns previously loaded archive file header

    LPBYTE GetHed() { return (m_pHeader); }

    SFileInfo*                  GetFileInfo( DWORD num )    { return (&(*m_pEnt)[num]); }
    SFileInfo*                  GetFileInfo( PCTSTR pszFileName, BOOL bCmpFileNameOnly = FALSE );
    const SFileInfo*            GetFileInfoForBinarySearch( LPCTSTR pszFileName );
    std::vector<SFileInfo>&	    GetFileInfo()               { return (*m_pEnt); }
    SFileInfo*                  GetOpenFileInfo()           { return (m_pInfFile); }
    DWORD                       GetOpenFileInfoNum()        { return (m_dwInfFileNum); }

    DWORD                       GetArcID()                  { return (m_dwArcID); }
    size_t                      GetStartEnt()	            { return (m_nStartEnt); }
    size_t                      GetCtEnt()                  { return (m_ctEnt); }
    CProgBar*                   GetProg()                   { return (m_pProg); }
    SOption*                    GetOpt()                    { return (m_pOption); }

    BOOL                        GetState()                  { return (m_bState); }

    DWORD                       GetBufSize();
    void                        SetBufSize(LPDWORD BufSize, DWORD WriteSize);
    void                        SetBufSize(LPDWORD BufSize, DWORD WriteSize, DWORD FileSize);

    void                        ReplaceBackslash(LPTSTR pFileName);
    void                        MakeDirectory(LPCTSTR wFileName);
        
    // Endian Conversion

    DWORD                       ConvEndian(DWORD value);
    WORD                        ConvEndian(WORD value);
    void                        ConvEndian(LPDWORD value);
    void                        ConvEndian(LPWORD value);

    BOOL                        CheckExe(LPCTSTR pExeName);
    BOOL                        CheckDir(LPCTSTR pDirName);

    // Susie plugin mounting

    void				        SetMountSusie() { m_bMountWasSusie = TRUE; }
    BOOL				        GetMountSusie() { return m_bMountWasSusie; }

    // MD5 value setting

    void				        SetMD5(SMD5 stmd5File);

    // Set flag of MD5 value

    void				        SetMD5OfFlag(BOOL bSetMD5) { m_bSetMD5 = bSetMD5; }

    // Get MD5 value

    std::vector<SMD5>	        GetMD5() { return m_vtstmd5File; }

    // Check if MD5 value is set

    BOOL				        CheckMD5OfSet() { return m_bSetMD5; }

    // Clear MD5 value

    void				        ClearMD5();

    // Work flag setting

    void				        SetFlag(BOOL bFlag) { m_bWork = bFlag; }

    // Workload capture flag

    BOOL				        GetFlag() { return m_bWork; }

    // Comparison function for sorting

    static BOOL                 CompareForFileInfo( const SFileInfo& rstfiTarget1, const SFileInfo& rstfiTarget2 );

    // Binary search

    static SFileInfo*           SearchForFileInfo( std::vector<SFileInfo>& rvcFileInfo, LPCTSTR pszFileName );


protected:



private:

    YCMultiFile             m_clmfArc;
    YCFile                  m_clfOutput;

    // File information in the archive

    std::vector<SFileInfo>* m_pEnt;
    SFileInfo*              m_pInfFile;

    std::vector<SFileInfo>  m_vcFileInfoOfFileNameSorted;

    // Archive file split support

    DWORD                   m_dwArcsID;
    std::vector<HANDLE>	    m_hArcs;
    std::vector<YCString>   m_pclArcPaths;
    std::vector<YCString>   m_pclArcNames;
    std::vector<YCString>   m_pclArcExtens;

    BYTE	                m_pHeader[2048];
    DWORD	                m_dwArcID;
    size_t	                m_nStartEnt;    // Starting index of the archive file information
    size_t	                m_ctEnt;        // Number of archive file information
    DWORD	                m_dwInfFileNum;

    BOOL                    m_bState;

    CProgBar*               m_pProg;
    SOption*                m_pOption;

    HANDLE                  m_hFile;
    YCString                m_clsPathToFile;
    LPCTSTR                 m_pSaveDir;

    DWORD                   m_deckey;

    BOOL                    m_bMountWasSusie;   // Check if Susie was used to mount plugin

    // MD5 value

    std::vector<SMD5>       m_vtstmd5File;

    // Set MD5 value flag

    BOOL                    m_bSetMD5;

    // Flag variable that can be set for each archive

    BOOL                    m_bWork;
};
