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
3. Test over Google Test and cmake/ctest
4. Make files must be obsolite and removed complitelly, Cmake build only
5. Gith hub actions CI instead of Travis
6. Asynchronous IO for network with TLS security support		
7. HTTP client, use (preffered) libnghttp2 or create own libnghttp2 like implementaton, need proff of concept
8. Fix character set detection lib for full fertured, need proff of concept
9 Tutorial over the examples	
	
### Deep backlog i.e. for version 1.0.1 etc: 	
* YAML data format streaming API
* Named pipes sources
* Support for clang compiller
* Support for FreeBSD, Mac OS X Android and iOS	
* JSON data format sreaming like Boost JSON
* ASN 1.0 data format streaming 
* BSON data format streaming
* Extend COLLADA demo viewer with glTF add pathes support with geometry/calc shaders
* Support for Intel compiller
			
IO is modern C++ (std 11 +) library for general propose input/output library read and write binary and textual common data formats

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

#### CHARACTER SET LIBRARY

Character set library for detecting and converting between characters sets (code pages) without depending on current C/C++ 
locale. Library designed to be useful for applications, which should work in worldwide like a web server or web browser 
for example. Conversion between character sets based on iconv library standardized by POSIX. Microsoft Windows version 
uses port of GNU iconv. There are minimal port of Mozilla universal character detector, which can detect latin1 
or UNICODE representation from a sort of raw bytes. As well as byte order mark ( BOM ) utility classes bundled.
 

#### CONSOLE 

Console mode input/output and color manipulation library. Allows input and output into system console/terminal without 
back effect of C/C++ locale library, and in full UNICODE mode. Unix and Windows versions supports full UNICODE console
including UTF-8 and system WCHAR mode. There is possibility to define console colors, in respect to portability
16 standard colors used so that it should work with most terminals. Can be used in combination with iostreams compatibility
to replace std::cout and add coloring and UNICODE (Windows) support for console input output.

#### STRINGS

Constant string class - io::const_string a constant small string optimized wrapper on raw character array. When string size is less then double mashine word characters data
stored inside the class, therwise all equal instatnces of const_string sharing the same string pointer with intrusive atomic reference counting.
Background data array is considered to be always in UFT-8 UNICODE, and there are utility functions for trans-coding for char16_t,
char32_t or wchar_t std::basic_string. const_string can count UNICODE character lenght as well as provide bytes size.

const_string provides MurMur3 hash function for 32 bit instruction set and Google CityHash function for 64 bit CPUS.

#### LECICAL CASTING  
A common implemetion for lexical casting with the similar to to_chars/from_char to_string/from_string design. 

### XML 

Streaming i.e. iterator lile API for XML parsing and writing. Documentation and examples provided

Libray provides event and [cursor]([reading C++ structure from XML](https://github.com/incoder1/IO/wiki/Reading-structures-from-XML)) based API's.

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

- [GNU TLS v 3.0+](https://www.gnutls.org/) 
#### UNIX
	Most Unix distributions have an implementation, use your package manager to install development package
	i.e. 
	Debian based i.e. Debian/Ubuntu/Lint etc.
```bash	
	sudo apt-get install -y gnutls-dev 
```	
	RPM based Fedora,RHEL,CentOS, Mandriva etc.
```bash	
	sudo dnf install gnutls-devel 
```	
	Pacman based Arch Linux, Chakra etc.
```bash	
	pacman -S libgnutls-devel
```	

#### Windows
   Compiler | instruction
    MSYS2 | install developement package i.e. `pacman -S libgnutls-devel`
    MS Visual C++ | Download a release built from [ShiftMediaProject/gnutls](https://github.com/ShiftMediaProject/gnutls) and put it to deps/msvc sub folder

- [Goole Test](https://github.com/google/googletest) 
	Optional if you'd like to run tests

## SUPPORTED OPERATING SYSTEMS AND COMPILERS

Library tested on next configuratons

// TODO: insert build status to this marix when on github actions
// and move on the file header to check status

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
cmake -S . -B CBuild -DCMAKE_BUILD_TYPE=<Release|Debug> -DBUILD_LIBRARY_TYPE=<Shared|Static> [-DNO_EXCEPTIONS=ON] [-DNO_RTTI=ON] [-DRUN_TESTS=ON]
cmake --build CBuild
```

- BUILD_LIBRARY_TYPE use Shared with this flag to build shared library (DLL) or Static for the static library
- NO_EXCEPTIONS optional flag for disiabling C++ exceptions, by default exceptions will be used
- NO_RTTI optional flag for disiabling C++ runtime type information, by default RTTI generated
- RUN_TESTS optional flag to build and run tests, use this flag with Release and shared libraries builds

Build result can be found at _CBuild/target/<Release|Debug>/lib_ sub-folder, if you don't whant to install libary with cmake.

When you whant to build with MS VC++ solution as the Cmake output and d;like to build from command line e.g. the CI/CD build
```bash
cmake . -G "Visual Studio 17 2022" 
```
you can goto target sub floder and build with next command, instead of `cmake --build .`
```bash
MSBuild io.sln /p:Configuration=[Release|Debug]
```
Otherwise simply build solution with IDE. 

#### Windows with Microsoft VC++ and NMake

TODO: make files must be obsolite and removed complitelly, Cmake build only

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
