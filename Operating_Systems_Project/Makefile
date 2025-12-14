CC = gcc
CFLAGS = -Wall -Wextra -g -I. -Isrc -IFreeRTOS/include -IFreeRTOS/portable/ThirdParty/GCC/Posix -DPROJ_COVERAGE=0
LDFLAGS = -lpthread

FREERTOS_DIR = FreeRTOS
FREERTOS_SRC = $(FREERTOS_DIR)/tasks.c \
               $(FREERTOS_DIR)/list.c \
               $(FREERTOS_DIR)/queue.c \
               $(FREERTOS_DIR)/timers.c \
               $(FREERTOS_DIR)/portable/ThirdParty/GCC/Posix/port.c \
               $(FREERTOS_DIR)/portable/ThirdParty/GCC/Posix/utils/wait_for_event.c \
               $(FREERTOS_DIR)/portable/MemMang/heap_3.c

APP_SRC = src/main.c src/scheduler.c src/tasks.c
OBJ = $(FREERTOS_SRC:.c=.o) $(APP_SRC:.c=.o)
TARGET = freertos_sim

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

clean:
	rm -f $(OBJ) $(TARGET) src/*.o FreeRTOS/*.o
