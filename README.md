IO
======
[![Build Status](https://travis-ci.org/incoder1/IO.svg?branch=master)](https://travis-ci.org/incoder1/IO)
[![bs1-1.0](https://img.shields.io/badge/license-boost-blue.svg)](https://www.boost.org/LICENSE_1_0.txt)


IO is modern C++ (C++ 11 +) library for general propose input/output and work with XML data.

## LIBRARY FEATURES

### GENEGIC INPUT/OUTPUT interface.

Generic input/output interface for low-level binary synchronous and asynchronous input/output operations,
implemented on top of system calls and without Libc FILE or lib std C++ file buffer. 
Interfaces and implementers guaranty for work with exceptions and RTTI off. Input/output or memory error handling implemented 
using std::error_code.
 
Channels interface allow you easily implement basic binary input/output for any source like files, sockets, named 
and unnamed pipes, network sockets etc. Channels API user do not care where data comes from or where is should be written,
API is always the same.

There are Windows and UNIX: files, memory, standar streams and synchronous BSD network channels out of the box. 

### Network INPUT/OUTPUT

There are Unix Domain sockets TCP/IP channels. With TCP, UDP and ICMP support. For TCP and UDP there is a TLS and DTLS support 
over the gnutls library. IP V4 and V6 both can be used with the same API. There is a minimal http client library, can be used for obtain external XML XSD schema or DTD. If you need something more powerful
you can use Windows API HTTP components or libcurl.

### C++ IOSTREAMS COMPATIBILITY

C++ iostreams compatibility library – provides input and output streams which works on top of channel, so you don’t need 
to create your own streams & stream buffers depending on files/shared memory/named and unnamed pipes/network sockets each time.

### CHARACTER SET LIBRARY

Character set library for detecting and converting between characters sets (code pages) without depending on current C/C++ 
locale. Library designed to be useful for applications, which should work in worldwide like a web server or web browser 
for example. Conversion between character sets based on iconv library standardized by POSIX. Microsoft Windows version 
uses port of GNU iconv. There are minimal port of Mozilla universal character detector, which can detect latin1 
or UNICODE representation from a sort of raw bytes. As well as byte order mark ( BOM ) utility classes bundled.
 

### UNICODE CONSOLE

Console mode input/output and color manipulation library. Allows input and output into system console/terminal without 
back effect of C/C++ locale library, and in full UNICODE mode. Unix and Windows versions supports full UNICODE console
including UTF-8 and system WCHAR mode. There is possibility to define console colors, in respect to portability
16 standard colors used so that it should work with most terminals.

### STRINGS

Constant string class - io::const_string a constant small string optimized wrapper on raw character array. When string size is less then double mashine word characters data
stored inside the class, therwise all equal instatnces of const_string sharing the same string pointer with intrusive atomic reference counting.
Background data array is considered to be always in UFT-8 UNICODE, and there are utility functions for trans-coding for char16_t,
char32_t or wchar_t std::basic_string. const_string can count UNICODE character lenght as well as provide bytes size.

const_string provides MurMur3 hash function for 32 bit instruction set and Google CityHash function for 64 bit CPUS.

### StAX XML PARSER

Streaming API for XML parsing  - pull parser for XML. 
Exceptions and RTTI can be off during the parsing. 
Errors including out of memory can be handled over the [std::error_code](http://en.cppreference.com/w/cpp/error/error_code).

Libray provides event and [cursor]([reading C++ structure from XML](https://github.com/incoder1/IO/wiki/Reading-structures-from-XML)) based API's.

### XML MARSHALLER

A reflection like XML marshaller (XML serialize) – a library for serialize Plain Old CLR Object (POCO)
into XML of specific format. You can save a value as XML tag or like a XML tag attribute. 
Library based on compile time reflection with ability to specify XML format using sets of 
std::tuple.  When you have RTTI on, you also able to auto-generate the XSD shema based on model defined by your C++ code.

Serialization is based on std::ostream, so it is not exception safe, e.g. you need some custom unexpected/terminate handlers.

### DEPENDENCIES 

You should have C++ 11 compatible compiler with STD C++ library. Optionally you can use Boost library for the boost::intrusive_ptr.

- [iconv](https://en.wikipedia.org/wiki/Iconv) with support of the following character sets 

    Type | Code pages
     --- | ---
    **UNICODE** | `UTF-8`  `UTF-16LE`  `UTF-16BE`  `UTF-32LE`  `UTF-32BE`  `UTF-7` 
    **One byte** | `US ASCII` `ISO-8859-1…ISO-8859-16` `KOI8-R` `KOI8-U` `KOI8-RU` 
    **Windows** | `CP-1250 … CP-1258`

Tested implementations:
    * [GNU iconv](https://www.gnu.org/software/libiconv/)
    *  GNU C Library iconv

- [GNU TLS v 3.0+] 
#### UNIX
	Most Unix distributions have an implementation, use your package manager to install development package
#### Windows
   Compiler | instruction
    MSYS2 | install developement package i.e. `pacman -S libgnutls-devel`
    MS Visual C++ | build can be found at: [ShiftMediaProject/gnutls](https://github.com/ShiftMediaProject/gnutls)

## SUPPORTED OPERATING SYSTEMS AND COMPILERS

Library supports Windows 7+ and GNU Linux Operating systems With GCC 10+ or MS Visual C++ 17+ compilers. 

### TESTED CONFIGURATIONS

OS | Compiler | Version | Arhitecture
--- | --- | --- | ---
**Windows** | | |
 Windows 10 | GCC/G++ | 12.1.0 MinGW64 (MSYS2 build) | x86_64
 Windows 10 | MS Visual C++ | 19 | x64 
**GNU/Lunux** | | | 
 Fedora 36 | GCC/G++ | 12.1.0 |  x86_64

## BUILDING

### Building with CMake
To build with CMake build tool to can use following command

You can build shared or static library release or debug version, optionally you can on or off exceptions and rtti.


```bash
cmake . -DCMAKE_BUILD_TYPE=<Release|Debug> -DBUILD_LIBRARY_TYPE=<Shared|Static> [-DNO_EXCEPTIONS=ON] [-DNO_RTTI=ON]
cmake --build .
```

- BUILD_LIBRARY_TYPE use Shared with this flag to build shared library (DLL) or Static for the static library
- NO_EXCEPTIONS optional flag for disiabling C++ exceptions, by default exceptions will be used
- NO_RTTI optional flag for disiabling C++ runtime type information, by default RTTI generated

Build result can be found at _target/<Release|Debug>/lib_ sub-folder, if you don't whant to install libary with cmake.

When you've choose to build with MS VC++ solution as the Cmake output you can goto target sub floder and build with next command
```bash
MSBuild io.sln /p:Configuration=[Release|Debug]
```
If you'd like to build from command line. Otherwise simply build solution with IDE. As alternative you can build the DLL with
[nmake](#Windows with Microsoft VC++ and NMake). 

#### Windows with Microsoft VC++ and NMake

You need Visual Studio or Visual Studio BuildTools version 15+
Source tree cont pre-build gnu iconv MS VC++ binaries with import librarian x64 (amd64|x86_64) in deps sub folder, in case of
x32 or arm/arm64 build this library first. 

To build the DLL, open command prompt execute [vcvars64.bat](https://docs.microsoft.com/en-us/dotnet/framework/tools/developer-command-prompt-for-vs) from visual studio. 

Then goto IO source code root directory and execute
```bash
nmake -f NMakefile-msvcx64-dll.mak
```

Build result can be foound in _target/release-win-msvc-dll-x64_ sub folder

### Building with Code::Blocks IDE

Code::Blocks IDE project files bundled. There are predefined configurations for GCC. 

#### Windows GCC MinGW64

Preferable MinGW64 distribution is [MSYS2 MinGW64](https://www.msys2.org/).
When you are using Msys2 make sure that Code::Blocks using [MSYS GCC](https://www.youtube.com/watch?v=G3QguXOVJM4) rather then any another compiler.
Make sure that you have dependencies installed, install them if not yet
```bash
pacman -S libiconv-devel mingw-w64-x86_64-gnutls
```

#### Linux/Unix
Chekc that you have dependencies installed, install them if not yet with your package manager i.e. dnf, apt, pacman, port etc.Choose UNIX configration in Code::Blocks, and build the library.

### EXAMPLES
Code examples can be found examples sub-folder. 
Where:
* xmlparse – XML parsing with event API, prints into console XML or any type
* xml_parse_to_structures - reads XML into C++ structure data type with cursor API
* write_structures_to_xml – compile time reflection plain C object XML marshalling and XSD generation example
* chconv – converting characters between different code pages example
* iostreams – writes UNICODE strings into file and console with reconverting characters between multiple UNICODE representation, using C++ iostreams build on top of IO channels.
* collada - limited set of [COLLADA](https://www.khronos.org/collada/) 3D model OpenGL 4+ based viewer. Contans even based collada parser and simple 3D graphical model rendering engine.
