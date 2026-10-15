# Implementações de confusão e difusão

O projeto trabalha com imagens RGB. Cada pixel é representado pela estrutura `Pixel`, com os campos `r`, `g` e `b`. A estrutura `Img` guarda as dimensões e o ponteiro para os pixels da imagem.

As duas implementações atuam de formas diferentes: a confusão transforma os valores dos canais, enquanto a difusão reorganiza as posições dos pixels.

## Programas gerados

O Makefile gera três programas:

| Programa | Arquivo-fonte | Funcionamento |
|---|---|---|
| `confusao` | `confusao.c` | Aplica ou desfaz a transformação dos valores dos canais RGB. |
| `difusao` | `difusao.c` | Aplica ou desfaz os deslocamentos das linhas e colunas. |
| `bagunceitor` | `main.c` | Une as duas implementações, aplicando confusão e depois difusão. |

No `bagunceitor`, a reconstrução segue a ordem contrária: primeiro desfaz a difusão e depois desfaz a confusão. Uma imagem intermediária armazena o resultado entre as etapas.

## Confusão

A implementação está em `confusao.c`, nas funções `confundir` e `desconfundir`.

### Transformação dos valores

A função `confundir` percorre a imagem como uma sequência de bytes. Cada byte corresponde a um dos canais R, G ou B.

O processamento começa com um estado inicial definido pela chave. Para cada byte, a função `proximaChave` atualiza esse estado:

```c
*estado = *estado * 1664525u + 1013904223u;
```

Do estado atualizado são extraídos três parâmetros:

- `k1`: valor somado ao byte.
- `giro`: quantidade de posições para rotacionar seus bits.
- `k2`: valor usado na operação XOR.

Cada byte passa por três operações:

```c
unsigned char v = (unsigned char)(*src + k1);
v = rol8(v, giro);
*dst = (unsigned char)(v ^ k2);
```

Primeiro, a soma é reduzida ao intervalo de 0 a 255. Depois, `rol8` rotaciona os bits para a esquerda: os bits que saem de uma extremidade reaparecem na outra. Por último, o XOR combina o resultado com `k2`.

Os ponteiros `src` e `dst` avançam um byte após cada transformação. Dessa forma, todos os canais são processados, mantendo suas posições na imagem.

### Recuperação dos valores

A função `desconfundir` começa com a mesma chave e gera os mesmos parâmetros para cada byte. Ela desfaz as operações na ordem contrária:

```c
unsigned char v = (unsigned char)(*src ^ k2);
v = ror8(v, giro);
*dst = (unsigned char)(v - k1);
```

O XOR é desfeito repetindo o mesmo XOR. A rotação para a esquerda é desfeita pela rotação para a direita, e a soma é desfeita pela subtração. Assim, cada canal recupera seu valor original.

## Difusão

A implementação está em `difusao.c`, nas funções `embaralhar` e `recuperar`. Ela move pixels inteiros, preservando seus valores RGB.

### Justificativa da solução

Na solução de difusão, buscou-se uma abordagem mais artística, voltada à criação de efeitos visuais pela reorganização dos pixels, sem foco em criptografia. Os deslocamentos triangulares foram escolhidos por produzirem padrões geométricos semelhantes a pirâmides, cuja aparência pode ser explorada variando o limite e o número de iterações. A reversibilidade permite desfazer essas transformações e reconstruir a imagem original, preservando suas cores.

### Deslocamentos triangulares

As funções recebem `limite`, que define o maior deslocamento, e `iteracoes`, que define quantas vezes o processo completo será aplicado.

O período da sequência é calculado por:

```c
int periodo = 2 * (limite - 1);
```

O deslocamento de cada linha ou coluna parte da expressão:

```c
int movimento = 1 + abs(indice % periodo - (limite - 1));
```

Com limite 3, a sequência é:

```text
3, 2, 1, 2, 3, 2, 1, 2...
```

O módulo pelo comprimento da faixa ajusta o movimento ao tamanho da linha ou coluna.

### Embaralhamento

A função `embaralhar` copia a entrada para a saída e cria um buffer temporário. Em cada iteração, executa duas etapas:

1. Desloca as linhas para a direita, gravando no temporário.
2. Desloca as colunas para baixo, gravando novamente na saída.

As posições de destino são calculadas por:

```c
int novaColuna = (coluna + deslocamento) % largura;
int novaLinha = (linha + deslocamento) % altura;
```

Os movimentos são circulares, ou seja, pixels que saem pela direita reaparecem à esquerda, e pixels que saem por baixo reaparecem no topo. A sequência triangular reinicia na primeira linha e na primeira coluna.

Os acessos usam aritmética de ponteiros. Dentro de uma linha, o avanço é de um pixel; dentro de uma coluna, é de `largura` pixels.

### Recuperação e iterações

A função `recuperar` desfaz primeiro as colunas e depois as linhas. Ela calcula os mesmos deslocamentos, lê os pixels das posições deslocadas e os escreve nas posições originais.

Cada iteração trabalha sobre o resultado anterior:

```text
Embaralhar: original → imagem 1 → imagem 2 → imagem 3
Recuperar:  imagem 3 → imagem 2 → imagem 1 → original
```

Usando o mesmo limite e número de iterações, cada passagem da recuperação desfaz uma passagem do embaralhamento. O buffer temporário é reutilizado nas repetições e liberado ao final da função.

## Alternância entre embaralhamento e reconstrução

Nos três programas, a variável `modo`, dentro de `main`, seleciona a operação:

```c
int modo = 0; // Aplica a transformação.
```

Para reconstruir a imagem, altere para:

```c
int modo = 1; // Desfaz a transformação.
```

Essa alteração deve ser feita no arquivo-fonte do programa que será executado: `confusao.c`, `difusao.c` ou `main.c`. Depois, recompile o programa.

Os parâmetros que controlam as transformações são:

| Parâmetro | Onde aparece | Finalidade |
|---|---|---|
| `modo` | Nos três programas | Seleciona transformação (`0`) ou reconstrução (`1`). |
| `CHAVE_CONFUSAO` | `confusao.c` e `main.c` | Define a semente que gera os parâmetros de transformação dos bytes. |
| `limite` | `difusao.c` e `main.c` | Define o maior deslocamento da sequência triangular. |
| `iteracoes` | `difusao.c` e `main.c` | Define quantas vezes os deslocamentos são aplicados. |

Na configuração atual, a chave é `0xC0FFEE42u`, o limite é `128` e o número de iterações é `8`. Para reconstruir, altere somente o `modo`, mantendo a mesma chave, limite e quantidade de iterações usados na transformação correspondente.

Por exemplo, para reconstruir uma imagem produzida pelo programa `difusao`:

1. Em `difusao.c`, altere `modo` de `0` para `1`.
2. Mantenha os valores de `limite` e `iteracoes` usados no embaralhamento.
3. Recompile com `make difusao`.
4. Execute passando a imagem embaralhada como entrada: `./difusao saida.png`.

O resultado é gravado em `saida.png`. O mesmo procedimento se aplica aos outros programas, usando o executável correspondente e mantendo seus parâmetros.

## Uso de inteligência artificial

Durante o desenvolvimento, foi utilizada inteligência artificial como ferramenta de apoio quando necessário, auxiliando no esclarecimento de dúvidas, na discussão da lógica dos algoritmos, na organização do código e na elaboração das explicações sobre as implementações.
