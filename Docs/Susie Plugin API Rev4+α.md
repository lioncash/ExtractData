# Susie Plugin API Rev. 4+α

This document aims to describe the interface for Susie plugins revision 4+α.

## Table of Contents

1. [Preface/Credit](#preface-credit)
2. [Structures](#structures)
    * [PictureInfo](#pictureinfo)
    * [fileInfo](#fileinfo)
3. [Error Codes](#error-codes)
4. [Functions](#functions)
    * [GetPluginInfo()](#getplugininfo)
    * [IsSupported()](#issupported)
    * [GetArchiveInfo()](#getarchiveinfo)
    * [GetPictureInfo()](#getpictureinfo)
    * [GetPicture()](#getpicture)
    * [GetFileInfo()](#getfileinfo)
    * [GetPreview()](#getpreview)
    * [GetFile()](#getfile)
    * [ConfigurationDlg](#configurationdlg)

## Preface/Credit

This document was translated from the original Japanese web version written by kana, which can be found [here](http://www2f.biglobe.ne.jp/~kana/spi_api/index.html). If there are any inconsistencies with this document and the original Japanese version. Please feel free to point them out.

## Structures

### PictureInfo

```cpp
typedef struct PictureInfo {
    long left,top;          // Image origin
    long width;             // Image width (in pixels)
    long height;            // Image height (in pixels)
    WORD x_density;         // Horizontal pixel density
    WORD y_density;         // Vertical pixel density
    short colorDepth;       // Number of bits per pixel
    HLOCAL hInfo;           // Image text information
} PictureInfo;
```

In Susie, it's assumed that the alignment (byte bounday) is 1 byte. As the default alignment of structures with Visual C++ is 8 bytes, it's important to ensure that the structure is packed appropriately, otherwise junk values or outright crashing can occur when `hInfo` is accessed.

### fileInfo

```cpp
typedef struct fileInfo {
    unsigned char method[8];   // Compression method
    unsigned long position;    // Position of the file in the archive
    unsigned long compsize;    // Compressed file size
    unsigned long filesize;    // Uncompressed file size
    time_t timestamp;          // File date and time
    char path[200];            // Relative path
    char filename[200];        // File name
    unsigned long crc;         // CRC checksum
} fileInfo;
```

* Since `method` is required to be null-terminated, this only has `7` usable characters.
* If a timestamp is not available, `timestamp` should be set to `0`, not `-1`.

## Error Codes

| Value |                    Description                         |
|:-----:|:-------------------------------------------------------|
|   0   | Successful                                             |
|  -1   | Function not implemented                               |
|   1   | Callback returned non-zero value; extraction canceled. |
|   2   | Unknown format                                         |
|   3   | Invalid/Corrupted data                                 |
|   4   | Memory allocation error                                |
|   5   | Memory error (unable to Lock memory)                   |
|   6   | File read error                                        |
|   7   | Reserved                                               |
|   8   | Internal error                                         |

## Functions

### GetPluginInfo()

```cpp
int _export PASCAL GetPluginInfo(int infono, LPSTR buf, int buflen)
```

##### Description
Retrieves information about a plugin.

##### Parameters

`infono` - Index indicating what kind of information to retrieve.

| Value  |                                Meaning                                   |
|:-------|:-------------------------------------------------------------------------|
|   0    | Plugin API version code                                                  |
|   1    | Plugin name, version, and copyright (displayed in Susie's About… dialog) |
| 2n + 2 | File format extension ("*.JPG" "*.RGB;.Q0", etc)                         |
| 2n + 3 | File format name (appears in Susie's Open dialog)                        |

`buf` - Buffer to store information in.

`buflen` - Buffer size in bytes.

##### Return value

If successful, the number of characters written into `buf` is returned. If an invalid `infono` is supplied, then `0` is returned.

##### Explanation

`infono` values `0` and `1` are common to all versions. `2` and onwards signify information that is used in Susie's Open dialog in pairs. For example, `2` retrieves the extension of a supported plugin format, while `3` retrieves the name of that format. If a plugin supports multiple image formats, then the count of file extension indices and file name indices should be the same.

To allow for further expansion of the API in the future, the plugin API version code is stored in the plugin. This can be retrieved by calling this function with an `infono` of `0`. The version code is a sequence of 4 bytes stored as follows:

`VVTS`

| Field |                                             Meaning                                                       |
|:------|:----------------------------------------------------------------------------------------------------------|
|  VV   | Version number                                                                                            |
|  T    | Type of plugin (`'I'`for an import filter, `'E'` for an export filter, or `'A'` for an archive extractor) |
|  S    | Image support type (`'N'` for normal, or `'M'` for multi-image)                                           |

##### Remarks

- While Susie retrieves information about supported file format names from this function, they are only used within the Open dialog. Susie *does not* use this function to determine if a given file is valid or not. Determining whether or not a file is valid for a particular format is determined by the return value of the `IsSupported` function.

- When Susie is performing information retrieval, it calls `GetPluginInfo`, increasing `infono` sequentially by `1` each time, until it returns `0` (invalid).

A sample `GetPluginInfo` implementation could be as follows:

```cpp
int _export PASCAL GetPluginInfo(int infono, LPSTR buf, int buflen)
{
    constexpr std::array<const char*, 4> information{{
        /* 0 */     "00IN",              // Assuming 00IN
        /* 1 */     "Sample Code Plug-in Ver x.xx",
        /* 2n+1 */  "*.SMP",
        /* 2n+2 */  "Sample Plug-in"
    }};

    if (infono < static_cast<int>(information.size())) {
        const char* s = information[infono];

        std::size_t size = std::strlen(s) + 1;
        size = (static_cast<int>(size) < buflen) ? size : static_cast<std::size_t>(buflen);

        std::memcpy(buf, information[infono], size);

        return static_cast<int>(size);
    }

    return 0;
}
```

### IsSupported()

```cpp
int _export PASCAL IsSupported(LPSTR filename, DWORD dw)
```

##### Description

Determines if the file represented by `filename`  is supported by this plugin.

##### Parameters

`filename` - Path to the file to check.

`dw` - Handle type

* When the upper 16 bytes of `dw` are zero, then `dw` is a Windows `HANDLE` that may be used with `ReadFile`, etc.
* When the upper 16 bytes of `dw` are not zero, then `dw` is a pointer to an `unsigned char` buffer in memory where the file header was read. This buffer is at minimum 2 KB in size. Even if the file itself is not 2 KB in size, the buffer will still be 2 KB, but with the excess space filled with `0`.

##### Return value

If the file is a supported format, then a non-zero value is returned. If the file is not supported, then `0` is returned.

##### Explanation

In order to determine if a given file is truly conforming to a file format that can be handled by a plugin, the header of said file needs to be inspected and validated. In rare cases, it's also necessary to check the file extension as well, since it's possible that a whole file needs to be composed from several other files (like a split archive). `filename` currently isn't used in any default plugins distributed at this time<sup>[1](#issupported-footnote1)</sup>

The file handle passed to `dw` is pointer returned by either `_lopen` or `CreateFile` from the Win32 API. In the case of `dw` signifying a memory buffer, it's reasonable to assume the pointer points to a data region that is 2 KB in size.

If using a Mac Binary, the above doesn't need to be taken into consideration, as processing needs to be done by the caller.

##### Remarks

Susie will call `IsSupported` regardless of the file extensions retrieved from `GetPluginInfo`. For example, if a file such as `Sample.pic` actually has JPEG file header data and is determined to be a JPEG file from a plugin that deals with JPEG files via `IsSupported`, then the file will be handed off to that plugin.

If any plugin out of all plugins available to Susie returns success (a non-zero value) for a particular file, then none of the other plugins will attempt to process that file. It's important to note that if any subsequent calls to `GetPictureInfo` or `GetPicture` fail, then Susie **will not** attempt to call `IsSupported` from other plugins and try to hand the file off to another plugin. Therefore, the return value and handling of files within `IsSupported` is very important and a plugin implementation should ensure that incompatible files aren't erroneously accepted by it.

In Susie, almost 100% of the time, the type of memory signified by `dw` will be a memory buffer containing the file header. However the case where a file handle is passed in should still be handled. Especially since there may be other Susie-compatible programs that could rely on this.

##### Footnotes

<a name="issupported-footnote1">1</a>. The original document for this text is SPI_API.TXT, which was included in SPI 32005.LZH, which also included plugins for JPEG/GIF/PI/PIC/PIC2.

### GetArchiveInfo()

```cpp
int _export PASCAL GetArchiveInfo(LPSTR buf, long len, unsigned int flag, HLOCAL* lphInf)
```

##### Description

Retrieves information on all files within an archive.

##### Parameters

`buf` - Source data buffer
* If dealing with a file, this contains the path to the archive file.
* If dealing with memory, this is a pointer to the file data.

`len` - Sorce data length
* If dealing with a file, this is the starting offset for reading (for MacBin support).
* If dealing with memory, this is the size of the data.

`flag` - Bit flags that provide additional information. Currently only the lower 3 bits are used, which signify what kind of data `buf` will be.

| Value |       Meaning        |
|:-----:|:---------------------|
|   0   | Disk file            |
|   1   | File image in memory |

`lphInf` - Output pointer that receives a handle containing file information. An array of [fileInfo](#fileinfo) structures is written into LOCAL memory allocated by the plugin, and its handle is placed into this pointer after the function exits. A [fileInfo](#fileInfo) entry with `method[0] == '\0'` is considered the terminating entry in the array.

##### Return value

If successful, then `0` is returned, otherwise an error code is returned.

##### Remarks

It's only necessary to allocate `(number of files + 1) * sizeof(fileInfo)` as the array to pass to LOCAL memory to be returned to `lphInf`.

### GetPictureInfo()

```cpp
int _export PASCAL GetPictureInfo(LPSTR buf, long len, unsigned int flag, PictureInfo* lpInfo)
```

##### Description

Retrieves information from image files.

##### Parameters

`buf` - Source data buffer
* If dealing with a file, this contains the path to the image file.
* If dealing with memory, this is a pointer to the image data.

`len` - Sorce data length
* If dealing with a file, this is the starting offset for reading (for MacBin support).
* If dealing with memory, this is the size of the data.

`flag` - Bit flags that provide additional information. Currently only the lower 3 bits are used, which signify what kind of data `buf` will be.

| Value |       Meaning        |
|:-----:|:---------------------|
|   0   | Disk file            |
|   1   | File image in memory |

`lpInfo` - Output pointer that retrieved image information is stored into.

##### Return value

If successful, then `0` is returned, otherwise an error code is returned.

##### Explanation

Retrieves information about the specified image data.

If no information exists in the data specified by `buf`, set `lpInfo->hInfo` to `NULL`. Also **it is the caller's responsibility to free `lpInfo->hInfo` if it is not NULL**.


##### Remarks

In Susie, this function will only ever be called if `IsSupported` returns a success value for the supplied data. However, some applications that support Susie plugins may directly call this function without properly checking if the data is valid. Therefore, it may be necessary to ensure the data in `buf` is still valid if you plan to make a plugin for applications other than Susie.

### GetPicture()

```cpp
int _export PASCAL GetPicture(LPSTR buf, long len, unsigned int flag, HANDLE* pHBInfo, HANDLE* pHBm, FARPROC lpPrgressCallback, long lData)
```

##### Description

Extracts an image

##### Parameters

`buf` - Source data buffer
* If dealing with a file, this contains the path to the image file.
* If dealing with memory, this is a pointer to the file data.

`len` - Sorce data length
* If dealing with a file, this is the starting offset for reading (for MacBin support).
* If dealing with memory, this is the size of the data.

`flag` - Bit flags that provide additional information. Currently only the lower 3 bits are used, which signify what kind of data `buf` will be.

| Value |       Meaning        |
|:-----:|:---------------------|
|   0   | Disk file            |
|   1   | File image in memory |

`pHBInfo` - Output parameter that will contain the pointer to a `BITMAPINFO` structure.

`pHBm` - Output parameter that will contain the pointer to the main bitmap data.

`lpPrgressCallback` - Pointer to a callback function to display the extraction progress. If `NULL`, the plugin will continue until extraction is finished and cannot be interrupted.

`lData` - Data that can be passed to the callback. This can be used to pass pointer values, etc into the callback function (**NOTE:** `long` is unsuitable for passing pointer on 64-bit Windows, as `long` is still a 32-bit type, which can truncate a 64-bit pointer address).

##### Return value

If successful, `0` is returned; otherwise an error code is returned.

##### Explanation

Extracts the image specified by `buf`.

The plugin allocates the required amount of memory by calling `LocalAlloc`. It's the application's responsibility to free `pHBInfo` and `pHBm` with `LocalFree` if the function succeeds. It's also important to note that upon success, the memory returned via these two parameters is unlocked via `LocalUnlock` before this function returns.

**NOTE:** If you do not intend to support 16-bit Windows variants, then it is sufficient to also use the heap memory management functions in the Win32 API instead, such as `HeapFree`, `HeapLock` and `HeapUnlock`, as the local memory APIs are simply a wrapper for them on 32-bit and 64-bit Windows variants.

##### Callback function

Callback functions have the following prototype:

```cpp
int PASCAL ProgressCallback(int nNum, int nDenom, long lData)
```

It's first called with `nNum` as `0`, and is then called periodically until `nNum` is equal to `nDenom`. If the return value of the callback function is not `0`, then the plugin will abort processing and `GetPicture` will return `1`.

The contents of `lData` will be the same as what was passed into `GetPicture`'s `lData` parameter.

### GetFileInfo

```cpp
int _export PASCAL GetFileInfo(LPSTR buf, long len, LPSTR filename, unsigned int flag, fileInfo* lpInfo)
```

##### Description

Retrieves information about a specified file within an archive.

##### Parameters

`buf` - Source data buffer
* If dealing with a file, this contains the path to the archive file.
* If dealing with memory, this is a pointer to the file data.

`len` - Sorce data length
* If dealing with a file, this is the starting offset for reading (for MacBin support).
* If dealing with memory, this is the size of the data.

`filename` - The name of the file to get the information of. Includes the relative path in the archive.

`flag` - Bit flags that provide additional information. Currently only bits 7 and 0–3 are used.

Bits 0–3 specify what type of data is being dealt with:

| Value |       Meaning        |
|:-----:|:---------------------|
|   0   | Disk file            |
|   1   | File image in memory |

While bit 7 determines filename case-sensitivity:

| Value |      Meaning     |
|:-----:|:-----------------|
|   0   | Case-sensitive   |
|   1   | Case-insensitive |

`lpInfo` - Pointer to the `fileInfo` struct that will receive the file information.

##### Return value

If successful, `0` is returned; otherwise an error code is returned.

### GetPreview()

```cpp
int _export PASCAL GetPreview(LPSTR buf, long len, unsigned int flag, HANDLE* pHBInfo, HANDLE* pHBm, FARPROC lpPrgressCallback, long lData)
```

##### Description

Extracts an image and reduces its size to be suitable for use as a preview image.

##### Parameters

`buf` - Source data buffer
* If dealing with a file, this contains the path to the image file.
* If dealing with memory, this is a pointer to the file data.

`len` - Sorce data length
* If dealing with a file, this is the starting offset for reading (for MacBin support).
* If dealing with memory, this is the size of the data.

`flag` - Bit flags that provide additional information. Currently only the lower 3 bits are used, which signify what kind of data `buf` will be.

| Value |       Meaning        |
|:-----:|:---------------------|
|   0   | Disk file            |
|   1   | File image in memory |

`pHBInfo` - Output parameter that will contain the pointer to a `BITMAPINFO` structure.

`pHBm` - Output parameter that will contain the pointer to the main bitmap data.

`lpPrgressCallback` - Pointer to a callback function to display the extraction progress. If `NULL`, the plugin will continue until extraction is finished and cannot be interrupted.

`lData` - Data that can be passed to the callback. This can be used to pass pointer values, etc into the callback function (**NOTE:** `long` is unsuitable for passing pointer on 64-bit Windows, as `long` is still a 32-bit type, which can truncate a 64-bit pointer address).

##### Return value

If successful, then `0` is returned; otherwise an error code is returned. As this function is optional, if it is not implemented then `-1` will be returned by the default implementation to signify this operation is unsupported.

##### Explanation

Creates a reduced image that can be used as a preview for a file. This function is essentially the same as `GetPicture`, except the image output is a reduced resolution instead. This function should only be implemented when files can be extracted and converted very quickly. For example, this would be suitable for a JPEG plugin.

Currently `IFJPEG.SPI` supports this, only expanding JPEG images to 1/4th of their original size.

In the case this function is unimplemented, Susie will attempt to fully extract the image first using built-in functions and then try to reduce the image.

The plugin allocates the required amount of memory by calling `LocalAlloc`. It's the caller's responsibility to free `pHBInfo` and `pHBm` with `LocalFree` if the function succeeds. It's also important to note that upon success, the memory returned via these two parameters is unlocked via `LocalUnlock` before this function returns.

**NOTE:** If you do not intend to support 16-bit Windows variants, then it is sufficient to also use the heap memory management functions in the Win32 API instead, such as `HeapFree`, `HeapLock` and `HeapUnlock`, as the local memory APIs are simply a wrapper for them on 32-bit and 64-bit Windows variants.

##### Callback function

Callback functions have the following prototype:

```cpp
int PASCAL ProgressCallback(int nNum, int nDenom, long lData)
```

It's first called with `nNum` as `0`, and is then called periodically until `nNum` is equal to `nDenom`. If the return value of the callback function is not `0`, then the plugin will abort processing and `GetPreview` will return `1`.

The contents of `lData` will be the same as what was passed into `GetPreview`'s `lData` parameter.

### GetFile()

```cpp
int _export PASCAL GetFile(LPSTR src, long len, LPSTR dest, unsigned int flag, FARPROC prgressCallback, long lData)
```

##### Description

Retrieves a file from an archive

##### Parameters

`src` - Source data buffer
* If dealing with a file, this contains the path to the file.
* If dealing with memory, this is a pointer to the file data.

`len` - Sorce data length
* If dealing with a file, this is the starting offset for reading.
* If dealing with memory, this is the size of the data.

`dest` - Destination buffer
* If dealing with a file, this is an input parameter containg the destination file path.
* If dealing with memory, this is an output parameter receives a pointer to `LOCAL` memory.

`flag` - Bit flags that provide additional information. Currently only bits 8–10 and 0–3 are used.

Bits 0–3 specify what type of input data is being dealt with:

| Value |       Meaning        |
|:-----:|:---------------------|
|   0   | Disk file            |
|   1   | File image in memory |

While bits 8–10 specify what type of output data is being dealt with:

| Value |       Meaning        |
|:-----:|:---------------------|
|   0   | Disk file            |
|   1   | File image in memory |

`lpPrgressCallback` - Pointer to a callback function to display the extraction progress. If `NULL`, the plugin will continue until extraction is finished and cannot be interrupted. This follows the same specifications as [GetPicture](#getpicture) and [GetPreview](#getpreview)'s callback functions.

`lData` - Data that can be passed to the callback. This can be used to pass pointer values, etc into the callback function (**NOTE:** `long` is unsuitable for passing pointer on 64-bit Windows, as `long` is still a 32-bit type, which can truncate a 64-bit pointer address).

##### Return value

If successful, `0` is returned; otherwise an error code is returned.

##### Explanation

This plugin allocates necessary memory via `LocalAlloc`. It's the caller's responsibility to call `LocalFree` on `dest` whenever it functions as an output parameter if the function succeeds. **NOTE:** If you do not intend to support 16-bit Windows variants, then it is sufficient to also use the heap memory management functions in the Win32 API instead, such as `HeapFree`, `HeapLock` and `HeapUnlock`, as the local memory APIs are simply a wrapper for them on 32-bit and 64-bit Windows variants.

The caller calls [GetArchiveInfo](#getarchiveinfo) or [GetFileInfo](#getfileinfo) beforehand to acquire infomation about the file they want to extract, and then using the resulting [fileInfo](#fileinfo) struct to pass necessary values into this function. The following table describes how these values should be passed.

| Input Type |                   `src`                  |                   `len`                 |
|:-----------|:-----------------------------------------|:----------------------------------------|
|   File     | File name                                | `fileInfo`'s `position` member variable |
|   Memory   | Pointer to start of file data + position | Archive file size − position            |

Applications using Susie plugins should take note of the following points when the data retrieved from `GetFile` is less than 2 KB (2048 bytes):

* When using the in-memory means of extracting files, if the data is being passed to `IsSupported`, the data **must** be placed into a buffer that is 2 KB or more in size.
* When dealing with MacBin, consider skipping the first `n` bytes.


##### Callback function

Callback functions have the following prototype:

```cpp
int PASCAL ProgressCallback(int nNum, int nDenom, long lData)
```

It's first called with `nNum` as `0`, and is then called periodically until `nNum` is equal to `nDenom`. If the return value of the callback function is not `0`, then the plugin will abort processing and `GetFile` will return `1`.

The contents of `lData` will be the same as what was passed into `GetFile`'s `lData` parameter.

### ConfigurationDlg()

```cpp
int _export PASCAL ConfigurationDlg(HWND parent, int fnc)
```

##### Description

Display's the plugin's configuration dialog.

##### Parameters

`parent` - Parent window handle

`fnc` - Function code

|      Code     |               Meaning              |
|:--------------|:-----------------------------------|
| 0             | Plugin about dialog (if necessary) |
| 1             | Settings dialog                    |
| 2 and greater | Reserved                           |

##### Return value

If successful, `0` is returned; otherwise an error code is returned. As this function is optional, if not implemented, the default implementation will return `-1` to signify that it's unimplemented.

##### Explanation

Allows setting plugin-specific settings.

The key in the Registry where the plugin settings are saved to isn't bound to a specific name. For example, it could be `HKEY_CURRENT_USER\Software\Takechin\Susie\Plug-in\Plugin` or `HKEY_CURRENT_USER\Software\Author Name\Plugin Name`.

To switch plugin dialog language string resources according to what Susie has its language set to, it's necessary to determine where the setting is stored, which can vary on different Windows platforms. If registry data can't be found, or the relevant value is set to `1`, it's assumed to mean Japanese. If the relevant value is `0`, it's assumed to be English. The following table provides the location of Susie's resource settings.

|        OS       |                     Location                       |       Type       |   Name   |
|:----------------|:---------------------------------------------------|:-----------------|:---------|
| Win95/NT(Win32) | HKEY_CURRENT_USER\Software\Takechin\Susie\Resource | `REG_DWORD`      | Japanese |
| Win3.1 + Win32s | SUSIE.INI                                          | Resource section | Japanese |