# NAME: Bradley Mont
# EMAIL: bradleymont@gmail.com
# ID: 804993030

CC = gcc
CFLAGS = -Wall -Wextra -g -lm -lmraa

default:
	$(CC) $(CFLAGS) lab4b.c -o lab4b
check: checkCorrectArgs checkIncorrectArgs
clean:
	rm -f lab4b *.tar.gz *.o
dist:
	tar -czf lab4b-804993030.tar.gz lab4b.c Makefile README
checkCorrectArgs:
	{ echo "START"; sleep 1; echo "STOP"; sleep 1; echo "OFF"; } | ./lab4b --log=test1.txt --scale=C; \
	if [ $$? -eq 0 ]; then \
		echo "TEST #1 PASSED: lab4b correctly exited with status 0 when given perfect input"; \
	else \
		echo "TEST #1 FAILED: lab4b did not correctly exit with status 0 when given perfect input"; \
	fi; \

	grep START test1.txt; \
	if [ $$? -eq 0 ]; then \
		echo "TEST #2 PASSED: program logged START correctly"; \
	else \
		echo "TEST #2 FAILED: program did not log START correctly"; \
	fi; \

	grep STOP test1.txt; \
	if [ $$? -eq 0 ]; then \
		echo "TEST #3 PASSED: program logged STOP correctly"; \
	else \
		echo "TEST #3 FAILED: program did not log STOP correctly"; \
	fi; \

	grep OFF test1.txt; \
	if [ $$? -eq 0 ]; then \
		echo "TEST #4 PASSED: program logged OFF correctly"; \
	else \
		echo "TEST #4 FAILED: program did not log OFF correctly"; \
	fi; \

	grep SHUTDOWN test1.txt; \
	if [ $$? -eq 0 ]; then \
		echo "TEST #5 PASSED: program logged SHUTDOWN correctly"; \
	else \
		echo "TEST #5 FAILED: program did not log SHUTDOWN correctly"; \
	fi; \
	rm -f test1.txt
checkIncorrectArgs:
	./lab4b --bogusArg; \
	if [ $$? -eq 1 ]; then \
		echo "TEST #6 PASSED: program exited with status 1 when given a bogus argument"; \
	else \
		echo "TEST #6 FAILED: program did not exit with status 1 when given a bogus argument"; \
	fi
