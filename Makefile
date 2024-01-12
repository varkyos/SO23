CC = gcc
CFLAGS = -Wall

.DEFAULT_GLOBAL = merge2 merge2val merge4 mergeN msort

all: merge2 merge2val merge4 mergeN msort

merge2: merge2.c
	$(CC) $(CFLAGS) -o merge2 merge2.c

merge2val: merge2val.c
	$(CC) $(CFLAGS) -o merge2val merge2val.c

merge4: merge4.c
	$(CC) $(CFLAGS) -o merge4 merge4.c

mergeN: mergeN.c
	$(CC) $(CFLAGS) -o mergeN mergeN.c

msort: msort.c
	$(CC) $(CFLAGS) -o msort msort.c

clean:
	rm -f merge2 merge2val merge4 mergeN msort
