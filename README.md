# Atividade - CSV sem identificadores duplicados

Implementacao em C para inserir novas entradas no final de um CSV sem duplicar a coluna `Identificador`.

## Fonte dos dados

O trabalho usa como referencia o conjunto **Compras - Contratos** disponibilizado no Portal de Dados Abertos e a consulta publica de contratos do Compras.gov.br.

- Portal: https://dados.gov.br/dados/conjuntos-dados/compras_contratos
- Consulta usada para verificar a ordem dos identificadores: https://compras.dados.gov.br/contratos/v1/contratos?cnpj_contratada=05513573000132

Na consulta publica, os identificadores aparecem, por exemplo, como `17021450000032011`, `78481050000052008`, `15851550000082013`, `17021750000122009` e `15306350000132008`, nessa ordem, portanto nao estao ordenados.

No escopo do programa, `Identificador` e tratado como a chave unica que deve ser preservada sem duplicidade.

## Compilacao

```bash
make
```

Ou diretamente:

```bash
gcc -std=c11 -Wall -Wextra -Wpedantic -O2 src/main.c src/csv_utils.c src/sequential_search.c src/mergesort.c src/binary_search.c -o csv_unico
```

## Execucao

Os dois primeiros argumentos sao os exigidos pela atividade:

```bash
./csv_unico <novas.csv> <destino.csv>
```

Existe um terceiro argumento opcional somente para demonstrar cada implementacao:

```text
seq-it    busca sequencial iterativa
seq-rec   busca sequencial recursiva
merge-it  MergeSort iterativo + busca binaria iterativa
merge-rec  MergeSort recursivo + busca binaria recursiva
```

Exemplo:

```bash
./csv_unico data/novas_exemplo.csv data/destino_exemplo.csv merge-it
```

O programa:

1. localiza a coluna `Identificador` pelo cabecalho;
2. le as entradas novas;
3. rejeita identificadores ja existentes;
4. rejeita tambem duplicatas que aparecam dentro do proprio arquivo de novas entradas;
5. acrescenta no final do arquivo de destino apenas as entradas novas.

Na Solucao 2, os identificadores do destino sao carregados na RAM, ordenados com MergeSort e pesquisados por busca binaria. Ao aceitar uma nova entrada, a chave tambem e inserida na posicao correta do vetor ordenado para que as entradas seguintes continuem sendo verificadas.

## Fixture de teste

A base pequena em `data/` e apenas uma massa de teste reproduzivel. Ela nao substitui o CSV oficial.

Para executar quatro demonstracoes, faca uma copia do destino a cada vez, porque o programa altera o arquivo.

## Estrutura

- `src/`: implementacoes em C.
- `data/`: fixtures de teste.
- `docs/complexidade.md`: analise de complexidade.
- `relatorio/`: PDF da entrega.

## Publicacao

Consulte `PUBLICAR_NO_GITHUB.md` para publicar o projeto como um repositorio publico. O relatorio foi preparado para ser entregue em PDF; depois de publicar o codigo, substitua no relatorio o campo de repositorio pela URL final, caso deseje deixar o PDF totalmente preenchido.
