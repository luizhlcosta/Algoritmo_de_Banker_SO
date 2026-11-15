#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>

int main(int argc, char *argv[]) {

    if (argc < 2) {
        fprintf(stderr, "Erro: informe os recursos.\n");
        return EXIT_FAILURE;
    }

    int numero_recursos = argc - 1;

    int *recursos = calloc(numero_recursos, sizeof(int));

    if (recursos == NULL) {
        fprintf(stderr, "Erro ao alocar memoria.\n");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < numero_recursos; i++) {

        char *fim;
        errno = 0;

        long valor = strtol(argv[i + 1], &fim, 10);

        if (errno == ERANGE || fim == argv[i + 1] ||
            *fim != '\0' || valor < 0 || valor > INT_MAX) {

            fprintf(stderr, "Erro: recurso invalido: %s\n",
                    argv[i + 1]);

            free(recursos);
            return EXIT_FAILURE;
        }

        recursos[i] = (int)valor;
    }

    printf("Quantidade de recursos: %d\n", numero_recursos);

    for (int i = 0; i < numero_recursos; i++) {
        printf("Recurso %d: %d instancias\n",
               i, recursos[i]);
    }

    free(recursos);

    return EXIT_SUCCESS;
}
