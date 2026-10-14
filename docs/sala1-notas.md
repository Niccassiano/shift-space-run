# Notas de design — Sala 1

## Tema

Primeiro contato do jogador com a mecânica de furtividade. Sala pequena,
sem inimigos móveis nesta etapa, focada em ensinar movimento e modo
furtivo.

## Elementos da sala

- **Cenário fixo**: paredes e obstáculos simples, usados para ensinar
  colisão.
- **Cartazes interativos**: ao se aproximar e pressionar Espaço, exibem um
  fragmento de fato (texto curto relacionado ao tema da sala).
- **Fragmentos de fato**: colecionáveis que, somados, liberam o card
  "Você sabia?" ao final da sala.

## Vidas / HUD

- O jogador começa com um número fixo de vidas (a definir, ex.: 3).
- HUD exibe vidas restantes e progresso de fragmentos coletados
  (ex.: 2/4).

## Critério de conclusão da sala

Coletar todos os fragmentos de fato disponíveis. Ao concluir, exibir o
card "Você sabia?" com a curiosidade da sala antes de liberar a
transição para a próxima etapa.
