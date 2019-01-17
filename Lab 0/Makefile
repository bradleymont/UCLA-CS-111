# NAME: Bradley Mont
# EMAIL: bradleymont@gmail.com
# ID: 804993030

CC = gcc
CFLAGS = -Wall -Wextra -g

default:
	$(CC) $(CFLAGS) lab0.c -o lab0 
clean:
	rm -f lab0 *.tar.gz *.o *.txt
check: checkNoArgs checkIncorrectArgs checkBadInputFile checkBadOutputFile checkCatchSegfault checkFilesMatch
dist:
	tar -czf lab0-804993030.tar.gz lab0.c Makefile backtrace.png breakpoint.png README
checkNoArgs:
	echo "This is test input" > input.txt; \
	./lab0 < input.txt > output.txt; \
	if [ $$? -eq 0 ]; then \
		echo "TEST #1 PASSED: program exited with status 0 for no arguments"; \
	else \
		echo "TEST #1 FAILED: program did not exit with status 0 for no arguments"; \
	fi; \
	rm -f output.txt
checkIncorrectArgs:
	./lab0 --bogusArg < input.txt; \
	if [ $$? -eq 1 ]; then \
		echo "TEST #2 PASSED: program exited with status 1 for incorrect argument(s)"; \
	else \
		echo "TEST #2 FAILED: program did not exit with status 1 for incorrect argument(s)"; \
	fi
checkBadInputFile:
	./lab0 --input=does_not_exist.txt; \
	if [ $$? -eq 2 ]; then \
		echo "TEST #3 PASSED: program exited with status 2 for invalid input file"; \
	else \
		echo "TEST #3 FAILED: program did not exit with status 2 for invalid input file"; \
	fi
checkBadOutputFile:
	echo "We do not have write permissions for this file" > badOutput.txt; \
	chmod -w badOutput.txt; \
	./lab0 --input=input.txt --output=badOutput.txt; \
	if [ $$? -eq 3 ]; then \
		echo "TEST #4 PASSED: program exited with status 3 for invalid output file"; \
	else \
		echo "TEST #4 FAILED: program did not exit with status 3 for invalid output file"; \
	fi; \
	rm -f input.txt badOutput.txt
checkCatchSegfault:
	./lab0 --catch --segfault; \
	if [ $$? -eq 4 ]; then \
		echo "TEST #5 PASSED: program exited with status 4 for caught segmentation fault"; \
	else \
		echo "TEST #5 FAILED: program did not exit with status 4 for caught segmentation fault"; \
	fi
checkFilesMatch:
	echo "these file contents should match" > file1.txt; \
	./lab0 --input=file1.txt --output=file2.txt; \
	cmp file1.txt file2.txt; \
	if [ $$? -eq 0 ]; then \
		echo "TEST #6 PASSED: successfully took in file1.txt as input and outputted to file2.txt"; \
	else \
		echo "TEST #6 FAILED: did not successfully take in file1.txt as input and output to file2.txt"; \
	fi; \
	rm -f file1.txt file2.txt
