main:
	gcc server.c -o server
	gcc client.c -o client
clear:
	rm server client
