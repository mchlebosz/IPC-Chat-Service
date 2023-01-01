# Specify the compiler and any flags
CC = gcc
CFLAGS = -g -pedantic -Wall -Werror -O2 -std=gnu11

#Folders
BIN = bin
SRC = src

#Programs
PROG = client server

# Specify the target executables
TARGETS = $(addprefix $(BIN)/, $(PROG))

# Use the wildcard function to find all .c and .h files in the src/client and src/server directories
CLIENT_SRC = $(wildcard $(SRC)/client/*.c)
SERVER_SRC = $(wildcard $(SRC)/server/*.c)
CLIENT_HEADERS = $(wildcard $(SRC)/client/*.h)
SERVER_HEADERS = $(wildcard $(SRC)/server/*.h)


# Specify the object files for each target
CLIENT_OBJS = $(patsubst %.c,%.o,$(CLIENT_SRC))
SERVER_OBJS = $(patsubst %.c,%.o,$(SERVER_SRC))

#build all
all: $(BIN) $(TARGETS)

#create binaries directories if not exists
$(BIN):
	mkdir $(BIN)

# Create a rule to build the client executable
$(BIN)/client: $(CLIENT_OBJS) $(CLIENT_HEADERS)
	$(CC) $(CFLAGS) $^ -o $@

# Create a rule to build the server executable
$(BIN)/server: $(SERVER_OBJS) $(SERVER_HEADERS)
	$(CC) $(CFLAGS) $^ -o $@



# Create a rule to clean up the built executables and object files
.PHONY: clean
clean:
	rm -f $(TARGETS) $(CLIENT_OBJS) $(SERVER_OBJS)
