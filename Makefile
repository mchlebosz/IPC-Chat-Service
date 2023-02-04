
#Folders
BIN = bin
SRC = src



# Specify the compiler and any flags
CC = gcc 
CFLAGS = -g -pedantic -Wall -Werror -O2 -std=gnu11 

#Programs
PROG = client server
# Specify the target executables
TARGETS = $(addprefix $(BIN)/, $(PROG))

# Use the wildcard function to find all .c and .h files in the src/client and src/server directories
# Specify the object files for each target

$(foreach prog, $(PROG), \
$(eval $(prog)_SRC = $(wildcard $(SRC)/$(prog)/*.c) $(wildcard $(SRC)/*.c)) \
$(eval $(prog)_OBJS = $(patsubst %.c,%.o,$($(prog)_SRC))) \
$(eval $(prog)_DEPS = $(patsubst %.c,%.d,$($(prog)_SRC))) \
$(info $($(prog)_SRC)))

-include $(foreach prog, $(PROG), $($(prog)_DEPS))

$(foreach prog, $(PROG), \
$(eval $(prog)_HEADERS = $(wildcard $(SRC)/$(prog)/*.h) $(wildcard $(SRC)/*.h)))




#build all
.PHONY: all
all: $(BIN) $(TARGETS) $(foreach prog, $(PROG), $($(prog)_OBJS))


#create binaries directories if not exists
$(BIN):
	mkdir $(BIN)

#define building
define make-target
$(BIN)/$(1): $$($1_OBJS) $$($1_HEADERS)
	$$(CC) $$(CFLAGS) $$($1_OBJS) -o $$@
endef

#Create compiling rules
$(foreach prog,$(PROG), $(eval $(call make-target,$(prog))))

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Create a rule to clean up the built executables and object files
.PHONY: clean
clean:
ifeq ($(OS),Windows_NT)
	del /F /Q $(BIN)\*
	del /F /s *.o *.d *.elf *.map *.log
else
	rm -f $(foreach prog, $(PROG), $($(prog)_OBJS), $($(prog)_DEPS))
	rm -f -r $(BIN) 
	rm -f **/**.o **/**.d **/**.elf **/**.map **/**.log

endif
