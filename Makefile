PROGS=spl
IMPLS=ast.cpp
HEADERS=value.hpp st.hpp colorout.hpp $(IMPLS:.cpp=.hpp)
CXX=clang++
CPPFLAGS=-Wextra -Wno-sign-compare -Wno-deprecated-register -std=gnu++11

# Default target
all: $(PROGS) libspl.o 

# Dependencies
$(PROGS:=.yy.o): %.yy.o: %.tab.hpp
$(IMPLS:.cpp=.o) $(PROGS:=.tab.o): %.o: %.hpp
$(PROGS:=.yy.o) $(PROGS:=.tab.o): $(HEADERS)

# Rules to generate the final compiled parser programs
$(PROGS): %: %.tab.o %.yy.o $(IMPLS:.cpp=.o)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ $^ -lreadline

# Generic rule for compiling C++ programs from source
# (Actually, make also defines this by default.)
%.o: %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $<

# Generic rule for running C++-style flex code generation
# For instance, this will make 'pat.yy.cpp' from 'pat.lpp'.
%.yy.cpp: %.lpp
	flex -o $@ $<

# Generic rule for bison code generation
%.tab.cpp %.tab.hpp: %.ypp
	bison -d $<

%.output: %.ypp
	bison -v $<

libspl.o: libspl.asm
	nasm -felf libspl.asm -o libspl.o

.PHONY: clean all
clean:
	rm -f *.o *.yy.cpp *.tab.* $(PROGS) $(PROGS:=.dot) $(PROGS:=.pdf) $(PROGS:=.output)
