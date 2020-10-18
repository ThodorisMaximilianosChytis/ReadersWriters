objects= sharedmemory.o semaphores.o coordinator.o

ReadersWriters :$(objects)
	gcc -o ReadersWriters $(objects) -lm

sharedmemory.o :sharedmemory.c
	gcc -c sharedmemory.c sharedmemory.h semaphores.h 

semaphores.o :semaphores.c
	gcc -c semaphores.c sharedmemory.h semaphores.h

coordinator.o :coordinator.c
	gcc -c coordinator.c sharedmemory.h semaphores.h 

clean  :
	rm ReadersWriters $(objects) sharedmemory.h.gch semaphores.h.gch

