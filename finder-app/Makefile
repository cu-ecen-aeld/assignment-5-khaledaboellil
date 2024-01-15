# If we put @ before the command, it will only print the output, not the command itself

CROSS_COMPILE ?=

CFLAGS = -Wall

writer: writer.c
	$(CROSS_COMPILE)gcc $(CFLAGS) -o writer writer.c



clean:
	rm -f writer 
