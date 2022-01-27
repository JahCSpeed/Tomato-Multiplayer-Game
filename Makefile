OUTPUT = client server
CFLAGS = -g -Wall -Wvla -I inc -D_REENTRANT
CSFLAGS = -g -Wall -Wvla 
LFLAGS = -L lib -lSDL2 -lSDL2_image -lSDL2_ttf -lpthread

%.o: %.c %.h
	gcc $(CFLAGS) -c -o $@ $<

%.o: %.c
	gcc $(CFLAGS) -c -o $@ $<

all: $(OUTPUT)

runclient: $(OUTPUT)
	LD_LIBRARY_PATH=lib ./client 128.6.4.28 2068
client: client.o
	gcc $(CFLAGS) -o $@ $^ $(LFLAGS)
server: server.o
	gcc $(CSFLAGS) -o $@ $^ $(LFLAGS)

clean:
	rm -f $(OUTPUT) *.o
