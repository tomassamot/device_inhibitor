BIN:=device_inhibitor
SRCS:=$(wildcard *.c)
OBJS:=$(SRCS:.c=.o)
CPPFLAGS:= -I/home/studentas/Documents/tuya-iot-core-sdk-main/utils -I/home/studentas/Documents/tuya-iot-core-sdk-main/interface -I/home/studentas/Documents/tuya-iot-core-sdk-main/include
LDFLAGS:= -L/home/studentas/Documents/tuya-iot-core-sdk-main/build/lib -llink_core -lmiddleware_implementation -lplatform_port -lutils_modules

.PHONY: all clean debug

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(CPPFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	$(RM) $(BIN) $(OBJS)