#define	_WIN32_WINNT 0x0500
#define	_CRT_SECURE_NO_WARNINGS
#define	_CRT_NON_CONFORMING_SWPRINTFS

#include    <windows.h>
#include    <windowsx.h>
#include    <commctrl.h>
#include    <shlwapi.h>
#include    <shlobj.h>
#include    <tchar.h>
#include    <mbstring.h>
#include    <math.h>
#include    <process.h>
#include    <algorithm>
#include    <array>
#include    <cstddef>
#include    <map>
#include    <memory>
#include    <set>
//#include  <string>
//#include  <valarray>
#include    <vector>

#include    <zlib.h>
#include    <png.h>
//#include  <jpeglib.h>
//#include  <jerror.h>

#ifdef	_DEBUG
#define	_CRTDBG_MAP_ALLOC
#include    <stdlib.h>
#include    <crtdbg.h>
//#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

#include    "Types.h"

#include    "Base/YCMemory.h"
#include    "Base/YCString.h"
#include    "Base/YCFileFind.h"
#include    "Base/YCFileFindSx.h"
#include    "Base/YCLibrary.h"
#include    "Base/YCLocalMemory.h"
#include    "Base/YCIni.h"
#include    "Base/YCFile.h"
#include    "Base/YCStdioFile.h"

