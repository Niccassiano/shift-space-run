#include <stdio.h>
#include <string.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>

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
#define LIMITE_CIMA_SALA2 250
#define LIMITE_BAIXO 700

// tamanho do cenario, 6 salas, cada sala tem 1280x720
#define TOTAL_SALAS 6
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

#define MAX_OBJETOS 14     // quantidade maxima de objetos que o jogador pode interagir na sala 01
#define RAIO_INTERACAO 60 // distancia para poder interagir
#define ACERTOS_NECESSARIOS 3 // quantidade de acertos necessarios para passar de sala

#define ESTADO_JOGANDO   0 // andando 
#define ESTADO_LENDO     1 // caixa de texto aberta, personagem parado
#define ESTADO_ESCOLHA   2 // escolhendo uma opcao
#define ESTADO_RESULTADO 3 // resultado da escolha

#define OBJ_CARTAZ    0 // cartaz que o jogador pode interagir e ler
#define OBJ_FRAGMENTO 1 // fragmento que o jogador pode pegar para ganhar pontos

#define ESTADO_PROVA     4        // pergunta final no pulpito
#define ESTADO_CARD      5        // cartao "Voce sabia?"
#define ESTADO_PORTA     6        // som da porta mais tela escurecendo
#define ESTADO_CONCLUIDA 7        // fim da sala 1

#define OBJ_PODIO 2               // pulpito (prova final)

#define TOTAL_OPCOES_PROVA 3
#define OPCAO_CORRETA_PROVA 1     // indice da resposta certa (GABARITO A SEGUNDA E A CORRETA)
#define PORTA_DURACAO 180         // frames da transicao (3 segundos a 60 FPS)
#define ABERTURA_TITULO 95        // duracao do titulo antes de escurecer (~0,75 s)
#define ABERTURA_FADE 30          // duracao de cada fade (~0,3 s)

// define SALA 02

#define ARQ_FUNDO_SALA2 "assets/sala02_fundo.png"
#define ARQ_MAPA_SALA2  "sprites/mapaSala2.png"
#define ARQ_PINOS       "sprites/pinos.png"
#define ARQ_PAPEIS      "sprites/papeis.png"

// quantidades
#define TOTAL_RELATORIOS 3
#define TOTAL_CIDADES    3
#define INDICE_MESA      3     // serve pra saber ser ele ta interagindo com a mesa ou o papel

// interacao 
#define RAIO_RELATORIO 90
#define MESA_X    777          // onde o jogador fica em frente a mesa do mapa
#define MESA_Y    640
#define MESA_RAIO 130

// papeis desenhados 
#define PAPEL_LARG 84
#define PAPEL_ALT  54

// mapa desenhado na tela, com os pinos que o jogador deve colocar
#define MAPA_LARG  780
#define MAPA_POS_Y 105
#define PINO_TAM   40          // tamanho do pino desenhado na tela

// pinos
#define BANDEJA_FY  1.10f
#define PINO_VEL_X  0.006f     // quanto o pino anda ao segurar a seta
#define PINO_VEL_Y  0.011f
#define TOL_PINO_X  0.05f
#define TOL_PINO_Y  0.08f

//  tempos em frames, 60 = 1 segundo
#define ESPERA_PINOS   90      // tempo mostrando o resultado depois do 3o pino
#define FADE_ENTRADA   45      //  ao entrar na sala 2
#define PORTA2_DURACAO 180     //  para preto ao sair da sala 2

// estado sala 02
#define ESTADO2_JOGANDO   0    // andando
#define ESTADO2_LENDO     1    // caixa de texto aberta
#define ESTADO2_MAPA      2    // mapa aberto, colocando pinos
#define ESTADO2_CARD      3    // cartao "Voce sabia?"
#define ESTADO2_PORTA     4    // tela escurecendo
#define ESTADO2_CONCLUIDA 5    // fim da sala 2


// cria uma variavel global para a pergunta final e as opcoes de resposta, para que o jogador possa interagir com o pulpito e responder a pergunta final
static const char *PERGUNTA_FINAL =
    "Em 1935, as Leis de Nuremberg foram anunciadas. O que elas fizeram?";

static const char *OPCOES_FINAL[TOTAL_OPCOES_PROVA] = {
    "Garantiram os mesmos direitos a todos os alemães.",
    "Tiraram a cidadania dos judeus alemães e criaram regras de exclusão.",
    "Marcaram o início da guerra contra a Polônia."
};

static const char *CARD_TEXTO =
    "Em 1935, as Leis de Nuremberg tiraram a cidadania dos judeus alemães. "
    "Um decreto do mesmo ano definiu, por ascendência, quem seria tratado como judeu. "
    "A exclusão virou lei quatro anos antes de a guerra começar.";


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

// cartazes ficam nas paredes, mas o ponto de interacao fica no chao: y=560 ou y=650, dentro da faixa 500 a 700
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
    int tempoPorta;      // frames desde que a porta comecou a abrir
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
    int limiteCima = j->sala >= 2 ? LIMITE_CIMA_SALA2 : LIMITE_CIMA;

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
    if (j->y < limiteCima) j->y = limiteCima;
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

// tela inicial apenas o titulo, seguido de fade
void desenharTelaInicial(ALLEGRO_FONT *fonte_titulo, int quadro) {
    int alfa = 255;
    int inicioFade = ABERTURA_TITULO;
    int fimFade = ABERTURA_TITULO + ABERTURA_FADE;

    al_clear_to_color(al_map_rgb(0, 0, 0));
    if (quadro >= inicioFade) {
        alfa = 255 * (fimFade - quadro) / ABERTURA_FADE;
        if (alfa < 0) alfa = 0;
    }
    al_draw_text(fonte_titulo, al_map_rgba(238, 224, 196, alfa),
                 LARGURA_TELA / 2, ALTURA_TELA / 2 - 24,
                 ALLEGRO_ALIGN_CENTER, "Shift Space Run");
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
    s->tempoPorta = 0;


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
    adicionarObjeto(s, OBJ_FRAGMENTO, 700, 650, 0, "Nota oficial (sem fonte)",
        "A imprensa alemã continua livre. Cada jornal publica o que quiser, sem ordens do governo.",
        "O Ministério da Propaganda mandava instruções diárias aos jornais sobre o que publicar e o que calar. Jornais de oposição foram fechados.");
    adicionarObjeto(s, OBJ_FRAGMENTO, 710, 560, 0, "Nota oficial (sem fonte)",
        "As Leis de Nuremberg protegem todos os alemães e garantem os mesmos direitos a cada cidadão.",
        "A lei fez o contrário: tirou a cidadania dos judeus e proibiu casamentos entre judeus e outros alemães. Propaganda costuma dar nome bonito ao que exclui.");

    // jogador responde a pergunta final no pulpito, com base nos fragmentos que ele leu e pegou, so quando ele tiver 3/3
    adicionarObjeto(s, OBJ_PODIO, 640, 505, 0, "Púlpito",
        "O púlpito está fechado. Antes de subir, prove que sabe separar fato de propaganda: examine os documentos da sala.",
        "Releia os documentos. Pista: o que dizia a Lei da Cidadania do Reich, de 1935? E a nota oficial que prometia 'direitos iguais'?");

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

// setas cima/baixo trocam a alternativa da pergunta final para o jogador escolher, e a funcao mudarOpcaoProva eh chamada a cada aperto de tecla para mudar a opcao selecionada
void mudarOpcaoProva(Sala1 *s, int passo) {
    if (s->estado != ESTADO_PROVA) return;
    s->opcao = s->opcao + passo;
    if (s->opcao < 0) s->opcao = TOTAL_OPCOES_PROVA - 1;
    if (s->opcao >= TOTAL_OPCOES_PROVA) s->opcao = 0;
}

// confere a resposta da pergunta final 
void confirmarProva(Sala1 *s, Jogador *j) {
    if (s->opcao == OPCAO_CORRETA_PROVA) {
        s->estado = ESTADO_CARD;
    } else {
        s->acertouUltimo = 0;
        s->vidasPerdidas++;
        s->reiniciar = perderVida(j);
        s->estado = ESTADO_RESULTADO;   // mostra  ERRADO mais a dica do pulpito
    }
}

// chamada UMA vez a cada aperto da tecla ESPACO, para interagir com o objeto mais proximo do jogador, ou fechar a caixa de texto, ou confirmar a escolha
void interagir(Sala1 *s, Jogador *j) {
    if (s->estado == ESTADO_JOGANDO) {
        int i = objetoProximo(j, s);
        if (i != -1) {
            s->objetoAtivo = i;
            // pulpito so abre a prova se o jogador ja fez as 3 provas
            if (s->objetos[i].tipo == OBJ_PODIO && s->acertos >= ACERTOS_NECESSARIOS) {
                s->opcao = 0;
                s->estado = ESTADO_PROVA;
            } else {
                s->estado = ESTADO_LENDO;
            }
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
            // guarda as vidas perdidas, errar ate zerar tambem tira o ponto da sala
            int perdidas = s->vidasPerdidas;
            iniciarSala1(s); // vidas zeram e reinicia a sala
            s->vidasPerdidas = perdidas;
        } else {
            s->estado = ESTADO_JOGANDO;
        }
    } else if (s->estado == ESTADO_PROVA) {
        confirmarProva(s, j);
    } else if (s->estado == ESTADO_CARD) {
        // o ponto da sala so conta se nao perdeu nenhuma vida
        if (s->vidasPerdidas == 0) {
            j->pontos++;
        }
        s->tempoPorta = 0;
        s->estado = ESTADO_PORTA;
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
// painel de leitura, escolha e resultado (fica na parte de baixo da tela)
void desenharPainel(Sala1 *s, ALLEGRO_FONT *fonte) {
    Objeto *o;
    ALLEGRO_COLOR bege = al_map_rgb(238, 224, 196);
    ALLEGRO_COLOR dourado = al_map_rgb(210, 190, 150);
    ALLEGRO_COLOR cinza = al_map_rgb(150, 145, 135);

    if (s->estado != ESTADO_LENDO && s->estado != ESTADO_ESCOLHA &&
        s->estado != ESTADO_RESULTADO) return;
    o = &s->objetos[s->objetoAtivo];

    // mesmo layout do HUD
    al_draw_filled_rounded_rectangle(80, 450, LARGURA_TELA - 80, 695, 12, 12,
        al_map_rgba(9, 12, 16, 235));
    al_draw_rounded_rectangle(80, 450, LARGURA_TELA - 80, 695, 12, 12,
        al_map_rgba(210, 190, 150, 170), 1);
    al_draw_text(fonte, dourado, 105, 467, 0, o->titulo);

    // resultado da escolha, se acertou ou errou, e explicacao do fragmento
    if (s->estado == ESTADO_RESULTADO) {
        if (s->acertouUltimo) {
            al_draw_text(fonte, al_map_rgb(120, 170, 100), 105, 505, 0, "CORRETO");
        } else {
            al_draw_text(fonte, al_map_rgb(200, 60, 50), 105, 505, 0, "ERRADO  (-1 vida)");
        }
        al_draw_multiline_text(fonte, bege, 105, 541, LARGURA_TELA - 210, 26, 0,
            o->explicacao);
        if (s->reiniciar) {
            al_draw_text(fonte, al_map_rgb(200, 60, 50), 105, 630, 0,
            "Suas vidas acabaram. A sala será reiniciada.");
        }
        al_draw_text(fonte, cinza, LARGURA_TELA - 105, 668, ALLEGRO_ALIGN_RIGHT,
            "[ESPAÇO] continuar");
        return;
    }

    // lendo ou escolhendo, exibe o texto do cartaz ou fragmento
    al_draw_multiline_text(fonte, bege, 105, 505, LARGURA_TELA - 210, 26, 0, o->texto);

    if (s->estado == ESTADO_ESCOLHA) {
        // destaca a opcao selecionada
        if (s->opcao == 0) {
            al_draw_filled_rounded_rectangle(105, 625, 265, 660, 6, 6,
                al_map_rgb(60, 105, 55));
        } else {
            al_draw_filled_rounded_rectangle(290, 625, 490, 660, 6, 6,
                al_map_rgb(135, 38, 32));
        }
        al_draw_text(fonte, bege, 185, 634, ALLEGRO_ALIGN_CENTER, "FATO");
        al_draw_text(fonte, bege, 390, 634, ALLEGRO_ALIGN_CENTER, "PROPAGANDA");
        al_draw_text(fonte, cinza, LARGURA_TELA - 105, 634, ALLEGRO_ALIGN_RIGHT,
            "SETAS escolhem | [ESPAÇO] confirma");
    } else {
        if (o->tipo == OBJ_FRAGMENTO && !o->resolvido) {
            al_draw_text(fonte, cinza, LARGURA_TELA - 105, 668, ALLEGRO_ALIGN_RIGHT,
            "[ESPAÇO] analisar este documento");
        } else {
            al_draw_text(fonte, cinza, LARGURA_TELA - 105, 668, ALLEGRO_ALIGN_RIGHT,
            "[ESPAÇO] fechar");
        }
    }
}

// cartao de progresso logo abaixo do header PROVAS 1/3
void desenharProgresso(Sala1 *s, ALLEGRO_FONT *fonte) {
    int a = s->acertos;
    if (a > ACERTOS_NECESSARIOS) a = ACERTOS_NECESSARIOS;

    al_draw_filled_rounded_rectangle(540, 104, 740, 136, 8, 8,
                                     al_map_rgba(9, 12, 16, 220));
    al_draw_rounded_rectangle(540, 104, 740, 136, 8, 8,
                              al_map_rgba(210, 190, 150, 150), 1);
    al_draw_textf(fonte, al_map_rgb(238, 224, 196), LARGURA_TELA / 2, 112,
                  ALLEGRO_ALIGN_CENTER, "PROVAS %d/%d", a, ACERTOS_NECESSARIOS);
}

// pergunta final no pulpito, com 3 alternativas
void desenharProvaFinal(Sala1 *s, ALLEGRO_FONT *fonte) {
    int i;
    ALLEGRO_COLOR bege = al_map_rgb(238, 224, 196);
    ALLEGRO_COLOR dourado = al_map_rgb(210, 190, 150);
    ALLEGRO_COLOR cinza = al_map_rgb(150, 145, 135);

    if (s->estado != ESTADO_PROVA) return;

    al_draw_filled_rounded_rectangle(80, 400, LARGURA_TELA - 80, 695, 12, 12,
        al_map_rgba(9, 12, 16, 240));
    al_draw_rounded_rectangle(80, 400, LARGURA_TELA - 80, 695, 12, 12,
        al_map_rgba(210, 190, 150, 170), 1);
    al_draw_text(fonte, dourado, 105, 417, 0, "PROVA FINAL");
    al_draw_multiline_text(fonte, bege, 105, 455, LARGURA_TELA - 210, 26, 0,
                           PERGUNTA_FINAL);

    for (i = 0; i < TOTAL_OPCOES_PROVA; i++) {
        float y = 530 + i * 42;
        if (i == s->opcao) {
            al_draw_filled_rounded_rectangle(105, y - 6, LARGURA_TELA - 105, y + 30, 6, 6,
                al_map_rgb(70, 58, 38));
            al_draw_rounded_rectangle(105, y - 6, LARGURA_TELA - 105, y + 30, 6, 6,
            dourado, 1);
        }
        al_draw_textf(fonte, bege, 125, y + 2, 0, "%c)  %s", 'A' + i, OPCOES_FINAL[i]);
    }
    al_draw_text(fonte, cinza, LARGURA_TELA - 105, 668, ALLEGRO_ALIGN_RIGHT,
                 "SETAS cima/baixo escolhem | [ESPAÇO] confirma");
}

// cartao Voce sabia?, so aparece depois de acertar a prova final
void desenharCard(Sala1 *s, ALLEGRO_FONT *fonte, ALLEGRO_FONT *fonte_titulo) {
    if (s->estado != ESTADO_CARD) return;

    // escurece a sala por tras do cartao
    al_draw_filled_rectangle(0, 0, LARGURA_TELA, ALTURA_TELA, al_map_rgba(0, 0, 0, 170));

    al_draw_filled_rounded_rectangle(250, 170, 1030, 530, 14, 14,
        al_map_rgba(9, 12, 16, 245));
    al_draw_rounded_rectangle(250, 170, 1030, 530, 14, 14,
        al_map_rgba(210, 190, 150, 200), 2);
    al_draw_text(fonte_titulo, al_map_rgb(190, 45, 37), LARGURA_TELA / 2, 195,
                 ALLEGRO_ALIGN_CENTER, "VOCÊ SABIA?");
    al_draw_multiline_text(fonte, al_map_rgb(238, 224, 196), LARGURA_TELA / 2, 280,
                680, 28, ALLEGRO_ALIGN_CENTER, CARD_TEXTO);
    al_draw_text(fonte, al_map_rgb(150, 145, 135), LARGURA_TELA / 2, 480,
                 ALLEGRO_ALIGN_CENTER, "[ESPAÇO] abrir a porta");
}

// escurece a tela durante a porta e mostra uma previa da sala 2
void desenharTransicao(Sala1 *s, ALLEGRO_FONT *fonte) {
    if (s->estado == ESTADO_PORTA) {
        int alfa = (s->tempoPorta * 255) / PORTA_DURACAO;
        if (alfa > 255) alfa = 255;
        al_draw_filled_rectangle(0, 0, LARGURA_TELA, ALTURA_TELA, al_map_rgba(0, 0, 0, alfa));
    } else if (s->estado == ESTADO_CONCLUIDA) {
        al_draw_filled_rectangle(0, 0, LARGURA_TELA, ALTURA_TELA, al_map_rgb(0, 0, 0));
        al_draw_text(fonte, al_map_rgb(238, 224, 196), LARGURA_TELA / 2, ALTURA_TELA / 2,
                     ALLEGRO_ALIGN_CENTER, "SALA 2 - em construção");
    }
}

// para a musica
ALLEGRO_AUDIO_STREAM *trocarMusica(ALLEGRO_AUDIO_STREAM *atual, const char *arquivo) {
    ALLEGRO_AUDIO_STREAM *nova;

    if (atual != NULL) {
        al_destroy_audio_stream(atual);
    }
    nova = al_load_audio_stream(arquivo, 4, 2048);
    if (nova == NULL) {
        fprintf(stderr, "aviso: nao carregou %s\n", arquivo);
        return NULL;
    }
    al_attach_audio_stream_to_mixer(nova, al_get_default_mixer());
    al_set_audio_stream_playmode(nova, ALLEGRO_PLAYMODE_LOOP);
    al_set_audio_stream_gain(nova, 0.6);
    return nova;
}

// SALA 2

// um relatorio (papel) em cima do balcao
typedef struct {
    float x, y;                // onde o jogador fica para poder ler
    float desenhoX, desenhoY;  // onde o papel aparece desenhado no cenario centro do papel
    int lido;                  
    char titulo[48];
    char texto[320];
} Relatorio;

// uma cidade marcavel no mapa
typedef struct {
    const char *nome;
    float fx, fy;
} Cidade;

static const Cidade CIDADES[TOTAL_CIDADES] = {
    { "Londres",  0.275f, 0.427f },
    { "Roterdã",  0.366f, 0.420f },
    { "Varsóvia", 0.635f, 0.451f }
};

// indica a ordem certa dos pinos no mapa
static const int ORDEM_CORRETA[TOTAL_CIDADES] = { 2, 1, 0 };

static const char *CARD_TEXTO_SALA2 =
    "A invasão da Polônia, em 1º de setembro de 1939, marca o início da Segunda Guerra Mundial na Europa. "
    "Com o avanço rápido e os bombardeios em cidades como Varsóvia, Roterdã e Londres, o conflito deixou de "
    "atingir apenas frentes militares e passou a castigar áreas urbanas e civis, mostrando a lógica da guerra total.";

static const char *AVISO_MESA =
    "Esta é a mesa de comando, com o mapa da Europa. Antes de marcar os ataques, leia os 3 relatórios "
    "em cima do balcão, ao lado da mesa. Procure os papéis com brilho azul.";

typedef struct {
    int estado;
    Relatorio relatorios[TOTAL_RELATORIOS];
    int relatorioAtivo;            // 0, 1, 2 = relatorio / INDICE_MESA = aviso da mesa
    int lidos;                     // quantos relatorios diferentes ja foram lidos
    float movFx, movFy;            // posicao (0.0 a 1.0 no mapa) do pino que o jogador esta movendo
    float pinoFx[TOTAL_CIDADES];   // posicao dos pinos que ja foram colocados no mapa
    float pinoFy[TOTAL_CIDADES];
    int qtdPinos;                  // quantos pinos ja estao no mapa
    int espera;                    // contador de frames mostrando o resultado
    int acertou;                   // resultado da ultima conferencia
    int tempo;                     // conta frames (para piscar e fade)
    int tempoPorta;
} Sala2;

// guarda um relatorio no vetor, na posicao i
void adicionarRelatorio(Sala2 *s, int i, float x, float y, float desenhoX, float desenhoY,
                        const char *titulo, const char *texto) {
    Relatorio *r = &s->relatorios[i];
    r->x = x;
    r->y = y;
    r->desenhoX = desenhoX;
    r->desenhoY = desenhoY;
    r->lido = 0;
    strcpy(r->titulo, titulo);
    strcpy(r->texto, texto);
}

// posicao (de 0.0 a 1.0) do lugar do pino numero i na bandeja embaixo do mapa
float bandejaFx(int i) {
    return 0.385f + i * 0.115f;
}

// o proximo pino volta para o lugar dele na bandeja, pronto para ser movido
void pegarProximoPino(Sala2 *s) {
    s->movFx = bandejaFx(s->qtdPinos);
    s->movFy = BANDEJA_FY;
}

void iniciarSala2(Sala2 *s) {
    int i;
    s->estado = ESTADO2_JOGANDO;
    s->relatorioAtivo = -1;
    s->lidos = 0;
    s->qtdPinos = 0;
    s->espera = 0;
    s->acertou = 0;
    s->tempo = 0;
    s->tempoPorta = 0;
    for (i = 0; i < TOTAL_CIDADES; i++) {
        s->pinoFx[i] = 0;
        s->pinoFy[i] = 0;
    }
    pegarProximoPino(s);
    // os 2 primeiros numeros: onde o jogador fica para ler (no chao, perto do balcao)
    // os 2 ultimos numeros: onde o papel aparece desenhado em cima do balcao
    adicionarRelatorio(s, 0, 215, 570, 115, 478, "Relatório 1 - 1º de setembro de 1939",
        "1º de setembro de 1939: ataques rápidos e coordenados dão início à guerra na Europa com a invasão da Polônia. "
        "Em seguida, bombardeios atingem Varsóvia, mostrando o avanço acelerado desse novo estilo de guerra.");
    adicionarRelatorio(s, 1, 270, 505, 170, 418, "Relatório 2 - Maio de 1940",
        "Maio de 1940: o avanço segue para o oeste da Europa. Bombardeios intensos atingem Roterdã, "
        "deixando claro que áreas urbanas e civis estavam dentro da rota do conflito.");
    adicionarRelatorio(s, 2, 330, 430, 235, 358, "Relatório 3 - 1940 a 1941 (a Blitz)",
        "1940-1941: na campanha aérea conhecida como Blitz, bombardeios repetidos castigam Londres por meses. "
        "A guerra se estende contra a população civil, marcando o avanço da chamada \"guerra total\".");
}

// devolve 0, 1 ou 2 (relatorio mais perto), INDICE_MESA (3) se estiver perto da mesa, ou -1 se nao tem nada perto
int objetoProximoSala2(Jogador *j, Sala2 *s) {
    int i, melhor = -1;
    float menor = 0;
    float dx, dy, d;

    for (i = 0; i < TOTAL_RELATORIOS; i++) {
        dx = j->x - s->relatorios[i].x;
        dy = j->y - s->relatorios[i].y;
        d = dx * dx + dy * dy;
        if (d <= RAIO_RELATORIO * RAIO_RELATORIO) {
            if (melhor == -1 || d < menor) {
                melhor = i;
                menor = d;
            }
        }
    }

    // a mesa so ganha se nenhum relatorio estiver mais perto
    dx = j->x - MESA_X;
    dy = j->y - MESA_Y;
    d = dx * dx + dy * dy;
    if (d <= MESA_RAIO * MESA_RAIO) {
        if (melhor == -1 || d < menor) {
            melhor = INDICE_MESA;
        }
    }
    return melhor;
}

// chamada UMA vez a cada aperto de ESPACO fora do mapa
void interagirSala2(Sala2 *s, Jogador *j) {
    if (s->estado == ESTADO2_JOGANDO) {
        int i = objetoProximoSala2(j, s);
        if (i == -1) return;

        if (i == INDICE_MESA) {
            // chegou perto da mesa e apertou ESPACO: abre o mapa na tela
            pegarProximoPino(s);              // o pino volta para a bandeja
            s->estado = ESTADO2_MAPA;
        } else {
            s->relatorioAtivo = i;
            if (!s->relatorios[i].lido) {
                s->relatorios[i].lido = 1;
                s->lidos++;
            }
            s->estado = ESTADO2_LENDO;
        }
    } else if (s->estado == ESTADO2_LENDO) {
        s->estado = ESTADO2_JOGANDO;
    } else if (s->estado == ESTADO2_CARD) {
        // sem punicao de vida na sala 2, entao o ponto da sala sempre conta
        j->pontos++;
        s->tempoPorta = 0;
        s->estado = ESTADO2_PORTA;
    }
}

// as setas movem o pino pelo mapa (chamada a cada tick, enquanto a seta esta apertada)
void moverPino(Sala2 *s, ALLEGRO_KEYBOARD_STATE *teclado) {
    if (s->estado != ESTADO2_MAPA || s->espera > 0) return;
    if (s->qtdPinos >= TOTAL_CIDADES) return;

    if (al_key_down(teclado, ALLEGRO_KEY_LEFT) || al_key_down(teclado, ALLEGRO_KEY_A)) {
        s->movFx = s->movFx - PINO_VEL_X;
    }
    if (al_key_down(teclado, ALLEGRO_KEY_RIGHT) || al_key_down(teclado, ALLEGRO_KEY_D)) {
        s->movFx = s->movFx + PINO_VEL_X;
    }
    if (al_key_down(teclado, ALLEGRO_KEY_UP) || al_key_down(teclado, ALLEGRO_KEY_W)) {
        s->movFy = s->movFy - PINO_VEL_Y;
    }
    if (al_key_down(teclado, ALLEGRO_KEY_DOWN) || al_key_down(teclado, ALLEGRO_KEY_S)) {
        s->movFy = s->movFy + PINO_VEL_Y;
    }

    // o pino nao sai da area do mapa + bandeja
    if (s->movFx < 0.0f) s->movFx = 0.0f;
    if (s->movFx > 1.0f) s->movFx = 1.0f;
    if (s->movFy < 0.0f) s->movFy = 0.0f;
    if (s->movFy > BANDEJA_FY + 0.04f) s->movFy = BANDEJA_FY + 0.04f;
}

// ESPACO dentro do mapa: gruda o pino (1, 2 ou 3) no lugar onde ele esta
void colocarPino(Sala2 *s) {
    int i;
    if (s->estado != ESTADO2_MAPA || s->espera > 0) return;
    if (s->qtdPinos >= TOTAL_CIDADES) return;

    // se o pino ainda esta na bandeja (abaixo do mapa), nao da para grudar
    if (s->movFy > 1.0f) return;

    s->pinoFx[s->qtdPinos] = s->movFx;
    s->pinoFy[s->qtdPinos] = s->movFy;
    s->qtdPinos++;

    if (s->qtdPinos < TOTAL_CIDADES) {
        pegarProximoPino(s);   // o proximo pino aparece na bandeja
    } else {
        // colocou o terceiro: confere se cada pino esta perto da cidade certa
        s->acertou = 1;
        for (i = 0; i < TOTAL_CIDADES; i++) {
            int c = ORDEM_CORRETA[i];
            float dx = s->pinoFx[i] - CIDADES[c].fx;
            float dy = s->pinoFy[i] - CIDADES[c].fy;
            if (dx < 0) dx = -dx;
            if (dy < 0) dy = -dy;
            if (dx > TOL_PINO_X || dy > TOL_PINO_Y) {
                s->acertou = 0;
            }
        }
        s->espera = ESPERA_PINOS;   // mostra o resultado por um tempo
    }
}

// BACKSPACE: tira o ultimo pino colocado
void tirarPino(Sala2 *s) {
    if (s->estado != ESTADO2_MAPA || s->espera > 0) return;
    if (s->qtdPinos > 0) {
        s->qtdPinos--;
        pegarProximoPino(s);
    }
}

// recebe cada tecla apertada (KEY_DOWN) enquanto estiver na sala 2
void teclasSala2(Sala2 *s, Jogador *j, int tecla) {
    if (tecla == ALLEGRO_KEY_SPACE) {
        if (s->estado == ESTADO2_MAPA) {
            colocarPino(s);
        } else {
            interagirSala2(s, j);
        }
    }
    if (s->estado == ESTADO2_MAPA) {
        if (tecla == ALLEGRO_KEY_BACKSPACE) {
            tirarPino(s);
        }
        if (tecla == ALLEGRO_KEY_Q && s->espera == 0) {
            s->estado = ESTADO2_JOGANDO;   // fecha o mapa sem perder os pinos
        }
    }
}

// chamada UMA vez por tick do timer (60 vezes por segundo)
void atualizarSala2(Sala2 *s) {
    s->tempo++;

    // contagem do resultado dos pinos
    if (s->espera > 0) {
        s->espera--;
        if (s->espera == 0) {
            if (s->acertou) {
                s->estado = ESTADO2_CARD;   // acertou: cartao "Voce sabia?"
            } else {
                s->qtdPinos = 0;            // errou: pinos voltam para a bandeja
                pegarProximoPino(s);
            }
        }
    }

    // transicao de saida
    if (s->estado == ESTADO2_PORTA) {
        s->tempoPorta++;
        if (s->tempoPorta >= PORTA2_DURACAO) {
            s->estado = ESTADO2_CONCLUIDA;
            // quando a sala 3 existir, e aqui que se troca para ela (jogador.sala = 3)
        }
    }
}

// ----------------------------- DESENHO DA SALA 2 -----------------------------

// os 3 papeis em cima do balcao, usando o sprite papeis.png
// depois de lido, o papel fica cinza (mas pode ser lido de novo)
void desenharRelatorios(Sala2 *s, ALLEGRO_BITMAP *papeis) {
    int i;

    for (i = 0; i < TOTAL_RELATORIOS; i++) {
        Relatorio *r = &s->relatorios[i];
        ALLEGRO_COLOR cor = al_map_rgb(255, 255, 255);   // branco = desenha sem mudar a cor

        if (r->lido) {
            cor = al_map_rgb(120, 120, 120);
        }

        // recorta a celula do papel i e desenha no tamanho real, centralizado no ponto do papel
        al_draw_tinted_bitmap_region(papeis, cor,
            i * PAPEL_LARG, 0, PAPEL_LARG, PAPEL_ALT,
            r->desenhoX - PAPEL_LARG / 2, r->desenhoY - PAPEL_ALT / 2, 0);
    }
}

// quando os 3 relatorios foram lidos, a mesa do mapa pisca em azul para chamar o jogador
void desenharDicaMesa(Sala2 *s) {
    if (s->estado != ESTADO2_JOGANDO || s->lidos < TOTAL_RELATORIOS) return;
    if ((s->tempo / 30) % 2 == 0) {
        al_draw_rectangle(555, 279, 999, 532, al_map_rgb(110, 190, 255), 3);
    }
}

// "ESPACO" piscando perto de um papel ou da mesa
void desenharIconeSala2(Jogador *j, Sala2 *s, ALLEGRO_FONT *fonte) {
    if (s->estado != ESTADO2_JOGANDO) return;
    if (objetoProximoSala2(j, s) == -1) return;

    if ((s->tempo / 30) % 2 == 0) {
        float x = j->x;
        float y = j->y - DESENHO_ALT - 40;
        al_draw_filled_rectangle(x - 45, y, x + 45, y + 28, al_map_rgb(20, 20, 20));
        al_draw_rectangle(x - 45, y, x + 45, y + 28, al_map_rgb(230, 230, 230), 2);
        al_draw_text(fonte, al_map_rgb(230, 230, 230), x, y + 4,
                     ALLEGRO_ALIGN_CENTER, "ESPAÇO");
    }
}

// cartao RELATORIOS 1/3 logo abaixo do header
void desenharProgressoSala2(Sala2 *s, ALLEGRO_FONT *fonte) {
    al_draw_filled_rounded_rectangle(520, 104, 760, 136, 8, 8,
                                     al_map_rgba(9, 12, 16, 220));
    al_draw_rounded_rectangle(520, 104, 760, 136, 8, 8,
                              al_map_rgba(210, 190, 150, 150), 1);
    al_draw_textf(fonte, al_map_rgb(238, 224, 196), LARGURA_TELA / 2, 112,
                  ALLEGRO_ALIGN_CENTER, "RELATÓRIOS %d/%d", s->lidos, TOTAL_RELATORIOS);
}

// caixa de leitura na parte de baixo (mesmo layout da sala 1)
void desenharPainelSala2(Sala2 *s, ALLEGRO_FONT *fonte) {
    const char *titulo;
    const char *texto;

    if (s->estado != ESTADO2_LENDO) return;

    if (s->relatorioAtivo == INDICE_MESA) {
        titulo = "Mesa de comando";
        texto = AVISO_MESA;
    } else {
        titulo = s->relatorios[s->relatorioAtivo].titulo;
        texto = s->relatorios[s->relatorioAtivo].texto;
    }

    al_draw_filled_rounded_rectangle(80, 450, LARGURA_TELA - 80, 695, 12, 12,
        al_map_rgba(9, 12, 16, 235));
    al_draw_rounded_rectangle(80, 450, LARGURA_TELA - 80, 695, 12, 12,
        al_map_rgba(210, 190, 150, 170), 1);
    al_draw_text(fonte, al_map_rgb(210, 190, 150), 105, 467, 0, titulo);
    al_draw_multiline_text(fonte, al_map_rgb(238, 224, 196), 105, 505,
                           LARGURA_TELA - 210, 26, 0, texto);
    al_draw_text(fonte, al_map_rgb(150, 145, 135), LARGURA_TELA - 105, 668,
                 ALLEGRO_ALIGN_RIGHT, "[ESPAÇO] fechar");
}

// o mapa grande no meio da tela, com a bandeja de pinos embaixo
void desenharMapa(Sala2 *s, ALLEGRO_BITMAP *mapa, ALLEGRO_BITMAP *pinos, ALLEGRO_FONT *fonte) {
    int i;
    ALLEGRO_COLOR bege = al_map_rgb(238, 224, 196);
    ALLEGRO_COLOR dourado = al_map_rgb(240, 210, 120);
    ALLEGRO_COLOR cinza = al_map_rgb(150, 145, 135);

    // tamanho do mapa na tela: largura fixa e altura proporcional
    float larg = MAPA_LARG;
    float alt = al_get_bitmap_height(mapa) * (larg / al_get_bitmap_width(mapa));
    float mx = (LARGURA_TELA - larg) / 2;   // centralizado na horizontal
    float my = MAPA_POS_Y;
    float yBandeja = my + BANDEJA_FY * alt; // altura (na tela) onde ficam os pinos que sobram

    // o arquivo pinos.png tem 3 pinos lado a lado: cada um ocupa 1/3 da largura
    int pinoLarg = al_get_bitmap_width(pinos) / TOTAL_CIDADES;
    int pinoAlt = al_get_bitmap_height(pinos);

    if (s->estado != ESTADO2_MAPA) return;

    // escurece a sala por tras do mapa
    al_draw_filled_rectangle(0, 0, LARGURA_TELA, ALTURA_TELA, al_map_rgba(0, 0, 0, 190));

    // o mapa
    al_draw_scaled_bitmap(mapa,
        0, 0, al_get_bitmap_width(mapa), al_get_bitmap_height(mapa),
        mx, my, larg, alt, 0);

    // bandeja embaixo do mapa, onde ficam os pinos que ainda nao foram usados
    al_draw_filled_rounded_rectangle(mx + 0.30f * larg, yBandeja - 32,
        mx + 0.70f * larg, yBandeja + 32, 10, 10, al_map_rgba(9, 12, 16, 235));
    al_draw_rounded_rectangle(mx + 0.30f * larg, yBandeja - 32,
        mx + 0.70f * larg, yBandeja + 32, 10, 10, al_map_rgba(210, 190, 150, 170), 1);

    // pinos que ainda faltam: o primeiro da fila e o que o jogador esta movendo
    for (i = s->qtdPinos; i < TOTAL_CIDADES; i++) {
        float fx = bandejaFx(i);
        float fy = BANDEJA_FY;
        float px, py;

        if (i == s->qtdPinos) {
            fx = s->movFx;
            fy = s->movFy;
        }
        px = mx + fx * larg;
        py = my + fy * alt;

        al_draw_scaled_bitmap(pinos,
            i * pinoLarg, 0, pinoLarg, pinoAlt,
            px - PINO_TAM / 2, py - PINO_TAM / 2, PINO_TAM, PINO_TAM, 0);

        // anel dourado piscando no pino que esta sendo movido
        if (i == s->qtdPinos && s->espera == 0) {
            float raio = PINO_TAM / 2 + 4 + ((s->tempo / 15) % 2) * 3;
            al_draw_circle(px, py, raio, dourado, 2);
        }
    }

    // pinos ja colocados: o numero do pino e a posicao dele no vetor (0 -> pino 1)
    for (i = 0; i < s->qtdPinos; i++) {
        float px = mx + s->pinoFx[i] * larg;
        float py = my + s->pinoFy[i] * alt;
        al_draw_scaled_bitmap(pinos,
            i * pinoLarg, 0, pinoLarg, pinoAlt,
            px - PINO_TAM / 2, py - PINO_TAM / 2, PINO_TAM, PINO_TAM, 0);
    }

    // textos abaixo do mapa
    if (s->espera > 0) {
        if (s->acertou) {
            al_draw_text(fonte, al_map_rgb(120, 170, 100), LARGURA_TELA / 2, 640,
                         ALLEGRO_ALIGN_CENTER, "ORDEM CORRETA!");
        } else {
            al_draw_text(fonte, al_map_rgb(200, 60, 50), LARGURA_TELA / 2, 640,
                         ALLEGRO_ALIGN_CENTER, "ORDEM ERRADA. Os pinos voltam ao início, tente de novo (sem perder vida).");
        }
    } else {
        al_draw_textf(fonte, bege, LARGURA_TELA / 2, 626, ALLEGRO_ALIGN_CENTER,
                      "Pino %d de %d: leve até a cidade do ataque", s->qtdPinos + 1, TOTAL_CIDADES);
        al_draw_text(fonte, cinza, LARGURA_TELA / 2, 656, ALLEGRO_ALIGN_CENTER,
                     "SETAS movem o pino | [ESPAÇO] gruda o pino no mapa");
        al_draw_text(fonte, cinza, LARGURA_TELA / 2, 684, ALLEGRO_ALIGN_CENTER,
                     "[BACKSPACE] tira o último pino | [Q] fecha o mapa");
    }
}

// cartao "Voce sabia?" da sala 2
void desenharCardSala2(Sala2 *s, ALLEGRO_FONT *fonte, ALLEGRO_FONT *fonte_titulo) {
    if (s->estado != ESTADO2_CARD) return;

    al_draw_filled_rectangle(0, 0, LARGURA_TELA, ALTURA_TELA, al_map_rgba(0, 0, 0, 170));

    al_draw_filled_rounded_rectangle(250, 170, 1030, 540, 14, 14,
        al_map_rgba(9, 12, 16, 245));
    al_draw_rounded_rectangle(250, 170, 1030, 540, 14, 14,
        al_map_rgba(210, 190, 150, 200), 2);
    al_draw_text(fonte_titulo, al_map_rgb(190, 45, 37), LARGURA_TELA / 2, 195,
                 ALLEGRO_ALIGN_CENTER, "VOCÊ SABIA?");
    al_draw_multiline_text(fonte, al_map_rgb(238, 224, 196), LARGURA_TELA / 2, 270,
                680, 28, ALLEGRO_ALIGN_CENTER, CARD_TEXTO_SALA2);
    al_draw_text(fonte, al_map_rgb(150, 145, 135), LARGURA_TELA / 2, 495,
                 ALLEGRO_ALIGN_CENTER, "[ESPAÇO] continuar");
}

// fade-in ao entrar, fade-out ao sair e tela final da sala 2
void desenharTransicaoSala2(Sala2 *s, ALLEGRO_FONT *fonte) {
    if (s->estado == ESTADO2_JOGANDO && s->tempo < FADE_ENTRADA) {
        int alfa = 255 - (s->tempo * 255) / FADE_ENTRADA;
        al_draw_filled_rectangle(0, 0, LARGURA_TELA, ALTURA_TELA, al_map_rgba(0, 0, 0, alfa));
    } else if (s->estado == ESTADO2_PORTA) {
        int alfa = (s->tempoPorta * 255) / PORTA2_DURACAO;
        if (alfa > 255) alfa = 255;
        al_draw_filled_rectangle(0, 0, LARGURA_TELA, ALTURA_TELA, al_map_rgba(0, 0, 0, alfa));
    } else if (s->estado == ESTADO2_CONCLUIDA) {
        al_draw_filled_rectangle(0, 0, LARGURA_TELA, ALTURA_TELA, al_map_rgb(0, 0, 0));
        al_draw_text(fonte, al_map_rgb(238, 224, 196), LARGURA_TELA / 2, ALTURA_TELA / 2,
                     ALLEGRO_ALIGN_CENTER, "SALA 3 - em construção");
    }
}

// junta tudo da sala 2 na ordem certa (o que vem depois aparece por cima)
void desenharSala2(Sala2 *s, Jogador *j,
                   ALLEGRO_BITMAP *fundo, ALLEGRO_BITMAP *spritesheet,
                   ALLEGRO_BITMAP *mapa, ALLEGRO_BITMAP *pinos, ALLEGRO_BITMAP *papeis,
                   ALLEGRO_FONT *fonte, ALLEGRO_FONT *fonte_titulo, ALLEGRO_FONT *fonte_descricao) {
    desenharCenario(fundo);
    desenharDicaMesa(s);
    desenharRelatorios(s, papeis);
    desenharJogador(j, spritesheet);
    desenharIconeSala2(j, s, fonte);
    desenharHUD(j, fonte);
    desenharProgressoSala2(s, fonte);
    desenharPainelSala2(s, fonte);
    desenharIdentidade(fonte_titulo, fonte_descricao);
    desenharFlash(j);
    desenharMapa(s, mapa, pinos, fonte);
    desenharCardSala2(s, fonte, fonte_titulo);
    desenharTransicaoSala2(s, fonte);
}

// ====================== FIM DA SALA 2 ======================

int main(void) {
    ALLEGRO_DISPLAY *tela = NULL;
    ALLEGRO_EVENT_QUEUE *fila = NULL;
    ALLEGRO_TIMER *timer = NULL;
    ALLEGRO_BITMAP *fundo = NULL;
    ALLEGRO_BITMAP *spritesheet = NULL; // troca 9 matriz por apenas 1 sprite sheet
    ALLEGRO_FONT *fonte = NULL;
    ALLEGRO_FONT *fonte_titulo = NULL; 
    ALLEGRO_FONT *fonte_descricao = NULL;
    ALLEGRO_AUDIO_STREAM *musica = NULL;
    ALLEGRO_SAMPLE *somPorta = NULL;
    Jogador jogador;
    Sala1 sala;
    int rodando = 1;
    int redesenhar = 1;
    int audioOk = 1;
    int portaTocada = 0;
    int quadroAbertura = 0;
    // utilizando a biblioteca allegro para pegar o estado do teclado
    ALLEGRO_KEYBOARD_STATE teclado;

    ALLEGRO_BITMAP *fundoSala2 = NULL;
    ALLEGRO_BITMAP *mapaSala2 = NULL;
    ALLEGRO_BITMAP *pinos = NULL;
    ALLEGRO_BITMAP *papeis = NULL;
    Sala2 sala2;
    int sala2Iniciada = 0;

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
    if (!al_install_audio() || !al_init_acodec_addon() || !al_reserve_samples(4)) {
        fprintf(stderr, "aviso: audio indisponivel, o jogo roda sem som\n");
        audioOk = 0;
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

    fundoSala2 = al_load_bitmap(ARQ_FUNDO_SALA2);
    if (fundoSala2 == NULL) {
        fprintf(stderr, "erro da funcao al_load_bitmap() para o fundo da sala 2 (%s)\n", ARQ_FUNDO_SALA2);
        return 1;
    }
    mapaSala2 = al_load_bitmap(ARQ_MAPA_SALA2);
    if (mapaSala2 == NULL) {
        fprintf(stderr, "erro da funcao al_load_bitmap() para o mapa da sala 2 (%s)\n", ARQ_MAPA_SALA2);
        return 1;
    }
    pinos = al_load_bitmap(ARQ_PINOS);
    if (pinos == NULL) {
        fprintf(stderr, "erro da funcao al_load_bitmap() para os pinos (%s)\n", ARQ_PINOS);
        return 1;
    }
    papeis = al_load_bitmap(ARQ_PAPEIS);
    if (papeis == NULL) {
        fprintf(stderr, "erro da funcao al_load_bitmap() para os papeis (%s)\n", ARQ_PAPEIS);
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

    if (audioOk) {
        musica = trocarMusica(NULL, "assets/audio/leberch-dark-628052.mp3");
    }

    al_register_event_source(fila, al_get_display_event_source(tela));
    al_register_event_source(fila, al_get_timer_event_source(timer));
    al_register_event_source(fila, al_get_keyboard_event_source());

    iniciarJogador(&jogador);
    iniciarSala1(&sala);
    iniciarSala2(&sala2);
    al_start_timer(timer);

    while (rodando) {
        ALLEGRO_EVENT evento;
        al_wait_for_event(fila, &evento);

        if (evento.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            rodando = 0;
        }
        if (evento.type == ALLEGRO_EVENT_KEY_DOWN) {
            int tecla = evento.keyboard.keycode;

            if (tecla == ALLEGRO_KEY_ESCAPE) {
                rodando = 0;
            }
            if (quadroAbertura >= ABERTURA_TITULO + ABERTURA_FADE) {
                if (tecla == ALLEGRO_KEY_SPACE) {
                    interagir(&sala, &jogador);
                }
                if (tecla == ALLEGRO_KEY_LEFT || tecla == ALLEGRO_KEY_A) {
                    escolherOpcao(&sala, 0);    // FATO
                }
                if (tecla == ALLEGRO_KEY_RIGHT || tecla == ALLEGRO_KEY_D) {
                    escolherOpcao(&sala, 1);    // PROPAGANDA
                }
                if (tecla == ALLEGRO_KEY_UP || tecla == ALLEGRO_KEY_W) {
                    mudarOpcaoProva(&sala, -1);
                }
                if (tecla == ALLEGRO_KEY_DOWN || tecla == ALLEGRO_KEY_S) {
                    mudarOpcaoProva(&sala, 1);
                }
                // As funcoes da sala 1 nao fazem nada depois que ela e concluida.
                if (jogador.sala >= 2) {
                    teclasSala2(&sala2, &jogador, tecla);
                }
            }
        }
        if (evento.type == ALLEGRO_EVENT_TIMER) {
            if (quadroAbertura < ABERTURA_TITULO + ABERTURA_FADE) {
                quadroAbertura++;
            } else if (jogador.sala >= 2) {
                if (!sala2Iniciada) {
                    // primeiro tick da sala 2: posiciona o jogador
                    sala2Iniciada = 1;
                    iniciarSala2(&sala2);
                    jogador.x = 640;
                    jogador.y = 690;
                    jogador.direcao = 0;
                    jogador.frame = 0;
                }
                if (sala2.estado == ESTADO2_JOGANDO) {
                    al_get_keyboard_state(&teclado);
                    moverJogador(&jogador, &teclado);
                } else {
                    jogador.frame = 0;
                    if (sala2.estado == ESTADO2_MAPA) {
                        // mapa aberto: as setas movem o pino em vez do personagem
                        al_get_keyboard_state(&teclado);
                        moverPino(&sala2, &teclado);
                    }
                }
                atualizarSala2(&sala2);
            } else {
                sala.tempo++;
                if (sala.estado == ESTADO_JOGANDO) {
                    al_get_keyboard_state(&teclado);
                    // faz o personagem andar de acordo com a tecla pressionada
                    moverJogador(&jogador, &teclado);
                } else {
                    jogador.frame = 0;
                }
                // transicao da porta: silencia a musica durante o fade
                if (sala.estado == ESTADO_PORTA) {
                    if (!portaTocada) {
                        portaTocada = 1;
                        if (musica != NULL) {
                            al_set_audio_stream_playing(musica, false);
                        }
                        if (somPorta != NULL) {
                            al_play_sample(somPorta, 1.0, 0.0, 1.0,
                                           ALLEGRO_PLAYMODE_ONCE, NULL);
                        }
                    }
                    sala.tempoPorta++;
                    if (sala.tempoPorta >= PORTA_DURACAO) {
                        jogador.sala = 2;
                        sala.estado = ESTADO_CONCLUIDA;
                    }
                }
            }
            redesenhar = 1;
        }

        if (redesenhar && al_is_event_queue_empty(fila)) {
            redesenhar = 0;
            if (quadroAbertura < ABERTURA_TITULO + ABERTURA_FADE) {
                desenharTelaInicial(fonte_titulo, quadroAbertura);
            } else if (jogador.sala >= 2) {
                desenharSala2(&sala2, &jogador, fundoSala2, spritesheet, mapaSala2, pinos, papeis,
                              fonte, fonte_titulo, fonte_descricao);
            } else {
                desenharCenario(fundo);
                desenharJogador(&jogador, spritesheet);
                desenharIcone(&jogador, &sala, fonte);
                desenharHUD(&jogador, fonte);
                desenharProgresso(&sala, fonte);
                desenharPainel(&sala, fonte);
                desenharProvaFinal(&sala, fonte);
                desenharIdentidade(fonte_titulo, fonte_descricao);
                desenharFlash(&jogador);
                desenharCard(&sala, fonte, fonte_titulo);
                desenharTransicao(&sala, fonte);
            }
            al_flip_display();
        }
    }

    if (somPorta != NULL) al_destroy_sample(somPorta);
    if (musica != NULL) al_destroy_audio_stream(musica);
    al_destroy_font(fonte);
    al_destroy_font(fonte_descricao);
    al_destroy_font(fonte_titulo);
    al_destroy_bitmap(papeis);
    al_destroy_bitmap(pinos);
    al_destroy_bitmap(mapaSala2);
    al_destroy_bitmap(fundoSala2);
    al_destroy_bitmap(spritesheet);
    al_destroy_bitmap(fundo);
    al_destroy_timer(timer);
    al_destroy_event_queue(fila);
    al_destroy_display(tela);
    return 0;
}
