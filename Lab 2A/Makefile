# NAME: Bradley Mont
# EMAIL: bradleymont@gmail.com
# ID: 804993030

CC = gcc
CFLAGS = -Wall -Wextra -g -pthread

build:
	$(CC) $(CFLAGS) lab2_add.c -o lab2_add
	$(CC) $(CFLAGS) lab2_list.c SortedList.c -o lab2_list
tests: build
	chmod +x lab2_test.sh
	-./lab2_test.sh
graphs: tests
	gnuplot lab2_add.gp
	gnuplot lab2_list.gp
clean:
	rm -rf lab2_add lab2_list *.tar.gz *.o
dist: build graphs
	tar -czf lab2a-804993030.tar.gz lab2_add.c SortedList.h SortedList.c lab2_list.c Makefile *.csv *.png *.gp lab2_test.sh README
