#include "proj4.h"

int pcbid;	// Shared memory ID of process control block table
int clockid;	// Shared memory ID of simulated system clock
int msgid;	// Message queue ID
FILE *fp;	// Log file

void handler(int signo) {
	if (signo == SIGINT) {
		shmctl(pcbid, IPC_RMID, NULL);		// Release process control block memeory
		shmctl(clockid, IPC_RMID, NULL);	// Release simulated system clock memory
		msgctl(msgid, IPC_RMID, NULL);		// Release message queue memory
		fclose(fp);
	}
	printf("OSS: Terminated by signal\n");
}

int main() {
	// Initialize signal handlers
	signal(SIGINT, handler);
	signal(SIGALRM, handler);

	// Pointer(s) to shared memory
	pcb *pcbTable;

	// Message queue
	message_buf buf;
	size_t buf_length;

	// Open log file for output
	fp = fopen("data.log", "w");
	if (fp == NULL) {
		fprintf(stderr, "Can't open file\n");
		exit(1);
	}

// Setup IPC

	// Create memory segment for process control table
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

// Main loop

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

	shmctl(pcbid, IPC_RMID, NULL); // Release process control block memeory
	msgctl(msgid, IPC_RMID, NULL); // Release message queue memory
	fclose(fp);

	return 0;
}
