# Algoritmo de Banker

Implementação do Algoritmo de Banker em linguagem C, desenvolvida para a disciplina de Infraestrutura de Software da CESAR School.

O programa realiza o gerenciamento de recursos, verificando se as solicitações dos clientes podem ser atendidas sem deixar o sistema em um estado inseguro.

## Arquivos do projeto

- `banker.c`: código-fonte do programa.
- `Makefile`: compilação e limpeza do projeto.
- `customer.txt`: quantidade máxima de recursos que cada cliente pode solicitar.
- `commands.txt`: comandos de solicitação, liberação e exibição do estado.
- `result.txt`: arquivo gerado com os resultados das operações.
- `evidencias.log`: registro das compilações e dos testes realizados.

## Compilação

No terminal Linux, dentro do diretório do projeto, execute:

```bash
make
```

Esse comando gera o executável `banker`.

## Execução

Execute o programa informando a quantidade de instâncias de cada tipo de recurso:

```bash
./banker 10 5 7
```

Nesse exemplo, o sistema possui três tipos de recursos, com 10, 5 e 7 instâncias, respectivamente.

Os arquivos `customer.txt` e `commands.txt` devem estar no diretório de execução.

## Comandos disponíveis

- `RQ`: solicita recursos para um cliente.
- `RL`: libera recursos de um cliente.
- `*`: exibe as matrizes Maximum, Allocation e Need, além dos recursos disponíveis.

Os comandos são lidos de `commands.txt`, e os resultados são armazenados em `result.txt`.

## Limpeza

Para remover o executável gerado:

```bash
make clean
```

## Tratamento de erros

O programa verifica entradas inválidas, problemas na leitura dos arquivos, solicitações acima do limite permitido e recursos insuficientes. Também utiliza o teste de segurança do Banker para evitar alocações que resultem em estados inseguros.