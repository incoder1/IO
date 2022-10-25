# This make file configured to MSYS2 and statically linked IO library build with 
#    make -f Makefile-mingw64-static

# Follow the file to switch options

CC=gcc
CXX=g++
AS=g++
LD=g++


# Comment this for Unix
#LIBDIR=../../target/release-win-gcc-static-x64

# Uncomment this for Unix
 LIBDIR=../../target/release-unix-gcc-so-x86_64

# Comment is for Unix
# LIBS ?= -lio -liconv

# Uncommnetn next line for Unix
LIBS ?= -lio `pkg-config --libs gnutls`

DIST_DIR=./target
# comment it for Unix
#EXEC_EXT=.exe

#Uncoment it for unix
EXEC_EXT=


# Uncomment next line for standalone windows app, which can be run without MSYS2 
# DEFLIBS_DIR=/mingw64/bin

INCLUEDS?=-I../../include
MMX_AND_SSE?= -mmmx -msse -msse2 -mssse3 -msse3 -msse4 -mavx
OPTIMIZE?=-mtune=generic -fwhole-program -O3 -s -fno-exceptions -fno-rtti -minline-stringops-dynamically -fdeclone-ctor-dtor -fdevirtualize-at-ltrans -mmovbe $(MMX_AND_SSE)
DEFINES ?= -DNDEBUG
# Comment next line for standalone windows app, which can be run without MSYS2 
DEFINES := $(DEFINES) -DUNICODE 
# Uncomment next line for Unix
DEFINES := $(DEFINES) -DIO_SHARED_LIB
CPPFLAGS?= -std=gnu++14 -llto -fno-exceptions -fno-rtti -Winline -pedantic $(INCLUEDS) $(DEFINES)
LDFLAGS?= -L$(LIBDIR) $(LIBS) -llto $(OPTIMIZE)


 
all: clean main.obj link dist

clean:
	rm -rf $(DIST_DIR) main.obj
	mkdir $(DIST_DIR)
	
main.obj:
	$(CXX) $(INCLUEDS) $(CPPFLAGS) -c main.cpp -o main.obj
	
link:
	$(LD) main.obj -o $(DIST_DIR)/xmlparse$(EXEC_EXT) $(LDFLAGS)
	
dist:
# uncomment this block for standalone windows app, which can be run without MSYS2 
#	cp $(DEFLIBS_DIR)/libgcc_s_seh-1.dll $(DIST_DIR)/libgcc_s_seh-1.dll
#	cp $(DEFLIBS_DIR)/libstdc++-6.dll $(DIST_DIR)/libstdc++-6.dll
#	cp $(DEFLIBS_DIR)/libwinpthread-1.dll $(DIST_DIR)/libwinpthread-1.dll
#	cp $(DEFLIBS_DIR)/libiconv-2.dll $(DIST_DIR)/libiconv-2.dll
# Uncomment next line for  Unix
# and dont fogot to 
# export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:`pwd` in target folder
	cp $(LIBDIR)/libio.so $(DIST_DIR)/libio.so
	cp ./*.xml $(DIST_DIR)
