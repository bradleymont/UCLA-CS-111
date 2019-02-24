# NAME: Bradley Mont
# EMAIL: bradleymont@gmail.com
# ID: 804993030

CC = gcc
CFLAGS = -Wall -Wextra -g

default:
	$(CC) $(CFLAGS) lab3a.c -o lab3a
dist:
	tar -czf lab3a-804993030.tar.gz lab3a.c ext2_fs.h Makefile README
clean:
	rm -f lab3a *.tar.gz *.o
image:
	rm -f trivial.img
	wget http://web.cs.ucla.edu/classes/winter19/cs111/labs/P3A_fs/ASSIGNMENT/trivial.img
