// user.c
#include "proj5.h"

int main() {
	printf("Child!\n");
//	int pcbShmid;		// Shared memory ID of process control block table
	int clkid;		// Shared memory ID of simclock
	int msgid;		// Message queue ID


//	pcb *pcbTable;		// Pointer to table of process control blocks
	sim_time *ossClock;	// Pointer to simulated system clock
	message_buf buf;	// Message buffer
	size_t buf_length;	// Length of send message buffer

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

	// Send a message
	buf.mtype = (long)getpid();
	sprintf(buf.mtext, "requesting resource R5");
	buf_length = strlen(buf.mtext) + 1;
	if (msgsnd(msgid, &buf, buf_length, 0) < 0) {
		perror("user: msgsnd");
		exit(1);
	}



	exit(0);
}
