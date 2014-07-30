#pragma once

#include "Common.h"
#include "ArcFile.h"

struct SSusieInfo
{
	YCString                      clsName;             // Filename
	YCString                      clsVersion;          // Version info
	YCString                      clsInfo;             // Plugin info
	YCString                      clsSupportFormat;    // Supported formats
	BOOL                          bConfig;             // Presence or absence of a configuration dialog
	YCString                      clsPath;             // File path
	BOOL                          bValidity;           // Validity
	YCLibrary                     cllPlugin;           // Module
};

class CSusie
{
public:

	BOOL                           Mount(CArcFile* pclArc);
	BOOL                           Decode(CArcFile* pclArc);

	void                           Init();
	void                           Apply();

	void                           LoadSpi(const YCString& rclsPathToSusieFolder);
	void                           SaveSpi();

	std::vector<SSusieInfo>&       GetSusie() { return m_stsiMain; }
	std::vector<SSusieInfo>&       GetSusieTmp() { return m_stsiTemporary; }


private:

	static std::vector<SSusieInfo> m_stsiMain;
	static std::vector<SSusieInfo> m_stsiTemporary;
};
