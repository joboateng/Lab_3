#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <time.h>

// Message structure
struct msgbuf {
    long mtype;  // Message type
    int value;   // Message value
};

// Define PCB structure
struct PCB {
    int occupied;       // either true or false
    pid_t pid;           // process id of this child
    int startSeconds;    // time when it was forked (seconds)
    int startNano;       // time when it was forked (nanoseconds)
};

// Function to get the current system time
void getCurrentTime(int *seconds, int *nanoseconds) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    *seconds = tv.tv_sec;
    *nanoseconds = tv.tv_usec * 1000;
}

// Function to initialize the process table
void initProcessTable(struct PCB processTable[], int size) {
    for (int i = 0; i < size; i++) {
        processTable[i].occupied = 0;
        processTable[i].pid = 0;
        processTable[i].startSeconds = 0;
        processTable[i].startNano = 0;
    }
}

// Function to handle timeout signal
void timeoutHandler(int signo) {
    printf("Timeout: Killing all child processes and terminating.\n");
    kill(0, SIGTERM);  // Send SIGTERM to all child processes
    exit(EXIT_SUCCESS);
}




int main(int argc, char *argv[]) {
    // Parse command line arguments

    // Open log file
    FILE *logFile = fopen(argv[5], "w");
    if (logFile == NULL) {
        perror("Error opening log file");
        exit(EXIT_FAILURE);
    }

    // Set up message queue
    key_t key = ftok(".", 'm');
    int msgid = msgget(key, IPC_CREAT | 0666);
    if (msgid == -1) {
        perror("Error creating message queue");
        fclose(logFile);
        exit(EXIT_FAILURE);
    }

    // Initialize the process table
    struct PCB processTable[20];
    initProcessTable(processTable, 20);

    // Set up signal handlers
    signal(SIGALRM, timeoutHandler);
    signal(SIGINT, timeoutHandler);

    // Set up timer for 60-second timeout
    alarm(60);

    // Simulated clock variables
    int seconds = 0;
    int nanoseconds = 0;

    // Variables for child launching
    int numChildrenLaunched = 0;
    int totalChildren = atoi(argv[2]);
    int interval_ms = atoi(argv[4]);
    int timeLimit = atoi(argv[6]);
    int nextChild = 0;

    // Main loop
    while (numChildrenLaunched < totalChildren) {
        // Calculate clock increment based on the number of current children
        int clockIncrement = (numChildrenLaunched == 0) ? 250000000 : 250000000 / numChildrenLaunched;

        // Increment simulated clock
        nanoseconds += clockIncrement;

        // Output the process table every 0.5 seconds
        if (nanoseconds >= 500000000) {
            seconds++;
            nanoseconds -= 500000000;
            fprintf(logFile, "OSS: PID:%d SysClockS: %d SysClockNano: %d\nProcess Table:\nEntry Occupied PID StartS StartN\n",
                    getpid(), seconds, nanoseconds);

            for (int i = 0; i < 20; i++) {
                fprintf(logFile, "%-5d %-8d %-3d %-6d %-9d\n", i, processTable[i].occupied,
                        processTable[i].pid, processTable[i].startSeconds, processTable[i].startNano);
            }
            fprintf(logFile, "\n");
            fflush(logFile);
        }

        // Calculate next child launch time
        int nextLaunchTime = seconds * 1000000000 + nanoseconds + (rand() % interval_ms) * 1000000;

        // Launch child process if it's time
        if (nextLaunchTime >= timeLimit * 1000000000) {
            break;
        } else if (nextLaunchTime >= nextChild * 1000000000) {
            pid_t child_pid = fork();

            if (child_pid == -1) {
                perror("Fork failed");
                exit(EXIT_FAILURE);
            } else if (child_pid == 0) {
                // Execute worker process
                execl("./worker", "worker", argv[6], argv[7], (char *)NULL);
                perror("Exec failed");
                exit(EXIT_FAILURE);
            } else {
                // Update process table entry
                processTable[nextChild].occupied = 1;
                processTable[nextChild].pid = child_pid;
                processTable[nextChild].startSeconds = seconds;
                processTable[nextChild].startNano = nanoseconds;

                printf("OSS: Sending message to worker %d PID %d at time %d:%d\n",
                       nextChild, child_pid, seconds, nanoseconds);

                nextChild++;
                numChildrenLaunched++;
            }
        }

        // Check for child termination
        struct msgbuf message;
        if (msgrcv(msgid, &message, sizeof(message), 1, IPC_NOWAIT) != -1) {
            printf("OSS: Receiving message from worker %d PID %d at time %d:%d\n",
                   message.value, processTable[message.value].pid, seconds, nanoseconds);

            if (message.value == 0) {
                printf("OSS: Worker %d PID %d is planning to terminate.\n",
                       message.value, processTable[message.value].pid);

                // Update process table for terminated child
                processTable[message.value].occupied = 0;
                processTable[message.value].pid = 0;
                processTable[message.value].startSeconds = 0;
                processTable[message.value].startNano = 0;
            }
        }
    }

    // Wait for remaining children to terminate
    while (wait(NULL) > 0);

    // Remove message queue
    if (msgctl(msgid, IPC_RMID, NULL) == -1) {
        perror("Error removing message queue");
    }

    // Close log file
    fclose(logFile);

    return 0;
}
