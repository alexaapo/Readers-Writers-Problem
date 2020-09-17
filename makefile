coordinator: coordinator.o semaphores.o shared_memory.o coordinator_help.o
	gcc -Wall coordinator.o semaphores.o shared_memory.o coordinator_help.o -o coordinator -lm
coordinator.o: coordinator.c coordinator.h
	gcc -c coordinator.c 
semaphores.o: semaphores.c semaphores.h
	gcc -c semaphores.c
shared_memory.o: shared_memory.c shared_memory.h
	gcc -c shared_memory.c
coordinator_help.o: coordinator_help.c coordinator.h semaphores.h
	gcc -c coordinator_help.c
clean: 
	rm coordinator.o semaphores.o shared_memory.o coordinator_help.o coordinator
