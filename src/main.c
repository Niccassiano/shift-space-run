#include <stdio.h>
#include <string.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>

#define LARGURA_TELA 1280
#define ALTURA_TELA  720
#define FPS 60

// tamanho de cada framer, fundo sala 01 
#define FRAME_LARG 128
#define FRAME_ALT  192

 // tamanho do personagem na tela 
#define DESENHO_LARG 72
#define DESENHO_ALT  108

// registro de cada jogador 
typedef struct {
    char nome[30];
    int vidas;       // comecs com 3 vidas 
    int pontos;      // 1 ponto por sala (caso nao perca vida) 
    int sala;        // sala (sala 01 agora) 
    float x;         // poisocao x e y 
    float y;
    float velocidade;
    int direcao;     // 0 = frente, 1 = esquerda, 2 = costas, 3 = direita
    int frame;       // 0 = parado, 1 e 2 = passos
} Jogador;

void iniciarJogador(Jogador *j) {
    strcpy(j->nome, "Prisioneiro");
    j->vidas = 3;
    j->pontos = 0;
    j->sala = 1;
    j->x = 640;
    j->y = 620;
    j->velocidade = 3.5;
    j->direcao = 0;
    j->frame = 0;
}
// inicia com uma funcao que carrega o ponteiro do jogo e suas vidas, pontos e etc de acordo 

// pega sprites do personagem e faz ele andar
int carregarSprites(ALLEGRO_BITMAP *sprites[3][3]) {
    char *nomes[3] = {"frente", "esquerda", "costas"};
    char caminho[100];
    int d, f;

    for (d = 0; d < 3; d++) {
        for (f = 0; f < 3; f++) {
            sprintf(caminho, "sprites/prisioneiro_%s_%d.png", nomes[d], f);
            sprites[d][f] = al_load_bitmap(caminho);
            if (sprites[d][f] == NULL) {
                return 0;
            }
        }
    }
    return 1;
}

void liberarSprites(ALLEGRO_BITMAP *sprites[3][3]) {
    int d, f;

    for (d = 0; d < 3; d++) {
        for (f = 0; f < 3; f++) {
            if (sprites[d][f] != NULL) {
                al_destroy_bitmap(sprites[d][f]);
            }
        }
    }
}

// desenhando o cenario e personagem
void desenharCenario(ALLEGRO_BITMAP *fundo) {
    al_draw_scaled_bitmap(fundo,
        0, 0, al_get_bitmap_width(fundo), al_get_bitmap_height(fundo),
        0, 0, LARGURA_TELA, ALTURA_TELA, 0);
}

void desenharJogador(Jogador *j, ALLEGRO_BITMAP *sprites[3][3]) {
    int linha = j->direcao;
    int flags = 0;

    /* pega o png do personagem e inverte a posicao para a direita */
    if (j->direcao == 3) {
        linha = 1;
        flags = ALLEGRO_FLIP_HORIZONTAL;
    }

    al_draw_scaled_bitmap(sprites[linha][j->frame],
        0, 0, FRAME_LARG, FRAME_ALT,
        j->x - DESENHO_LARG / 2, j->y - DESENHO_ALT,
        DESENHO_LARG, DESENHO_ALT, flags);
}

// main 

int main(void) {
    ALLEGRO_DISPLAY *tela = NULL;
    ALLEGRO_EVENT_QUEUE *fila = NULL;
    ALLEGRO_TIMER *timer = NULL;
    ALLEGRO_BITMAP *fundo = NULL;
    ALLEGRO_BITMAP *sprites[3][3] = {{NULL}};
    Jogador jogador;
    int rodando = 1;
    int redesenhar = 1;

    if (!al_init()) {
        return 1;
    }
    if (!al_init_image_addon()) {
        return 1;
    }
    if (!al_install_keyboard()) {
        return 1;
    }

    tela = al_create_display(LARGURA_TELA, ALTURA_TELA);
    timer = al_create_timer(1.0 / FPS);
    fila = al_create_event_queue();
    if (tela == NULL || timer == NULL || fila == NULL) {
        return 1;
    }
    al_set_window_title(tela, "Shift Space Run");

    fundo = al_load_bitmap("assets/sala01_fundo.png");
    if (fundo == NULL) {
        return 1;
    }
    if (!carregarSprites(sprites)) {
        liberarSprites(sprites);
        return 1;
    }

    al_register_event_source(fila, al_get_display_event_source(tela));
    al_register_event_source(fila, al_get_timer_event_source(timer));
    al_register_event_source(fila, al_get_keyboard_event_source());

    iniciarJogador(&jogador);
    al_start_timer(timer);

    while (rodando) {
        ALLEGRO_EVENT evento;
        al_wait_for_event(fila, &evento);

        if (evento.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            rodando = 0;
        }
        if (evento.type == ALLEGRO_EVENT_KEY_DOWN &&
            evento.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
            rodando = 0;
        }
        if (evento.type == ALLEGRO_EVENT_TIMER) {
            redesenhar = 1;
        }

        if (redesenhar && al_is_event_queue_empty(fila)) {
            redesenhar = 0;
            desenharCenario(fundo);
            desenharJogador(&jogador, sprites);
            al_flip_display();
        }
    }

    liberarSprites(sprites);
    al_destroy_bitmap(fundo);
    al_destroy_timer(timer);
    al_destroy_event_queue(fila);
    al_destroy_display(tela);
    return 0;
}