DEBUG = no

ifeq ($(DEBUG),yes)
	EXTRA = -D__DEBUG__
else
	EXTRA = -O3
endif

CC = clang++
AR = llvm-ar
RANLIB = llvm-ranlib
CFLAGS = -std=c++17 $(EXTRA)
LDFLAGS = -lfolly -lhs

.PHONY: all
.SUFFIXES: $(SUFFIXES) .cpp .o

srcs = service.cpp engine.cpp matcher.cpp trigger.cpp pred.cpp var.cpp be.cpp util.cpp
objs = $(srcs:.cpp=.o)

.cpp.o:
	$(CC) $(CFLAGS) -o $@ -c $<

all: trigger bench

libtrigger.a: $(objs)
	rm -f $@
	$(AR) cr $@ $(objs)
	$(RANLIB) $@

trigger: libtrigger.a main.o
	$(CC) $(LDFLAGS) -o $@ main.o $<

bench: libtrigger.a bench.o
	$(CC) $(LDFLAGS) -o $@ bench.o $<

clean:
	@rm -f trigger libtrigger.a *.o
