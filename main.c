/*

Para compilar de dentro do VS Code:

Windows/Linux: CTRL+SHIFT+B (Terminal -> Run Build Task)
macOS: COMMAND+SHIFT+B

Para executar de dentro do VS Code:

Executar normalmente: CTRL+F5 (Run -> Run Without Debugging)
Debugar: F5 (Run -> Start Debugging)

Pelo terminal:

Windows: mingw32-make
Linux/macOS: make

Para executar:

./bagunceitor [arquivo com a imagem de entrada]

*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>  // Para usar strings
#include <time.h>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include <stb_image.h>
#include <stb_image_write.h>

// Um pixel Pixel (24 bits)
typedef struct {
  unsigned char r, g, b;
} Pixel;

// Uma imagem Pixel
typedef struct {
  int width, height;  // largura, altura
  int channels;       // qtd de canais (geralmente 3, RGB)
  Pixel* pixels;
} Img;

// As 2 imagens
Img in, out;

// Protótipos
void load(char* name, Img* pic);
void embaralhar(const Img *entrada, Img *saida, int limite, int iteracoes);
void recuperar(const Img *entrada, Img *saida, int limite, int iteracoes);

int main(int argc, char* argv[]) {
  if (argc < 2) {
    printf("bagunceitor [origem]\n");
    exit(1);
  }

  // Carrega a imagem original
  load(argv[1], &in);

  // Exibe as dimensões na tela, para conferência
  printf("Origem   : %s %d x %d\n", argv[1], in.width, in.height);

  printf("Processando...\n");

  // Cria imagem de saída e "zera" ela
  int tam = in.width * in.height;
  out = in;
  out.pixels = malloc(tam * sizeof(Pixel));
  memset(out.pixels, 0, tam * sizeof(Pixel));

  // Converte para interpretar como matrizes
  Pixel(*pin)[in.width] = (Pixel(*)[in.height])in.pixels;
  Pixel(*pout)[in.width] = (Pixel(*)[in.height])out.pixels;

  // Algoritmo de difusao

  // 0 = embaralhar 
  // 1 = recuperar
  int modo = 0;
  int limite = 128;
  int iteracoes = 8;

  if (modo == 0) {
    embaralhar(&in, &out, limite, iteracoes);
  } else {
    recuperar(&in, &out, limite, iteracoes);
  }

  // Grava a imagem como PNG para registro
  stbi_write_png("saida.png", out.width, out.height, 3, pout, 0);

  free(in.pixels);
  free(out.pixels);
}

void load(char* name, Img* pic) {
  pic->pixels =
      (Pixel*)stbi_load(name, &pic->width, &pic->height, &pic->channels, 0);
  if (!pic->pixels) {
    printf("Erro de leitura: %s\n", stbi_failure_reason());
    exit(1);
  }
  printf("Load: %d x %d x %d\n", pic->width, pic->height, pic->channels);
  // Exibe um bloco de 8 x 8 pixels em hexadecimal (teste)
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      printf("[%02X %02X %02X] ", pic->pixels[i].r, pic->pixels[i].g,
             pic->pixels[i].b);
    }
    printf("\n");
  }
}

void embaralhar(const Img *entrada, Img *saida, int limite, int iteracoes) {
  if (limite <= 0 || iteracoes <= 0) {
    printf("Limite e número de iterações devem ser maiores que 0.\n");
    exit(1);
  }

  int largura = entrada->width;
  int altura = entrada->height;
  int periodo = limite > 1 ? 2 * (limite - 1) : 1;

  // Copia a entrada para a saída
  const Pixel *origem = entrada->pixels;
  Pixel *destino = saida->pixels;

  for (int linha = 0; linha < altura; linha++) {
    for (int coluna = 0; coluna < largura; coluna++) {
      *destino = *origem;
      origem++;
      destino++;
    }
  }

  Pixel *temporario = malloc(sizeof(*temporario) * largura * altura);

  for (int repeticao = 0; repeticao < iteracoes; repeticao++) {
    // Desloca as linhas para a direita
    for (int linha = 0; linha < altura; linha++) {
      int movimento = 1 + abs(linha % periodo - (limite - 1));
      int deslocamento = movimento % largura;

      const Pixel *origemLinha = saida->pixels + linha * largura;
      Pixel *destinoLinha = temporario + linha * largura;

      for (int coluna = 0; coluna < largura; coluna++) {
        int novaColuna = (coluna + deslocamento) % largura;

        *(destinoLinha + novaColuna) = *(origemLinha + coluna);
      }
    }

    // Desloca as colunas para baixo
    for (int coluna = 0; coluna < largura; coluna++) {
      int movimento = 1 + abs(coluna % periodo - (limite - 1));
      int deslocamento = movimento % altura;

      const Pixel *origemColuna = temporario + coluna;
      Pixel *destinoColuna = saida->pixels + coluna;

      for (int linha = 0; linha < altura; linha++) {
        int novaLinha = (linha + deslocamento) % altura;

        *(destinoColuna + novaLinha * largura) = *(origemColuna + linha * largura);
      }
    }
  }

  free(temporario);
}

void recuperar(const Img *entrada, Img *saida, int limite, int iteracoes) {
  if (limite <= 0 || iteracoes <= 0) {
    printf("Limite e número de iterações devem ser maiores que 0.\n");
    exit(1);
  }

  int largura = entrada->width;
  int altura = entrada->height;
  int periodo = 2 * (limite - 1);

  // Copia a entrada para a saída
  if (entrada->pixels != saida->pixels) {
    const Pixel *origem = entrada->pixels;
    Pixel *destino = saida->pixels;

    for (int linha = 0; linha < altura; linha++) {
      for (int coluna = 0; coluna < largura; coluna++) {
        *destino = *origem;
        origem++;
        destino++;
      }
    }
  }

  Pixel *temporario = malloc(
      sizeof(*temporario) * largura * altura
  );

  for (int repeticao = 0; repeticao < iteracoes; repeticao++) {
    // Desfaz o deslocamento das colunas
    for (int coluna = 0; coluna < largura; coluna++) {
      int movimento = 1 + abs(coluna % periodo - (limite - 1));
      int deslocamento = movimento % altura;

      const Pixel *origemColuna = saida->pixels + coluna;
      Pixel *destinoColuna = temporario + coluna;

      for (int linha = 0; linha < altura; linha++) {
        int linhaDeslocada = (linha + deslocamento) % altura;

        *(destinoColuna + linha * largura) = *(origemColuna + linhaDeslocada * largura);
      }
    }

    // Desfaz o deslocamento das linhas
    for (int linha = 0; linha < altura; linha++) {
      int movimento = 1 + abs(linha % periodo - (limite - 1));
      int deslocamento = movimento % largura;

      const Pixel *origemLinha = temporario + linha * largura;
      Pixel *destinoLinha = saida->pixels + linha * largura;

      for (int coluna = 0; coluna < largura; coluna++) {
        int colunaDeslocada = (coluna + deslocamento) % largura;

        *(destinoLinha + coluna) = *(origemLinha + colunaDeslocada);
      }
    }
  }

  free(temporario);
}