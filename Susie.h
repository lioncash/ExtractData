#pragma once

class CArcFile;

struct SSusieInfo
{
	YCString  name;                // Filename
	YCString  version;             // Version info
	YCString  info;                // Plugin info
	YCString  supported_formats;   // Supported formats
	BOOL      has_config_dialog;   // Presence or absence of a configuration dialog
	YCString  path;                // File path
	BOOL      validity;            // Validity
	YCLibrary plugin;              // Module
};

class CSusie
{
public:
	bool Mount(CArcFile* archive);
	bool Decode(CArcFile* archive);

	void Init();
	void Apply();

	void LoadSpi(const YCString& susie_folder_path);
	void SaveSpi();

	std::vector<SSusieInfo>& GetSusie() { return m_main; }
	std::vector<SSusieInfo>& GetSusieTmp() { return m_temporary; }

private:
	static std::vector<SSusieInfo> m_main;
	static std::vector<SSusieInfo> m_temporary;
};
