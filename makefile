# Choose a C++ compiler. Defaults to g++ if not set externally.
CXX ?= g++

# C++ standard and common flags.
CXXFLAGS ?= -std=c++17 -O2 -Wall

# Include directories for Eigen and Spectra.
# Users can override these by setting EIGEN_INC and SPECTRA_INC in their environment.
EIGEN_INC ?= /usr/include/eigen3
SPECTRA_INC ?= ./spectra/include

INCLUDES = -I$(EIGEN_INC) -I$(SPECTRA_INC)

# List your source files.
SRCS = main.cpp functions.cpp
OBJS = $(SRCS:.cpp=.o)

# Target executable name.
TARGET = main

all: $(TARGET)


%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(TARGET) $(OBJS)

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
