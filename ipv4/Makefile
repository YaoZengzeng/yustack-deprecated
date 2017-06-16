CURSRCS = $(wildcard *.c)
CUROBJS = $(patsubst %.c, $(OBJ)/%.o, $(CURSRCS))

all:	$(CUROBJS)
	@echo ipv4 build finished

$(OBJ)/%.o: %.c $(LIB)
	@echo "[cc] $@"
	$(CC) $(CFLAGS) -c -o $@ $<
