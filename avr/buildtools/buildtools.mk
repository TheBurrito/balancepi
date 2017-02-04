include commands.mk

#even though this makefile is in the buildtools folder, it is executed in the
#context of the projetc root, so all targets must be specified relative to
#the project root.

all: buildtools/bin/fixdeps

buildtools/bin/fixdeps: buildtools/build/fixdeps.o
	$(quiet)  $(LD) -o $@ $^

buildtools/build/fixdeps.o: buildtools/src/fixdeps.cpp
	@echo "Building fixdeps tool"
	$(quiet)  $(CXX) -o $@ -c $^
	
.PHONY: distclean
distclean:
	$(rm) -f buildtools/bin/* buildtools/build/*
