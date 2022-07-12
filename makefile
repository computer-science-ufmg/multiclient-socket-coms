CC=gcc
CFLAGS=-Wno-format-overflow

SERVER_TARGET_NAME=./server
EQUIPEMENT_TARGET_NAME=./equipement
BUILD_PATH=./build

all: $(SERVER_TARGET_NAME) $(EQUIPEMENT_TARGET_NAME)

$(BUILD_PATH)/%.o: %.c %.h
	dirname $@ | xargs mkdir -p
	$(CC) -c $(CFLAGS) $< -o $@

$(SERVER_TARGET_NAME): ./server.c $(BUILD_PATH)/common.o
	$(CC) $(CFLAGS) ./server.c $(BUILD_PATH)/common.o -o $(SERVER_TARGET_NAME)

$(EQUIPEMENT_TARGET_NAME): ./equipement.c $(BUILD_PATH)/common.o
	$(CC) $(CFLAGS) ./equipement.c $(BUILD_PATH)/common.o -o $(EQUIPEMENT_TARGET_NAME)

clean:
	rm -rf $(BUILD_PATH)/*
	rm $(SERVER_TARGET_NAME)
	rm $(EQUIPEMENT_TARGET_NAME)
