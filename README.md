IO
======
[![Build Status](https://travis-ci.org/incoder1/IO.svg?branch=master)](https://travis-ci.org/incoder1/IO)
[![bs1-1.0](https://img.shields.io/badge/license-boost-blue.svg)](https://www.boost.org/LICENSE_1_0.txt)

# WARNING!
This branch in active development and not stabile

## Road map by the priority

### Version 1.0.0:
1. Updates for C++ 20 standard, backward compatibility for C++11 for release 1.0
2. Simplify some APIs and implemations i.e. console, network, etc lecal casting for work with text data formats
3. Test over Google Test and cmake/CTest
4. CMake section for Clang
5. Gith hub actions CI instead of Travis (Unix/GCC done without tests (CTest can't start test suite), Add Windows and CLang)
6. Asynchronous IO for network with TLS security support (Windows only with IO Copletition port, need Unix and TLS encription for both)
7. HTTP client, use (preffered) libnghttp2 or create own libnghttp2 like implementaton, need proff of concept
8. Improve character set detector feature, or simply replace with ready to use like [compact_enc_det]()ttps://github.com/google/compact_enc_det
9. Tutorial over the examples, review examples and replace with tests when needed
11. Binary format examples with Google Protocol Buffer, Apache Avro and ASN1 using additional libs or tools
12. Named pipes channels and input/output

### Deep backlog i.e. for version 1.0.1 etc:
* CSV data format parser and writer
* YAML data format streaming API
* Support for FreeBSD, Mac OS X Android and iOS
* Extend COLLADA demo viewer, replace self made OpenGL viewer with some more high level toolkit with Vulkan backedn like : [BGFX](https://github.com/bkaradzic/bgfx), [OGRE Next](https://www.ogre3d.org/download/sdk/sdk-ogre-next) or [Filament](https://github.com/google/filament)
* Add support for Intel compiller

IO is modern C++ (std 11 +) library for general propose input/output and common data textual and binary formats.

## LIBRARY FEATURES

### GENEGIC INPUT/OUTPUT interface with C++ IOSTREAMS COMPATIBILITY

Generic input/output interface for low-level binary synchronous and asynchronous input/output operations,
implemented on top of system calls and without Libc FILE or lib std C++ file buffer. 

Input-output interface design based on resource acquisition is initialization (RAII) and Separation of concerns idioms.
Low level generic read and write interfaces doing nothing except read and write raw binary data from source or destination
i.e. All more compliacated operations like reading and writing textual data, lexical casting etc build on the top of lower levels,
and have facades.
With this interfaces and implementers able to work without exceptions and runtime type information, unlike standard streams library. 
Input-output or memory errors can be processed using std::error_code instead of dealing with the exceptions, as well as if
you still need to use exceptions, error code can be thrown as system_error with single line of the code.
 
Channels interface allow you easily implement basic binary input/output for any source and destination like:
files, sockets, named and unnamed pipes, network sockets etc. 
Channels API user do not care where data comes from or where is should be written, API is always the same.

There are Windows and UNIX: files, memory, console and TCP/IP network channels bundled.

Network channels are build for Unix Domain sockets TCP/IP channels. With TCP, UDP and ICMP support. For TCP and UDP there is a TLS and DTLS support 
over the gnutls library. IP V4 and V6 both can be used with the same input output API. 
Asynchronous implementation use OS specific i.e. IO completition  ports for Windows, epoll for GNU/Linux and kqueue for BSD like kernels.

### TEXT DATA INPUT OUTPUT AND CHARACTER SET MANIPULATION

Functionality to work with different text data representations and characters sets (code pages) without depending on current
C/C++ locale. Functionality allow you to work with UNICODE including UTF-8, UTF-16 and UTF-32 LE  
and char, char8_t, char16_t, char32_t, wchar_t underlying data types as well as another single bye code pages from ISO/Windows
and convert between them.

#### CHARACTER SET CONVERTER

Converting between different characters sets (code pages) without depending on current C/C++ 
locale. Conversion between character sets based on iconv library standardized by POSIX to be a part of C standard library i.e. no dependency on ICU etc.
Microsoft Windows version uses port of GNU iconv.

#### CHARACTER SET DETECTOR

Functionality for detecting text character set from stream of unknown data.

#### CONSOLE 

Console mode input/output and color manipulation library. Allows input and output into system console/terminal without 
back effect of C/C++ locale library, and in full UNICODE mode. Unix and Windows versions supports full UNICODE console
including UTF-8 and system WCHAR mode. Colored output poroveded for all supported platfroms.

#### STRINGS

Constant string class - io::const_string a constant small string optimized wrapper on raw character array. When string size is less then double mashine word characters data
stored inside the class, therwise all equal instatnces of const_string sharing the same string pointer with intrusive atomic reference counting.
Background data array is considered to be always in UFT-8 UNICODE, and there are utility functions for trans-coding for char16_t,
char32_t or wchar_t std::basic_string. const_string can count UNICODE character lenght as well as provide bytes size.

IO implements [Google CityHash](https://github.com/google/cityhash) for const_sting as well as you can use this hasing with any
cind of data needs to be caches, as well as combine hahes with inspired CityHash algorytm.

#### LECICAL CASTING  

A compiler and standard library agnostic lexical cast implementation similar to C++ 17 [charconv](https://en.cppreference.com/w/cpp/header/charconv) to_chars&from_chars and [to_string](https://en.cppreference.com/w/cpp/string/basic_string/to_string) 

### XML 

Streaming - [pull](https://www.xmlpull.org/) i.e. iterator lile API for XML parsing and writing. Allow parse XML directly into POD (plain old data) stuctures, with out waste memory for building
additional tree memory structures such as Document Object Model (DOM) and back extracting data from this structure, or dealling with callbacks like SAX API.

API gives you full control of XML parsing and writing process, skip some: tags, text or comments etc

Parser and writer designed for dealling with huge XML document files such as [COLLADA](https://collada.org/) 3D models or [Scalable Vector Graphics](https://www.w3.org/TR/SVG2/)

See [reading C++ structure from XML](https://github.com/incoder1/IO/wiki/Reading-structures-from-XML)

COLLADA 3D model partial parser and 3D model viewer example provided

### INSTALL DEPENDENCIES 

You should have one of the C++ 11 compatible compiler GNU GCC, LLVM Clang or Microsoft Visual C++

You'll need following additional libraries

- [iconv](https://en.wikipedia.org/wiki/Iconv) following implemenations supported
    - [GNU iconv](https://www.gnu.org/software/libiconv/)
    -  GNU C Library iconv

- [GNU TLS v 3.0+](https://www.gnutls.org/)

#### GNU Linux

Use your package manager to install development package

##### Debian based i.e. Debian/Ubuntu/Lint etc.

```bash    
sudo apt-get install -y gnutls-dev 
```

#####  RPM based Fedora,RHEL,CentOS, Mandriva etc.

```bash    
sudo dnf install gnutls-devel 
```

#####  Pacman based Arch Linux, Chakra etc.
```bash    
pacman -S libgnutls-devel
```    

#### Windows

##### MSYS2 GCC Universal CRT (UCRT) preffered GCC on Windows
  
```bash
pacman -S mingw-w64-ucrt-x86_64-libiconv mingw-w64-ucrt-x86_64-gnutls mingw-w64-ucrt-x86_64-gtest
```

##### MSYS2 GCC MSVCRT - Legacy systems

```bash
pacman -S mingw-w64-x86_64-libiconv mingw-w64-x86_64-gnutls mingw-w64-x86_64-gtest
```

##### MSYS2 Clang
```bash
pacman -S mingw-w64-clang-x86_64-libiconv mingw-w64-clang-x86_64-gnutls mingw-w64-clang-x86_64-gtest
```
#####  MS Visual C++
 
 Download release ShiftMediaProject builds 
 + [iconv](https://github.com/ShiftMediaProject/libiconv/tags)
 + [gnutls](https://github.com/ShiftMediaProject/gnutls/tags).
    
Create __deps\msvc__ in the IO souce directory root, and extract both archives into this folder

## TESTED OPERATING SYSTEMS AND COMPILERS MATRIX

// TODO: insert build status to this marix when on github actions
// and move on the file header to check status

OS | Compiler | Version | Arhitecture
--- | --- | --- | ---
**Windows** | | |
 Windows 10 | GCC/G++ | 12.1.0 MinGW64 (MSYS2 build) | x86_64
 Windows 10 | GCC/G++ | 13.0.1 MinGW64 (MSYS2 build) | x86_64-UCRT64
 Windows 10 | Clang | 16.0.5-3 | x86_64
 Windows 10 | MS Visual C++ | 22 | x64
**GNU/Lunux** | | | 
 Fedora 36 | GCC/G++ | 12.1.0 | x86_64
 Ubuntu 22 | GCC/G++ | 13.0.1 | x86_64

## BUILDING

### Building with CMake

#### Configure

General syntax

```bash
cmake -S . -B CBuild -DCMAKE_BUILD_TYPE=<Release|Debug> -DBUILD_LIBRARY_TYPE=<Shared|Static> [-DNO_EXCEPTIONS=ON] [-DNO_RTTI=ON] [-DBUILD_TESTING=ON]
```

- BUILD_LIBRARY_TYPE use Shared with this flag to build shared library (DLL) or Static for the static library
- NO_EXCEPTIONS optional flag for disiabling C++ exceptions, by default exceptions will be used
- NO_RTTI optional flag for disiabling C++ runtime type information, by default RTTI generated
- DBUILD_TESTING optional flag to build with GTest and run with CTest

Build result can be found at _CBuild/<arh(x86,x64)>/<Release|Debug>/lib_ sub-folder, if you don't whant to install libary with cmake.

####  Configure debug static libray and run test
```bash
cmake -S . -B CBuild -DCMAKE_BUILD_TYPE=Debug -DBUILD_LIBRARY_TYPE=Static -DBUILD_TESTING=ON
```

#### Configure release dynamic library
```bash
cmake -S . -B CBuild -DCMAKE_BUILD_TYPE=Debug -DBUILD_LIBRARY_TYPE=Shared
```

## Generate MS VC++ solution files
Ensure [Visual C++ desktop tools installed](https://learn.microsoft.com/en-us/cpp/build/vscpp-step-0-installation?view=msvc-170)
Open [Power Shell for visual studio](https://learn.microsoft.com/en-us/visualstudio/ide/reference/command-prompt-powershell?view=vs-2022)
Follow confire and build steps

#### Configure

General syntax

```bash
cmake -S . -B CBuild -G "Visual Studio 17 2022" -A <x86|x64> -DCMAKE_BUILD_TYPE=<Release|Debug> -DBUILD_LIBRARY_TYPE=<Shared|Static> [-DNO_EXCEPTIONS=ON] [-DNO_RTTI=ON] [-DBUILD_TESTING=ON] --preset release
```

####  Configure debug static libray x64 and run test
```bash
cmake -S . -B CBuild -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Debug -DBUILD_LIBRARY_TYPE=Static -DBUILD_TESTING=ON --preset release
```

####  Configure release x64 DLL library
```bash
cmake -S . -B CBuild -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release -DBUILD_LIBRARY_TYPE=Shared --preset release
```

#### Build

From command line with CBuild

```PowerShell
MSBuild Solution_IO.sln /p:Configuration=Release
```

Or open IDE Solution_IO.sln file and preform build from IDE


### Code::Blocks IDE
Open __io.cbp__ progect file, select configuration you'd like and build. Same procedure for test and examples. MS VC++ compiler is not supported, use Visual Studio as described above.


### EXAMPLES
Code examples can be found examples sub-folder. 
Where:
* channels - basic input/output functionality
* chconv – converting characters between different code pages example
* collada - limited set of [COLLADA](https://www.khronos.org/collada/) 3D model OpenGL 4+ based viewer. Contans even based collada parser and simple 3D graphical model rendering engine.
* iostreams – writes UNICODE strings into file and console with transcoding characters between multiple UNICODE representation, using C++ iostreams build on top of IO channels.
* xml_parse_to_structures - reads XML into C++ structure data type with cursor API
* xml_event_parsing – reads any format XML document and prints parsed result into console with color highlight
* xml_write_events – writes custom XML document with features like comments, tag prefixes and namespaces, CDATA. Format the result to human readable (pretty-print)
