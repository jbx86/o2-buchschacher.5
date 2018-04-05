// user.c
#include "proj4.h"

int main() {
	int pcbShmid;		// Shared memory ID of process control block table
	int clkShmid;		// Shared memory ID of simclock
	int msgShmid;		// Message queue ID

	pcb *pcbTable;		// Pointer to table of process control blocks
	sim_time *ossClock;	// Pointer to simulated system clock
	message_buf buf;	// Message buffer
	size_t buf_length;	// Length of send message buffer

	int i;

// Setup IPC

	// Locate shared process control table
	if ((pcbShmid = shmget(PCBKEY, SIZE*sizeof(pcbTable), 0666)) < 0 ) {
		perror("user: shmget pcbShmid");
		exit(1);
	}
	pcbTable = shmat(pcbShmid, NULL, 0);

	// Locate shared simulated system clock
	if ((clkShmid = shmget(CLKKEY, sizeof(sim_time), 0666)) < 0 ) {
		perror("user: shmget clkShmid");
		exit(1);
	}
	ossClock = shmat(clkShmid, NULL, 0);

	// Locate message queue
	if ((msgShmid = msgget(MSGKEY, 0666)) < 0) {
		perror("user: shmget msgShmid");
		exit(1);
	}

// Main loop

	// Wait on message from OSS
	printf("%d: Waiting for message\n", pcbTable[myPid].simpid);
	if (msgrcv(msgShmid, &buf, MSGSZ, myPid + 1, 0) < 0) {
		perror("user: msgrcv");
		exit(1);
	}

	// send back mtype 5 to tell OSS a process was run at this level
	buf.mtype = SIZE + 1;
	sprintf(buf.mtext, "Message from user");
	buf_length = strlen(buf.mtext) + 1;
	if (msgsnd(msgShmid, &buf, buf_length, 0) < 0) {
		perror("user: msgsend");
		exit(1);
	}

	exit(0);
}
