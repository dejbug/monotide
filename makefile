TARGET ?= console
DEBUG ?= 1
PRECISE ?= 1

SHELL := cmd.exe
CXX := g++

SYMBOLS := WIN32_LEAN_AND_MEAN STRICT UNICODE _UNICODE
WINLIBS := gdi32

SOURCES := $(wildcard src/*.cpp)
PREREQS := $(SOURCES:src/%.cpp=build/%.d)
OBJECTS := $(SOURCES:src/%.cpp=build/%.o)

CXXFLAGS :=
CXXFLAGS += -std=c++11 -fabi-version=11 -Wall -Wextra -pedantic
CXXFLAGS += $(addprefix -D,$(SYMBOLS))

LDFLAGS :=
LDFLAGS += $(addprefix -l,$(WINLIBS))

ifeq ($(TARGET),windows)
LDFLAGS += -Wl,--subsystem=windows
DEBUG := 0
else
LDFLAGS += -Wl,--subsystem=console
endif

ifneq ($(DEBUG),0)
CXXFLAGS += -g -Og
else
CXXFLAGS += -O3 -DNDEBUG
endif

.PHONY : clean reset run

deploy/monotide.exe : $(OBJECTS) build/resource.o | deploy
deploy/monotide.exe : ; $(call LINK,$@,$^)

deploy : ; $(call MKDIR,deploy)
build : | deploy ; $(call MKDIR,build)
build/resource.o : src/main.rc src/resource.h | build ; windres $< $@

build/%.o : src/%.cpp | build ; $(call COMPILE,$@,$^)
build/%.d : src/%.cpp | build ; $(call GEN_PREREQ,$@,$<)

clean reset :: ; $(call DELTREE,build)
reset :: ; $(call DELTREE,deploy)

run : deploy/monotide.exe ; $<

ifneq ($(PRECISE),0)
ifeq (,$(call HAS_NON_BUILD_GOAL))
include $(PREREQS)
endif
endif

.DELETE_ON_ERROR :

define MKDIR
@echo [ -- creating dir "$1" ]
@IF NOT EXIST $1 mkdir $1
endef

define DELTREE
@echo [ -- removing dir "$1" ]
@IF EXIST $1 ( RMDIR /S /Q $1 )
endef

define COMPILE
@echo [ -- compiling "$1" ]
$(CXX) -c $(filter %.cpp,$2) -o $1 $(CXXFLAGS)
endef

define LINK
@echo [ -- linking "$1" ]
$(CXX) $(filter %.o %.a,$2) -o $1 $(CXXFLAGS) $(LDFLAGS)
endef

define GEN_PREREQ
@echo [ -- generating prerequisites "$1" ]
$(CXX) -MF $1 -MM $2 -MT "$(subst .d,.o,$1) $1" $(CXXFLAGS)
endef

define HAS_GOAL
$(findstring $1,$(MAKECMDGOALS))
endef

define HAS_NON_BUILD_GOAL
$(or $(call HAS_GOAL,clean),$(call HAS_GOAL,reset))
endef
