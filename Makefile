CXX := g++
CXXFLAGS := -Wall -Wextra -std=c++17 -I.

TARGET := main
TARGET_SOURCES := \
	main.cpp \
	utils/Submission.cpp \
	utils/Reviewer.cpp \
	utils/FileParser.cpp \
	utils/EdmondKarp.cpp utils/AssignmentLogic.cpp \
	data_structures/createGraphs.cpp
TARGET_OBJECTS := $(TARGET_SOURCES:.cpp=.o)

UNIT_TEST_TARGET := maxflow_tests
UNIT_TEST_SOURCES := \
	tests/maxflow_tests.cpp \
	utils/Submission.cpp \
	utils/Reviewer.cpp \
	utils/EdmondKarp.cpp utils/AssignmentLogic.cpp \
	data_structures/createGraphs.cpp
UNIT_TEST_OBJECTS := $(UNIT_TEST_SOURCES:.cpp=.o)

.PHONY: all unit-test integration-test test run clean

all: $(TARGET)

$(TARGET): $(TARGET_OBJECTS)
	$(CXX) $(CXXFLAGS) $(TARGET_OBJECTS) -o $(TARGET)

unit-test: $(UNIT_TEST_TARGET)
	./$(UNIT_TEST_TARGET)

$(UNIT_TEST_TARGET): $(UNIT_TEST_OBJECTS)
	$(CXX) $(CXXFLAGS) $(UNIT_TEST_OBJECTS) -o $(UNIT_TEST_TARGET)

integration-test: $(TARGET)
	bash tests/run_integration_tests.sh

test: unit-test integration-test

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET_OBJECTS) $(UNIT_TEST_OBJECTS) $(TARGET) $(UNIT_TEST_TARGET)
	rm -f output_dataset*.csv assign.csv
