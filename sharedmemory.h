#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/shm.h>


struct ent {
	int stat_reader_counter;
	int stat_writer_counter;
	double time_read;
	double time_written;
	int rwsemid;
	int rsemid;
	int readercounter;
};

typedef struct ent entry; 



int shmCreate(key_t,int);
entry * shmAttach(int);
void shmDelete(int);
void shmDetach(entry*);