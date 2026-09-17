# Complexidade

Considere `n` como o numero de registros no destino e `k` como o numero de entradas novas.

## Solucao 1 - busca sequencial

### Iterativa
- Melhor caso por busca: O(1), quando a chave esta no primeiro registro.
- Caso medio: O(n).
- Pior caso por busca: O(n), quando a chave esta no final ou nao existe.
- Espaco auxiliar: O(1), desconsiderando o armazenamento do arquivo em disco.

### Recursiva
- Tempo melhor/medio/pior: O(1), O(n) e O(n) por busca.
- Espaco auxiliar: O(n) no pior caso devido a pilha de chamadas recursivas.

Para `k` entradas, no pior caso a busca repetida custa O(kn) como aproximacao. Como entradas aceitas sao anexadas e tambem passam a ser procuradas, uma forma mais precisa e O(k(n+k)) para a etapa de verificacao quando todas as comparacoes percorrem o prefixo completo.

## Solucao 2 - MergeSort + busca binaria

### MergeSort iterativo
- Tempo: O(n log n) em todos os casos.
- Espaco auxiliar: O(n) para o vetor auxiliar.
- Pilha de recursao: O(1).

### MergeSort recursivo
- Tempo: O(n log n) em todos os casos.
- Espaco auxiliar: O(n) para o vetor auxiliar + O(log n) de pilha.

### Busca binaria iterativa
- Melhor caso: O(1).
- Caso medio: O(log n).
- Pior caso: O(log n).
- Espaco auxiliar: O(1).

### Busca binaria recursiva
- Melhor caso: O(1).
- Caso medio/pior: O(log n).
- Espaco auxiliar: O(log n) de pilha.

Depois da ordenacao, a busca das `k` entradas custa O(k log n). O programa tambem precisa manter o vetor ordenado quando uma nova chave e aceita; a insercao por deslocamento custa O(n) no pior caso por entrada. Assim, para o processamento completo, uma analise conservadora e O(n log n + kn + k log n).

## Comparacao

A principal diferenca didatica entre as solucoes esta na busca: O(n) na sequencial contra O(log n) na binaria depois da ordenacao. A Solucao 2 troca mais memoria e uma etapa inicial de ordenacao por buscas muito menores. A implementacao recursiva de cada algoritmo adiciona custo de pilha, enquanto a iterativa nao usa essa pilha adicional.
