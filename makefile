CC=gcc
CFLAGS=-Wno-format-overflow

SERVER_TARGET_NAME=./server
EQUIPMENT_TARGET_NAME=./equipment
BUILD_PATH=./build

all: $(SERVER_TARGET_NAME) $(EQUIPMENT_TARGET_NAME)

$(BUILD_PATH)/%.o: %.c %.h
	dirname $@ | xargs mkdir -p
	$(CC) -c $(CFLAGS) $< -o $@

$(SERVER_TARGET_NAME): ./server.c $(BUILD_PATH)/common.o
	$(CC) $(CFLAGS) ./server.c $(BUILD_PATH)/common.o -o $(SERVER_TARGET_NAME)

$(EQUIPMENT_TARGET_NAME): ./equipment.c $(BUILD_PATH)/common.o
	$(CC) $(CFLAGS) ./equipment.c $(BUILD_PATH)/common.o -o $(EQUIPMENT_TARGET_NAME)

clean:
	rm -rf $(BUILD_PATH)/*
	rm $(SERVER_TARGET_NAME)
	rm $(EQUIPMENT_TARGET_NAME)
