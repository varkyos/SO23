#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_REGISTO_SIZE 21

void sortFileInPlace(char* fileName) 
{
    //Esta função cria um processo filho para chamar execlp e ordenar os counteúdods do respetivo ficheiro
    int pipa[2];
    if(pipe(pipa) < 0)
    {
        perror("pipa");
        exit(EXIT_FAILURE);
    }
    pid_t pid = fork();
    if(pid < 0)
    {
        perror("Erro a forkar");
        exit(EXIT_FAILURE);
    }
    if(pid == 0)
    {
        dup2(pipa[1], STDOUT_FILENO);
        close(pipa[0]);
        printf("Antes de execlpar... %s  \n", fileName);
        if(execlp("sort", fileName, fileName, (char*)NULL) < 0)
        {
            perror("Error no execlp");
            exit(EXIT_FAILURE);
        }
        printf("Depois de execlpar...\n");
        exit(EXIT_SUCCESS);
    }else{
        close(pipa[1]);
    }
}

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

    // If there are remaining lines in one of the files, write the rest
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
    int fi[4];
    int* array[1] = {fi};
    printf("%i\n", argc);
    switch(argc) {
        case 5: {
            for (int i = 1; i < 5; i++)
            {
                sortFileInPlace(argv[i]);
                printf("Fiz %i ficheiros\n", i);
            }
            //preciso mandar os pipes como args da merge4
            for(int i = 0; i < 4; i++)
            {
                fi[i] = open(argv[i + 1], O_RDONLY | O_CREAT, 0664);
            }
            merge4(fi, STDOUT_FILENO);
        }break;
        
        case 2: {
            int fd = open(argv[1], O_RDONLY);
            if (fd == -1) {
                perror("Erro ao abrir o ficheiro");
                exit(EXIT_FAILURE);
            }

             //Contar o n de linhas
            int num_lines = 0;
            char buffer[MAX_REGISTO_SIZE];
            ssize_t bytes_read;

            while ((bytes_read = read(fd, buffer, MAX_REGISTO_SIZE)) > 0) {
                for (ssize_t i = 0; i < bytes_read; ++i) {
                    if (buffer[i] == '\n') {
                        num_lines++;
                    }
                }
            }

            // dar reset ao ponteiro
            lseek(fd, 0, SEEK_SET);

            // calcular n de linhas por parte
            int lines_per_part = num_lines / 4;
            int remainder_lines = num_lines % 4;

            // Create array to store file descriptors
            int fi[4];

            // Initialize file descriptors
            fi[0] = 0; // Initial file descriptor
            // Divide the file into 4 parts
            for (int i = 1; i <= 4; ++i) {
                int lines_to_read = lines_per_part + (i <= remainder_lines ? 1 : 0);

                for (int j = 0; j < lines_to_read; ++j) {
                    while ((bytes_read = read(fd, buffer, MAX_REGISTO_SIZE)) > 0) 
                    {
                        if (buffer[bytes_read - 1] == '\n') {
                            break;
                        }
                    }
                }

                // Criar um fd para a próxima parte
                fi[i - 1] = dup(fd);
            }
        }
        default: printf("Defaultei\n");
    }
    
    for (int i = 0; i < 4; ++i) {
        wait(NULL);
    }

    
    merge4(fi, STDOUT_FILENO);


    //Fechar os descritores
    for (int i = 0; i < 4; ++i) {
        close(fi[i]);
    }

    return 0;
}