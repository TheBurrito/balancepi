#These are the command-line targets that are specialized for the non-recursive
#make used with this build system.

.PHONY: all

all:

#debug target adds debugger flags to compiler flags
.PHONY: debug
debug: CXXFLAGS+= -DDEBUG -ggdb
debug: CFLAGS+= -DDEBUG -ggdb
debug: all

#clean erases any *~ autosave files and empties the build folder. archived
#libraries and linked binaries are left
.PHONY: clean
clean: scratch=$(shell find . -name "*~")
clean:
	$(rm) $(BUILD_DIR)/* $(scratch)

#realclean also empties the lib and bin folders
.PHONY: realclean
realclean: clean
	$(rm) $(LIB_DIR)/* $(BIN_DIR)/*

#if any automatically generated dependency files have been created, include them
deps=$(wildcard $(BUILD_DIR)/*.d)
-include $(deps)

#generic rules for working on individual target files
%.o: %.c
	$(cc)

%.o: %.cpp
	$(cxx)

%: %.o
	$(ld)

%.a:
	$(ar)

#these are special rules to compile the tools used by this build system:
#note that for the build tools there is a recursive call to their makefile
#since the main project may be cross-compiled but we need the tools compiled
#for the current machine

#distclean adds the host generated build tools to the cleaning process
.PHONY: distclean
distclean: realclean
	@$(MAKE) -s -f buildtools/buildtools.mk distclean

#fixdeps modifies the gcc/g++ generated dependency files to work with this
#makefile build system. This is only used for C/C++ sources. when generating
#object files.
all: buildtools/bin/fixdeps
buildtools/bin/fixdeps:
	@$(MAKE) -s -f buildtools/buildtools.mk buildtools/bin/fixdeps
