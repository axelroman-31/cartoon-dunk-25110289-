#pragma once
#include <string>

// ================================================================
//  CARTOON DUNK  -  Constantes globales
//  Inspirado en Street Hoop / Street Slam (Data East, 1994)
// ================================================================

// --- Ventana ---
constexpr int   W_ANCHO  = 1024;
constexpr int   W_ALTO   = 620;

// --- Cancha (área jugable) ---
constexpr float C_X      = 55.f;
constexpr float C_Y      = 90.f;
constexpr float C_ANCHO  = W_ANCHO - 110.f;
constexpr float C_ALTO   = W_ALTO  - 155.f;

// --- Aros ---
constexpr float ARO_RADIO   = 26.f;
constexpr float ARO_IZQ_X   = C_X + 2.f;
constexpr float ARO_DER_X   = C_X + C_ANCHO - 2.f;
constexpr float ARO_Y       = C_Y + C_ALTO * 0.5f;

// Centro del aro (donde debe entrar la pelota)
constexpr float ARO_IZQ_CX  = ARO_IZQ_X + ARO_RADIO + 8.f;
constexpr float ARO_DER_CX  = ARO_DER_X - ARO_RADIO - 8.f;

// --- Línea de 3 puntos (distancia desde el centro del aro) ---
constexpr float DIST_3P     = 210.f;

// --- Pelota ---
constexpr float P_RADIO     = 13.f;

// --- Jugadores ---
constexpr float J_RADIO      = 20.f;
constexpr float VEL_BASE     = 175.f;
constexpr float VEL_SPRINT   = 265.f;
constexpr float RADIO_ROBO   = 48.f;
constexpr float RADIO_BLOQUEO = 55.f;   // rango para bloquear un tiro

// --- Super meter ---
constexpr float SM_MAX              = 100.f;
constexpr float SM_POR_ENCESTE      = 22.f;
constexpr float SM_POR_PASE         = 6.f;
constexpr float SM_POR_ROBO         = 18.f;
constexpr float SM_POR_ALLEYOOP     = 30.f;   // alley-oop premia mucho
constexpr float SM_TIEMPO_REGEN     = 4.5f;   // +1 por cada 4.5s (igual que original)

// --- Tiempo ---
constexpr float TIEMPO_MITAD = 120.f;   // 2 minutos por mitad (original)

// --- Equipos disponibles ---
enum class IDEquipo { CARTOON, RIVALES };

// --- Estados del juego ---
enum class EstadoJuego {
    SELECCION,
    JUGANDO,
    MEDIO_TIEMPO,
    FIN
};

// --- Estados de un jugador ---
enum class EstadoJ {
    IDLE,
    CORRIENDO,
    LANZANDO,        // tiro normal / en arco
    EN_AIRE,         // saltó para dunk (A+B luego A)
    DUNKEANDO,       // ejecutando el dunk
    PASANDO,
    BLOQUEANDO,      // salta para bloquear tiro rival
    ROBANDO,
    SUPER_SHOT,
    // Nuevos estados para mecánicas de Street Hoop
    FINTANDO,        // finta: amaga tiro sin soltar la pelota
    ALLEYOOP_VUELO,  // receptor en el aire esperando alley-oop
    REBOTEANDO,      // disputa de rebote tras fallo
    EMPUJANDO,       // choque físico con rival (shoving)
    ATURDIDO         // tras recibir empujón fuerte
};
