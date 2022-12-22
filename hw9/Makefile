all: hashtable undef user
#DBG=-DVERBOSE
CC=g++
# CC=clang
# CFLAGS=-std=gnu99 -gdwarf-3 -Wall -O1 -fdetach
CFLAGS= -Wall -O1 
# LDFLAGS += -ldl -static
LDFLAGS += -ldl -pthread -lpthread
REPLAY=1
# non-cilk, single-threaded only record replay support with gdb 7.5
ifeq ($(REPLAY),1)
hashtable: CFLAGS += -fcilkplus -DCILK
hashtable: LDFLAGS +=-lcilkrts
hashlock-unit: CFLAGS += -fcilkplus -DCILK
hashlock-unit: LDFLAGS +=-lcilkrts
endif
hashtable: LDFLAGS += -lpthread
hashlock-unit: LDFLAGS += -lpthread

hashtable1: ./hashtable
	./hashtable
hashtable10: ./hashtable
	bash -c "for((i=0;i<10;i++)) do ./hashtable; done"
hashtable1000: ./hashtable
	bash -c "for((i=0;i<1000;i++)) do ./hashtable; done"

# XXX modify this target so that 1000 runs all succeed
hashtable1000-good: ./hashtable
	bash -c "for((i=0;i<1000;i++)) ./hashtable; done"
# bash -c "for((i=0;i<1;i++)) do setarch x86_64 -R ./hashtable 1 0; done" // always succeed
# XXX modify this target so the program always fails
hashtable1-bad: ./hashtable
# ./hashtable
	bash -c "for((i=0;i<1;i++)) do setarch x86_64 -R ./hashtable 1 3; done" // always fail

hashtable-mt: ./hashtable
	./hashtable 10
hashtable-mt1000: ./hashtable
	bash -c "for((i=0;i<1000;i++)) do ./hashtable 10; done"

whoami: ./user
# @env USER=professoramarasinghe./user
	@./user

clean: 
	@-rm -f ./undef ./hashtable ./hashlock-unit ./user

undef-run: ./undef
	./undef
	./undef

undef-compare: 
	make CFLAGS=-O3 clean undef; ./undef
	make CFLAGS=-O1 clean undef; ./undef

undef-noaslr: ./undef
	setarch x86_64 -R ./undef
	setarch x86_64 -R ./undef

undef-env: ./undef
	setarch x86_64 -R env USER=me ./undef
	setarch x86_64 -R env USER=professoramarasinghe ./undef

undef-all: undef-run undef-compare undef-env undef-noaslr 

hashtable: hashtable.cpp hashlock.cpp common.h
	$(CC) -o $@ -pthread $(DBG) $(CFLAGS) hashtable.cpp hashlock.cpp $(LDFLAGS)

%: %.cpp
	$(CC) -o $@ -pthread $(DBG) $(CFLAGS) $^ $(LDFLAGS)

hashlock-unit: hashlock.cpp Makefile
	$(CC) -o $@ -pthread -Wall -g -fdetach -DUNIT_TEST $< $(LDFLAGS)

testall: hashtable10 hashlock-unit undef-all
