SHELL:=/bin/bash

COMMONINCPATH=inc/
TESTINCPATH=tests/inc

#Compile for tyvak on actual build.
CC=gcc

#Optimize when done debugging.
CFLAGS= -g -Wall -std=c99 -D_BSD_SOURCE  -pedantic	-I	$(COMMONINCPATH)	-I	$(TESTINCPATH)	-c

LDFLAGS=

#Common files
SOURCES= src/rcp_packet.c src/rcp_queue.c src/rcp.c
INCLUDES= inc/rcp_packet.h inc/rcp_config.h inc/rcp_queue.h inc/rcp.h
OBJECTS=$(SOURCES:.c=.o)

#Tests for packet methods
PACKETTESTSRC= tests/src/rcp_packet_tests.c
PACKETTESTOBJ=$(PACKETTESTSRC:.c=.o)
PACKETTESTINC= tests/inc/rcp_packet_tests.h
PACKETTESTEXECUTABLE= bin/rcp_packet_tests

#Tests for RCP
RCPTESTSRC= tests/src/rcp_tests.c
RCPTESTOBJ=$(RCPTESTSRC:.c=.o)
RCPTESTINC= tests/inc/rcp_tests.h
RCPTESTEXECUTABLE= bin/rcp_tests

all:	binfolder clean rcp_tests rcp_packet_tests

binfolder:
	mkdir -p bin

run-tests: rcp_packet_tests rcp_tests
		@echo "\n"
		@echo "Running packet tests..."
		./bin/rcp_packet_tests
		@echo "\n\n"
		@echo "Running RCP tests..."
		./bin/rcp_tests

#Run valgrind on the tests to check for memory leaks
run-tests-val: rcp_packet_tests
		valgrind -v ./bin/rcp_packet_tests
		@echo "Press any key to continue to next test"
		@read -n 1
		valgrind -v ./bin/rcp_tests

rcp_packet_tests: $(INCLUDES) $(SOURCES)	$(PACKETTESTINC)	$(PACKETTESTSRC)	$(PACKETTESTEXECUTABLE)
		@echo "Built at " $(PACKETTESTEXECUTABLE)

rcp_tests: $(INCLUDES) $(SOURCES)	$(RCPTESTINC)	$(RCPTESTSRC)	$(RCPTESTEXECUTABLE)
		@echo "Built at " $(RCPTESTEXECUTABLE)

$(PACKETTESTEXECUTABLE):	$(OBJECTS)	$(PACKETTESTOBJ)
		$(CC)	$(LDFLAGS)	$(OBJECTS)	$(PACKETTESTOBJ)	-o	$@ -pthread

$(RCPTESTEXECUTABLE):	$(OBJECTS)	$(RCPTESTOBJ)
		$(CC)	$(LDFLAGS)	$(OBJECTS)	$(RCPTESTOBJ)	-o	$@ -pthread

clean:
		rm -rf $(OBJECTS) $(PACKETTESTOBJ) $(PACKETTESTEXECUTABLE) $(RCPTESTOBJ) $(RCPTESTEXECUTABLE)
		@echo "Project clean"

.c.o:
		$(CC)	$(CFLAGS)	$<	-o	$@
