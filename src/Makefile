BIN:=device_inhibitor
SRCS:=$(wildcard *.c)
CPPFLAGs:= -Wall

.PHONY: all clean

all: $(BIN)

$(BIN): $(SRCS)
	$(CC) $(CPPFLAGS) -o $@ $^ -largp -lubus -lubox -lblobmsg_json -llink_core -lmiddleware_implementation -lplatform_port -lutils_modules

clean:
	$(RM) $(BIN) $(OBJS)