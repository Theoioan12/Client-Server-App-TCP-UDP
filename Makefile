# Protocoale de comunicatii
# Laborator 7 - TCP
# Makefile

CFLAGS = -Wall -g -lm

# Portul pe care asculta serverul
PORT = 12363

# Adresa IP a serverului
IP_SERVER = 127.0.0.1

ID = C1

all: server subscriber

# Compileaza server.c
server: server.c

# Compileaza subscriber.c
subscriber: subscriber.c

.PHONY: clean run_server run_subscriber

# Ruleaza serverul
run_server:
	./server ${PORT}

# Ruleaza clientul
run_subscriber:
	./subscriber $(ID) ${IP_SERVER} ${PORT}

clean:
	rm -f server subscriber