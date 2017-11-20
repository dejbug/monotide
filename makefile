
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

# TARGET := release
TARGET := debug
WINLIBS := gdi32
SYMBOLS := WIN32_LEAN_AND_MEAN STRICT UNICODE _UNICODE

CXX := g++

CXXFLAGS :=
CXXFLAGS += -std=c++11 -Wall -pedantic
CXXFLAGS += $(addprefix -D,$(SYMBOLS))
CXXFLAGS += $(addprefix -l,$(WINLIBS))

ifeq ($(TARGET),release)
CXXFLAGS += -O2
CXXFLAGS += -Wl,--subsystem=windows
CXXFLAGS += -DNDEBUG
else
# CXXFLAGS += -g -O
CXXFLAGS += -g
CXXFLAGS += -Wl,--subsystem=console
endif

.PHONY: all
all: build
all: build/monotide.exe

build: ; $(call MKDIR,build)

build/monotide.exe: build/fontlist.o
build/monotide.exe: build/snippets.o
build/monotide.exe: build/lib_worker.o
build/monotide.exe: build/lib_window.o
build/monotide.exe: build/lib_font.o
build/monotide.exe: build/app_fontrenderer.o
build/monotide.exe: build/main.o
build/monotide.exe: ; $(CXX) $^ -o $@ $(CXXFLAGS)

define cmpl
$(CXX) -c $(filter %.cpp,$2) -o $1 $(CXXFLAGS)
endef

build/snippets.o: src/snippets.cpp src/snippets.h ; $(call cmpl,$@,$^)
build/app_%.o: src/app_%.cpp src/app_%.h ; $(call cmpl,$@,$^)
build/lib_%.o: src/lib_%.cpp src/lib_%.h ; $(call cmpl,$@,$^)

build/%.o: src/%.cpp ; $(call cmpl,$@,$^)

build/main.o : src/main.cpp src/lib_window.h src/fontlist.h
build/fontlist.o : src/fontlist.cpp src/fontlist.h

.PHONY: clean
clean:
	$(call F_DEL_TREE,build)

.PHONY: run
run: build build/monotide.exe
	@build\monotide.exe
