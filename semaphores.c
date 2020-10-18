#include "semaphores.h"

int semSet(int sem_id)
{
	union semun sem_union;
	sem_union.val = 1;
	if (semctl(sem_id, 0, SETVAL, sem_union) == -1) return(0);
	return(1);
}

int semCreate(key_t key){
	int semid = semget(key,1,  IPC_CREAT | 0666);
	if (!semSet(semid)) 
	{
			printf("Failed to initialize semaphore\n");
			exit(1);
	}
	return semid;
}

void semDel(int sem_id){
	union semun sem_union;
	if (semctl(sem_id, 0, IPC_RMID, sem_union) == -1){
		printf("Failed to delete semaphore\n");
		exit(1);
	}
}


void semV(int sem_id)
{
	struct sembuf sem_b;
	sem_b.sem_num = 0;
	sem_b.sem_op = 1;
	sem_b.sem_flg = SEM_UNDO;
	if (semop(sem_id, &sem_b, 1) == -1) {
		printf("semaphore_v failed\n");
		exit(1);
	}
}


void semP(int sem_id)
{
	struct sembuf sem_b;
	sem_b.sem_num = 0;
	sem_b.sem_op = -1;
	sem_b.sem_flg = SEM_UNDO;
	//printf("This sem_id is %d\n",sem_id);
	if (semop(sem_id, &sem_b, 1) == -1) {
		printf("semaphore_p failed %d \n",sem_id);
		exit(1);
	}

}





