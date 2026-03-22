CXX := g++
CXXFLAGS := -Wall -Wextra -std=c++17 -I. -MMD -MP

TARGET := main
TARGET_SOURCES := \
	main.cpp \
	ui/ConferenceApp.cpp \
	models/Submission.cpp \
	models/Reviewer.cpp \
	io/FileParser.cpp \
	algorithms/EdmondKarp.cpp services/AssignmentLogic.cpp \
	data_structures/GraphBuilder.cpp
TARGET_OBJECTS := $(TARGET_SOURCES:.cpp=.o)

UNIT_TEST_TARGET := maxflow_tests
UNIT_TEST_SOURCES := \
	tests/maxflow_tests.cpp \
	models/Submission.cpp \
	models/Reviewer.cpp \
	algorithms/EdmondKarp.cpp services/AssignmentLogic.cpp \
	data_structures/GraphBuilder.cpp
UNIT_TEST_OBJECTS := $(UNIT_TEST_SOURCES:.cpp=.o)

ALL_DEPS := $(TARGET_OBJECTS:.o=.d) $(UNIT_TEST_OBJECTS:.o=.d)

.PHONY: all unit-test integration-test test run clean docs

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

docs:
	doxygen Doxyfile

clean:
	rm -f $(TARGET_OBJECTS) $(UNIT_TEST_OBJECTS) $(TARGET) $(UNIT_TEST_TARGET)
	rm -f $(ALL_DEPS)
	rm -f output_dataset*.csv assign.csv
	rm -rf docs

-include $(ALL_DEPS)
