#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sem.h>

int semSet(int);
int semCreate(key_t);
void semDel(int);
void semP(int);
void semV(int);

union semun{
	int val;
	struct semid_ds *buf;
	unsigned short *array;
};