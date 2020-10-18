


#include "sharedmemory.h"


int shmCreate(key_t key,int size){
	int id = shmget (IPC_PRIVATE, size, IPC_CREAT | 0666);
	if(id<0){
		printf("shmget error\n");
		exit(1);
	}
	return id;
}


entry * shmAttach(int id){
	entry * shared_memory = (entry*) shmat (id, (void*) 0, 0);
	if (shared_memory ==(entry*)(-1)){
		printf("shmat error\n");
		exit(1);
	}	
	printf ("shared memory attached at address %p\n", shared_memory);
	return shared_memory;
}

void shmDelete(int id){
	int del=shmctl (id, IPC_RMID, 0);
	if (del==-1){
		printf("Error deleting memory\n");
		exit(1);
	}
}

void shmDetach(entry* entp){
	int det=shmdt(entp);
	if (det==-1){
		printf("Error detaching memory\n");
		exit(1);
	}
}

