USE_GDB    		 ?= false
USE_ASAN		 ?= false
USE_VALGRIND	 ?= false

HASH_TABLE_DEBUG ?= false
USE_LOGS		 ?= false

CXX         =   g++
OPT_FLAGS   =  -mavx2
OPT_LEVEL   ?= -O3
CXXFLAGS    =  -Wall -Wextra -std=c++17 $(OPT_FLAGS) $(OPT_LEVEL)

ASAN_FLAGS	   = -fsanitize=address,alignment,bool,bounds,enum,float-cast-overflow,float-divide-by-zero,integer-divide-by-zero,leak,nonnull-attribute,null,object-size,return,returns-nonnull-attribute,shift,signed-integer-overflow,undefined,unreachable,vla-bound,vptr
VALGRIND_FLAGS = --tool=callgrind --dump-instr=yes --collect-jumps=yes --simulate-cache=yes

HASH_SRC_DIR   = hash_table/src
HASH_LIB_DIR   = hash_table/lib

LIST_SRC_DIR   = LIST/src
LIST_LIB_DIR   = LIST/lib

LOGS_SRC_DIR   = logger/src
LOGS_LIB_DIR   = logger/lib


HASH_OBJ_DIR   = hash_table/build/obj
LIST_OBJ_DIR   = LIST/obj
LOGS_OBJ_DIR   = logger/obj

BUILD_DIR      = hash_table/build

HASH_SRC_CPP   = $(wildcard $(HASH_SRC_DIR)/*.cpp)
LIST_SRC_CPP   = $(wildcard $(LIST_SRC_DIR)/*.cpp)
LOGS_SRC_CPP   = $(wildcard $(LOGS_SRC_DIR)/*.cpp)

HASH_OBJ_CPP   = $(patsubst $(HASH_SRC_DIR)/%.cpp,$(HASH_OBJ_DIR)/%.o,$(HASH_SRC_CPP))
LIST_OBJ_CPP   = $(patsubst $(LIST_SRC_DIR)/%.cpp,$(LIST_OBJ_DIR)/%.o,$(LIST_SRC_CPP))
LOGS_OBJ_CPP   = $(patsubst $(LOGS_SRC_DIR)/%.cpp,$(LOGS_OBJ_DIR)/%.o,$(LOGS_SRC_CPP))

TARGET = $(BUILD_DIR)/hash_table

LDFLAGS  = -D _DEBUG -ggdb3 -std=c++17 -Wall -Wextra -Weffc++ -Waggressive-loop-optimizations -Wc++14-compat -Wmissing-declarations -Wcast-align -Wcast-qual                	\
 -Wchar-subscripts -Wconditionally-supported -Wconversion -Wctor-dtor-privacy -Wempty-body -Wfloat-equal -Wformat-nonliteral -Wformat-security -Wformat-signedness               \
 -Wformat=2 -Winline -Wlogical-op -Wnon-virtual-dtor -Wopenmp-simd -Woverloaded-virtual -Wpacked -Wpointer-arith -Winit-self -Wredundant-decls -Wshadow -Wsign-conversion         \
 -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=2 -Wsuggest-attribute=noreturn -Wsuggest-final-methods -Wsuggest-final-types -Wsuggest-override -Wswitch-default            \
 -Wswitch-enum -Wsync-nand -Wundef -Wunreachable-code -Wunused -Wuseless-cast -Wvariadic-macros -Wno-literal-suffix -Wno-missing-field-initializers -Wno-narrowing                  \
 -Wno-old-style-cast -Wno-varargs -Wstack-protector -fcheck-new -fsized-deallocation -fstack-protector -fstrict-overflow -flto-odr-type-merging -fno-omit-frame-pointer              \
 -Wlarger-than=81920 -Wstack-usage=81920 -pie -fPIE -Werror=vla


ifneq ($(filter true,$(USE_GDB) $(USE_ASAN) $(USE_VALGRIND)),)
    LDFLAGS += -g
endif

ifneq ($(USE_VALGRIND), true)
	ifeq ($(USE_ASAN), true)
		LDFLAGS += $(ASAN_FLAGS)
	endif
endif

ifeq ($(HASH_TABLE_DEBUG), true)
	CXXFLAGS += -D HASH_TABLE_DEBUG
endif

ifeq ($(LIST_DEBUG), true)
	CXXFLAGS += -D LIST_DEBUG
endif

ifeq ($(USE_LOGS), true)
	CXXFLAGS += -D USE_LOGS
endif

.PHONY: all clean rebuild run gdb

all: $(TARGET)

run:
	@$(TARGET)

gdb:
	make rebuild USE_GDB=true OPT_LEVEL=-O0 && gdb $(TARGET)

callgrind:
	make rebuild USE_VALGRIND=true && valgrind $(VALGRIND_FLAGS) $(TARGET)

clean:
	rm -rf $(HASH_OBJ_DIR) $(LIST_OBJ_DIR) $(LOGS_OBJ_DIR) $(TARGET)

rebuild: clean all


$(TARGET): $(HASH_OBJ_CPP) $(LIST_OBJ_CPP) $(LOGS_OBJ_CPP) | $(BUILD_DIR)
	$(CXX) $(HASH_OBJ_CPP) $(LIST_OBJ_CPP) $(LOGS_OBJ_CPP) -o $@ $(LDFLAGS)

$(HASH_OBJ_DIR)/%.o: $(HASH_SRC_DIR)/%.cpp | $(HASH_OBJ_DIR)
	$(CXX) $(CXXFLAGS) -I$(HASH_LIB_DIR) -I$(LIST_LIB_DIR) -I$(LOGS_LIB_DIR) -c $< -o $@

$(LIST_OBJ_DIR)/%.o: $(LIST_SRC_DIR)/%.cpp | $(LIST_OBJ_DIR)
	$(CXX) $(CXXFLAGS) -I$(LIST_LIB_DIR) -I$(LOGS_LIB_DIR) -c $< -o $@

$(LOGS_OBJ_DIR)/%.o: $(LOGS_SRC_DIR)/%.cpp | $(LOGS_OBJ_DIR)
	$(CXX) $(CXXFLAGS) -I$(LOGS_LIB_DIR) -c $< -o $@

$(HASH_OBJ_DIR):
	mkdir -p $@

$(LIST_OBJ_DIR):
	mkdir -p $@

$(LOGS_OBJ_DIR):
	mkdir -p $@

$(BUILD_DIR):
	mkdir -p $@