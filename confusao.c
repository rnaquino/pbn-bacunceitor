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

// [CONFUSAO] Chave secreta (semente do gerador de números)
#define CHAVE_CONFUSAO 0xC0FFEE42u

// Protótipos
void load(char* name, Img* pic);
void confundir(const Img* entrada, Img* saida, unsigned int chave);
void desconfundir(const Img* entrada, Img* saida, unsigned int chave);

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

  
  // 0 = confundir
  // 1 = desconfundir
  int modo = 0;

  // [CONFUSAO] teste: C seguido de C^-1 deve devolver a imagem original
  if (modo == 0) {
    Img teste = in;
    teste.pixels = malloc(tam * sizeof(Pixel));
    confundir(&in, &out, CHAVE_CONFUSAO);
    desconfundir(&out, &teste, CHAVE_CONFUSAO);
    printf("Confusao reversivel: %s\n",
           memcmp(in.pixels, teste.pixels, tam * sizeof(Pixel)) == 0 ? "SIM" : "NAO");
    free(teste.pixels);
  }

  if (modo == 0) {
    confundir(&in, &out, CHAVE_CONFUSAO);
  } else {
    desconfundir(&in, &out, CHAVE_CONFUSAO);
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

// =====================================================================
// [CONFUSAO] Substituição de valores dos bytes
// Para cada byte b: v = (b + k1) mod 256; v = rotaciona v g bits à
// esquerda; c = v XOR k2. k1, k2 e g vêm de um LCG iniciado pela chave.
// =====================================================================

static unsigned char rol8(unsigned char v, int n) {
  n &= 7;
  return (unsigned char)((v << n) | (v >> ((8 - n) & 7)));
}

static unsigned char ror8(unsigned char v, int n) {
  return rol8(v, 8 - (n & 7));
}

// Tem que ser idêntica na ida e na volta
static void proximaChave(unsigned int* estado, unsigned char* k1,
                         unsigned char* k2, int* giro) {
  *estado = *estado * 1664525u + 1013904223u;
  *k1 = (unsigned char)(*estado >> 24);
  *k2 = (unsigned char)(*estado >> 16);
  *giro = (int)((*estado >> 8) & 7u);
}

void confundir(const Img* entrada, Img* saida, unsigned int chave) {
  size_t total = (size_t)entrada->width * entrada->height * 3;
  const unsigned char* src = (const unsigned char*)entrada->pixels;
  unsigned char* dst = (unsigned char*)saida->pixels;
  const unsigned char* fim = src + total;
  unsigned int estado = chave;

  while (src < fim) {
    unsigned char k1, k2;
    int giro;
    proximaChave(&estado, &k1, &k2, &giro);

    unsigned char v = (unsigned char)(*src + k1);  // soma mod 256
    v = rol8(v, giro);                             // rotação
    *dst = (unsigned char)(v ^ k2);                // XOR

    src++;
    dst++;
  }
}

void desconfundir(const Img* entrada, Img* saida, unsigned int chave) {
  size_t total = (size_t)entrada->width * entrada->height * 3;
  const unsigned char* src = (const unsigned char*)entrada->pixels;
  unsigned char* dst = (unsigned char*)saida->pixels;
  const unsigned char* fim = src + total;
  unsigned int estado = chave;

  while (src < fim) {
    unsigned char k1, k2;
    int giro;
    proximaChave(&estado, &k1, &k2, &giro);

    unsigned char v = (unsigned char)(*src ^ k2);  // desfaz o XOR
    v = ror8(v, giro);                             // desfaz a rotação
    *dst = (unsigned char)(v - k1);                // desfaz a soma

    src++;
    dst++;
  }
}
