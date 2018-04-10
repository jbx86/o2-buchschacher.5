#include "proj5.h"
#define MAXUSERS 18

int pcbid;	// Shared memory ID of process control block table
int clkid;	// Shared memory ID of simulated system clock
int msgid;	// Message queue ID
int semid;	// Semaphore ID
FILE *fp;	// Log file

void outputTable();

void handler(int signo) {
	printf("OSS: Terminating by signal\n");
	shmctl(clkid, IPC_RMID, NULL);	// Release simulated system clock memory
/*	msgctl(msgid, IPC_RMID, NULL);		// Release message queue memory
	fclose(fp);
*/
}

int main(int argc, char *argv[]) {

	//Parse command line options:
	int opt;
	int vflag = 0;
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


	// Initialize signal handlers
	signal(SIGINT, handler);
	signal(SIGALRM, handler);
	alarm(2);

	int currentUsers;
	int pid;	
	sim_time *simClock;
	message_buf buf;
	size_t buf_length;	

	// Open log file for output
	fp = fopen("data.log", "w");
	if (fp == NULL) {
		fprintf(stderr, "Can't open file\n");
		exit(1);
	}

	// Allocate shared memory for system structures

	// Create clock in shared memory
	if ((clkid = shmget(CLKKEY, sizeof(sim_time), IPC_CREAT | 0666)) < 0 ) {
		perror("oss: shmget");
		exit(1);
	}
	simClock = shmat(clkid, NULL, 0);
	simClock->sec = 0;
	simClock->nano = 0;

	// Create resource descriptors in shared

/*	// Create memory segment for process control table
	if ((pcbid = shmget(PCBKEY, SIZE*sizeof(pcbTable), IPC_CREAT | 0666)) < 0 ) {
		perror("oss: pcbid");
		exit(1);
	}
	pcbTable = shmat(pcbid, NULL, 0);


	// Create message queue
	if ((msgid = msgget(MSGKEY, IPC_CREAT | 0666)) < 0) {
		perror("oss: msgget");
		exit(1);
	}

	// fork/execl child
	if ((pid = fork()) == 0) {
		execl("./user", "user", NULL);
		fprintf(stderr, "Fail to execute child process\n");
	}

			
	// Send message to nextPid
	buf.mtype = nextPid + 1;
	sprintf(buf.mtext, "Message from OSS");
	buf_length = strlen(buf.mtext) + 1;
	if (msgsnd(msgid, &buf, buf_length, 0) < 0) {
		error("oss: msgsend");
		exit(1);
	}
*/
	shmctl(clkid, IPC_RMID, NULL); // Release process control block memeory
	msgctl(msgid, IPC_RMID, NULL); // Release message queue memory
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
