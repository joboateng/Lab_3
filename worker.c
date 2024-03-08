#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>

// Message structure
struct msgbuf {
    long mtype;  // Message type
    int value;   // Message value
};

// Function to get the current system time
void getCurrentTime(int *seconds, int *nanoseconds) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    *seconds = tv.tv_sec;
    *nanoseconds = tv.tv_usec * 1000;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <runtime_seconds> <runtime_nanoseconds>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int runtime_seconds = atoi(argv[1]);
    int runtime_nanoseconds = atoi(argv[2]);

    // Get process information
    int workerIndex = atoi(argv[0]);

    // Initialize termination time
    int termination_seconds;
    int termination_nanoseconds;
    getCurrentTime(&termination_seconds, &termination_nanoseconds);
    termination_seconds += runtime_seconds;
    termination_nanoseconds += runtime_nanoseconds;

    printf("WORKER PID:%d PPID:%d SysClockS: %d SysClockNano: %d TermTimeS: %d TermTimeNano: %d\n--Just Starting\n",
           getpid(), getppid(), 0, 0, termination_seconds, termination_nanoseconds);

    // Set up message queue
    key_t key = ftok(".", 'm');
    int msgid = msgget(key, 0666);

    // Main loop
    int iterations = 0;
    do {
        // Receive message from oss
        struct msgbuf message;
        msgrcv(msgid, &message, sizeof(message), workerIndex + 1, 0);

        // Check the clock
        int current_seconds, current_nanoseconds;
        getCurrentTime(&current_seconds, &current_nanoseconds);

        // Output information
        printf("WORKER PID:%d PPID:%d SysClockS: %d SysClockNano: %d TermTimeS: %d TermTimeNano: %d\n--%d iterations have passed since starting\n",
               getpid(), getppid(), current_seconds, current_nanoseconds, termination_seconds, termination_nanoseconds,
               iterations);

        // Send message back to oss
        message.mtype = 1;
        message.value = (current_seconds * 1000000000 + current_nanoseconds >= termination_seconds * 1000000000 + termination_nanoseconds) ? 0 : 1;
        msgsnd(msgid, &message, sizeof(message.value), 0);

        // Sleep for a short period (e.g., 100 milliseconds)
        usleep(100000);

        // Increment the iteration counter
        iterations++;

    } while (1);

    return 0;
}
