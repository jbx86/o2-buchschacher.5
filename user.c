// user.c
#include "proj5.h"
#define RNDBND 20

int main() {
	int clkid;		// Shared memory ID of simclock
	int msgid;		// Message queue ID

	sim_time *ossClock;	// Pointer to simulated system clock
	message_buf buf;	// Message buffer
	size_t buf_length;	// Length of send message buffer

	long int ossPid = (long)getppid();
	long int userPid = (long)getpid();
	srand((time(NULL) + userPid) % UINT_MAX);
//	printf("%ld is alive and generated %d\n", userPid, rand() % 20);
	int i;

// Setup IPC

	// Locate shared simulated system clock
	if ((clkid = shmget(CLKKEY, sizeof(sim_time), 0666)) < 0 ) {
		perror("user: shmget clkid");
		exit(1);
	}
	ossClock = shmat(clkid, NULL, 0);

	// Locate message queue
	if ((msgid = msgget(MSGKEY, 0666)) < 0) {
		perror("user: shmget msgid");
		exit(1);
	}

	
/*
	// Send a message to OSS
	buf.mtype = ossPid;
	sprintf(buf.mtext, "%ld %d", userPid, rand() % 20);
	buf_length = strlen(buf.mtext) + 1;
	if (msgsnd(msgid, &buf, buf_length, 0) < 0) {
		perror("user: msgsnd");
		exit(1);
	}

/*
	// Wait for responce from OSS
	if (msgrcv(msgid, &buf, MSGSZ, userPid, 0) < 0) {
		perror("user: msgrcv");
		exit(1);
	}

	printf("%ld is terminating\n", userPid);
*/

// User 

	// Release resources

	// Tell OSS this process is terminating
	buf.mtype = ossPid;
	sprintf(buf.mtext, "%ld %d %d", userPid, 0, 0);
	buf_length = strlen(buf.mtext) + 1;
	if (msgsnd(msgid, &buf, buf_length, 0) < 0) {
		perror("user: msgsnd");
		exit(1);
	}



	exit(0);
}
