# Shift Space Run

Jogo 2D de stealth/plataforma feito em C com a biblioteca Allegro 5.

O jogador precisa atravessar salas evitando ser visto, coletando fragmentos
de fato e aprendendo curiosidades ao longo do caminho. O modo furtivo
(Shift) reduz a velocidade e o ruído do personagem, enquanto o Espaço é
usado para interagir com objetos do cenário.

## Estrutura do projeto

```
shift-space-run/
├── .vscode/            # configuração de build (tasks.json)
├── assets/             # sprites, fontes, sons
├── docs/               # Projeto de Software, One Sheet, notas de design
├── src/
│   └── main.c
├── .gitignore
└── README.md
```

## Como compilar

Requer o Allegro 5 instalado e configurado no PATH/compilador (MinGW).

```
gcc src/main.c -o build/shift-space-run -lallegro -lallegro_main -lallegro_image -lallegro_primitives -lallegro_font -lallegro_ttf
```

Ou use a task configurada em `.vscode/tasks.json` (Ctrl+Shift+B).

No macOS, o módulo `allegro_main-5` continua necessário para o linker encontrar a função `main` do Allegro.