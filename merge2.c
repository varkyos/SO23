#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_REGISTO_SIZE 17

int merge2(int fi1, int fi2, int fo) {
    char registo1[MAX_REGISTO_SIZE];
    char registo2[MAX_REGISTO_SIZE];

    ssize_t leu1 = read(fi1, registo1, MAX_REGISTO_SIZE);
    ssize_t leu2 = read(fi2, registo2, MAX_REGISTO_SIZE);

    //Ler enquanto pelo menos um dos ficheiros ainda tiver conteúdo
    while (leu1 > 0 && leu2 > 0) {
         /*Comparar e escrever o menor registo no ficheiro de saída
           Caso strcmp devolva < 0 registo1 < registo2 e vice versa*/
        if (strcmp(registo1, registo2) <= 0) 
        { //Em caso de igualdade, a ordem é indiferente
            write(fo, registo1, leu1);
            leu1 = read(fi1, registo1, MAX_REGISTO_SIZE);
        } else {
            write(fo, registo2, leu2);
            leu2 = read(fi2, registo2, MAX_REGISTO_SIZE);
        }
    }

    // Se restarem registos num dos ficheiros, escrever o restante
    // visto que já estará tudo ordenado
    while (leu1 > 0) {
        write(fo, registo1, leu1);
        leu1 = read(fi1, registo1, MAX_REGISTO_SIZE);
    }

    while (leu2 > 0) {
        write(fo, registo2, leu2);
        leu2 = read(fi2, registo2, MAX_REGISTO_SIZE);
    }

    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
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

    // Chamar a função merge2
    merge2(fi1, fi2, fo);

    // Fechar os ficheiros
    close(fi1);
    close(fi2);
    close(fo);

    return 0;
}