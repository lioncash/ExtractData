#pragma	once

//----------------------------------------------------------------------------------------
//-- Symbol Declaration  -----------------------------------------------------------------
//----------------------------------------------------------------------------------------

#define ID_TOOLBAR    10000
#define ID_LISTVIEW1  10001
#define ID_STATUS     10002
#define ID_TIMER      10003

#define SBL_STR_INI_EXTRACTDATA     _T("ExtractData.ini")
#define SBL_STR_INI_SUSIE           _T("Susie.ini")

//----------------------------------------------------------------------------------------
//-- Structure Declaration ---------------------------------------------------------------
//----------------------------------------------------------------------------------------

// Option Items

struct SOption
{
	// Related listview
	COLORREF            ListBkColor;                    // Background color of list
	TCHAR               szListBkColor[7];               // Background color of list
	COLORREF            ListTextColor;                  // List text color
	TCHAR               szListTextColor[7];             // List text color

	// Search file related
	std::vector<BOOL>   bSearch;                        // Types of files to search
	BOOL                bHighSearchOgg;                 // Increase the accuracy of searching for OGG files

	// Extraction related
	BOOL                bCreateFolder;                  // Each folder to extract
	BOOL                bFixOgg;                        // Fix an OGG file's CRC on extraction
	BOOL                bEasyDecrypt;                   // Enable/Disable simple decoding
	BOOL                bRenameScriptExt;               // Change the extension of a script
	DWORD               BufSize;                        // Buffer size

	// Image related
	BOOL                bDstBMP;                        // BMP Output
	BOOL                bDstPNG;                        // PNG Output
	DWORD               CmplvPng;                       // PNG Compression Level
	BOOL                bAlphaBlend;                    // Include/don't include alpha in the output image
	DWORD               BgRGB;                          // Background color
	TCHAR               szBgRGB[7];                     // Background color

	// Output destination related
	YCString            TmpDir;                         // Temporary folder
	BOOL                bSaveSel;                       // Specified output directory (if chosen to explicitly choose it)
	BOOL                bSaveSrc;                       // Output destination is the same as the input destination
	BOOL                bSaveDir;                       // Fixed output directory
	YCString            SaveDir;                        // Fixed output folder

	// Susie Plug-in related
	BOOL                bSusieUse;                      // Use/Not use Susie plugins
	YCString            SusieDir;                       // Susie Plugin Folder
	BOOL                bSusieFirst;                    // Give/Don't give priority to Susie plugins 
};

// File Info
struct SFileInfo
{
	YCString           name;                           // Filename
	u32                size_org = 0;                   // File Size
	u32                size_org2 = 0;                  // File Size 2
	YCString           size_org_comma;                 // File size with a comma
	u32                size_cmp = 0;                   // Compressed file size
	YCString           size_cmp_comma;                 // Compressed file size with a comma
	YCString           format;                         // File Format
	YCString           arc_name;                       // Archive file to load
	u64                start = 0;                      // Start address
	u64                end = 0;                        // End address
	u32                arc_id = 0;                     // Distinguishes between multiple open files
	u32                arcs_id = 0;                    // Archive file split IDs
	std::vector<u32>   sizes_org;                      // File size (Used when data is scattered)
	std::vector<u32>   sizes_cmp;                      // Compressed File size (Used when data is scattered)
	std::vector<u32>   starts;                         // Start Address(Used when data is scattered)
	std::vector<u32>   compress_checks;                // Check if it is compressed or not (Used when data is scattered)
	YCString           title;                          // Game title
	u32                key = 0;                        // File Key (File value)
	u32                type = 0;                       // File type
	std::set<YCString> tmp_file_paths;                 // Path to extracted files
};
