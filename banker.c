
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <ctype.h>
#include <sys/types.h>

typedef struct Banker {
    int numero_recursos;
    size_t numero_clientes;

    int *recursos;
    int *disponiveis;

    int *matriz_max;
    int *matriz_alocacao;
    int *matriz_necessidade;
} Banker;


typedef enum {
    SOLICITACAO_ACEITA,
    LIMITE_EXCEDIDO,
    RECURSOS_INSUFICIENTES,
    ESTADO_INSEGURO,
    ERRO_SOLICITACAO
} ResultadoSolicitacao;

int inicializar_banker(Banker *banker) {

    size_t clientes = banker->numero_clientes;
    size_t recursos = (size_t)banker->numero_recursos;

    if (recursos == 0 || clientes == 0 || clientes > SIZE_MAX / recursos || clientes * recursos > SIZE_MAX / sizeof(int)) {
        fprintf(stderr, "Erro: dimensoes invalidas.\n");
        return 0;
    }

    banker->disponiveis = calloc(recursos, sizeof(int));
    banker->matriz_alocacao = calloc(clientes * recursos, sizeof(int));
    banker->matriz_necessidade = calloc(clientes * recursos, sizeof(int));

    if (banker->disponiveis == NULL || banker->matriz_alocacao == NULL || banker->matriz_necessidade == NULL) {

        fprintf(stderr, "Erro ao alocar estruturas do Banker.\n");
        return 0;
    }

    for (size_t j = 0; j < recursos; j++) {
        banker->disponiveis[j] = banker->recursos[j];
    }

    for (size_t i = 0; i < clientes * recursos; i++) {
        banker->matriz_necessidade[i] = banker->matriz_max[i];
    }

    return 1;
}

void liberar_banker(Banker *banker) {

    free(banker->recursos);
    free(banker->disponiveis);
    free(banker->matriz_max);
    free(banker->matriz_alocacao);
    free(banker->matriz_necessidade);
}


int carregar_clientes(const char *nome_arquivo, int numero_recursos, const int *recursos, int **matriz_max, size_t *numero_clientes) {

    FILE *arquivo = fopen(nome_arquivo, "r");

    if (arquivo == NULL) {
        printf("Erro ao abrir o arquivo customer.txt\n");
        return 0;
    }

    char *linha = NULL;
    size_t tamanho = 0;
    int *matriz = NULL;
    size_t clientes = 0;
    int erro = 0;

    while (getline(&linha, &tamanho, arquivo) != -1) {

        if (clientes >= SIZE_MAX / sizeof(int) / (size_t)numero_recursos) {
            printf("Erro: muitos clientes.\n");
            erro = 1;
            break;
        }

        int *nova_matriz = realloc(matriz, (clientes + 1) * numero_recursos * sizeof(int));

        if (nova_matriz == NULL) {
            printf("Erro ao alocar memoria.\n");
            erro = 1;
            break;
        }

        matriz = nova_matriz;

        char *posicao = linha;

        for (int j = 0; j < numero_recursos; j++) {

            char *fim;
            errno = 0;

            long valor = strtol(posicao, &fim, 10);

            if (errno == ERANGE || posicao == fim || valor < 0 || valor > INT_MAX || valor > recursos[j]) {
                printf("Valor invalido na linha %zu.\n", clientes + 1);
                erro = 1;
                break;
            }

            matriz[clientes * numero_recursos + j] = (int)valor;

            if (j < numero_recursos - 1) {

                while (*fim == ' ' || *fim == '\t') {
                    fim++;
                }

                if (*fim != ',') {
                    printf("Erro de formato na linha %zu.\n", clientes + 1);
                    erro = 1;
                    break;
                }

                posicao = fim + 1;

            } else {

                while (*fim != '\0' && isspace((unsigned char)*fim)) {
                    fim++;
                }

                if (*fim != '\0') {
                    printf("Erro de formato na linha %zu.\n", clientes + 1);
                    erro = 1;
                    break;
                }
            }
        }

        if (erro) {
            break;
        }

        clientes++;
    }

    if (ferror(arquivo) || clientes == 0) {
        printf("Erro na leitura do arquivo.\n");
        erro = 1;
    }

    free(linha);
    fclose(arquivo);

    if (erro) {
        free(matriz);
        return 0;
    }

    *matriz_max = matriz;
    *numero_clientes = clientes;

    return 1;
}

int verificar_seguranca(Banker *banker) {

    size_t clientes = banker->numero_clientes;
    int recursos = banker->numero_recursos;

    int *trabalho = malloc(recursos * sizeof(int));
    int *finalizados = calloc(clientes, sizeof(int));

    if (trabalho == NULL || finalizados == NULL) {
        fprintf(stderr, "Erro ao alocar memoria.\n");
        free(trabalho);
        free(finalizados);
        return -1;
    }

    for (int j = 0; j < recursos; j++) {
        trabalho[j] = banker->disponiveis[j];
    }

    int progresso = 1;

    while (progresso) {
        progresso = 0;

        for (size_t i = 0; i < clientes; i++) {
            if (finalizados[i]) {
                continue;
            }

            int pode_finalizar = 1;

            for (int j = 0; j < recursos; j++) {
                if (banker->matriz_necessidade[i * recursos + j] > trabalho[j]) {
                    pode_finalizar = 0;
                    break;
                }
            }

            if (pode_finalizar) {
                for (int j = 0; j < recursos; j++) {
                    trabalho[j] += banker->matriz_necessidade[i * recursos + j];
                }

                finalizados[i] = 1;
                progresso = 1;
            }
        }
    }

    int seguro = 1;

    for (size_t i = 0; i < clientes; i++) {
        if (!finalizados[i]) {
            seguro = 0;
            break;
        }
    }

    free(trabalho);
    free(finalizados);

    return seguro;
}


int solicitar_recursos(Banker *banker, size_t cliente, const int *recursos_solicitados) {

    if (cliente >= banker->numero_clientes || recursos_solicitados == NULL) {
        return ERRO_SOLICITACAO;
    }

    int recursos = banker->numero_recursos;

    for (int j = 0; j < recursos; j++) {

        size_t index = cliente * (size_t)recursos + (size_t)j;

        if (recursos_solicitados[j] < 0) {
            return ERRO_SOLICITACAO;
        }

        if (recursos_solicitados[j] > banker->matriz_necessidade[index]) {
            return LIMITE_EXCEDIDO;
        }

        if (recursos_solicitados[j] > banker->disponiveis[j]) {
            return RECURSOS_INSUFICIENTES;
        }
    }

    for (int j = 0; j < recursos; j++) {

        size_t index = cliente * (size_t)recursos + (size_t)j;

        banker->disponiveis[j] -= recursos_solicitados[j];
        banker->matriz_alocacao[index] += recursos_solicitados[j];
        banker->matriz_necessidade[index] -= recursos_solicitados[j];
    }

    int resultado = verificar_seguranca(banker);

    if (resultado != 1) {

        for (int j = 0; j < recursos; j++) {

            size_t index = cliente * (size_t)recursos + (size_t)j;

            banker->disponiveis[j] += recursos_solicitados[j];
            banker->matriz_alocacao[index] -= recursos_solicitados[j];
            banker->matriz_necessidade[index] += recursos_solicitados[j];
        }

        if (resultado == -1) {
            return ERRO_SOLICITACAO;
        }

        return ESTADO_INSEGURO;
    }

    return SOLICITACAO_ACEITA;
}


int main(int argc, char *argv[]) {

    Banker banker = {0};

    if (argc < 2) {
        fprintf(stderr, "Erro: informe os recursos.\n");
        return EXIT_FAILURE;
    }

    int numero_recursos = argc - 1;

    int *recursos = calloc((size_t)numero_recursos, sizeof(int));

    if (recursos == NULL) {
        fprintf(stderr, "Erro ao alocar memoria.\n");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < numero_recursos; i++) {
        char *fim;
        errno = 0;

        long valor = strtol(argv[i + 1], &fim, 10);

        if (errno == ERANGE || fim == argv[i + 1] || *fim != '\0' || valor < 0 || valor > INT_MAX) {

            fprintf(stderr, "Erro: recurso invalido: %s\n", argv[i + 1]);

            free(recursos);
            return EXIT_FAILURE;
        }

        recursos[i] = (int)valor;
    }

    banker.numero_recursos = numero_recursos;
    banker.recursos = recursos;

    if (!carregar_clientes("customer.txt", banker.numero_recursos, banker.recursos, &banker.matriz_max, &banker.numero_clientes)) {

        liberar_banker(&banker);
        return EXIT_FAILURE;
    }

    if (!inicializar_banker(&banker)) {

        liberar_banker(&banker);
        return EXIT_FAILURE;
    }

    printf("Clientes encontrados: %zu\n", banker.numero_clientes);
    printf("Tipos de recursos: %d\n", banker.numero_recursos);

    printf("\nMatriz Maximum:\n");

    for (size_t i = 0; i < banker.numero_clientes; i++) {
        for (int j = 0; j < banker.numero_recursos; j++) {
            printf("%d ", banker.matriz_max[i * (size_t)banker.numero_recursos + (size_t)j]);
        }
        printf("\n");
    }

    printf("\nRecursos disponiveis:\n");

    for (int j = 0; j < banker.numero_recursos; j++) {
        printf("%d ", banker.disponiveis[j]);
    }
    printf("\n");

    printf("\nMatriz Allocation:\n");

    for (size_t i = 0; i < banker.numero_clientes; i++) {
        for (int j = 0; j < banker.numero_recursos; j++) {
            printf("%d ", banker.matriz_alocacao[i * (size_t)banker.numero_recursos + (size_t)j]);
        }
        printf("\n");
    }

    printf("\nMatriz Need:\n");

    for (size_t i = 0; i < banker.numero_clientes; i++) {
        for (int j = 0; j < banker.numero_recursos; j++) {
            printf("%d ", banker.matriz_necessidade[i * (size_t)banker.numero_recursos + (size_t)j]);
        }
        printf("\n");
    }

    liberar_banker(&banker);

    return EXIT_SUCCESS;
}
