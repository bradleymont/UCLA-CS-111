# NAME: Bradley Mont
# EMAIL: bradleymont@gmail.com
# ID: 804993030

CC = gcc
CFLAGS = -Wall -Wextra -g -lm -lmraa

default:
	$(CC) $(CFLAGS) lab4c_tcp.c -o lab4c_tcp
	$(CC) $(CFLAGS) -lssl -lcrypto lab4c_tls.c -o lab4c_tls	
clean:
	rm -f lab4c_tcp lab4c_tls *.tar.gz *.o
dist:
	tar -czf lab4c-804993030.tar.gz lab4c_tcp.c lab4c_tls.c Makefile README
