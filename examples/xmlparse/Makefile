# This make file configured to MSYS2 and statically linked IO library build with 
#    make -f Makefile-mingw64-static

# Follow the file to switch options

CC=gcc
CXX=g++
AS=g++
LD=g++


# Comment this for Unix
LIBDIR=../../target/release-win-gcc-static-x64

# Uncomment this for Unix
# LIBDIR=../../target/release-unix-gcc-so-x86_64

LIBS?=-lio -liconv
DIST_DIR=./target

# Uncomment next line for standalone windows app, which can be run without MSYS2 
# DEFLIBS_DIR=/mingw64/bin

INCLUEDS?=-I../../include
MMX_AND_SSE?= -mmmx -msse -msse2 -mssse3 -msse3 -msse4 -mavx
OPTIMIZE?=-mtune=generic -O3 -fno-exceptions -fno-rtti -minline-stringops-dynamically -fdeclone-ctor-dtor -fdevirtualize-at-ltrans -mmovbe $(MMX_AND_SSE)
DEFINES ?= -DNDEBUG
# Comment next line for standalone windows app, which can be run without MSYS2 
DEFINES := $(DEFINES) -DUNICODE 
# Uncomment next line for Unix
# DEFINES := $(DEFINES) -DIO_SHARED_LIB
CPPFLAGS?= -std=gnu++14 -fno-exceptions -fno-rtti -Winline -pedantic $(OPTIMIZE) $(INCLUEDS) $(DEFINES)
LDFLAGS?= -L$(LIBDIR) $(LIBS) -s


 
all: clean main.obj link dist

clean:
	rm -rf $(DIST_DIR) main.obj
	mkdir $(DIST_DIR)
	
main.obj:
	$(CXX) $(INCLUEDS) $(CPPFLAGS) -c main.cpp -o main.obj
	
link:
	$(LD) main.obj -o $(DIST_DIR)/xmlparse.exe $(LDFLAGS)
	
dist:
# uncomment this block for standalone windows app, which can be run without MSYS2 
#	cp $(DEFLIBS_DIR)/libgcc_s_seh-1.dll $(DIST_DIR)/libgcc_s_seh-1.dll
#	cp $(DEFLIBS_DIR)/libstdc++-6.dll $(DIST_DIR)/libstdc++-6.dll
#	cp $(DEFLIBS_DIR)/libwinpthread-1.dll $(DIST_DIR)/libwinpthread-1.dll
#	cp $(DEFLIBS_DIR)/libiconv-2.dll $(DIST_DIR)/libiconv-2.dll
	cp ./*.xml $(DIST_DIR)
