all: userver.c uclient.c
	gcc userver.c -o userver
	gcc uclient.c -o uclient

clean: 
	rm userver uclient
