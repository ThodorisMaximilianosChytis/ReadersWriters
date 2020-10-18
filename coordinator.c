#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/shm.h>
#include <math.h>
#include <sys/wait.h>
#include "semaphores.h"
#include "sharedmemory.h"
#define execution_num 6

double ran_expo(double);


int main(int argc,char** argv){


	int peer;
	int entnum;
	float ratio;
	key_t key;
	int status=0;
	if (argc != 4){
		printf("Input Error, wrong number of arguments\n");
		exit(1);
	}	
	else{
		peer=atoi(argv[1]);
		entnum=atoi(argv[2]);
		if (peer<=0 || entnum<=0){
			printf("Input Error,negative Inputs\n");
			exit(1);
		}
		ratio=atof(argv[3]);
		if (ratio<0.0 || ratio>1.0){
			printf("Input Error, out of bounds ratio number\n");
			exit(1);
		}
	} 
	key=ftok("coordinator.c",'T');
	if (key==-1){
		printf("Key creation problem\n");
		exit(1);
	}

	printf("--------------------------------------------\n");
	printf("-Number of peer processes %d \n\n",peer);
	printf("-Number of entries %d \n\n",entnum);
	printf("-This is reader/writer ratio is  %f \n",ratio);
	printf("--------------------------------------------\n");

	int size=entnum*sizeof(entry);			//Create shared memory of entries
	int shmid=shmCreate(key,size);
	entry * shmptr=shmAttach(shmid);		//Attach


	size=4*sizeof(int);				//Create shared memory of additional information 
	int rwshmid=shmCreate(key,size);//which shall be visible and accesible to all processes
	int* rwshmptr = (int*) shmat (rwshmid, (void*) 0, 0);	//Attach
	if (rwshmptr ==(int*)(-1)){
		printf("shmat error\n");
		exit(1);
	}
	printf ("shared memory attached at address %p\n", rwshmptr);
	printf("--------------------------------------------\n");

	rwshmptr[0]=ceil(ratio*peer)*execution_num;	//sto 0 readratio anw akeraio meros
	rwshmptr[1]=(peer-ceil(ratio*peer))*execution_num;	//sto 1 writeratio katw akeraio meros
	rwshmptr[2]=0;		//sto 2 sum_of_all_reads(the sum of all attempts of processes to read)
	rwshmptr[3]=0;		//stop 3 sum_of_all_writes(the sum of all attempts of processes to read)

	
	int i=0;
	for(i=0;i<entnum;i++){			//Initialize each entry's compartments
		shmptr[i].stat_reader_counter=0;	//The number of times the entry is read
		shmptr[i].stat_writer_counter=0; 	//The number of times the entry is written
		shmptr[i].time_read=0.0;			//The time the entry is read
		shmptr[i].time_written=0.0; 		//The time the entry is written on
		shmptr[i].rwsemid = semCreate((key_t)(i+1)*1892);	//Creation and Initilization of the rwsemaphore	
		shmptr[i].rsemid = semCreate((key_t)(i+1)*6737);	//Creation and Initiliaztion of the rsemaphore
		shmptr[i].readercounter=0;	//Counter of readers in the entry at the moment;

		//printf("rwsemid = %d\n", shmptr[i].rwsemid);
		//printf("rsemid = %d\n", shmptr[i].rsemid);
	}

	int choice;
	int entindex;
	double time; 	//time to sleep within Critical Section

	printf("	Each peer will attempt to execute reading/writing %d times \n\n",execution_num );
	
	int pid;
	for (i=0;i<peer;i++){		//Create child processes
		if (!(pid=fork()))
		{break;}
	}
	
	srand( (unsigned int) getpid() );	//Initialize rand using peer's unique id 
	int peer_reading=0;		//The number of times this specific process reads any entry
	int peer_writing=0;		//The number of times this specific process writes on any entry
	clock_t start;
	clock_t end;
	double time_waited;

	if (pid==0){	//Process is child
	
		for (i=0;i<execution_num;i++){			//Processes shall loop their attempt execution number of times
			
			choice = rand()%2;					//Random Choice beween reader or writer
			entindex = rand()%entnum;			//Random choice of entry in shared memory
			
			if ((choice==0 || rwshmptr[1]==0) && rwshmptr[0]>0){		//If reader or no more writers to create
				rwshmptr[0]--;
				peer_reading++;	
				start=clock();		//Reader waiting to access


				 semP(shmptr[entindex].rsemid);			//reader Semaphore wait

				 shmptr[entindex].readercounter++;		//one reader in the entry right now	

				 if (shmptr[entindex].readercounter==1){	//if only one reader	
				 	semP(shmptr[entindex].rwsemid);			//semaphore wait for reader writer semaphore
				 }

				semV(shmptr[entindex].rsemid);				//semaphore signal for readersemaphore
				 
				////////START OF CRITICAL SECTION//////////////
				 
				 end=clock();		//end of waiting
				 time_waited += ((double) (end - start)) / CLOCKS_PER_SEC;		//waiting time for reader
				 time=0.3*ran_expo(0.05);		//sleep an amount of time which is approximately a user's reading time
				 shmptr[entindex].time_read += time;		//time the entry has been read
				 sleep(time);
				 printf("\n*CS* Process reading in entry = %d  pid is = %d\n",entindex, getpid() );
				 shmptr[entindex].stat_reader_counter++;	//sum of readers which have entered the entry

				 //////END OF CRITICAL SECTION//////////////

				semP(shmptr[entindex].rsemid);			//semaphore down for reader semaphore

				shmptr[entindex].readercounter--;		//the reader hass left the entry

				if (shmptr[entindex].readercounter==0){	//if there is no reader
					semV(shmptr[entindex].rwsemid);		//signal the rwsemaphore
				}

				semV(shmptr[entindex].rsemid);			//signal the writer semaphore
			}


			if ( (choice==1 || rwshmptr[0]==0) && rwshmptr[1]>0){	//If writer or no more readers to create			
				rwshmptr[1]--;

				peer_writing++;	//number of times this is peer is writing
				start=clock();	////Reader waiting to access


				 semP(shmptr[entindex].rwsemid);		//wait read write semaphore



				 /////////////////////START OF CRITICAL SECTION///////////////////


				 end=clock();		//end of waiting
				 time_waited += ((double) (end - start)) / CLOCKS_PER_SEC;	////waiting time for writer
				 time =0.9*ran_expo(0.05);				//sleep an amount of time which is approximately a user's writing time
				 shmptr[entindex].time_written += time;			//time this entry hass been written
				 sleep(time);	
				 printf("\n*CS* Process writing in entry = %d , pid is = %d\n",entindex, getpid() );
				 shmptr[entindex].stat_writer_counter++;	//the amount of times the entry has been written on 



				/////////////////////END OF CRITICAL SECTION///////////////////

				 semV(shmptr[entindex].rwsemid);	//signal read write semaphore
				
			}
		}

		printf("--------------------------------------------\n");
		printf("This peer %d has read %d times\n",getpid(),peer_reading );
		printf("This peer %d has written %d times \n",getpid(),peer_writing );
		printf("This peer %d has waited an average of %f sec \n",getpid(),time_waited / execution_num);
		rwshmptr[2]+=peer_reading;
		rwshmptr[3]+=peer_writing;
		printf("--------------------------------------------\n");


		exit(0);			//terminate chlid process succesfully



	}
	else {
		if (pid!=0)			//Is chlid process
		{
			for(int i=0;i<peer;i++)		//wait for children to  finish
				wait(&status);
			int sumr=0;
			int sumw=0;
			for(i=0;i<entnum;i++){
				printf("--------------------------------------------\n");
				printf("The number %d entry has been read %d times \n",i,shmptr[i].stat_reader_counter);
				printf("The number %d entry has been written on %d times \n",i,shmptr[i].stat_writer_counter);
				printf("The number %d entry has been read for time  %f  \n",i,shmptr[i].time_read);
				printf("The number %d entry has been written on for time  %f  \n",i,shmptr[i].time_written);
				printf("--------------------------------------------\n");
				sumr+=shmptr[i].stat_reader_counter;	
				sumw+=shmptr[i].stat_writer_counter;
				semDel(shmptr[i].rwsemid);
				semDel(shmptr[i].rsemid);

			}

			/////The sum of all the times all entries have been read/written on 
			/////is the same as the sum of all the times an entry has read/written
			printf("--------------------------------------------\n"); 
			printf("|sumr %d = sum_of_all_reads %d |\n",sumr,rwshmptr[2]);
			printf("|sumw %d = sum_of_all_writes %d|\n",sumw,rwshmptr[3]);
			printf("--------------------------------------------\n");

			shmDetach(shmptr);		//detach shared memory of entries
			shmDelete(shmid);		//delete shared memory of entries
			//shmDetach(rwshmptr);
			int det=shmdt(rwshmptr);	//detach shared memory of stat integers
			if (det==-1){
				printf("Error detaching memory\n");
				exit(1);
			}
			shmDelete(rwshmid);		//delete shared memory of integers
		}
	}
}


double ran_expo(double lambda){			//exponential distributio function
    double u;
    u = rand() / (RAND_MAX + 1.0);
    return -log(1- u) / lambda;
}