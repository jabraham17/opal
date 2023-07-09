
-include $(ROOT_PROJECT_DIRECTORY)color.mk

DEBUG=0
ifeq ($(DEBUG),1)
BUILD=debug
else
BUILD=build
endif
BUILD_DIRECTORY=$(ROOT_PROJECT_DIRECTORY)$(BUILD)/

BIN_DIRECTORY=$(BUILD_DIRECTORY)bin/
OBJ_DIRECTORY=$(BUILD_DIRECTORY)obj/
LIB_DIRECTORY=$(BUILD_DIRECTORY)lib/
GEN_DIRECTORY=$(BUILD_DIRECTORY)gen/

REL_PATH=$(shell realpath --relative-to $(ROOT_PROJECT_DIRECTORY) .)/
SRC_PATH=$(ROOT_PROJECT_DIRECTORY)$(REL_PATH)
OBJ_PATH=$(OBJ_DIRECTORY)$(REL_PATH)

# erase automatic vars
.SUFFIXES:

OS=$(shell uname)
ifeq ($(OS),Linux)
CC=/usr/bin/clang
CXX:=/usr/bin/clang++
LD:=$(CXX)
AR=/usr/bin/ar
RANLIB=/usr/bin/ranlib
YACC=/usr/bin/bison
LEX=/usr/bin/flex
PYTHON3=/usr/bin/python3
else ifeq ($(OS),Darwin)
CC=/usr/local/opt/llvm/bin/clang
CXX:=$(CC)
LD:=$(CC)
AR=/usr/local/opt/llvm/bin/llvm-ar
RANLIB=/usr/local/opt/llvm/bin/llvm-ranlib
YACC=/usr/local/opt/bison/bin/bison
LEX=/usr/local/opt/flex/bin/flex
PYTHON3=/usr/bin/python3
else
$(error Unsupported build on $(OS))
endif


VERBOSE=0
ifeq ($(VERBOSE),0)
AT=@
else
AT=
endif

define strip_root
$$(shell sed s%$(ROOT_PROJECT_DIRECTORY)%% <<< $1)
endef
define _generate_verbose_call
override $1_0=@printf "$(COLOR_GREEN)$(COLOR_BOLD)%s %s$(COLOR_RESET)\n" $1 $(call strip_root,$$@); $($1)
override $1_1=$($1)
override $1=$$($1_$(VERBOSE))
endef
define generate_verbose_call
$(eval $(call _generate_verbose_call,$1))
endef

map = $(foreach a,$(2),$(call $(1),$(a)))
$(call map,generate_verbose_call,CC CXX LD AR RANLIB YACC LEX PYTHON3)

COMPILE_FLAGS_=
LINK_FLAGS_=
COMPILE_FLAGS_+= -Wno-comment
COMPILE_FLAGS_+= -flto
LINK_FLAGS_+= -flto

ifeq ($(DEBUG),1)
COMPILE_FLAGS_+= -DDEBUG=1 -g -O0 -fstandalone-debug
LINK_FLAGS_+= -g
else
# #warnings are for developers
COMPILE_FLAGS_+= -O3 -Wno-\#warnings
endif

COMPILE_FLAGS_+= -masm=intel
COMPILE_FLAGS_+= -Wall -Wextra


ifdef LINKER
LINK_FLAGS_+= -fuse-ld=$(LINKER)
endif

# from user
COMPILE_FLAGS_+= $(COMPILE_FLAGS)
LINK_FLAGS_+= $(LINK_FLAGS)

# from user
BISON_FLAGS_=
BISON_FLAGS_+= -Wall
BISON_FLAGS_+= $(BISON_FLAGS)
ifeq ($(DEBUG),1)
BISON_FLAGS_+= -Dparse.trace
ifeq ($(DEBUG_PARSER),1)
COMPILE_FLAGS_+= -DDEBUG_PARSER=1
endif
endif


override CFLAGS+= $(COMPILE_FLAGS_) -std=c17 -D_XOPEN_SOURCE=700
override CXXFLAGS+= $(COMPILE_FLAGS_) -std=c++17
override ASFLAGS+=
override LDFLAGS+= $(LINK_FLAGS_)
override INCLUDE+=
override YFLAGS+= $(BISON_FLAGS_)
override LFLAGS+=

