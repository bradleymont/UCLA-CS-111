# NAME: Bradley Mont
# EMAIL: bradleymont@gmail.com
# ID: 804993030

CC = gcc
CFLAGS = -Wall -Wextra -g

default:
	$(CC) $(CFLAGS) lab1a.c -o simpsh
clean:
	rm -f simpsh *.tar.gz *.o *.txt
check: checkFileDNE checkInvalidFD checkSuccesfulCommand
dist:
	tar -czf lab1-804993030.tar.gz lab1a.c Makefile README
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
