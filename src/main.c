#include <stdio.h>
#include <string.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>

#define LARGURA_TELA 1280
#define ALTURA_TELA  720
#define FPS 60

// tamanho de cada framer, fundo sala 01 
#define FRAME_LARG 128
#define FRAME_ALT  192

 // tamanho do personagem na tela 
#define DESENHO_LARG 72
#define DESENHO_ALT  108

// fazer o personagem andar, definindo limites no chao  
#define LIMITE_ESQ 40
#define LIMITE_DIR 1240
#define LIMITE_CIMA 500
#define LIMITE_BAIXO 700

// tamanho do cenario, 10 salas, cada sala tem 1280x720
#define TOTAL_SALAS 10
// define quanto tempo o personagem ficara com o efeito visual de flash vermelho
#define FLASH_DURACAO 20   // frames do flash vermelho (~0,33 s a 60 FPS)
// o efeito visual de flash vermelho vai durar 20 frames -> 20/60 = 0,33 segundos que o personagem ficara com o efeito
// 60 frames = 1 segundo
// 30 frames = 0,5 segundo
// 20 frames = aproximadamente 0,33 segundo
// 10 frames = aproximadamente 0,16 segundo

// ele aciona um contador de framer
// início	20
// 1ª	19
// 2ª	18
// 3ª	17
// 10ª	10
// 19ª	1
// 20ª	0 -> quando chega a zero, o efeito termina.   

#define MAX_OBJETOS 12 // quantidade maxima de objetos que o jogador pode interagir na sala 01
#define RAIO_INTERACAO 60 // distancia para poder interagir
#define ACERTOS_NECESSARIOS 3 // quantidade de acertos necessarios para passar de sala

#define ESTADO_JOGANDO   0 // andando 
#define ESTADO_LENDO     1 // caixa de texto aberta, personagem parado
#define ESTADO_ESCOLHA   2 // escolhendo uma opcao
#define ESTADO_RESULTADO 3 // resultado da escolha

#define OBJ_CARTAZ    0 // cartaz que o jogador pode interagir e ler
#define OBJ_FRAGMENTO 1 // fragmento que o jogador pode pegar para ganhar pontos

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

    // efeito visual, contador de frames restantes do flash vermelho 
    int flash;       
} Jogador;

// objeto que o jogador pode interagir, ler e pegar pontos
typedef struct {
    int tipo;            // OBJ_CARTAZ ou OBJ_FRAGMENTO
    float x, y, raio;    // ponto de interacao
    int verdadeiro;      // fragmento verdadeiro ou falso
    int resolvido;       // fragmento ja foi resolvido, para nao contar pontos varias vezes
    char titulo[48];
    char texto[320];
    char explicacao[320];
} Objeto;

// o cartaz esta na parede, se o y do objeto e menor que 500, o jogador nao consegue interagir com ele, pois esta muito longe do chao 
// entao o x e o do cartaz e o y e 500, para que o jogador consiga interagir com ele, mesmo que o cartaz esteja na parede, pois o personagem esta no chao
typedef struct {
    int estado;
    Objeto objetos[MAX_OBJETOS];
// o estado e o que impede o personagem de andar enquanto esta lendo um cartaz
    int qtdObjetos;
    int objetoAtivo;     // -1 se nenhum, objeto sendo lido
    // objetoAtivo e o indice do cartaz que o jogador esta lendo, para que o texto do cartaz seja exibido na tela
    int tempo;           // conta frames, para fechar
    int acertos;
    int vidasPerdidas;   // vai servir para a pontuacao final, caso o jogador perca vidas, ele nao ganha pontos
    int opcao;           // 0 = FATO, 1 = PROPAGANDA
    int acertouUltimo;
    int reiniciar;       // 1 se a ultima resposta zerou as vidas
} Sala1;

// parei passo 3 Cadastro dos objetos

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

    // efeito visual de flash vermelho
    j->flash = 0;
}
// inicia com uma funcao que carrega o ponteiro do jogo e suas vidas, pontos e etc 

// desenhando o cenario e personagem
void desenharCenario(ALLEGRO_BITMAP *fundo) {
    al_draw_scaled_bitmap(fundo,
        0, 0, al_get_bitmap_width(fundo), al_get_bitmap_height(fundo),
        0, 0, LARGURA_TELA, ALTURA_TELA, 0);
}

// o recorte e calculado pela linha e coluna 
// sx = coluna × FRAME_LARG -> coluna e o frame 
// sy = linha × FRAME_ALT -> linha e a direcao do personagem

// ALLEGRO_BITMAP *spritesheet -> imagem do personagem
// Linha Direcao
// 0	frente
// 1	esquerda
// 2	costas

// Se j->direcao for igual a 2, o allegro vai selecionar a linha 2
void desenharJogador(Jogador *j, ALLEGRO_BITMAP *spritesheet) {
    int linha = j->direcao;
    int flags = 0; // qual transformacao ele deve aplicar na imagem antes de desenha la na tela
    
    // direita reaproveita a linha da esquerda, vira a imagem para a direita
    if (j->direcao == 3) {
        linha = 1;
        flags = ALLEGRO_FLIP_HORIZONTAL;
    }

    int recorteX = j->frame * FRAME_LARG;   // qual coluna que sera desenhada
    int recorteY = linha * FRAME_ALT;       // qual linha que sera desenhada

    al_draw_scaled_bitmap(spritesheet,
        recorteX, recorteY, FRAME_LARG, FRAME_ALT,           // seleciona o recorte da imagem
        j->x - DESENHO_LARG / 2, j->y - DESENHO_ALT,         // destino na tela, basicamente onde sera desenhado
        DESENHO_LARG, DESENHO_ALT, flags); // tamanho do personagem na tela e flags para virar a imagem
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

// posicao inical da sala -> hub e vidas
void reiniciarSala(Jogador *j) {
    j->vidas = 3;
    j->x = 640;
    j->y = 620;
    j->direcao = 0;
    j->frame = 0;
    j->contador = 0;
}

// erro ou deteccao custa 1 vida, com flash vermelho
// devolve 1 se as vidas acabaram e a sala foi reiniciada
int perderVida(Jogador *j) {
    j->vidas--;
    j->flash = FLASH_DURACAO;

    if (j->vidas <= 0) {
        reiniciarSala(j);
        return 1;
    }
    return 0; // 0 caso contrario
}

void desenharHUD(Jogador *j, ALLEGRO_FONT *fonte) {
    int i;

    // cartao de vidas no canto superior esquerdo seperando o header
    al_draw_filled_rounded_rectangle(18, 12, 158, 82, 10, 10,
        al_map_rgba(9, 12, 16, 220));
    al_draw_rounded_rectangle(18, 12, 158, 82, 10, 10,
        al_map_rgba(210, 190, 150, 150), 1);
    al_draw_text(fonte, al_map_rgb(210, 190, 150), 31, 17, 0, "VIDAS");

    // cheio = vida restantes, apenas contorno -> vida perdida
    for (i = 0; i < 3; i++) {
        float cx = 43 + i * 42;
        float cy = 58;
        if (i < j->vidas) {
            al_draw_filled_circle(cx, cy, 9, al_map_rgb(185, 42, 35));
        }
        al_draw_circle(cx, cy, 9, al_map_rgb(238, 224, 196), 2);
    }

    // identificador da sala no canto superior direito
    al_draw_filled_rounded_rectangle(1121, 12, 1262, 82, 10, 10,
            al_map_rgba(9, 12, 16, 220));
    al_draw_rounded_rectangle(1121, 12, 1262, 82, 10, 10,
            al_map_rgba(210, 190, 150, 150), 1);
    al_draw_textf(fonte, al_map_rgb(238, 224, 196),
            1250, 31, ALLEGRO_ALIGN_RIGHT,
            "%d/%d", j->sala, TOTAL_SALAS);
}

// header do jogo, com o nome do jogo e a frase de destaque
void desenharIdentidade(ALLEGRO_FONT *fonte_titulo, ALLEGRO_FONT *fonte_descricao) {
    const char *nome = "SHIFT SPACE RUN";
    const char *destaque = "SPACE";
    const char *prefixo = "SHIFT ";
    const char *frase = "INFILTRE-SE  /  COLETE FRAGMENTOS  /  DESCUBRA A HISTÓRIA";
    int larguraNome = al_get_text_width(fonte_titulo, nome);
    int larguraPrefixo = al_get_text_width(fonte_titulo, prefixo);
    float inicio = (LARGURA_TELA - larguraNome) / 2.0f;

    // reserva uma area propria para o titulo
    al_draw_filled_rounded_rectangle(174, 9, 1106, 96, 12, 12,
                                     al_map_rgba(9, 12, 16, 225));
    al_draw_rounded_rectangle(174, 9, 1106, 96, 12, 12,
                              al_map_rgba(210, 190, 150, 170), 1);

    // logo com sombra e destaque no SPACE
    al_draw_text(fonte_titulo, al_map_rgba(0, 0, 0, 220), inicio + 2, 15, 0, nome);
    al_draw_text(fonte_titulo, al_map_rgb(244, 232, 207), inicio, 13, 0, nome);
    al_draw_text(fonte_titulo, al_map_rgb(190, 45, 37),
                 inicio + larguraPrefixo, 13, 0, destaque);
    al_draw_text(fonte_descricao, al_map_rgb(220, 202, 166),
                 LARGURA_TELA / 2, 62, ALLEGRO_ALIGN_CENTRE, frase);
    al_draw_filled_rectangle(516, 89, 764, 91, al_map_rgb(166, 42, 34));
}

// flash vermelho por cima de tudo
void desenharFlash(Jogador *j) {
    if (j->flash > 0) {
        // vai diminuindo conforme o flash acaba 
        int alfa = (j->flash * 140) / FLASH_DURACAO;
        al_draw_filled_rectangle(0, 0, LARGURA_TELA, ALTURA_TELA,
                                 al_map_rgba(180, 0, 0, alfa));
        j->flash--;
    }
}

// sala 01, com cartazes que o jogador pode interagir e ler
void adicionarObjeto(Sala1 *s, int tipo, float x, float y, int verdadeiro,
                     const char *titulo, const char *texto, const char *explicacao) {
    Objeto *o = &s->objetos[s->qtdObjetos];
    o->tipo = tipo;
    o->x = x;
    o->y = y;
    o->raio = RAIO_INTERACAO;
    o->verdadeiro = verdadeiro;
    o->resolvido = 0;
    strcpy(o->titulo, titulo);
    strcpy(o->texto, texto);
    strcpy(o->explicacao, explicacao);
    s->qtdObjetos++;
}

// adicionando cartazes na sala 01, com posicao x e y e o texto que sera exibido quando o jogador interagir com ele
// trocar lugar de cada objeto 
void iniciarSala1(Sala1 *s) {
    s->estado = ESTADO_JOGANDO;
    s->qtdObjetos = 0;
    s->objetoAtivo = -1;
    s->tempo = 0;
    s->acertos = 0;
    s->vidasPerdidas = 0;
    s->opcao = 0;
    s->acertouUltimo = 0;
    s->reiniciar = 0;


    // CARTAZ PARA ELE CONSEGUIR ESTUDAR E PEGAR OS FRAGMENTOS, COM O TEXTO DE PROPAGANDA DO PARTIDO
    adicionarObjeto(s, OBJ_CARTAZ, 140, 560, 0, "Cartaz do partido",
        "'UM POVO, UM PAÍS, UM LÍDER.' Soa como união. Mas quem decidia quem fazia parte desse 'povo'? Guarde essa pergunta.",
        "");
    adicionarObjeto(s, OBJ_CARTAZ, 425, 650, 0, "Cartaz do partido",
        "'ORDEM E PROSPERIDADE PARA TODOS.' Está escrito na parede de um lugar onde ninguém é livre. Slogan não é prova: procure documentos.",
        "");
    adicionarObjeto(s, OBJ_CARTAZ, 1090, 560, 0, "Cartaz do partido",
        "'O LÍDER SABE. OUÇA E CONFIE.' Quem escuta uma voz só deixa de comparar. Regra de quem quer sobreviver: desconfie do que não tem data, fonte ou documento.",
        "");

    // FRAGMENTOS VERDADEIROS (com base nas fontes do USHMM (Museu do Holocausto dos EUA))
    adicionarObjeto(s, OBJ_FRAGMENTO, 235, 650, 1, "Jornal, 30 de janeiro de 1933",
        "O presidente Hindenburg nomeia Adolf Hitler chanceler. Na última eleição, o partido nazista teve cerca de um terço dos votos, longe da maioria.",
        "Hitler foi nomeado, não eleito. Chegou ao poder por um acordo entre políticos conservadores, e o partido nunca teve a maioria dos votos.");
    adicionarObjeto(s, OBJ_FRAGMENTO, 520, 560, 1, "Decreto de 13 de março de 1933",
        "Cria-se o Ministério da Propaganda, entregue a Joseph Goebbels. Rádio, imprensa, cinema e teatro passam a ficar sob a autoridade dele.",
        "Em poucos meses, o governo passou a decidir o que os alemães liam, ouviam e viam. Controlar a informação foi um dos primeiros passos da ditadura.");
    adicionarObjeto(s, OBJ_FRAGMENTO, 900, 560, 1, "Congresso do Partido, Nuremberg",
        "Todo ano, o partido tomava Nuremberg com desfiles, bandeiras e discursos encenados para impressionar. Em setembro de 1935, foi nesse palco que Hitler anunciou novas leis.",
        "Os congressos eram propaganda em forma de espetáculo, feitos para passar força e união. As Leis de Nuremberg foram anunciadas durante o congresso de 1935.");
    adicionarObjeto(s, OBJ_FRAGMENTO, 805, 650, 1, "Lei da Cidadania do Reich, 15/09/1935",
        "Só é cidadão quem tem 'sangue alemão'. Os judeus deixam de ser cidadãos e viram 'súditos do Estado', sem direitos políticos.",
        "Centenas de milhares de pessoas que eram alemãs perderam a cidadania e os direitos políticos, quatro anos antes de a guerra começar.");
    adicionarObjeto(s, OBJ_FRAGMENTO, 995, 650, 1, "Decreto de 14 de novembro de 1935",
        "Em geral, quem tinha três ou quatro avós judeus era considerado judeu pela lei, mesmo sem praticar a religião ou tendo se convertido a outra.",
        "A lei definiu por descendência: nada que a pessoa acreditasse ou fizesse mudava a classificação. Era perseguição pelo que se é, não pelo que se faz.");

    // FRAGMENTOS FALSOS
    adicionarObjeto(s, OBJ_FRAGMENTO, 330, 560, 0, "Cartaz do partido (sem data)",
        "'O POVO ALEMÃO ESCOLHEU O SEU LÍDER! Hitler venceu as eleições com a maioria dos votos.'",
        "Nas eleições de 1932 e 1933 o partido não teve a maioria dos votos, e Hitler assumiu por nomeação. Repare: o cartaz não cita data nem documento.");
    adicionarObjeto(s, OBJ_FRAGMENTO, 615, 650, 0, "Nota oficial (sem fonte)",
        "A imprensa alemã continua livre. Cada jornal publica o que quiser, sem ordens do governo.",
        "O Ministério da Propaganda mandava instruções diárias aos jornais sobre o que publicar e o que calar. Jornais de oposição foram fechados.");
    adicionarObjeto(s, OBJ_FRAGMENTO, 710, 560, 0, "Nota oficial (sem fonte)",
        "As Leis de Nuremberg protegem todos os alemães e garantem os mesmos direitos a cada cidadão.",
        "A lei fez o contrário: tirou a cidadania dos judeus e proibiu casamentos entre judeus e outros alemães. Propaganda costuma dar nome bonito ao que exclui.");
}
// 300, 560
// 750, 560
// 1200, 560



// devolve o indice do objeto mais perto do jogador, ou -1 se nao tem nenhum -> ver IndiceObjeto.txt
int objetoProximo(Jogador *j, Sala1 *s) {
    // os objetos ficam num vetor de struct, e objetoProximo percorre esse vetor com um for, 
    // verificando a distancia do jogador para cada objeto, e se a distancia for menor que o raio de interacao do objeto, 
    // entao o jogador esta perto do objeto e pode interagir com ele
    int i, melhor = -1;
    float menor = 0;
    for (i = 0; i < s->qtdObjetos; i++) {
        float dx = j->x - s->objetos[i].x;
        float dy = j->y - s->objetos[i].y;
        float d = dx * dx + dy * dy;
        if (d <= s->objetos[i].raio * s->objetos[i].raio) {
            if (melhor == -1 || d < menor) {
                melhor = i;
                menor = d;
            }
        }
    }
    return melhor;
}

// confimar escolha para saber de fato se acertou
void confirmarEscolha(Sala1 *s, Jogador *j) {
    Objeto *o = &s->objetos[s->objetoAtivo];
    int acertou;

    if (s->opcao == 0) {
        acertou = o->verdadeiro;        // marcou FATO
    } else {
        acertou = !o->verdadeiro;       // marcou PROPAGANDA
    }

    o->resolvido = 1;
    s->acertouUltimo = acertou;

    if (acertou) {
        s->acertos++;
    } else {
        s->vidasPerdidas++;
        s->reiniciar = perderVida(j);
    }
    s->estado = ESTADO_RESULTADO;
}

// chamada UMA vez a cada aperto da tecla ESPACO, para interagir com o objeto mais proximo do jogador, ou fechar a caixa de texto, ou confirmar a escolha
void interagir(Sala1 *s, Jogador *j) {
    if (s->estado == ESTADO_JOGANDO) {
        int i = objetoProximo(j, s);
        if (i != -1) {
            s->objetoAtivo = i;
            s->estado = ESTADO_LENDO;
        }
    } else if (s->estado == ESTADO_LENDO) {
        Objeto *o = &s->objetos[s->objetoAtivo];
        if (o->tipo == OBJ_FRAGMENTO && !o->resolvido) {
            s->opcao = 0;
            s->estado = ESTADO_ESCOLHA;
        } else {
            s->estado = ESTADO_JOGANDO;
        }
    } else if (s->estado == ESTADO_ESCOLHA) {
        confirmarEscolha(s, j);
    } else if (s->estado == ESTADO_RESULTADO) {
        if (s->reiniciar) {
            iniciarSala1(s);        // vidas zeraram -> sala recomeca TODA do zero
        } else {
            s->estado = ESTADO_JOGANDO;
        }
    }
}

// setas esquerda/direita -> quando tiver em opcao de ESCOLHA
void escolherOpcao(Sala1 *s, int opcao) {
    if (s->estado == ESTADO_ESCOLHA) {
        s->opcao = opcao;
    }
}

// funcao de "ESPACO" piscando quando tiver um objeto por perto
void desenharIcone(Jogador *j, Sala1 *s, ALLEGRO_FONT *fonte) {
    if (s->estado != ESTADO_JOGANDO) return;
    if (objetoProximo(j, s) == -1) return;

    // pisca para o jogador clicar na tecla ESPACO
    if ((s->tempo / 30) % 2 == 0) {
        float x = j->x;
        float y = j->y - DESENHO_ALT - 40;
        al_draw_filled_rectangle(x - 45, y, x + 45, y + 28, al_map_rgb(20, 20, 20));
        al_draw_rectangle(x - 45, y, x + 45, y + 28, al_map_rgb(230, 230, 230), 2);
        al_draw_text(fonte, al_map_rgb(230, 230, 230), x, y + 4,
                     ALLEGRO_ALIGN_CENTER, "ESPAÇO");
    }
}
// funcao de desenhar caixa de texto do cartaz que o jogador esta lendo, com o texto do cartaz
void desenharCaixaTexto(Sala1 *s, ALLEGRO_FONT *fonte) {
    if (s->estado != ESTADO_LENDO) return;

    // caixa na parte de baixo da tela, para nao atrapalhar o personagem, com o texto do cartaz que o jogador esta lendo
    al_draw_filled_rectangle(100, 540, LARGURA_TELA - 100, 690, al_map_rgb(15, 15, 15));
    al_draw_rectangle(100, 540, LARGURA_TELA - 100, 690, al_map_rgb(230, 230, 230), 3);

    al_draw_multiline_text(fonte, al_map_rgb(230, 230, 230),
            125, 560, LARGURA_TELA - 250, 28, 0,
            s->objetos[s->objetoAtivo].texto);

    al_draw_text(fonte, al_map_rgb(150, 150, 150),
            LARGURA_TELA - 125, 660, ALLEGRO_ALIGN_RIGHT,
            "[ESPAÇO] fechar");
}

// main 

int main(void) {
    ALLEGRO_DISPLAY *tela = NULL;
    ALLEGRO_EVENT_QUEUE *fila = NULL;
    ALLEGRO_TIMER *timer = NULL;
    ALLEGRO_BITMAP *fundo = NULL;
    ALLEGRO_BITMAP *spritesheet = NULL; // troca 9 matriz por apenas 1 sprite sheet
    ALLEGRO_FONT *fonte = NULL;
    ALLEGRO_FONT *fonte_titulo = NULL; 
    ALLEGRO_FONT *fonte_descricao = NULL;
    Jogador jogador;
    Sala1 sala;
    int rodando = 1;
    int redesenhar = 1;
    // utilizando a biblioteca allegro para pegar o estado do teclado
    ALLEGRO_KEYBOARD_STATE teclado;

    if (!al_init()) {
        fprintf(stderr, "erro da funcao al_init()\n");
        return 1;
    }
    if (!al_init_image_addon()) {
        fprintf(stderr, "erro da funcao al_init_image_addon()\n");
        return 1;
    }
    if (!al_init_primitives_addon()) {
        fprintf(stderr, "erro da funcao al_init_primitives_addon()\n");
        return 1;
    }
    if (!al_init_font_addon()) {
        fprintf(stderr, "erro da funcao al_init_font_addon() -> fonte\n");
        return 1;
    }
    if (!al_init_ttf_addon()) {
        fprintf(stderr, "erro da funcao al_init_ttf_addon() -> fonte \n");
        return 1;
    }
    if (!al_install_keyboard()) {
        fprintf(stderr, "erro da funcao al_install_keyboard()\n");
        return 1;
    }

    tela = al_create_display(LARGURA_TELA, ALTURA_TELA);
    timer = al_create_timer(1.0 / FPS);
    fila = al_create_event_queue();
    if (tela == NULL) {
        fprintf(stderr, "erro da funcao al_create_display()\n");
        return 1;
    }
    if (timer == NULL) {
        fprintf(stderr, "erro da funcao al_create_timer()\n");
        al_destroy_display(tela);
        return 1;
    }
    if (fila == NULL) {
        fprintf(stderr, "erro da funcao al_create_event_queue()\n");
        al_destroy_timer(timer);
        al_destroy_display(tela);
        return 1;
    }
    al_set_window_title(tela, "Shift Space Run");

    fundo = al_load_bitmap("assets/sala01_fundo.png");
    if (fundo == NULL) {
        fprintf(stderr, "erro da funcao al_load_bitmap() para o fundo\n");
        al_destroy_event_queue(fila);
        al_destroy_timer(timer);
        al_destroy_display(tela);
        return 1;
    }

    // pega spritesheet do personagem e faz ele andar 
    spritesheet = al_load_bitmap("sprites/prisioneiro.png");
    if (spritesheet == NULL) {
        fprintf(stderr, "erro da funcao al_load_bitmap() para o spritesheet\n");
        al_destroy_bitmap(fundo);
        al_destroy_event_queue(fila);
        al_destroy_timer(timer);
        al_destroy_display(tela);
        return 1;
    }

    // carrega a fonte do titulo
    fonte_titulo = al_load_ttf_font("fonts/hermes-1943/HERMES 1943.ttf", 40, 0);
    if (fonte_titulo == NULL) {
        fprintf(stderr, "erro da funcao al_load_ttf_font() para a fonte do titulo\n");
        al_destroy_bitmap(spritesheet);
        al_destroy_bitmap(fundo);
        al_destroy_event_queue(fila);
        al_destroy_timer(timer);
        al_destroy_display(tela);
        return 1;
    }

    // carrega a fonte da descricao
    fonte_descricao = al_load_ttf_font("fonts/courier_polski_1941/zai_CourierPolski1941.ttf", 18, 0);
    if (fonte_descricao == NULL) {
        fprintf(stderr, "erro da funcao al_load_ttf_font() para a fonte da descricao\n");
        al_destroy_font(fonte_titulo);
        al_destroy_bitmap(spritesheet);
        al_destroy_bitmap(fundo);
        al_destroy_event_queue(fila);
        al_destroy_timer(timer);
        al_destroy_display(tela);
        return 1;
    }

    // carrega a fonte usada pelo HUD (vidas e sala)
    fonte = al_load_ttf_font("fonts/courier_polski_1941/zai_CourierPolski1941.ttf", 20, 0);
    if (fonte == NULL) {
        fprintf(stderr, "erro da funcao al_load_ttf_font() para a fonte do HUD\n");
        al_destroy_font(fonte_descricao);
        al_destroy_font(fonte_titulo);
        al_destroy_bitmap(spritesheet);
        al_destroy_bitmap(fundo);
        al_destroy_event_queue(fila);
        al_destroy_timer(timer);
        al_destroy_display(tela);
        return 1;
    }

    // removido carregarSprites e liberarSprites, pois nao ha mais necessidade de carregar 9 imagens separadas

    al_register_event_source(fila, al_get_display_event_source(tela));
    al_register_event_source(fila, al_get_timer_event_source(timer));
    al_register_event_source(fila, al_get_keyboard_event_source());

    iniciarJogador(&jogador);
    iniciarSala1(&sala);
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
        if (evento.type == ALLEGRO_EVENT_KEY_DOWN &&
            evento.keyboard.keycode == ALLEGRO_KEY_SPACE) {
            interagir(&sala, &jogador);
        }
        if (evento.type == ALLEGRO_EVENT_TIMER) {
            sala.tempo++;
            if (sala.estado == ESTADO_JOGANDO) {
                al_get_keyboard_state(&teclado);
                // faz o personagem andar de acordo com a tecla pressionada
                moverJogador(&jogador, &teclado);
            } else {
                jogador.frame = 0;
            }
            redesenhar = 1;
        }

        if (redesenhar && al_is_event_queue_empty(fila)) {
            redesenhar = 0;
            desenharCenario(fundo);
            desenharJogador(&jogador, spritesheet);
            desenharIcone(&jogador, &sala, fonte);
            desenharHUD(&jogador, fonte);
            desenharCaixaTexto(&sala, fonte);
            desenharIdentidade(fonte_titulo, fonte_descricao);
            desenharFlash(&jogador);
            al_flip_display();
        }
    }

    al_destroy_font(fonte);
    al_destroy_font(fonte_descricao);
    al_destroy_font(fonte_titulo);
    al_destroy_bitmap(spritesheet);
    al_destroy_bitmap(fundo);
    al_destroy_timer(timer);
    al_destroy_event_queue(fila);
    al_destroy_display(tela);
    return 0;
}