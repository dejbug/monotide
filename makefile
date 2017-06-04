
SHELL := cmd.exe

define MKDIR
@echo -- creating dir "$(1)"
@IF NOT EXIST $(1) mkdir $(1)
@echo -- dir "$(1)" created
endef

define F_DEL_TREE
@echo -- removing dir "$(1)"
@IF EXIST $(1) ( RMDIR /S /Q $(1) )
@echo -- dir "$(1)" removed
endef

# SUBSYSTEM := windows
# TARGET := release
SUBSYSTEM := console
TARGET := debug
WINLIBS := gdi32 shell32

CXX := g++

CXXFLAGS :=
CXXFLAGS += -std=c++11 -Wall -pedantic
CXXFLAGS += -Wl,--subsystem=$(SUBSYSTEM)
CXXFLAGS += $(addprefix -l,$(WINLIBS))
CXXFLAGS += -DWIN32_LEAN_AND_MEAN -DSTRICT

ifeq ($(TARGET),release)
CXXFLAGS += -O2
else
CXXFLAGS += -g -O
endif

.PHONY: all
all: build
all: build/monotide.exe

build: ; $(call MKDIR,build)

build/monotide.exe: build/snippets.o
build/monotide.exe: build/lib_window.o
build/monotide.exe: build/lib_font.o
build/monotide.exe: build/main.o
build/monotide.exe: ; $(CXX) $^ -o $@ $(CXXFLAGS)

build/%.o: src/%.cpp ; $(CXX) -c $^ -o $@ $(CXXFLAGS)

.PHONY: clean
clean:
	$(call F_DEL_TREE,build)

.PHONY: run
run: build build/monotide.exe
	@build\monotide.exe
