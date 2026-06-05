#pragma once

// ============================================================
//  Cartoon Dunk - Street Hoop estilo 3v3
//  Constantes globales del juego
// ============================================================

// Ventana
static const int ANCHO_VENTANA   = 1024;
static const int ALTO_VENTANA    = 600;
static const float FPS_OBJETIVO  = 60.f;

// Cancha
static const float CANCHA_X      = 60.f;
static const float CANCHA_Y      = 80.f;
static const float CANCHA_ANCHO  = ANCHO_VENTANA - 120.f;
static const float CANCHA_ALTO   = ALTO_VENTANA  - 160.f;

// Canastas
static const float CANASTA_RADIO = 28.f;
static const float ARO_IZQUIERDA_X = CANCHA_X + 5.f;
static const float ARO_DERECHA_X   = CANCHA_X + CANCHA_ANCHO - 5.f;
static const float ARO_Y           = CANCHA_Y + CANCHA_ALTO * 0.5f;

// Linea de 3 puntos: distancia desde cada aro
static const float DIST_TRES_PUNTOS = 200.f;

// Pelota
static const float PELOTA_RADIO     = 14.f;
static const float PELOTA_VELOCIDAD = 200.f;

// Jugadores
static const float JUGADOR_RADIO    = 22.f;
static const float VELOCIDAD_BASE   = 180.f;
static const float VELOCIDAD_SPRINT = 270.f;
static const float RADIO_ROBO       = 50.f;
static const float RADIO_PASE       = 400.f;

// Super meter
static const float SUPER_MAX        = 100.f;
static const float SUPER_POR_ENCESTE = 20.f;
static const float SUPER_POR_PASE   = 5.f;
static const float SUPER_POR_ROBO   = 15.f;

// Tiempo de juego
static const float TIEMPO_MITAD     = 120.f;   // 2 minutos por mitad

// Atributos de equipo (escala 1-9 como en el original)
struct AtributosEquipo {
    std::string nombre;
    int dunk;     // 1-9
    int tresP;    // 1-9
    int velocidad;// 1-9
    int defensa;  // 1-9
};

// Nombres de equipos / personajes
enum class Equipo { CARTOON_DUNK, RIVAL };
enum class EstadoJuego {
    MENU,
    SELECCION_EQUIPO,
    JUGANDO,
    MEDIO_TIEMPO,
    FIN_JUEGO
};
enum class EstadoJugador {
    IDLE,
    CORRIENDO,
    SALTANDO,
    LANZANDO,
    PASANDO,
    BLOQUEANDO,
    SUPER_SHOT
};
