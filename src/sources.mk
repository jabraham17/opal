
C_SOURCES=$(wildcard *.c) $(wildcard */*.c) $(wildcard */*/*.c)
C_OBJECTS=$(patsubst %,%.o,$(C_SOURCES))

CXX_SOURCES=$(wildcard *.cc) $(wildcard */*.cc) $(wildcard */*/*.cc) $(wildcard *.cpp) $(wildcard */*.cpp) $(wildcard */*/*.cpp)
CXX_OBJECTS=$(patsubst %,%.o,$(CXX_SOURCES))

A_SOURCES=$(wildcard *.asm) $(wildcard */*.asm) $(wildcard */*/*.asm)
A_OBJECTS=$(filter-out $(START),$(patsubst %.asm,%_a.o,$(A_SOURCES)))

YL_SOURCES=$(wildcard *.l) $(wildcard */*.l) $(wildcard */*/*.l) $(wildcard *.yy) $(wildcard */*.yy) $(wildcard */*/*.yy)
YL_OBJECTS=$(patsubst %,%.o,$(YL_SOURCES))

DEP_OBJECTS=$(C_OBJECTS) $(CXX_OBJECTS) $(A_OBJECTS)
OBJECTS+=$(DEP_OBJECTS) $(YL_OBJECTS)
DEPENDS=$(patsubst %,%.d,$(DEP_OBJECTS))


OBJECTS:=$(addprefix $(OBJ_PATH),$(OBJECTS))
DEPENDS:=$(addprefix $(OBJ_PATH),$(DEPENDS))

# other objects depend on bison/flex objects
# $(patsubst %,$(OBJ_PATH)%,$(DEP_OBJECTS)): $(patsubst %,$(OBJ_PATH)%,$(YL_OBJECTS))

override INCLUDE+= -I$(ROOT_PROJECT_DIRECTORY)src
override INCLUDE+= -I$(OBJ_PATH)
override INCLUDE+= -I$(GEN_DIRECTORY)
LIBRARIES_FILE_NAMES=$(patsubst %,$(LIB_DIRECTORY)lib%.a,$(LIBRARIES))
override LDLIBS+=$(patsubst %,-l%,$(LIBRARIES)) $(patsubst %,-l%,$(SYSTEM_LIBRARIES))

.DEFAULT_GOAL: all
all: $(OBJ_PATH) $(TARGET)
	@:

$(OBJ_PATH):
	$(AT)mkdir -p $@

.PHONY: cppcheck
cppcheck: $(C_SOURCES)
	$(AT)cppcheck $(INCLUDE) $^ -q

$(OBJ_PATH)%.c.o: $(SRC_PATH)%.c Makefile
	$(AT)mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -MP -MF $(patsubst $(SRC_PATH)%,$(OBJ_PATH)%.d,$<) $(INCLUDE) -c $< -o $@

$(OBJ_PATH)%.cc.o: $(SRC_PATH)%.cc Makefile
	$(AT)mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MMD -MP -MF $(patsubst $(SRC_PATH)%,$(OBJ_PATH)%.d,$<) $(INCLUDE) -c $< -o $@

$(OBJ_PATH)%.cpp.o: $(SRC_PATH)%.cpp Makefile
	$(AT)mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MMD -MP -MF $(patsubst $(SRC_PATH)%,$(OBJ_PATH)%.d,$<) $(INCLUDE) -c $< -o $@

$(OBJ_PATH)%.asm.o: $(SRC_PATH)%.asm Makefile
	$(AT)mkdir -p $(dir $@)
	$(AS) -felf $(CONSTANTS) $< -o $@ -MD $(patsubst $(SRC_PATH)%.asm,$(OBJ_PATH)%.d,$<)


_make_generated_c_source = $(patsubst %,$(GEN_DIRECTORY)%.cpp,$(shell sed s%$(ROOT_PROJECT_DIRECTORY)%% <<< $1))

$(OBJ_PATH)%.l.o: $(SRC_PATH)%.l $(OBJ_PATH)%.yy.o Makefile
	$(AT)mkdir -p $(dir $@)
	$(AT)mkdir -p $(dir $(call _make_generated_c_source,$<))
	$(LEX) $(LFLAGS) -o $(call _make_generated_c_source,$<) $<
	$(CXX) $(CXXFLAGS) -c -x c++ $(call _make_generated_c_source,$<) -x none -o $@ $(INCLUDE) -I$(dir $(call _make_generated_c_source,$<))

$(OBJ_PATH)%.yy.o: $(SRC_PATH)%.yy Makefile
	$(AT)mkdir -p $(dir $@)
	$(AT)mkdir -p $(dir $(call _make_generated_c_source,$<))
	$(YACC) $(YFLAGS) -o $(call _make_generated_c_source,$<) $< -d
	$(CXX) $(CXXFLAGS) -c -x c++ $(call _make_generated_c_source,$<) -x none -o $@ $(INCLUDE)

-include $(DEPENDS)
