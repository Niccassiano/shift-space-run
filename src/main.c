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

/* fazer o personagem andar, definindo limites no chao  */
#define LIMITE_ESQ 40
#define LIMITE_DIR 1240
#define LIMITE_CIMA 500
#define LIMITE_BAIXO 700

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

    // fazer personagem andar
    int furtivo;     
    int contador;    
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

    // personagem andar
    j->furtivo = 0;
    j->contador = 0;
}
// inicia com uma funcao que carrega o ponteiro do jogo e suas vidas, pontos e etc 

// desenhando o cenario e personagem
void desenharCenario(ALLEGRO_BITMAP *fundo) {
    al_draw_scaled_bitmap(fundo,
        0, 0, al_get_bitmap_width(fundo), al_get_bitmap_height(fundo),
        0, 0, LARGURA_TELA, ALTURA_TELA, 0);
}

void desenharJogador(Jogador *j, ALLEGRO_BITMAP *sprites[3][3]) {
    int linha = j->direcao;
    int flags = 0;

    // pega o png do personagem e inverte a posicao para a direita
    if (j->direcao == 3) {
        linha = 1;
        flags = ALLEGRO_FLIP_HORIZONTAL;
    }

    al_draw_scaled_bitmap(sprites[linha][j->frame],
        0, 0, FRAME_LARG, FRAME_ALT,
        j->x - DESENHO_LARG / 2, j->y - DESENHO_ALT,
        DESENHO_LARG, DESENHO_ALT, flags);
}


// funcao para fazer o personagem andar 
// corrigir conforme o professor falou.
void moverJogador(Jogador *j, ALLEGRO_KEYBOARD_STATE *teclado) {
    float vel = j->velocidade;
    int andando = 0;
    int trocaFrame = 10;

    // segurando o shift = modo furtivo 
    if (al_key_down(teclado, ALLEGRO_KEY_LSHIFT) ||
        al_key_down(teclado, ALLEGRO_KEY_RSHIFT)) {
        j->furtivo = 1;
        vel = vel / 2;
        trocaFrame = 20;   
    } else {
        j->furtivo = 0;
    }

    if (al_key_down(teclado, ALLEGRO_KEY_LEFT) || al_key_down(teclado, ALLEGRO_KEY_A)) {
        j->x = j->x - vel;
        j->direcao = 1;
        andando = 1;
    }
    if (al_key_down(teclado, ALLEGRO_KEY_RIGHT) || al_key_down(teclado, ALLEGRO_KEY_D)) {
        j->x = j->x + vel;
        j->direcao = 3;
        andando = 1;
    }
    if (al_key_down(teclado, ALLEGRO_KEY_UP) || al_key_down(teclado, ALLEGRO_KEY_W)) {
        j->y = j->y - vel;
        j->direcao = 2;
        andando = 1;
    }
    if (al_key_down(teclado, ALLEGRO_KEY_DOWN) || al_key_down(teclado, ALLEGRO_KEY_S)) {
        j->y = j->y + vel;
        j->direcao = 0;
        andando = 1;
    }

    // limita a area de movimentacao do personagem para que ele nao saia
    if (j->x < LIMITE_ESQ) j->x = LIMITE_ESQ;
    if (j->x > LIMITE_DIR) j->x = LIMITE_DIR;
    if (j->y < LIMITE_CIMA) j->y = LIMITE_CIMA;
    if (j->y > LIMITE_BAIXO) j->y = LIMITE_BAIXO;

    // alterando os frames enquanto anda
    if (andando) {
        j->contador++;
        if (j->frame == 0) {
            j->frame = 1;
        }
        if (j->contador >= trocaFrame) {
            j->contador = 0;
            if (j->frame == 1) {
                j->frame = 2;
            } else {
                j->frame = 1;
            }
        }
    } else {
        j->frame = 0;
        j->contador = 0;
    }
}


// main 

int main(void) {
    ALLEGRO_DISPLAY *tela = NULL;
    ALLEGRO_EVENT_QUEUE *fila = NULL;
    ALLEGRO_TIMER *timer = NULL;
    ALLEGRO_BITMAP *fundo = NULL;
    ALLEGRO_BITMAP *spritesheet = NULL; // troca 9 matriz por apenas 1 sprite sheet
    Jogador jogador;
    int rodando = 1;
    int redesenhar = 1;
    // utilizando a biblioteca allegro para pegar o estado do teclado
    ALLEGRO_KEYBOARD_STATE teclado;

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

    // pega spritesheet do personagem e faz ele andar 
    spritesheet = al_load_bitmap("sprites/prisioneiro.png");
    if (spritesheet == NULL) {
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
            al_get_keyboard_state(&teclado);
            // faz o personagem andar de acordo com a tecla pressionada
            moverJogador(&jogador, &teclado);
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
    al_destroy_bitmap(spritesheet);
    al_destroy_bitmap(fundo);
    al_destroy_timer(timer);
    al_destroy_event_queue(fila);
    al_destroy_display(tela);
    return 0;
}