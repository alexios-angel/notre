CXX ?= c++
CXXFLAGS := -std=c++20 -I../include -O2 -Wall -Wextra -pedantic
CXX17FLAGS := -std=c++17 -I../include -O2 -Wall -Wextra

SOURCES := $(wildcard *.cpp)
TARGETS := $(SOURCES:%.cpp=%)

all: $(TARGETS)

# built in C++17 mode on purpose; no -pedantic so the _ctre UDL
# (a GNU extension) stays available where the compiler offers it
cxx17-syntax: cxx17-syntax.cpp
	$(CXX) $(CXX17FLAGS) $< -o $@

%: %.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

run: all
	@for t in $(TARGETS); do printf '== %s ==\n' "$$t"; ./$$t; echo; done

clean:
	rm -f $(TARGETS)

.PHONY: all run clean
