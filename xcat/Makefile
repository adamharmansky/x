all:
	cc xcat.c ../draw.c -g -lX11 -lXft -lpthread -o xcat
run: all
	./xcat < lorem.ipsum
debug: all
	st gdb -tui ./xcat
clean:
	rm -f /bin/xcat
install: all clean
	cp xcat  /bin/xcat
	chmod +x /bin/xcat
