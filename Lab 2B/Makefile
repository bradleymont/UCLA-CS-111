# NAME: Bradley Mont
# EMAIL: bradleymont@gmail.com
# ID: 804993030

CC = gcc
CFLAGS = -Wall -Wextra -g -pthread -lprofiler

build:
	$(CC) $(CFLAGS) lab2_list.c SortedList.c -o lab2_list
tests: build
	chmod +x lab2_test.sh
	-./lab2_test.sh
profile: build
	LD_PRELOAD=/usr/lib64/libprofiler.so \
	CPUPROFILE=./raw.gperf \
	./lab2_list --iterations=1000 --threads=12 --sync=s
	pprof --text ./lab2_list ./raw.gperf > profile.out
	pprof --list=sync ./lab2_list ./raw.gperf >> profile.out
	rm -f ./raw.gperf
graphs: tests
	gnuplot lab2_list.gp
clean:
	rm -rf lab2_list *.tar.gz *.o
dist: build graphs
	tar -czf lab2b-804993030.tar.gz SortedList.h SortedList.c lab2_list.c Makefile lab2b_list.csv profile.out *.png lab2_list.gp lab2_test.sh README
