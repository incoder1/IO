
ROOT=..\..

LDPATH=$(ROOT)\target\release-win-msvc-dll-x64
INCLUDES=/I$(ROOT)\include /I$(ROOT)\deps\msvc\include

SHARED_DEFINES= /DIO_SHARED_LIB 
DEFINES = $(SHARED_DEFINES) /DNDEBUG /DUNICODE
CPPFLAGS = /c /nologo /GL /Zl /std:c++latest /EHsc $(DEFINES) $(OPTIMIZE) $(INCLUEDS) 

link: compile
	link /LTCG /OUT:network.exe /LIBPATH:$(LDPATH) io.lib msvcrt.lib kernel32.lib user32.lib Ws2_32.lib main.obj
compile:
	cl $(CPPFLAGS) $(INCLUDES) /Fomain.obj  main.cpp