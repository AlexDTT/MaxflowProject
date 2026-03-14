CXX := g++
CXXFLAGS := -Wall -Wextra -std=c++14 -I.

TARGET := main
SOURCES := \
	main.cpp \
	utils/Submission.cpp \
	utils/Reviewer.cpp \
	utils/FileParser.cpp \
	utils/EdmondKarp.cpp \
	data_structures/createGraphs.cpp
OBJECTS := $(SOURCES:.cpp=.o)

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(TARGET)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJECTS) $(TARGET)
