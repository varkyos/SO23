#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_REGISTO_SIZE 17

int merge2(int fi1, int fi2, int fo) {
    char registo1[MAX_REGISTO_SIZE];
    char registo2[MAX_REGISTO_SIZE];

    ssize_t leu1 = read(fi1, registo1, MAX_REGISTO_SIZE);
    ssize_t leu2 = read(fi2, registo2, MAX_REGISTO_SIZE);

    // Read while at least one of the files still has content
    while (leu1 > 0 && leu2 > 0) {
        // Compare and write the smaller record to the output file
        // In case of equality, the order is indifferent
        if (strcmp(registo1, registo2) <= 0) {
            write(fo, registo1, leu1);
            leu1 = read(fi1, registo1, MAX_REGISTO_SIZE);
        } else {
            write(fo, registo2, leu2);
            leu2 = read(fi2, registo2, MAX_REGISTO_SIZE);
        }
    }

    // If there are remaining records in one of the files, write the rest
    // since everything will already be sorted
    while (leu1 > 0) {
        write(fo, registo1, leu1);
        leu1 = read(fi1, registo1, MAX_REGISTO_SIZE);
    }

    while (leu2 > 0) {
        write(fo, registo2, leu2);
        leu2 = read(fi2, registo2, MAX_REGISTO_SIZE);
    }

    // Close the write ends after merging
    close(fi1);
    close(fi2);

    return 0;
}

void merge4(int fi[4], int fo) {
    int pipe1[2], pipe2[2], pipe3[2];
    pipe(pipe1);
    pipe(pipe2);
    pipe(pipe3);

    pid_t child1 = fork();

    if (child1 == 0) {
        // Inside child process 1
        close(pipe1[0]);
        close(pipe2[0]);
        close(pipe3[0]);
        merge2(fi[0], fi[1], pipe1[1]);  // Merge and write to the first pipe
        close(pipe1[1]);  // Close the write end after merging
        exit(0);
    } else {
        pid_t child2 = fork();

        if (child2 == 0) {
            // Inside child process 2
            close(pipe1[0]);
            close(pipe2[0]);
            close(pipe3[0]);
            merge2(fi[2], fi[3], pipe2[1]);  // Merge and write to the second pipe
            close(pipe2[1]);  // Close the write end after merging
            exit(0);
        } else {
            // Inside parent process
            close(pipe1[1]);
            close(pipe2[1]);

            merge2(pipe1[0], pipe2[0], pipe3[1]);  // Merge sorted results from both pipes to third pipe
            close(pipe3[1]);  // Close the write end after merging

            // Wait for both child processes to complete
            waitpid(child1, NULL, 0);
            waitpid(child2, NULL, 0);

            // Read from the third pipe and write to the final output file
            char buffer[MAX_REGISTO_SIZE];
            ssize_t bytesRead = read(pipe3[0], buffer, sizeof(buffer));

            while (bytesRead > 0) {
                write(fo, buffer, bytesRead);
                bytesRead = read(pipe3[0], buffer, sizeof(buffer));
            }

            // Close remaining pipe ends
            close(pipe3[0]);
            close(fo);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 6) {
        fprintf(stderr, "Usage: %s file1 file2 file3 file4\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int fi[4];

    // Open input files
    for (int i = 0; i < 4; ++i) {
        fi[i] = open(argv[i + 1], O_RDONLY);
        if (fi[i] == -1) {
            perror("open");
            exit(EXIT_FAILURE);
        }
    }

    // Open output file
    int fo = open(argv[argc - 1], O_WRONLY | O_CREAT | O_TRUNC, 0664);
    if (fo == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    merge4(fi, fo);

    // Close input files
    for (int i = 0; i < 4; ++i) {
        close(fi[i]);
    }

    return 0;
}
