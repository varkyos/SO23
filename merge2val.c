#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define DEFAULT_BUFFER_SIZE 17408 // 1024 registos * 17
#define DEFAULT_BUFFER_REG_AMOUNT 1024

void merge2(const int fd1, const int fd2, const int fo) {
    char* c = getenv("M");
    int buffer_size, reg_amount;
    if (c != NULL) {
        buffer_size = atoi(c) * 17;
        reg_amount = atoi(c);
    } else {
        buffer_size = DEFAULT_BUFFER_SIZE;
        reg_amount = DEFAULT_BUFFER_REG_AMOUNT;
    }

    char buffer1[buffer_size];
    char buffer2[buffer_size];

    ssize_t bytes_read1, bytes_read2;
    //Enquanto ainda houver chars
    while ((bytes_read1 = read(fd1, buffer1, buffer_size)) > 0 &&
           (bytes_read2 = read(fd2, buffer2, buffer_size)) > 0) {

        int b1 = 0, b2 = 0;

        while (b1 < bytes_read1 && b2 < bytes_read2) {
            int cmp_result = strcmp(&buffer1[b1], &buffer2[b2]);

            if (cmp_result <= 0) {
                write(fo, &buffer1[b1], 17);
                b1 += 17;
            } else {
                write(fo, &buffer2[b2], 17);
                b2 += 17;
            }
        }

        // Write any remaining data in buffer1
        while (b1 < bytes_read1) {
            write(fo, &buffer1[b1], 17);
            b1 += 17;
        }

        // Write any remaining data in buffer2
        while (b2 < bytes_read2) {
            write(fo, &buffer2[b2], 17);
            b2 += 17;
        }
    }

    // Write any remaining data in fd1
    while ((bytes_read1 = read(fd1, buffer1, buffer_size)) > 0) {
        for (int b1 = 0; b1 < bytes_read1; b1 += 17) {
            write(fo, &buffer1[b1], 17);
        }
    }

    // Write any remaining data in fd2
    while ((bytes_read2 = read(fd2, buffer2, buffer_size)) > 0) {
        for (int b2 = 0; b2 < bytes_read2; b2 += 17) {
            write(fo, &buffer2[b2], 17);
        }
    }
}


int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("%d", argc);
        fprintf(stderr, "Uso: %s <fi1> <fi2> <fo>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Abrir ficheiros de entrada e saída
    int fi1 = open(argv[1], O_RDONLY);
    int fi2 = open(argv[2], O_RDONLY);
    int fo = open(argv[3], O_WRONLY | O_CREAT | O_TRUNC, 0664);

    if (fi1 == -1 || fi2 == -1 || fo == -1) {
        perror("Erro ao abrir os ficheiros");
        exit(EXIT_FAILURE);
    }

    
    merge2(fi1, fi2, fo);

    // Fechar os ficheiros
    close(fi1);
    close(fi2);
    close(fo);

    return 0;
}