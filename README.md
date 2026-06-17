# 🏀 Cartoon Dunk

> *Street Basketball 3v3 inspirado en Street Slam / Street Hoop (Data East, 1994)*

---

## 🎯 Objetivo del Juego

Cartoon Dunk es un juego de baloncesto callejero 3 contra 3 donde controlas a personajes icónicos del mundo animado. El objetivo es anotar más puntos que el equipo rival antes de que se acaben las dos mitades de 2 minutos cada una. Puedes ganar puntos con tiros de 2 o 3 puntos, mates espectaculares y Super Shots. ¡El equipo con más puntos al final gana!

---

## 🎮 Controles

### Movimiento
| Tecla | Acción |
|-------|--------|
| `W` `A` `S` `D` | Mover al jugador activo |
| `LShift` | Sprint (correr más rápido) |
| `Tab` | Cambiar de jugador activo |

### Ofensiva (con balón)
| Tecla | Acción |
|-------|--------|
| `J` | Tiro normal (2 o 3 puntos según la distancia) |
| `K` | Pasar al compañero libre |
| `J + K` | Saltar para ejecutar un mate |
| `J` *(en el aire)* | Ejecutar el mate (dunk) |
| `L` | Finta / amago de tiro |
| `Super + J` | **Super Shot** (cuando el medidor está lleno) |

### Defensiva (sin balón)
| Tecla | Acción |
|-------|--------|
| `J` | Bloquear tiro o empujar rival |
| `K` | Robo de balón |
| `L` | Empuje directo |
| `J + K` *(sin balón)* | Posicionarse como receptor de alley-oop |

---

## ⚙️ Mecánicas

- **Tiro 2/3 puntos** — La distancia al aro determina si el lanzamiento vale 2 o 3 puntos. La línea de tres puntos está a 210 unidades del centro del aro.
- **Dunk** — Presiona `J+K` para saltar y luego `J` en el aire para ejecutar un mate cuando estés cerca del aro.
- **Alley-Oop** — Sin el balón, presiona `J+K` para volar hacia el aro y recibir el pase de tu compañero para un mate en colaboración. ¡Llena mucho el Super Meter!
- **Super Shot** — Cuando el medidor especial llega al 100%, tu siguiente tiro se convierte en un Super Shot imparable.
- **Finta** — Con `L` puedes amagar un tiro para desequilibrar al defensor sin soltar el balón.
- **Bloqueo** — Con `J` sin el balón puedes saltar para bloquear los tiros rivales dentro del radio de bloqueo.
- **Robo** — Con `K` sin el balón intentas robar el esférico si estás dentro del radio de robo.
- **Empuje físico** — Los choques físicos pueden aturdir momentáneamente a los jugadores rivales.
- **Super Meter** — Se llena anotando encestes, realizando pases, robos, bloqueos y alley-oops. Se regenera lentamente con el tiempo.

---

## 🏆 Características

- Partido 1 Jugador vs CPU en formato 3v3
- Dos mitades de 2 minutos cada una (4 minutos totales por partido)
- Dos equipos seleccionables con estadísticas distintas: **CARTOON** (alto dunk y velocidad) y **RIVALES** (alto tiro de 3 y defensa)
- Personajes animados con sprites articulados de movimiento: Goku, Pocoyo, Bugs Bunny y Místico
- Cancha inspirada en el Toyota Center de los Houston Rockets
- Sistema de Super Meter con Super Shot especial
- Mecánicas arcade avanzadas: alley-oop, finta, bloqueo, robo y empuje físico
- Música de ambiente y fuente personalizada
- Pantalla de selección de equipo, menú principal con créditos y pantalla de fin de partida
- Pantalla de medio tiempo entre mitades

---

## 👥 Equipo

- **Desarrollador**: Carlo Axel Roman Martinez (@axelroman-31)

---

## 🛠️ Tecnologías

- **Motor/Framework**: SFML 2.6
- **Lenguaje**: C++17
- **Librerías adicionales**: SFML Graphics, SFML Audio
- **Build**: Makefile
- **Ejecutable**: `bin/CartoonDunk.exe`

---

## 📜 Créditos

- **Inspirado en**: Street Slam / Street Hoop (Data East, 1994)
- **Personajes**: Goku, Pocoyo, Bugs Bunny, Místico
- **Cancha**: Houston Rockets — Toyota Center
- **Música**: `Gang$tazz.ogg`
- **Fuente**: `texto.ttf`
- **Mecánicas**: Tiro 2/3 pts · Dunk · Super Shot · Alley-Oop · Finta · Bloqueo · Robo · Empuje físico

---

## 🚀 Cómo ejecutar

```bash
# Compilar (requiere SFML 2.6 instalado)
make

# Ejecutar el juego
./bin/CartoonDunk.exe
```

> Asegúrate de tener la carpeta `assets/` en el mismo directorio que el ejecutable.
