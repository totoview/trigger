CC = clang++
CFLAGS = -std=c++17
LDFLAGS = -lfolly -lhs

.PHONY: all
.SUFFIXES: $(SUFFIXES) .cpp .o

srcs = main.cpp engine.cpp matcher.cpp trigger.cpp pred.cpp var.cpp be.cpp util.cpp
objs = $(srcs:.cpp=.o)

.cpp.o:
	$(CC) $(CFLAGS) -o $@ -c $<

all: trigger

trigger: $(objs)
	$(CC) $(LDFLAGS) -o $@ $(objs)

clean:
	@rm -f trigger *.o
