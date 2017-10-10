#define WINVER 0x0603
#define _WIN32_WINNT 0x0603
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NON_CONFORMING_SWPRINTFS

#include    <windows.h>
#include    <windowsx.h>
#include    <commctrl.h>
#include    <shlwapi.h>
#include    <shlobj.h>
#include    <tchar.h>
#include    <mbstring.h>
#include    <process.h>
#include    <wrl/client.h>

#include    <algorithm>
#include    <array>
#include    <atomic>
#include    <bitset>
#include    <chrono>
#include    <cinttypes>
#include    <cmath>
#include    <condition_variable>
#include    <cstddef>
#include    <cstdlib>
#include    <cstring>
#include    <deque>
#include    <functional>
#include    <future>
#include    <iterator>
#include    <limits>
#include    <locale>
#include    <map>
#include    <memory>
#include    <mutex>
#include    <numeric>
#include    <queue>
#include    <ratio>
#include    <set>
#include    <stack>
#include    <thread>
#include    <tuple>
#include    <type_traits>
#include    <unordered_map>
#include    <unordered_set>
#include    <utility>
#include    <vector>

#include    <zlib.h>
#include    <png.h>

#ifdef	_DEBUG
#define	_CRTDBG_MAP_ALLOC
#include    <cstdlib>
#include    <crtdbg.h>
//#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

#include    "Types.h"

#include    "Base/YCString.h"
#include    "Base/YCFileFind.h"
#include    "Base/YCFileFindSx.h"
#include    "Base/YCLibrary.h"
#include    "Base/YCLocalMemory.h"
#include    "Base/YCIni.h"
#include    "Base/YCFile.h"
#include    "Base/YCStdioFile.h"

