IO

IO is modern C++ (C++ 11 +) library for general propose input/output and XML processing

LIBRARY FUTURES (Sub libraries)

GENEGIC INPUT/OUTPUT interface.

Generic input/output interface for low-level binary synchronous and asynchronous input/output operations,
implemented on top of system calls and without Libc FILE or lib std C++ file buffer. Interface and implementers guaranty
for work with exceptions and RTTI off. Input/output or memory error handling implemented using std::error_code.
Channels interface allow you easily implement basic binary input/output for any source like files, sockets, named 
and unnamed pipes, networks sockets etc. Channels API user do not care where data comes from or where is should be written,
API is always the same. Currently bundled and tested channels for Windows and UNIX files and standard streams (terminal/console).
Network and pipes support in progress.

C++ IOSTREAMS COMPATIBILITY

C++ iostreams compatibility library – provides input and output streams which works on top of channel, so you don’t need to create your own streams & stream buffers depending on files/shared memory/named and unnamed pipes/network sockets each time.

CHARACTER SET LIBRARY

Character set library for detecting and converting between characters sets (code pages) without depending on current C/C++ 
locale. Library designed to be useful for applications, which should work in worldwide like a web server or web browser 
for example. Conversion between character sets based on iconv library standardized by POSIX. Microsoft Windows version 
uses port of GNU iconv. There are minimal port of Mozilla universal character detector, which can detect latin1 
or UNICODE reorientation from a sort of raw bytes.

CONSOLE

Console mode input/output and color manipulation library. Allows input and output into system console/terminal without 
back effect of C/C++ locale library, and in full UNICODE mode. Unix and Windows versions supports full UNICODE console
including UTF-8 and system WCHAR mode. Library have possibility to define console colors, in respect to portability only 
16 standard colors used so that it should work with most terminals.

STRINGS

Constant string class and string poll support.
Io::const_string a constant string which work like a intrusive smart pointer, e.g. not copy background character array each time you put const std::string& reference into function parameter, or assign a values to std::string. Background data array is considered to be always in UFT-8 UNICODE, and there are utility functions to trunscode for char16_t, char32_t or wchar_t std::basic_string.
io::string_pool and io::cached_string is a pool to reduce memory usage for binary equal strings + support for better string hash function. For example GNU lib std++ uses MurMur2 hash function, which is legacy now days. IO provides MurMur3 hash function for 32 bit CPUs and Google CityHash function for 64 bit CPUS.

StAX XML PARSER
Streaming API for XML parsing – a non-validating (except for XML syntax) parser for XML document with StAX API. 
Exceptions and RTTI off supported. Errors including out of memory can be handled by error code.

XML MARSHALLER

A reflection like XML marshaller (XML serialize) – a library for serialize plan old C++ classes or structures 
into XML of specific format, i.e. you can save a value as XML tag or like a XML tag attribute. Library Idea – is to have 
a XML biding API like Java JAXB, but since C++ 11/17 have no standard reflection methodology as well as no annotations support,
IO is based on compile time reflection with ability to specify XML format using sets of std::tuple. 
When you have RTTI on, you also able to generate XSD from plain old C++ classes.
XML un-marshalling (deserialization) implementation is in progress.

DEPENDENCIES 

You should have C++ 11 compatible compiler with STD C++ library. Optionally you can use Boost library in order to have 
fully compatible boost::intrusive_ptr.

Lib iconv with support of the following character sets (code pages): 
	Full UNICODE: UTF-8,UTF-16LE, UTF-16BE, UTF-32LE, UTF-32BE, UTF-7;
	One byte code pages: US ASCII, ISO-8859-1…ISO-8859-16, KOI8-R, KOI8-U, KOI8-RU;
	Windows one byte code pages: CP-1250 … CP-1258.
	
	GNU iconv and iconv from GLIB C tested as fully compatible. 

SUPPORTED OPERATING SYSTEMS AND COMPILERS

Generally library should work with Micosoft Windows Vista + and any POSIX compliant (Unix like GNU/Linux FreeBSD or Mac OS X) 
operating system with any C++ 11 compatible (or partial compatible) compiler with support of:

	rvalue references and move semantic ( clazz&& obj, std::move(obj), std::forward<clazz>(obj) )
	noexept and variadic template arguments
	C++ standard library must provide std::error_code
	
TESTED COMPILERS AND PLATFIRMS

Microsoft Windows (Windows 7)
		GCC/G++ 5.1 -  MinGW64 (TDM) 
		(MS Visual C++ 15 should work [there is specific code] but not yet tested.
		
GNU/Lunux (Fedora 26)
		GCC/GC++ 7.1 

BUILDING

Building currently build system is in progress. However, to build MinGW32/64 version of static or runtime library you can use  
MinGW64 TDM and Msys or Msys2 (make sure configured properly to use your compiler).  
This builds have no configure state, i.e. you can build release only static library or DLL with exceptions and RTTI off. 
Cmake and boost JAM builds support is in progress.

In any case, you can use open source Code::Blocks IDE to build binaries. IDE project files bundled. 

To build static release library use:
		
		make –f   Makefile-mingw64-static
		
		build result available in lib/release-win-gcc-static-x64 subfolder
		
To build DLL release library use:

		make –f Makefile-mingw64-dll
		
		build result available in lib/release-win-gcc-dll-x64 subfolder

EXAMPLE CODE

Example applications located in examples sub-folder
	
	chconv – character set (code pages) converting example 
	iostreams – C++ input streams and console compatibility example
	stringpool – string pool example
	text – universal character set detector example
	xml_marshalling – compile time reflection plain C object XML marshalling and XSD generation example
	xmlparse – StAX XML parser example 

ANY HELP IS WELCOME

