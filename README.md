# 🏀 Cartoon Dunk

Videojuego de baloncesto callejero arcade **3 contra 3**, inspirado en **Street Slam / Street Hoop** de Data East (Neo Geo, 1994). Personajes de caricaturas icónicas se enfrentan en la cancha de los Houston Rockets en partidos rápidos y espectaculares.

---

## 📋 Estructura del Repositorio

```
cartoon-dunk/
├── .github/
│   └── workflows/
│       └── publish.yml          ← GitHub Action (auto-deploy)
│
├── video/
│   └── demo.mp4                 ← Video de gameplay (máx 2 min)
│
├── gallery/
│   └── cover.png                ← Portada del juego (720x1080)
│
├── screenshots/
│   ├── screenshot1.png          ← Pantalla de inicio
│   ├── screenshot2.png          ← Selección de equipo
│   └── screenshot3.png          ← Gameplay en cancha
│
├── bin/
│   └── CartoonDunk.exe          ← Ejecutable del juego
│
├── assets/
│   ├── imagenes/                ← Sprites y fondos
│   ├── front/                   ← Fuentes
│   └── musica/                  ← Música del juego
│
├── include/                     ← Headers C++
├── src/                         ← Código fuente
├── makefile
└── README.md
```

---

## 🎬 Video de Gameplay

> **📁 Coloca aquí el video:**
> `video/demo.mp4`
>
> - Duración máxima: **2 minutos**
> - Formato: MP4
> - Resolución recomendada: 1280×720 o superior
> - Contenido sugerido: intro animada → selección de equipo → partido completo con dunk y super shot

---

## 🖼️ Capturas de Pantalla

| Pantalla de Inicio | Selección de Equipo | Gameplay |
|---|---|---|
| screenshot1.png | screenshot2.png | screenshot3.png |

---

## 🎯 Objetivo del Juego

Derrota al equipo rival anotando más puntos en **2 mitades de 2 minutos**. Usa combos, pases estratégicos y el **Super Shot** para ganar. El equipo con más puntos al sonar el tiempo final gana el partido.

---

## 🎮 Controles

| Tecla | Acción |
|---|---|
| `W / A / S / D` | Mover al jugador activo |
| `LShift` | Sprint (correr más rápido) |
| `Tab` | Cambiar jugador activo |
| `J` | Tiro normal (2 o 3 pts según distancia) |
| `K` | Pasar al compañero más libre |
| `J + K` | Saltar para dunk |
| `J` (en el aire) | Ejecutar el dunk |
| `L` | Finta / amago de tiro |
| `J` (sin balón) | Bloquear tiro rival / Empujar |
| `K` (sin balón) | Intentar robo de balón |
| `J + K` (sin balón) | Activar Alley-Oop (receptor salta) |
| `ESC` | Volver al menú |

---

## ⚙️ Mecánicas Principales

### 🏀 Sistema de Tiro
- **Tiro de 2 puntos** — dentro de la línea de 3 puntos
- **Tiro de 3 puntos** — fuera de la línea de 3 puntos
- La probabilidad de anotar depende del atributo del jugador y la distancia al aro

### 💥 Dunk en Aire (Combo)
Inspirado directamente en el botón A+B del Street Hoop original:
1. Presiona `J + K` para que el jugador salte
2. Presiona `J` mientras está en el aire para ejecutar el dunk
3. El dunk tiene mayor probabilidad de anotar que un tiro normal

### ⚡ Super Meter
- Barra que se llena con: encestes, pases, robos, bloqueos y alley-oops
- Al llenarse completamente, el siguiente tiro se convierte en un **Super Shot** casi imparable
- Se consume al usarse

### 🤝 Alley-Oop
1. Sin balón, presiona `J + K` para que tu jugador salte hacia el aro
2. El compañero con balón debe presionar `K` para enviar el pase elevado
3. Al recibirlo, se ejecuta automáticamente un dunk espectacular

### 🛡️ Defensa
- **Robo**: acércate al portador y presiona `K`
- **Bloqueo**: cuando la pelota está en vuelo, presiona `J` cerca de ella
- **Empuje**: presiona `J` o `L` cerca de un rival para descolocarlo
- **Intercepción**: posiciónate en la trayectoria de un pase rival

### 🏃 Estamina
- El sprint consume estamina gradualmente
- La estamina se recupera sola al caminar o estar parado
- Con poca estamina, la velocidad y precisión de tiro se reducen

---

## 🏆 Pantallas del Juego

```
INTRO → MENÚ PRINCIPAL → SELECCIÓN DE EQUIPO → PARTIDO → MEDIO TIEMPO → RESULTADO FINAL
```

- **Intro**: Logo animado con arte de todos los personajes y partículas
- **Menú Principal**: Navegar con W/S, opciones de Jugar / Controles / Créditos / Salir
- **Selección de Equipo**: Elige entre Cartoon Dunk o Rivales, con stats visuales
- **Partido**: 2 mitades de 2 minutos, HUD con marcador, tiempo y Super Meter
- **Medio Tiempo**: Pausa entre mitades, presiona Enter para continuar
- **Resultado**: Marcador final + estadísticas de robos, bloqueos, asistencias y alley-oops

---

## 👥 Equipos

### 🟠 CARTOON DUNK
| Jugador | Dunk | 3 Pts | Velocidad | Defensa |
|---|---|---|---|---|
| Goku (SS3) | ████████░ 8/9 | ██████░░░ 6/9 | ███████░░ 7/9 | █████░░░░ 5/9 |
| Pocoyo | ████████░ 8/9 | ██████░░░ 6/9 | ███████░░ 7/9 | █████░░░░ 5/9 |
| Bugs Bunny | ████████░ 8/9 | ██████░░░ 6/9 | ███████░░ 7/9 | █████░░░░ 5/9 |

### 🔵 RIVALES
| Jugador | Dunk | 3 Pts | Velocidad | Defensa |
|---|---|---|---|---|
| Místico | ██████░░░ 6/9 | ████████░ 8/9 | ██████░░░ 6/9 | ████████░ 8/9 |
| Místico | ██████░░░ 6/9 | ████████░ 8/9 | ██████░░░ 6/9 | ████████░ 8/9 |
| Místico | ██████░░░ 6/9 | ████████░ 8/9 | ██████░░░ 6/9 | ████████░ 8/9 |

---

## 🛠️ Tecnologías

- **Motor gráfico**: SFML 2.6
- **Lenguaje**: C++17
- **Sistema de sprites**: Sprite sheets animados (walk / run / dribble / shoot / dunk)
- **Física**: Arco parabólico para tiros y pases, colisiones entre jugadores
- **IA**: Toma de decisiones ofensiva y defensiva con probabilidades dinámicas

---

## 📦 Cómo Compilar

```bash
# Requiere: g++ con C++17, SFML 2.6 instalado
make
./CartoonDunk
```

En Ubuntu/Debian:
```bash
sudo apt-get install libsfml-dev
make
```

---

## 👥 Equipo

- **Desarrollador**: [Carlo Axel Román Martínez] (@axelroman-31)

---

## 📜 Créditos

- **Inspirado en**: Street Slam / Street Hoop — Data East Corporation (1994, Neo Geo)
- **Sprites**: Goku (Dragon Ball Z) · Pocoyo · Bugs Bunny (Looney Tunes) · Místico (Lucha Libre AAA)
- **Cancha**: Astros de Jalisco — Arena astros
- **Música**: Gang$tazz.ogg
- **Motor**: SFML 2.6 — [sfml-dev.org](https://www.sfml-dev.org)

---

## ⚠️ Validaciones CETUS

El sistema verificará automáticamente:

- ✓ `video/demo.mp4` — video de gameplay
- ✓ `gallery/cover.png` — portada 720×1080
- ✓ `screenshots/screenshot1.png` — mínimo 3 capturas
- ✓ `screenshots/screenshot2.png`
- ✓ `screenshots/screenshot3.png`
- ✓ `bin/CartoonDunk.exe` — ejecutable
- ✓ `README.md` — este archivo

---

*Proyecto 252 — Cartoon Dunk © 2025*
