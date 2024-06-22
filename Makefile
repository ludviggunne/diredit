CC=gcc

.PHONY: clean

diredit: diredit.c
	$(CC) diredit.c -o diredit

clean:
	rm *.o diredit
