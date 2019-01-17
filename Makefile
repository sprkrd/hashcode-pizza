TARGETS = preprocess solver

CPPFLAGS=-Wall -Wextra -pedantic --std=c++11 -O3

all: $(TARGETS)

$(TARGETS): %: %.cpp
	g++ $(CPPFLAGS) $< -o $@ -lboost_serialization
	
clean:
	rm -f $(TARGETS)

