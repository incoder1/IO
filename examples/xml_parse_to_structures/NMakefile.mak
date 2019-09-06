
ROOT=..\..
LDPATH=$(ROOT)\target\release-win-msvc-dll-x64
INCLUDES=$(ROOT)\include

link: compile
	link /LTCG /LIBPATH:$(LDPATH) /DEFAULTLIB:msvcrt.lib /DEFAULTLIB:vcruntime.lib /OUT:xmlread.exe io.lib main.obj
compile:
	cl /c /nologo /utf-8 /std:c++latest /GL /O2 /Zl /EHsc /DNDEBUG /DUNICODE /DIO_SHARED_LIB  /I$(INCLUDES) /Fomain.obj  main.cpp