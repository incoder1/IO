# IO

IO is modern C++ (C++ 11 +) library for general propose input/output and XML processing

## LIBRARY FUTURES (Sub libraries)

### GENEGIC INPUT/OUTPUT interface.

Generic input/output interface for low-level binary synchronous and asynchronous input/output operations,
implemented on top of system calls and without Libc FILE or lib std C++ file buffer. 
Interfaces and implementers guaranty for work with exceptions and RTTI off. Input/output or memory error handling implemented 
using std::error_code.
 
Channels interface allow you easily implement basic binary input/output for any source like files, sockets, named 
and unnamed pipes, network sockets etc. Channels API user do not care where data comes from or where is should be written,
API is always the same.
 
Currently implemented channels for Windows and UNIX files and standard streams (terminal/console).

### Network INPUT/OUTPUT

There are Unix Domain sockets TCP/IP channels. With TCP, UDP and ICMP support. For TCP and UDP there is a TLS and DTLS support 
over the gnutls library. IP V4 and V6 both can be used with the same API (unlinke asio).
There is a minumal http client library, can be used for obtaing external XML XSD schemas or DTD. If you need somethign more powerfull
you can use Windows API HTTP components or libcurl (second is better).

Assynchonius IO operations is not yet done, but it you can easy implement requared channels on top of [boost] asio (non boost version is better choose)
in a few lines of code.

### C++ IOSTREAMS COMPATIBILITY

C++ iostreams compatibility library – provides input and output streams which works on top of channel, so you don’t need 
to create your own streams & stream buffers depending on files/shared memory/named and unnamed pipes/network sockets each time.

### CHARACTER SET LIBRARY

Character set library for detecting and converting between characters sets (code pages) without depending on current C/C++ 
locale. Library designed to be useful for applications, which should work in worldwide like a web server or web browser 
for example. Conversion between character sets based on iconv library standardized by POSIX. Microsoft Windows version 
uses port of GNU iconv. There are minimal port of Mozilla universal character detector, which can detect latin1 
or UNICODE representation from a sort of raw bytes. As whell as byte order mark ( BOM ) utility classes bundled.
 

### CONSOLE

Console mode input/output and color manipulation library. Allows input and output into system console/terminal without 
back effect of C/C++ locale library, and in full UNICODE mode. Unix and Windows versions supports full UNICODE console
including UTF-8 and system WCHAR mode. Library have possibility to define console colors, in respect to portability only 
16 standard colors used so that it should work with most terminals.

### STRINGS

Constant string class and string poll support.
Io::const_string a constant string which work like a intrusive smart pointer, e.g. not copy background character array each 
time you put const std::string& reference into function parameter, or assign a values to std::string. 
Background data array is considered to be always in UFT-8 UNICODE, and there are utility functions to trunscode for char16_t,
char32_t or wchar_t std::basic_string.
io::string_pool and io::cached_string is a pool to reduce memory usage for binary equal strings + support for better string 
hash function. For example GNU lib std++ uses MurMur2 hash function, which is legacy now days. IO provides MurMur3 hash function
for 32 bit CPUs and Google CityHash function for 64 bit CPUS.

### StAX XML PARSER

Streaming API for XML parsing – a non-validating (except for XML syntax) parser for XML document with StAX API. 
Exceptions and RTTI off mode supported. Errors including out of memory can be handled by error code.

### XML MARSHALLER

A reflection like XML marshaller (XML serialize) – a library for serialize Plain Old CLR Object (POCO)
into XML of specific format. You can save a value as XML tag or like a XML tag attribute. 
Library Idea – is to have a XML biding API like Java JAXB, but since C++ 11/17 have no standard reflection methodology
as well as no annotations support, IO is based on compile time reflection with ability to specify XML format using sets of 
std::tuple.  When you have RTTI on, you also able to generate XSD from plain old C++ classes.
XML un-marshalling (deserialization) implementation is in progress.
Serialization is based on std::ostream, so it is not exeption safe, e.g. you need some custom unexpected/terminate handlers.

### DEPENDENCIES 

You should have C++ 11 compatible compiler with STD C++ library. Optionally you can use Boost library in order to have 
fully compatible boost::intrusive_ptr.

Lib iconv with support of the following character sets (code pages): 
	Full UNICODE: UTF-8,UTF-16LE, UTF-16BE, UTF-32LE, UTF-32BE, UTF-7;
	One byte code pages: US ASCII, ISO-8859-1…ISO-8859-16, KOI8-R, KOI8-U, KOI8-RU;
	Windows one byte code pages: CP-1250 … CP-1258.
	
	GNU iconv and iconv from GLIB C tested as fully compatible
	gnutls - http://www.gnutls.org/ for SLL/TLS secured network connections

## SUPPORTED OPERATING SYSTEMS AND COMPILERS

Generally library should work with Micosoft Windows Vista + and any POSIX compliant (Unix like GNU/Linux FreeBSD or Mac OS X) 
operating system with any C++ 11 compatible (or partial compatible) compiler. C++ standard library must provide:

	* <functional>
	* <system_error>
	* <tuple>
	* <type_traits>
	* <utility>
	
### TESTED COMPILERS AND PLATFORMS

#### Microsoft Windows 
			- Windows 7
			- Windows 10
		GCC/G++ 5.1 -  MinGW64 (TDM build)
		GCC/G++ 7.2.0 MinGW64 (MSYS2 build)
	
(MS Visual C++ 15 should work [there is specific code] but not yet tested.
		
#### GNU/Lunux (Fedora 26)
		GCC/GC++ 7.1 

## BUILDING
 

### Building with Code::Blocks IDE

Code::Blocks IDE project files bundled. There are predefined configurations for GCC. 


For Windows GCC you can use MSYS2 MinGW64. Otherwise (for example TDM GCC etc. ) you need to 
find/build 
	- libiconv
	- gnutls  with all dependencies 
	
### Building with CMake
To build with CMake build tool to can use following command

> cmake . -DCMAKE_BUILD_TYPE=<Release|Debug> [-DBUILD_SHARED_LIBS=ON] [-DNO_EXCEPTIONS=ON] [-DNO_RTTI=ON]

Result is stored in  target/lib sub-folder

You can build shared or static library release or debug version, optionally you can on or off exceptions and rtti.

### Building with GNU make

This make files have no configuration state, i.e. all compile options are predefined. Only release version with exceptions 
and RTTI off provided.

#### Windows with GNU make

In case of Windows you must use MSYS2. 

To build static release library use:
		
>		make –f   Makefile-mingw64-static
		
		build result available in lib/release-win-gcc-static-x64 subfolder
		
To build DLL release library use:

>	 	make –f Makefile-mingw64-dll
		
		build result available in target/release-win-gcc-dll-x64 subfolder

#### Unix with GNU make

>	 	make –f Makefile-unix-shared

		build result available in target/release-unix-gcc-so-x86_64

### EXAMPLE CODE

Example applications located in examples sub-folder
	
	- chconv – character set (code pages) converting example 
	- iostreams – C++ input streams and console compatibility example
	- stringpool – string pool example
	- text – universal character set detector example
	- xml_marshalling – compile time reflection plain C object XML marshalling and XSD generation example
	- xmlparse – StAX XML parser example 

