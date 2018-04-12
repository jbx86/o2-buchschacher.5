#include "proj5.h"

int pcbid;	// Shared memory ID of process control block table
int clkid;	// Shared memory ID of simulated system clock
int msgid;	// Message queue ID
int semid;	// Semaphore ID
int descid;	// Resource description ID
FILE *fp;	// Log file

message_buf buf;	// Message content
size_t buf_length;	// Message length

void outputTable();
void allocate(int, pid_t);
void block(pid_t);
void unblock(pid_t);


void handler(int signo) {
	printf("OSS: Terminating by signal\n");
	shmctl(clkid, IPC_RMID, NULL);	// Release simulated system clock memory
	shmctl(descid, IPC_RMID, NULL);	// Release resource description memory
	msgctl(msgid, IPC_RMID, NULL);	// Release message queue
	fclose(fp);
	exit(1);
}

int main(int argc, char *argv[]) {
	int procNum;

	// getopt() vars
	int opt;
	int vflag;
	
	// Child tracking vars
	int currentUsers = 0;
	int totalUsers = 0;

	pid_t ossPid = getpid();
	pid_t pid;
	pid_t P[18] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	sim_time *simClock;	// Pointer to simulated clock in shared memeory
	resource *resDesc;	// Pointer to resource descriptor array in chared memeory

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
					printf("Could not spawn child\n");
					break;
				case 0 :
					execl("./user", "user", NULL);
					fprintf(stderr, "Fail to execute child process\n");
					exit(1);
				default :
					// Find empty slot in pid table
					for (procNum = 0; procNum < MAXUSERS; procNum++)
						if (P[procNum] == 0)
							break;
					P[procNum] = pid;
					totalUsers++;
					currentUsers++;
					printf("Creating P%2d (PID: %ld)\n", procNum, pid);
			}
		}

		// Handle messages from children
		if (msgrcv(msgid, &buf, MSGSZ, (long)ossPid, IPC_NOWAIT) != (ssize_t)-1) {

			// Parse message
			char *ptr;
			pid_t userPid = (pid_t)strtol(buf.mtext, &ptr, 10);
			int msgType = (int)strtol(ptr, &ptr, 10);
			int msgData = (int)strtol(ptr, &ptr, 10);
			printf("Message type %d recieved from %ld regarding %ld\n", msgType, userPid, msgData);

			// Determine which process sent the message
			for (procNum = 0; procNum < MAXUSERS; procNum++)
				if (P[procNum] == userPid)
					break;

			// Handle message
			switch (msgType) {
				case TERM:
					printf("P%2d is terminating\n", procNum);
					waitpid(P[procNum], NULL, 0);
					P[procNum] = 0;
					currentUsers--;
					break;
				case REQ:
					fprintf(fp, "Master has detected Process P%2d requesting R%2d at time %d:%09d\n", procNum, msgData, simClock->sec, simClock->nano);
					break;
				case REL:
					fprintf(fp, "Master has detected Process P%2d releasing R%2d at time %d:%09d\n", procNum, msgData, simClock->sec, simClock->nano);
					break;

			}
			// Respond to user


		}
	}

	// Kill remaining user processes
	for (procNum = 0; procNum < MAXUSERS; procNum++)
		if (P[procNum] != 0) {
			kill(P[procNum], SIGKILL);
			waitpid(P[procNum], NULL, 0);
		}

	// Release memeory
	shmctl(clkid, IPC_RMID, NULL);	// Release process control block memeory
	shmctl(descid, IPC_RMID, NULL);	// Release resource description memory
	msgctl(msgid, IPC_RMID, NULL);  // Release message queue memory
	fclose(fp);

	return 0;
}

void outputTable() {
	int procNum, rsrcNum;
	for (procNum = -1; procNum < MAXUSERS; procNum++) {
		if (procNum < 0) {
			printf("\t");
			for (rsrcNum = 0; rsrcNum < SIZE; rsrcNum++) {
				printf("R%2d\t", rsrcNum);
			}
		}
		else {
			printf("P%2d\t", procNum);
			for (rsrcNum = 0; rsrcNum < SIZE; rsrcNum++) {
				printf("%d\t", (procNum * SIZE + rsrcNum));
			}
		}
		printf("\n");
	}
}

// Allocate resource
void allocate(int resource, pid_t userPid) {
	buf.mtype = (long)userPid;
	sprintf(buf.mtext, "%ld %d %d", (long)getpid(), ALC, resource);
	buf_length = strlen(buf.mtext) + 1;
	if (msgsnd(msgid, &buf, buf_length, 0) < 0) {
		perror("oss: msgsnd");
		exit(1);
	}
}

// Block user process
void block(pid_t userPid) {
	buf.mtype = (long)userPid;
	sprintf(buf.mtext, "%ld %d %d", (long)getpid(), BLK, 0);
	buf_length = strlen(buf.mtext) + 1;
	if (msgsnd(msgid, &buf, buf_length, 0) < 0) {
		perror("oss: msgsnd");
		exit(1);
	}
}

// Unblock user process
void unblock(pid_t userPid) {
	buf.mtype = (long)userPid;
	sprintf(buf.mtext, "%ld %d %d", (long)getpid(), UBLK, 0);
	buf_length = strlen(buf.mtext) + 1;
	if (msgsnd(msgid, &buf, buf_length, 0) < 0) {
		perror("oss: msgsnd");
		exit(1);
	}
}
