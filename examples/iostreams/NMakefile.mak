
ROOT=..\..
LDPATH=$(ROOT)\target\release-win-msvc-dll-x64
INCLUDES=$(ROOT)\include

link: compile
	link /LTCG /LIBPATH:$(LDPATH) /OUT:iostreams.exe msvcrt.lib io.lib main.obj
compile:
	cl /c /GL /Zl /Ox /Oi /std:c++latest /utf-8 /DNDEBUG /DUNICODE /DIO_SHARED_LIB  /I$(INCLUDES) /Fomain.obj  main.cpp