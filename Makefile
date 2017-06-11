COMMONINCPATH=inc/
TESTINCPATH=tests/inc

#Compile for tyvak on actual build.
CC=gcc

#Optimize when done debugging.
CFLAGS=-g -Wall -std=c99 -pedantic	-I	$(COMMONINCPATH)	-I	$(TESTINCPATH)	-c

LDFLAGS=

#Common files
SOURCES=src/rcp_packet.c src/rcp_queue.c
INCLUDES=inc/rcp_packet.h inc/rcp_config.h inc/rcp_queue.h
OBJECTS=$(SOURCES:.c=.o)

#Tests for packet methods
PACKETTESTSRC=tests/src/rcp_packet_tests.c
PACKETTESTOBJ=$(PACKETTESTSRC:.c=.o)
PACKETTESTINC=tests/inc/rcp_packet_tests.h
PACKETTESTEXECUTABLE=bin/rcp_packet_tests

all:	rcp_packet_tests

run-tests: rcp_packet_tests
		./bin/rcp_packet_tests

#Run valgrind on the tests to check for memory leaks
run-tests-val: rcp_packet_tests
		valgrind ./bin/rcp_packet_tests

rcp_packet_tests: $(INCLUDES) $(SOURCES)	$(PACKETTESTINC)	$(PACKETTESTSRC)	$(PACKETTESTEXECUTABLE)
		@echo "Built at " $(PACKETTESTEXECUTABLE)

$(PACKETTESTEXECUTABLE):	$(OBJECTS)	$(PACKETTESTOBJ)
		$(CC)	$(LDFLAGS)	$(OBJECTS)	$(PACKETTESTOBJ)	-o	$@

clean:
		rm -rf $(OBJECTS) $(PACKETTESTOBJ) $(PACKETTESTEXECUTABLE)
		@echo "Project clean"

.c.o:
		$(CC)	$(CFLAGS)	$<	-o	$@
