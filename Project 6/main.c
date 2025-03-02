#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>

#define NUM_CHILDREN 5
#define BUFFER_SIZE 256   // Buffer size for reading/writing messages
#define MSG_BUFFER 256

// Helper function: formats elapsed time (from start) as "m:ss.mmm"
void format_timestamp(struct timeval start, struct timeval now, char *buffer, size_t bufsize) {
    long seconds = now.tv_sec - start.tv_sec;
    long usec = now.tv_usec - start.tv_usec;
    if (usec < 0) {
        seconds--;
        usec += 1000000;
    }
    int minutes = seconds / 60;
    int sec = seconds % 60;
    int millis = usec / 1000;
    snprintf(buffer, bufsize, "%d:%02d.%03d", minutes, sec, millis);
}

int main(void) {
    int pipes[NUM_CHILDREN][2];
    pid_t pids[NUM_CHILDREN];
    struct timeval start_time;

    // Record common start time (time = 0 for the program)
    gettimeofday(&start_time, NULL);

    // Create one pipe per child
    for (int i = 0; i < NUM_CHILDREN; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    // Spawn five children
    for (int i = 0; i < NUM_CHILDREN; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            // CHILD PROCESS
            // Close unused pipe ends for all pipes.
            for (int j = 0; j < NUM_CHILDREN; j++) {
                if (j != i) {
                    close(pipes[j][0]);
                    close(pipes[j][1]);
                }
            }
            close(pipes[i][0]);  // Close read end for its own pipe

            if (i < 4) {
                // First four children: generate automatic messages.
                int msg_count = 1;
                srand(getpid());  // Seed random generator
                while (1) {
                    struct timeval current;
                    gettimeofday(&current, NULL);
                    double elapsed = (current.tv_sec - start_time.tv_sec) +
                                     (current.tv_usec - start_time.tv_usec) / 1000000.0;
                    if (elapsed >= 30.0)
                        break;
                    char timebuf[32];
                    format_timestamp(start_time, current, timebuf, sizeof(timebuf));

                    // Create message: child's timestamp, id and message counter.
                    char message[MSG_BUFFER];
                    snprintf(message, sizeof(message), "%s: Child %d message %d\n", 
                             timebuf, i + 1, msg_count);
                    write(pipes[i][1], message, strlen(message));
                    msg_count++;

                    // Sleep for a random time: 0, 1, or 2 seconds.
                    int sleep_time = rand() % 3;
                    sleep(sleep_time);
                }
            } else {
                // Fifth child: interactive process using select() to avoid blocking indefinitely.
                char input[MSG_BUFFER];
                while (1) {
                    struct timeval current;
                    gettimeofday(&current, NULL);
                    double elapsed = (current.tv_sec - start_time.tv_sec) +
                                     (current.tv_usec - start_time.tv_usec) / 1000000.0;
                    if (elapsed >= 30.0)
                        break;

                    // Prompt the user
                    printf("Enter message: ");
                    fflush(stdout);

                    // Use select() to wait for input on STDIN with a timeout.
                    fd_set readfds;
                    FD_ZERO(&readfds);
                    FD_SET(0, &readfds);
                    struct timeval tv;
                    tv.tv_sec = 1;  // Wait up to 1 second for input.
                    tv.tv_usec = 0;
                    int ret = select(1, &readfds, NULL, NULL, &tv);
                    if (ret < 0) {
                        perror("select in child 5");
                        break;
                    } else if (ret == 0) {
                        // Timeout: no input. Check the loop condition again.
                        continue;
                    }
                    // Input is ready.
                    if (fgets(input, sizeof(input), stdin) == NULL)
                        break;  // EOF or error

                    // Remove trailing newline if present.
                    size_t len = strlen(input);
                    if (len > 0 && input[len - 1] == '\n')
                        input[len - 1] = '\0';

                    gettimeofday(&current, NULL);  // Update current time after input.
                    char timebuf[32];
                    format_timestamp(start_time, current, timebuf, sizeof(timebuf));

                    // Format message: include child's id (5) and user input.
                    char message[MSG_BUFFER];
                    snprintf(message, sizeof(message), "%s: Child %d: %s\n", 
                             timebuf, i + 1, input);
                    write(pipes[i][1], message, strlen(message));
                }
            }
            close(pipes[i][1]);  // Close write end before exiting.
            exit(0);
        } else {
            // Parent process: record child pid.
            pids[i] = pid;
        }
    }

    // In the parent, close all write ends (we only read from pipes)
    for (int i = 0; i < NUM_CHILDREN; i++) {
        close(pipes[i][1]);
    }

    // Open output file for writing the collected messages.
    FILE *outfile = fopen("output.txt", "w");
    if (outfile == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    // Parent uses select() to multiplex input from all pipes.
    int activePipes = NUM_CHILDREN;
    fd_set readfds;
    char buffer[BUFFER_SIZE];
    while (activePipes > 0) {
        FD_ZERO(&readfds);
        int max_fd = 0;
        // Add active pipe read-ends to the fd_set.
        for (int i = 0; i < NUM_CHILDREN; i++) {
            if (pipes[i][0] != -1) {
                FD_SET(pipes[i][0], &readfds);
                if (pipes[i][0] > max_fd)
                    max_fd = pipes[i][0];
            }
        }
        // Set a timeout (1 second) for select.
        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        int ret = select(max_fd + 1, &readfds, NULL, NULL, &timeout);
        if (ret < 0) {
            perror("select");
            break;
        } else if (ret == 0) {
            // Timeout: no pipe ready; continue looping.
            continue;
        } else {
            // Check which pipes have data ready.
            for (int i = 0; i < NUM_CHILDREN; i++) {
                if (pipes[i][0] != -1 && FD_ISSET(pipes[i][0], &readfds)) {
                    int n = read(pipes[i][0], buffer, sizeof(buffer) - 1);
                    if (n > 0) {
                        buffer[n] = '\0';
                        struct timeval current;
                        gettimeofday(&current, NULL);
                        char parent_time[32];
                        format_timestamp(start_time, current, parent_time, sizeof(parent_time));

                        // Process the buffer line-by-line.
                        char *saveptr = NULL;
                        char *line = strtok_r(buffer, "\n", &saveptr);
                        while (line != NULL) {
                            fprintf(outfile, "%s %s\n", parent_time, line);
                            printf("%s %s\n", parent_time, line);
                            line = strtok_r(NULL, "\n", &saveptr);
                        }
                        fflush(outfile);
                    } else if (n == 0) {
                        // Pipe closed by child.
                        close(pipes[i][0]);
                        pipes[i][0] = -1;
                        activePipes--;
                    }
                }
            }
        }
    }

    fclose(outfile);

    // Wait for all children to terminate.
    for (int i = 0; i < NUM_CHILDREN; i++) {
        wait(NULL);
    }

    return 0;
}
