CC=gcc

.PHONY: clean

diredit: diredit.c
	$(CC) main.c -o diredit

clean:
	rm *.o diredit
