CXX := g++
CXXFLAGS := -Wall -Wextra -std=c++14 -I.

TARGET := main
TARGET_SOURCES := \
	main.cpp \
	utils/Submission.cpp \
	utils/Reviewer.cpp \
	utils/FileParser.cpp \
	utils/EdmondKarp.cpp \
	data_structures/createGraphs.cpp
TARGET_OBJECTS := $(TARGET_SOURCES:.cpp=.o)

TEST_TARGET := maxflow_tests
TEST_SOURCES := \
	tests/maxflow_tests.cpp \
	utils/Submission.cpp \
	utils/Reviewer.cpp \
	utils/EdmondKarp.cpp \
	data_structures/createGraphs.cpp
TEST_OBJECTS := $(TEST_SOURCES:.cpp=.o)

.PHONY: all test run clean

all: $(TARGET)

$(TARGET): $(TARGET_OBJECTS)
	$(CXX) $(CXXFLAGS) $(TARGET_OBJECTS) -o $(TARGET)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

$(TEST_TARGET): $(TEST_OBJECTS)
	$(CXX) $(CXXFLAGS) $(TEST_OBJECTS) -o $(TEST_TARGET)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: test $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET_OBJECTS) $(TEST_OBJECTS) $(TARGET) $(TEST_TARGET)
