$(shell mkdir -p out)

CC := gcc
CFLAGS := -Iinc

PREF_SRC := ./src/
PREF_INC := ./inc/
PREF_OUT := ./out/
PREF_LIB := ./examples/lib/

.PHONY: daemon clean 

daemon: $(PREF_OUT)daemon.o $(PREF_OUT)apue.o # $(PREF_OUT)error.o
	$(CC) $(CFLAGS) $(PREF_OUT)daemon.o $(PREF_OUT)apue.o -lpthread -lc -o daemon

test: $(PREF_OUT)daemon_test.o $(PREF_OUT)apue.o 
	$(CC) $(CFLAGS) $(PREF_OUT)daemon_test.o $(PREF_OUT)apue.o -lpthread -lc -o daemon_test

$(PREF_OUT)daemon.o: $(PREF_SRC)daemon.c
	$(CC) $(CFLAGS) -c $< -o $@

$(PREF_OUT)apue.o: $(PREF_SRC)apue.c
	$(CC) $(CFLAGS) -c $< -o $@

$(PREF_OUT)daemon_test.o: $(PREF_SRC)daemon_test.c
	$(CC) $(CFLAGS) -c $< -o $@

# $(PREF_OUT)error.o: $(PREF_LIB)error.c
# 	$(CC) -c $< -o $@

include $(wildcard $(PREF_OUT)*.d)

clean : 
	rm -Rf $(PREF_OUT) daemon