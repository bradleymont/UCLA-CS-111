# NAME: Bradley Mont
# EMAIL: bradleymont@gmail.com
# ID: 804993030

CC = gcc
CFLAGS = -Wall -Wextra -g

default:
	$(CC) $(CFLAGS) lab1b.c -o simpsh
clean:
	rm -f simpsh *.tar.gz *.o *.txt
check: checkFileDNE checkInvalidFD checkSuccesfulCommand checkCreat checkPipe checkAbort checkWait
dist:
	tar -czf lab1-804993030.tar.gz lab1b.c Makefile README
checkFileDNE:
	touch a.txt b.txt; \
	./simpsh --rdonly DNE.txt --wronly a.txt --wronly b.txt --command 0 1 2 cat; \
	if [ $$? -eq 1 ]; then \
		echo "TEST #1 PASSED: program reported an error for a file that does not exist"; \
	else \
		echo "TEST #1 FAILED: program did not report an error for a file that does not exist"; \
	fi
checkInvalidFD:
	touch c.txt; \
	./simpsh --rdonly a.txt --wronly b.txt --wronly c.txt --command -1 3 6 ls; \
	if [ $$? -eq 1 ]; then \
		echo "TEST #2 PASSED: program reported an error for invalid file descriptors passed to the --command flag"; \
	else \
		echo "TEST #2 FAILED: program did not report an error for invalid file descriptors passed to the --command flag"; \
	fi
checkSuccesfulCommand:
	echo "this is test input for Lab 1A" > a.txt; \
	./simpsh --rdonly a.txt --wronly b.txt --wronly c.txt --command 0 1 2 cat; \
	if [ $$? -eq 0 ]; then \
		echo "TEST #3 PASSED: successfully executed --command with perfect input"; \
	else \
		echo "TEST #3 FAILED: did not successfully execute --command with perfect input"; \
	fi; \
	rm -f a.txt b.txt c.txt
checkCreat:
	./simpsh --creat --rdonly a.txt --creat --wronly b.txt --creat --wronly c.txt --command 0 1 2 cat; \
	if [ $$? -eq 0 ]; then \
		echo "TEST #4 PASSED: successfully created file with --creat flag"; \
	else \
		echo "TEST #4 FAILED: did not successfully create files with --creat flag"; \
	fi; \
	rm -f a.txt b.txt c.txt
checkPipe:
	echo "test input" > pipe.txt; \
	./simpsh --rdonly pipe.txt --creat --wronly test.txt --pipe --command 0 3 1 cat; \
	if [ $$? -eq 0 ]; then \
		echo "TEST #5 PASSED: successfully created a pipe and used it in a command"; \
	else \
		echo "TEST #5 FAILED: did not successfully create a pipe and use it in a command"; \
	fi; \
	rm -f pipe.txt test.txt
checkAbort:
	./simpsh --abort; \
	if [ $$? -eq 139 ]; then \
		echo "TEST #6 PASSED: exited with status 139 due to a segmentation fault for --abort flag"; \
	else \
		echo "TEST #6 FAILED: did not exit with status 139 due to a segmentation fault for --abort flag"; \
	fi
checkWait:
	touch a.txt b.txt c.txt; \
	echo "test input for wait" > a.txt; \
	./simpsh --rdonly a.txt --wronly b.txt --wronly c.txt --command 0 1 2 cat --wait; \
	if [ $$? -eq 0 ]; then \
		echo "TEST #7 PASSED: successfully executed command with --wait flag"; \
	else \
		echo "TEST #7 FAILED: did not successfully execute command with --wait flag"; \
	fi; \
	rm -f a.txt b.txt c.txt
