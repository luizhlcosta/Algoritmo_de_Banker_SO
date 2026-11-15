#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <ctype.h>
#include <sys/types.h>

int carregar_clientes(const char *nome_arquivo, int numero_recursos, const int *recursos, int **matriz_max, size_t *numero_clientes) {

    FILE *arquivo = fopen(nome_arquivo, "r");

    if (arquivo == NULL) {
        perror("Erro ao abrir customer.txt");
        return 0;
    }

    char *linha = NULL;
    size_t tamanho_linha = 0;
    ssize_t lidos;

    int *matriz = NULL;
    size_t clientes = 0;
    size_t linha_atual = 0;
    size_t colunas = (size_t)numero_recursos;

    while ((lidos = getline(&linha, &tamanho_linha, arquivo)) != -1) {
        (void)lidos;
        linha_atual++;

        if (clientes >= SIZE_MAX / sizeof(int) / colunas) {
            fprintf(stderr, "Erro: matriz muito grande.\n");
            goto erro;
        }

        int *nova = realloc(
            matriz,
            (clientes + 1) * colunas * sizeof(int)
        );

        if (nova == NULL) {
            fprintf(stderr, "Erro ao alocar matriz matriz_max.\n");
            goto erro;
        }

        matriz = nova;
        char *cursor = linha;

        for (int j = 0; j < numero_recursos; j++) {
            char *fim;
            errno = 0;

            long valor = strtol(cursor, &fim, 10);

            if (errno == ERANGE || fim == cursor || valor < 0 || valor > INT_MAX || valor > recursos[j]) {

                fprintf(stderr,
                        "Erro: valor invalido na linha %zu.\n",
                        linha_atual);
                goto erro;
            }

            while (*fim == ' ' || *fim == '\t') {
                fim++;
            }

            if (j < numero_recursos - 1) {
                if (*fim != ',') {
                    fprintf(stderr,
                            "Erro de formato na linha %zu.\n",
                            linha_atual);
                    goto erro;
                }
                cursor = fim + 1;
            } else {
                while (*fim != '\0' &&
                       isspace((unsigned char)*fim)) {
                    fim++;
                }

                if (*fim != '\0') {
                    fprintf(stderr,
                            "Erro de formato na linha %zu.\n",
                            linha_atual);
                    goto erro;
                }
            }

            matriz[clientes * colunas + (size_t)j] = (int)valor;
        }

        clientes++;
    }

    if (ferror(arquivo) || clientes == 0) {
        fprintf(stderr, "Erro ao ler customer.txt.\n");
        goto erro;
    }

    free(linha);
    fclose(arquivo);

    *matriz_max = matriz;
    *numero_clientes = clientes;

    return 1;

erro:
    free(linha);
    free(matriz);
    fclose(arquivo);
    return 0;
}

int main(int argc, char *argv[]) {

    if (argc < 2) {
        fprintf(stderr, "Erro: informe os recursos.\n");
        return EXIT_FAILURE;
    }

    int numero_recursos = argc - 1;

    int *recursos = calloc(
        (size_t)numero_recursos, sizeof(int)
    );

    if (recursos == NULL) {
        fprintf(stderr, "Erro ao alocar memoria.\n");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < numero_recursos; i++) {
        char *fim;
        errno = 0;

        long valor = strtol(argv[i + 1], &fim, 10);

        if (errno == ERANGE || fim == argv[i + 1] || *fim != '\0' || valor < 0 || valor > INT_MAX) {

            fprintf(stderr, "Erro: recurso invalido: %s\n",
                    argv[i + 1]);

            free(recursos);
            return EXIT_FAILURE;
        }

        recursos[i] = (int)valor;
    }

    int *matriz_max = NULL;
    size_t numero_clientes = 0;

    if (!carregar_clientes("customer.txt", numero_recursos, recursos, &matriz_max, &numero_clientes)) {

        free(recursos);
        return EXIT_FAILURE;
    }

    printf("Clientes encontrados: %zu\n", numero_clientes);
    printf("Tipos de recursos: %d\n", numero_recursos);

    printf("\nMatriz matriz_max:\n");

    for (size_t i = 0; i < numero_clientes; i++) {
        for (int j = 0; j < numero_recursos; j++) {
            printf("%d ",
                   matriz_max[i * (size_t)numero_recursos + (size_t)j]);
        }
        printf("\n");
    }

    free(matriz_max);
    free(recursos);

    return EXIT_SUCCESS;
}
