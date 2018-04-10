#include "proj5.h"

int pcbid;	// Shared memory ID of process control block table
int clkid;	// Shared memory ID of simulated system clock
int msgid;	// Message queue ID
int semid;	// Semaphore ID
int descid;	// Resource description ID
FILE *fp;	// Log file

void outputTable();

void handler(int signo) {
	printf("OSS: Terminating by signal\n");
	shmctl(clkid, IPC_RMID, NULL);	// Release simulated system clock memory
	shmctl(descid, IPC_RMID, NULL);	// Release resource description memory
	fclose(fp);
	exit(1);
}

int main(int argc, char *argv[]) {

	// getopt() vars
	int opt;
	int vflag = 0;
	
	// Child tracking vars
	int currentUsers = 0;
	const int maxUsers = 5;
	pid_t pid;

	sim_time *simClock;	// Pointer to simulated clock in shared memeory
	resource *resDesc;	// Pointer to resource descriptor array in chared memeory
	message_buf buf;	// Message content
	size_t buf_length;	// Message length

	// Parse command line options:
	while ((opt = getopt(argc, argv, "v")) != -1) {
		switch (opt) {
			case 'v':
				// Set -h flag
				vflag = 1;
				break;
		}
	}
	if (vflag) {
		outputTable();
	}

	signal(SIGINT, handler);	// Catch interupt signal
//	signal(SIGALRM, handler);	// Catch alarm signal
//	alarm(2);			// Send alarm signal after 2 seconds of realtime

	// Open log file for output
	fp = fopen("data.log", "w");
	if (fp == NULL) {
		fprintf(stderr, "Can't open file\n");
		exit(1);
	}

	// Create clock in shared memory
	if ((clkid = shmget(CLKKEY, sizeof(sim_time), IPC_CREAT | 0666)) < 0 ) {
		perror("oss: shmget");
		exit(1);
	}
	simClock = shmat(clkid, NULL, 0);
	simClock->sec = 0;
	simClock->nano = 0;

	// Create resource descriptors in shared
	if ((descid = shmget(DESCKEY, 20*sizeof(resource), IPC_CREAT | 0666)) < 0) {
		perror("oss: shmget");
		exit(1);
	}
	resDesc = shmat(descid, NULL, 0);

	// Create message queue
	if ((msgid = msgget(MSGKEY, IPC_CREAT | 0666)) < 0) {
		perror("oss: msgget");
		exit(1);
	}


	if ((pid = fork()) == 0) {
		execl("./user", "user", NULL);
		fprintf(stderr, "Fail to execute child process\n");
		exit(1);
	}



/*
	buf.mtype = 1;
	sprintf(buf.mtext, "OSS is ready", (long)getpid());
	buf_length = strlen(buf.mtext) + 1;
	if (msgsnd(msgid, &buf, buf_length, 0) < 0) {
		perror("oss: msgsnd");
		exit(1);
	}
*/

//	printf("buf.mtype = %d\nbuf_length = %d\nbuf_mtext = %s\n", buf.mtype, buf_length, buf.mtext);


	// Repeatedly wait for a type 1 message in the queue
	while(msgrcv(msgid, &buf, MSGSZ, 1, IPC_NOWAIT) == (ssize_t)-1) {
		printf("OSS: No message in queue\n");
		sleep(1);
	}
	printf("recieved \'%s\'\n", buf.mtext);


/*	if (msgrcv(msgid, &buf, MSGSZ, 1, IPC_NOWAIT) < 0) {
		perror("msgrcv");
		exit(1);
	}
	else {
		printf("recieved \'%s\'\n", buf.mtext);
	}
*/
	//if (msgrcv(msgid, &buf

/*	//Repeating loop
	while (1) {
		/*
		if (currentUsers < MAXUSERS) {
			if ((pid = fork()) == 0) {
				execl("./user", "user", NULL);
				fprintf(stderr, "Fail to execute child process\n");
			}
			else
				printf("oss: Child %ld spawned\n" , (long)pid);
			currentUsers++; 
		}

		wait(NULL);

		printf("Are we waiting?\n");
		if (msgrcv(msgid, &buf, MSGSZ, 1, 0) < 0) {
			perror("oss: msgrcv");
			exit(1);
		}
		//printf("oss: %s\n", buf.mtext);	
		
	}
*/

	printf("OSS: DONE\n");
	shmctl(clkid, IPC_RMID, NULL);	// Release process control block memeory
	msgctl(msgid, IPC_RMID, NULL);	// Release message queue memory
	shmctl(descid, IPC_RMID, NULL);	// Release resource description memory
	fclose(fp);

	return 0;
}

void outputTable() {
	int i, j;
	for (i = -1; i < SIZE; i++) {
		if (i < 0) {
			printf("\t");
			for (j = 0; j < SIZE; j++) {
				printf("R%d\t", j);
			}
		}
		else {
			printf("P%d\t", i);
			for (j = 0; j < SIZE; j++) {
				printf("%d\t", (i * SIZE + j));
			}
		}
		printf("\n");
	}
}
