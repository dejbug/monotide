
SHELL := cmd.exe

define MKDIR
@echo -- creating dir "$1"
@IF NOT EXIST $1 mkdir $1
@echo -- dir "$1" created
endef

define DELTREE
@echo -- removing dir "$1"
@IF EXIST $1 ( RMDIR /S /Q $1 )
@echo -- dir "$1" removed
endef

define COMPILE
$(CXX) -c $(filter %.cpp,$2) -o $1 $(CXXFLAGS)
endef

define LINK
$(CXX) $(filter %.o %.a,$2) -o $1 $(CXXFLAGS) $(LDFLAGS)
endef

PRECISE := 0
TARGET := debug
SYMBOLS := WIN32_LEAN_AND_MEAN STRICT UNICODE _UNICODE
WINLIBS := gdi32

CXX := g++

CXXFLAGS :=
CXXFLAGS += -std=c++11 -Wall -Wextra -pedantic
CXXFLAGS += $(addprefix -D,$(SYMBOLS))

LDFLAGS :=
LDFLAGS += $(addprefix -l,$(WINLIBS))

ifeq ($(TARGET),release)
CXXFLAGS += -O2
CXXFLAGS += -DNDEBUG
LDFLAGS += -Wl,--subsystem=windows
else
# CXXFLAGS += -g -O
CXXFLAGS += -g
LDFLAGS += -Wl,--subsystem=console
endif

.PHONY: all
all: build
all: build/monotide.exe

build: ; $(call MKDIR,build)

build/%.target : src/%.cpp | build
	g++ -MF $@ -MM $< -MT $(subst .target,.o,$@)
	TYPE $(subst /,\\,$@) >> build\.target

build/.target : $(patsubst src/%.cpp,build/%.target,$(wildcard src/*.cpp))

ifeq ($(PRECISE),0)
else
include build/.target
endif

OBJ := $(patsubst src/%.cpp,build/%.o,$(wildcard src/*.cpp))

build/monotide.exe: $(OBJ)
build/monotide.exe: build/resource.o
# build/monotide.exe: ; $(CXX) $^ -o $@ $(CXXFLAGS) $(LDFLAGS)
build/monotide.exe: ; $(call LINK,$@,$^)

build/%.o: src/%.cpp ; $(call COMPILE,$@,$^)
build/resource.o : src/main.rc src/resource.h ; windres $< $@

.PHONY: clean
clean:
	$(call DELTREE,build)

.PHONY: reset
reset:
	$(call DELTREE,build)
	$(call DELTREE,deploy)

.PHONY: run
run: all
	@build\monotide.exe
