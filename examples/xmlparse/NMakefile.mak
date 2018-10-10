
ROOT=..\..
LDPATH=$(ROOT)\target\release-win-msvc-dll-x64
INCLUDES=$(ROOT)\include

link: compile
	link /LTCG /LIBPATH:$(LDPATH) /OUT:xmlparse.exe io.lib main.obj
compile:
	cl /c /nologo /utf-8 /std:c++latest /GL /O2 /MT /DNDEBUG /DUNICODE /DIO_SHARED_LIB  /I$(INCLUDES) /Fomain.obj  main.cpp