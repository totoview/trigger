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

all: bench_trigger bench_service

libtrigger.a: $(objs)
	rm -f $@
	$(AR) cr $@ $(objs)
	$(RANLIB) $@

bench_trigger: libtrigger.a bench_trigger.o
	$(CC) $(LDFLAGS) -o $@ bench_trigger.o $<

bench_service: libtrigger.a bench_service.o
	$(CC) $(LDFLAGS) -o $@ bench_service.o $<

clean:
	@rm -f bench_trigger bench_service libtrigger.a *.o
