CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra

sched: main.cpp simulator.cpp simulator.hpp scheduler.hpp
	$(CXX) $(CXXFLAGS) -o $@ main.cpp simulator.cpp

clean:
	rm -f sched sched.exe
