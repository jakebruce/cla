BASE_DIR=$(shell pwd)

SWIG_DIR=$(BASE_DIR)/swig
SRC_DIR=$(BASE_DIR)/src
INC_DIR=$(BASE_DIR)/include
TEST_DIR=$(BASE_DIR)/test
BIN_DIR=$(BASE_DIR)/bin
OBJ_DIR=$(BASE_DIR)/obj

CC=g++
PYFLAGS=-c -fPIC -O3 -I$(INC_DIR) $$(pkg-config --cflags --libs python)
CFLAGS=-c -fPIC -Wall -O3 -I$(INC_DIR)
OUTFLAGS=-Wall -O3 -I$(INC_DIR)

CLA_SOURCE=Cell.cpp Column.cpp Cortex.cpp DendriteSegment.cpp DistalSegment.cpp ProximalSegment.cpp Region.cpp Synapse.cpp CLA.cpp Classifier.cpp
CLA_SOURCE_FILES=$(addprefix $(SRC_DIR)/,$(CLA_SOURCE))

CLA_INCLUDE=$(CLA_SOURCE:.cpp=.h)
CLA_INCLUDE_FILES=$(addprefix $(INC_DIR)/,$(CLA_INCLUDE))

CLA_OBJECTS=$(CLA_SOURCE:.cpp=.o)
CLA_OBJECT_FILES=$(addprefix $(OBJ_DIR)/,$(CLA_OBJECTS))

PY_SOURCE=cla_wrap.cxx
PY_SOURCE_FILES=$(addprefix $(SWIG_DIR)/,$(PY_SOURCE))
PY_OBJECTS=$(PY_SOURCE:.cxx=.o)
PY_OBJECT_FILES=$(addprefix $(SWIG_DIR)/,$(PY_OBJECTS))
PY_SO=clamodule.so

TEST_SOURCE=main.cpp
TEST_SOURCE_FILES=$(addprefix $(TEST_DIR)/,$(TEST_SOURCE))

TEST_EXEC=test_cla

.PHONY: all build_obj build_test build_py clean unbuild

all: build_obj build_test

build_obj: $(CLA_SOURCE_FILES) $(CLA_INCLUDE_FILES)
	$(info ==========BUILDING CLA LIBRARY==========)
	$(CC) $(CFLAGS) $(CLA_SOURCE_FILES)
	mv *.o $(OBJ_DIR)

build_test: build_obj $(TEST_SOURCE_FILES)
	$(info ==========BUILDING TEST SUITE===========)
	$(CC) $(TEST_SOURCE_FILES) $(CLA_OBJECT_FILES) $(OUTFLAGS) -o $(BIN_DIR)/$(TEST_EXEC)

build_py: build_obj
	$(info ========BUILDING PYTHON FRONTEND========)
	swig -c++ -python -lembed.i $(SWIG_DIR)/cla.i
	g++ -DHAVE_CONFIG_H $(PYFLAGS) $(PY_SOURCE_FILES)
	mv *.o $(SWIG_DIR)
	g++ -shared -fPIC $(CLA_OBJECT_FILES) $(PY_OBJECT_FILES) -o $(SWIG_DIR)/$(PY_SO)

clean:
	rm -f $(CLA_OBJECT_FILES)
	rm -f ./*.stackdump

unbuild: clean
	rm -f $(TEST_EXEC_FILE)

test: all
	$(BIN_DIR)/$(TEST_EXEC)

test_win: all
	$(BIN_DIR)/$(TEST_EXEC).exe
