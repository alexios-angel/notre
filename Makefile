.PHONY: default all clean grammar compare single-header single-header/notre.hpp single-header/notre-unicode.hpp single-header/unicode-db.hpp pch

default: all
	
TARGETS := $(wildcard tests/benchmark-exec/*.cpp)
IGNORE := $(wildcard tests/benchmark/*.cpp) $(wildcard tests/benchmark-exec/*.cpp)

# LL1q parser generator: https://github.com/alexios-angel/Tablewright
# (open-source replacement for the original closed-source desatomat tool;
# needs python3 with the lark package)
TABLEWRIGHT := tablewright

CXX_STANDARD := 20

PYTHON := python3.9

PEDANTIC:=-pedantic

override CXXFLAGS := $(CXXFLAGS) -std=c++$(CXX_STANDARD) -Iinclude -O3 $(PEDANTIC) -Wall -Wextra -Werror -Wconversion

# precompiled header: notre.hpp is parsed once instead of once per test
CXX_IS_CLANG := $(shell $(CXX) --version 2>/dev/null | grep -qi clang && echo yes)
ifeq ($(CXX_IS_CLANG),yes)
PCH := notre.pch
PCH_USE = -include-pch $(PCH)
else
PCH := include/notre.hpp.gch
PCH_USE =
endif
LDFLAGS := 

TESTS := $(wildcard tests/*.cpp) $(wildcard tests/benchmark/*.cpp)
TRUE_TARGETS := $(TARGETS:%.cpp=%)
override TRUE_TARGETS := $(filter-out $(IGNORE:%.cpp=%), $(TRUE_TARGETS))
OBJECTS := $(TARGETS:%.cpp=%.o) $(TESTS:%.cpp=%.o)
override OBJECTS := $(filter-out $(IGNORE:%.cpp=%.o),$(OBJECTS))
DEPEDENCY_FILES := $(OBJECTS:%.o=%.d)

all: $(TRUE_TARGETS) $(OBJECTS)
	
list:
	echo $(SUPPORTED_CPP20)
	
$(TRUE_TARGETS): %: %.o
	$(CXX)  $< $(LDFLAGS) -o $@ 
	
$(OBJECTS): %.o: %.cpp $(PCH)
	$(CXX) $(CXXFLAGS) $(PCH_USE) -MMD -c $< -o $@

pch: $(PCH)

$(PCH): include/notre.hpp
	$(CXX) $(CXXFLAGS) -x c++-header $< -o $@

-include $(DEPEDENCY_FILES)

benchmark:
	@$(MAKE) clean
	@$(MAKE) IGNORE=""
	
benchmark-clean:
	@$(MAKE) IGNORE="" clean

clean:
	rm -f $(TRUE_TARGETS) $(OBJECTS) $(DEPEDENCY_FILES) mtent12.txt mtent12.zip notre.pch include/notre.hpp.gch
	
grammar: include/notre/pcre.hpp
	
regrammar: 
	@rm -f include/notre/pcre.hpp
	@$(MAKE) grammar
	
include/notre/pcre.hpp: include/notre/pcre.gram
	@echo "LL1q $<"
	@$(TABLEWRIGHT) --ll --q --input=include/notre/pcre.gram --output=include/notre/ --generator=cpp_ctll_v2  --cfg:fname=pcre.hpp --cfg:namespace=notre --cfg:guard=NOTRE__PCRE__HPP --cfg:grammar_name=pcre
	
mtent12.zip:
	curl -s http://www.gutenberg.org/files/3200/old/mtent12.zip -o mtent12.zip
	
mtent12.txt: mtent12.zip
	unzip -o mtent12.zip
	touch mtent12.txt

single-header: single-header/notre.hpp single-header/notre-unicode.hpp single-header/unicode-db.hpp

single-header/unicode-db.hpp: include/unicode-db/unicode-db.hpp
	cp $+ $@

single-header/notre.hpp:
	${PYTHON} -m quom include/notre.hpp notre.hpp.tmp
	echo "/*" > single-header/notre.hpp
	cat LICENSE >> single-header/notre.hpp
	echo "*/" >> single-header/notre.hpp
	cat notre.hpp.tmp >> single-header/notre.hpp
	rm notre.hpp.tmp

single-header/notre-unicode.hpp:
	${PYTHON} -m quom include/notre-unicode.hpp notre-unicode.hpp.tmp
	echo "/*" > single-header/notre-unicode.hpp
	cat LICENSE >> single-header/notre-unicode.hpp
	echo "*/" >> single-header/notre-unicode.hpp
	cat notre-unicode.hpp.tmp >> single-header/notre-unicode.hpp
	rm notre-unicode.hpp.tmp
	
REPEAT:=10

compare: mtent12.txt
	$(CXX) $(CXXFLAGS) -MMD -march=native -DPATTERN="\"(${PATTERN})\"" -c tests/benchmark-range/measurement.cpp -o tests/benchmark-range/measurement.o
	$(CXX) tests/benchmark-range/measurement.o -lboost_regex -lpcre2-8 -lre2 -o tests/benchmark-range/measurement
	tests/benchmark-range/measurement all mtent12.txt ${REPEAT}
