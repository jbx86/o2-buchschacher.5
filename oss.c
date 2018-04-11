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
	msgctl(msgid, IPC_RMID, NULL);	// Release message queue
	fclose(fp);
	exit(1);
}

int main(int argc, char *argv[]) {

	// getopt() vars
	int opt;
	int vflag;
	
	// Child tracking vars
	int currentUsers = 0;
	const int maxUsers = 5;
	pid_t pid;
	pid_t pidTable[18] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	sim_time *simClock;	// Pointer to simulated clock in shared memeory
	resource *resDesc;	// Pointer to resource descriptor array in chared memeory
	message_buf buf;	// Message content
	size_t buf_length;	// Message length

	// Parse command line options:
	while ((opt = getopt(argc, argv, "v")) != -1) {
		switch (opt) {
			case 'v':
				// Run in verbose mode
				vflag = 1;
				break;
			default:
				vflag = 0;
		}
	}
	if (vflag) {
		outputTable();
	}

	signal(SIGINT, handler);	// Catch interupt signal
	signal(SIGALRM, handler);	// Catch alarm signal
	alarm(5);			// Send alarm signal after 2 seconds of realtime

	// Open log file for output
	fp = fopen("data.log", "w");
	if (fp == NULL) {
		fprintf(stderr, "Can't open file\n");
		raise(SIGINT);
	}

	// Create clock in shared memory
	if ((clkid = shmget(CLKKEY, sizeof(sim_time), IPC_CREAT | 0666)) < 0 ) {
		perror("oss: shmget");
		raise(SIGINT);
	}
	simClock = shmat(clkid, NULL, 0);
	simClock->sec = 0;
	simClock->nano = 0;

	// Create resource descriptors in shared
	if ((descid = shmget(DESCKEY, 20*sizeof(resource), IPC_CREAT | 0666)) < 0) {
		perror("oss: shmget");
		raise(SIGINT);
	}
	resDesc = shmat(descid, NULL, 0);

	// Create message queue
	if ((msgid = msgget(MSGKEY, IPC_CREAT | 0666)) < 0) {
		perror("oss: msgget");
		raise(SIGINT);
	}




	// Infinitely repeated loop
	while(1) {

		// Fork children if max has not been reached
		if (currentUsers < MAXUSERS) {
			pid = fork();
			switch (pid) {
				case -1 :
					printf("Could not spawn child");
					break;
				case 0 :
					execl("./user", "user", NULL);
					fprintf(stderr, "Fail to execute child process\n");
					exit(1);
				default :
					currentUsers++;
					printf("OSS: %d users spawned\n", currentUsers);
			}
		}

		// Handle messages from children
		if (msgrcv(msgid, &buf, MSGSZ, 0, IPC_NOWAIT) != (ssize_t)-1) {
			// Parse message
			char *ptr;
			long int userPid;
			long int resourceNo;
			
			userPid = strtol(buf.mtext, &ptr, 10);
			resourceNo = strtol(ptr, &ptr, 10);

			printf("Message type %ld recieved from %ld regarding %ld\n", buf.mtype, userPid, resourceNo);

			// Send kill signal to user
			buf.mtype = userPid;
			sprintf(buf.mtext, "Okay");
			buf_length = strlen(buf.mtext) + 1;
			if (msgsnd(msgid, &buf, buf_length, 0) < 0) {
				perror("oss: msgsnd");
				exit(1);
			}
			wait(NULL);
			currentUsers--;
			
		}
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

/*
	// Repeatedly wait for a type 1 message in the queue
	while(msgrcv(msgid, &buf, MSGSZ, 1, IPC_NOWAIT) == (ssize_t)-1) {
		printf("OSS: No message in queue\n");
		sleep(1);
	}
	printf("recieved \'%s\'\n", buf.mtext);
*/

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
	printf("OSS: DONE ------------------------------------\n");
	shmctl(clkid, IPC_RMID, NULL);	// Release process control block memeory
	shmctl(descid, IPC_RMID, NULL);	// Release resource description memory
	msgctl(msgid, IPC_RMID, NULL);  // Release message queue memory
	fclose(fp);

	return 0;
}

void outputTable() {
	int i, j;
	for (i = -1; i < MAXUSERS; i++) {
		if (i < 0) {
			printf("\t");
			for (j = 0; j < SIZE; j++) {
				printf("R%d\t", j);
			}
		}
		else {
			printf("P%02d\t", i);
			for (j = 0; j < SIZE; j++) {
				printf("%d\t", (i * SIZE + j));
			}
		}
		printf("\n");
	}
}
