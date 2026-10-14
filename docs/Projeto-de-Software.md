# Projeto de Software — Shift Space Run

## Visão geral

Shift Space Run é um jogo 2D de stealth/plataforma em que o jogador explora
salas, evita ser detectado e coleta fragmentos de fato espalhados pelo
cenário. Ao completar cada sala, o jogador recebe uma curiosidade ("Você
sabia?") relacionada ao tema da sala.

## Objetivos do jogo

- Atravessar cada sala sem ser detectado (ou minimizando detecções).
- Coletar os fragmentos de fato disponíveis em cada sala.
- Aprender curiosidades reais ao final de cada sala.

## Mecânicas principais

| Mecânica        | Tecla(s)        | Descrição                                            |
|-----------------|-----------------|-------------------------------------------------------|
| Movimento       | WASD / Setas    | Move o personagem pelo cenário.                      |
| Modo furtivo    | Shift (segurar)  | Reduz velocidade e ruído, dificultando a detecção.   |
| Interagir       | Espaço          | Interage com cartazes e objetos do cenário.          |
| Vidas           | —               | O jogador perde uma vida ao ser detectado.            |

## Tecnologia

- Linguagem: C
- Biblioteca gráfica: Allegro 5
- Build: GCC/MinGW + tasks.json do VS Code

## Estrutura de pastas

Ver `README.md` na raiz do repositório.
