#pragma once
#include <SFML/Graphics.hpp>
#include <array>
#include <string>
#include "Constantes.hpp"
#include "Jugador.hpp"
#include "Pelota.hpp"

// ============================================================
//  EquipoJuego  –  triplete de jugadores + marcador + superMeter
// ============================================================
struct DatosEquipo {
    std::string nombre;
    std::string sprites[3]; // rutas
    sf::Color   color;
    int dunk, tresP, vel, def;
};

class EquipoJuego {
public:
    std::array<Jugador, 3> jugadores;
    int              puntos     = 0;
    float            superMeter = 0.f; // 0–100
    std::string      nombre;
    sf::Color        color;
    bool             esHumano   = false;

    // Qué jugador tiene el control (jugador humano) / es el atacante (CPU)
    int  jugadorActivo = 0;  // índice en jugadores[]

    void configurar(const DatosEquipo& datos, bool humano,
                    sf::Vector2f posInicial[3]) {
        nombre  = datos.nombre;
        color   = datos.color;
        esHumano = humano;

        for (int i = 0; i < 3; i++) {
            jugadores[i].nombre    = datos.sprites[i]; // reutilizamos como nombre
            jugadores[i].colorEquipo = datos.color;
            jugadores[i].atk_dunk  = datos.dunk;
            jugadores[i].atk_tres  = datos.tresP;
            jugadores[i].atk_vel   = datos.vel;
            jugadores[i].atk_def   = datos.def;
            jugadores[i].esJugadorHumano = humano;
            jugadores[i].esCPU     = !humano;
            jugadores[i].superMeter = &superMeter;
            jugadores[i].setPosicion(posInicial[i]);
            if (!datos.sprites[i].empty())
                jugadores[i].cargarTextura(datos.sprites[i]);
        }
        jugadorActivo = 0;
        jugadores[0].tienePelota = humano; // el humano inicia con pelota
    }

    void agregarPuntos(int pts) {
        puntos += pts;
        float ganancia = (pts == 3) ? SUPER_POR_ENCESTE * 1.5f : SUPER_POR_ENCESTE;
        superMeter = std::min(SUPER_MAX, superMeter + ganancia);
    }

    void aumentarSuperPorPase() {
        superMeter = std::min(SUPER_MAX, superMeter + SUPER_POR_PASE);
    }

    void aumentarSuperPorRobo() {
        superMeter = std::min(SUPER_MAX, superMeter + SUPER_POR_ROBO);
    }

    bool tieneSuper() const { return superMeter >= SUPER_MAX; }
    void gastarSuper()       { superMeter = 0.f; }

    Jugador& activo()  { return jugadores[jugadorActivo]; }
    Jugador* conPelota() {
        for (auto& j : jugadores) if (j.tienePelota) return &j;
        return nullptr;
    }

    void quitarPelota() {
        for (auto& j : jugadores) j.tienePelota = false;
    }

    void actualizar(float dt) {
        for (auto& j : jugadores) j.actualizar(dt);
    }

    void dibujar(sf::RenderWindow& w) {
        for (auto& j : jugadores) j.dibujar(w);
    }

    // Ajustar posiciones defensivas cuando el otro equipo tiene pelota
    void posicionarDefensiva(bool atacaIzquierda) {
        // No hace nada aún; se maneja en la IA del GameManager
    }
};
