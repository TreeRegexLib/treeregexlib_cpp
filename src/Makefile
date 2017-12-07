CXX=g++
#CXXFLAGS=-O3 --std=c++11 -g
CXXFLAGS=--std=c++11 -g

all: tsearch treplace tstrip

tsearch: tsearch.cpp *.h
	$(CXX) $(CXXFLAGS) -o tsearch tsearch.cpp

treplace: treplace.cpp *.h
	$(CXX) $(CXXFLAGS) -o treplace treplace.cpp

tstrip: tstrip.cpp *.h
	$(CXX) $(CXXFLAGS) -o tstrip tstrip.cpp

test: *.h *.cpp
	$(CXX) $(CXXFLAGS) -o test.out tree_test.cpp
	./test.out
	$(CXX) $(CXXFLAGS) -o test.out treeregex_impl_test.cpp
	./test.out
	$(CXX) $(CXXFLAGS) -o test.out treeregex_to_nfa_test.cpp
	./test.out
	$(CXX) $(CXXFLAGS) -o test.out treeregex_matching_test.cpp
	./test.out
	$(CXX) $(CXXFLAGS) -o test.out transformer_test.cpp
	./test.out

clean:
	rm treplace tsearch test.out
