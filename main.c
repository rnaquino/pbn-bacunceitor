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
void embaralhar(const Img *entrada, Img *saida, int limite);
void recuperar(const Img *entrada, Img *saida, int limite);

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
  int limite = 50;

  if (modo == 0) {
    embaralhar(&in, &out, limite);
  } else {
    recuperar(&in, &out, limite);
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

void embaralhar(const Img *entrada, Img *saida, int limite) {
  int largura = entrada->width;
  int altura = entrada->height;

  Pixel *temporario = malloc(
      sizeof(*temporario) * largura * altura
  );

  int periodo = limite > 1 ? 2 * (limite - 1) : 1;

  for (int etapa = 0; etapa < 2; etapa++) {
    // Primeiro linhas, depois colunas
    int moverLinhas = etapa == 0;

    const Pixel *origem =
        etapa == 0 ? entrada->pixels : temporario;

    Pixel *destino =
        etapa == 0 ? temporario : saida->pixels;

    int totalFaixas = moverLinhas ? altura : largura;
    int comprimento = moverLinhas ? largura : altura;
    int passo = moverLinhas ? 1 : largura;

    for (int faixa = 0; faixa < totalFaixas; faixa++) {
      int inicio = moverLinhas ? faixa * largura : faixa;

      int posicao = faixa % periodo;
      int movimento;

      if (posicao < limite) {
        movimento = limite - posicao;
      } else {
        movimento = posicao - limite + 2;
      }

      int deslocamento = movimento % comprimento;

      for (int i = 0; i < comprimento; i++) {
        int novaPosicao = (i + deslocamento) % comprimento;

        destino[inicio + novaPosicao * passo] =
            origem[inicio + i * passo];
      }
    }
  }

  free(temporario);
}

void recuperar(const Img *entrada, Img *saida, int limite) {
  int largura = entrada->width;
  int altura = entrada->height;

  Pixel *temporario = malloc(
      sizeof(*temporario) * largura * altura
  );

  int periodo = limite > 1 ? 2 * (limite - 1) : 1;

  for (int etapa = 0; etapa < 2; etapa++) {
    // Primeiro desfaz as colunas, depois as linhas
    int moverLinhas = etapa == 1;

    const Pixel *origem =
        etapa == 0 ? entrada->pixels : temporario;

    Pixel *destino =
        etapa == 0 ? temporario : saida->pixels;

    int totalFaixas = moverLinhas ? altura : largura;
    int comprimento = moverLinhas ? largura : altura;
    int passo = moverLinhas ? 1 : largura;

    for (int faixa = 0; faixa < totalFaixas; faixa++) {
      int inicio = moverLinhas ? faixa * largura : faixa;

      int posicao = faixa % periodo;
      int movimento;

      if (posicao < limite) {
        movimento = limite - posicao;
      } else {
        movimento = posicao - limite + 2;
      }

      int deslocamento = movimento % comprimento;

      // Inverte o sentido do movimento
      deslocamento =
          (comprimento - deslocamento) % comprimento;

      for (int i = 0; i < comprimento; i++) {
        int novaPosicao = (i + deslocamento) % comprimento;

        destino[inicio + novaPosicao * passo] =
            origem[inicio + i * passo];
      }
    }
  }

  free(temporario);
}
