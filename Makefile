TARGETS = pizza

CPPFLAGS=-Wall -Wextra -pedantic --std=c++11 -O3 -pthread

all: $(TARGETS)

$(TARGETS): %: %.cpp
	g++ $(CPPFLAGS) $< -o $@
	
clean:
	rm -f $(TARGETS)

