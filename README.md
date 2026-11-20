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

Compile os comandos a partir da raiz do projeto. Eles criam o executável dentro da pasta `build/`.

### Windows (MSYS2 UCRT64)

Instale o MSYS2 e, no terminal **UCRT64**, instale o compilador, o `pkg-config` e o Allegro 5:

```bash
pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-pkgconf mingw-w64-ucrt-x86_64-allegro
```

Compile o jogo:

```bash
mkdir -p build && gcc -g src/main.c -o build/shift-space-run.exe $(pkg-config --cflags allegro-5 allegro_image-5 allegro_primitives-5 allegro_font-5 allegro_ttf-5 allegro_audio-5 allegro_acodec-5) $(pkg-config --libs allegro-5 allegro_main-5 allegro_image-5 allegro_primitives-5 allegro_font-5 allegro_ttf-5 allegro_audio-5 allegro_acodec-5)
```

Para iniciar o jogo no mesmo terminal UCRT64:

```bash
./build/shift-space-run.exe
```

### macOS

Instale o Allegro 5 e o `pkg-config` pelo Homebrew:

```bash
brew install allegro pkg-config
```

Compile o jogo:

```bash
mkdir -p build && /usr/bin/clang -fdiagnostics-color=always -g src/main.c -o build/shift-space-run $(pkg-config --cflags allegro-5 allegro_image-5 allegro_primitives-5 allegro_font-5 allegro_ttf-5 allegro_audio-5 allegro_acodec-5) $(pkg-config --libs allegro-5 allegro_main-5 allegro_image-5 allegro_primitives-5 allegro_font-5 allegro_ttf-5 allegro_audio-5 allegro_acodec-5)
```

Para iniciar o jogo a partir da raiz do projeto:

```bash
./build/shift-space-run
```

O módulo `allegro_main-5` é necessário para o linker encontrar a função `main` do Allegro. No VS Code para macOS, também é possível compilar com **Terminal > Run Build Task** (ou `Cmd+Shift+B`).