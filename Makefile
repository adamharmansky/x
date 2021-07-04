all:
	cc test.c toolkit.c draw.c -lX11 -lXft -lpthread -g -o test
run: all
	./test
debug: all
	gdb -tui ./test
