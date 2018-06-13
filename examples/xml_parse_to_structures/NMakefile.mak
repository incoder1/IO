
ROOT=..\..
LDPATH=$(ROOT)\target\release-win-msvc-dll-x64
INCLUDES=$(ROOT)\include

link: compile
	link /LTCG /LIBPATH:$(LDPATH) /OUT:xmlread.exe msvcrt.lib io.lib main.obj
compile:
	cl /c /GL /Zl /Ox /Oi /EHsc /std:c++latest /DNDEBUG /DUNICODE /DIO_SHARED_LIB  /I$(INCLUDES) /Fomain.obj  main.cpp