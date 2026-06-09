#pragma once
#include <SFML/Graphics.hpp>
#include <array>
#include <string>
#include <algorithm>
#include <cmath>
#include "Constantes.hpp"
#include "Jugador.hpp"

// ================================================================
//  DatosEquipo  -  plantilla de configuración
// ================================================================
struct DatosEquipo {
    std::string nombre;
    std::string sprites[3];
    sf::Color   color;
    int dunk, tres, vel, def;
};

// ================================================================
//  Equipo  -  3 jugadores, marcador, super meter
// ================================================================
class Equipo {
public:
    std::array<Jugador, 3> j;
    std::string  nombre;
    sf::Color    color;
    bool         esHumano   = false;

    int   puntos      = 0;
    float superMeter  = 0.f;
    float superRegenTimer = 0.f;   // para la regeneración por tiempo

    int   activo      = 0;
    int   receptorPase = -1;
    int   receptorAlleyOop = -1;   // para alley-oops

    // Estadísticas de partido
    int   asistencias  = 0;
    int   robos        = 0;
    int   bloqueos     = 0;
    int   alleyOops    = 0;

    void configurar(const DatosEquipo& d, bool humano) {
        nombre   = d.nombre;
        color    = d.color;
        esHumano = humano;
        for (int i = 0; i < 3; i++) {
            j[i].nombre      = d.sprites[i];
            j[i].colorEquipo = d.color;
            j[i].aDunk       = d.dunk;
            j[i].aTres       = d.tres;
            j[i].aVel        = d.vel;
            j[i].aDef        = d.def;
            j[i].esHumano    = humano;
            if (!d.sprites[i].empty()) j[i].cargarSprite(d.sprites[i]);
        }
        activo = 0;
    }

    Jugador& porActivo()  { return j[activo]; }

    Jugador* conPelota() {
        for (auto& jj : j) if (jj.tienePelota) return &jj;
        return nullptr;
    }
    int idxConPelota() const {
        for (int i = 0; i < 3; i++) if (j[i].tienePelota) return i;
        return -1;
    }

    void quitarPelota() { for (auto& jj : j) jj.tienePelota = false; }

    void darPelota(int idx) {
        quitarPelota();
        j[idx].tienePelota = true;
        activo = idx;
    }

    void autoSwitch(sf::Vector2f punto) {
        int   mejor  = 0;
        float minD   = 1e9f;
        for (int i = 0; i < 3; i++) {
            float dx = j[i].pos.x - punto.x;
            float dy = j[i].pos.y - punto.y;
            float d  = std::sqrt(dx*dx+dy*dy);
            if (d < minD) { minD = d; mejor = i; }
        }
        activo = mejor;
    }

    // Jugador más libre cerca del aro rival (para alley-oops)
    int mejorReceptorAlleyOop(sf::Vector2f aroPos) const {
        int   mejor = -1;
        float minD  = 200.f;  // solo si está cerca del aro
        for (int i = 0; i < 3; i++) {
            if (j[i].tienePelota) continue;
            float dx = j[i].pos.x - aroPos.x;
            float dy = j[i].pos.y - aroPos.y;
            float d  = std::sqrt(dx*dx+dy*dy);
            if (d < minD) { minD = d; mejor = i; }
        }
        return mejor;
    }

    void sumarPuntos(int pts) {
        puntos += pts;
        float gain = (pts == 3) ? SM_POR_ENCESTE * 1.5f : SM_POR_ENCESTE;
        superMeter = std::min(SM_MAX, superMeter + gain);
    }
    void sumarSuperPase()       { superMeter = std::min(SM_MAX, superMeter + SM_POR_PASE);     asistencias++; }
    void sumarSuperRobo()       { superMeter = std::min(SM_MAX, superMeter + SM_POR_ROBO);     robos++;       }
    void sumarSuperBloqueo()    { superMeter = std::min(SM_MAX, superMeter + SM_POR_ROBO);     bloqueos++;    }
    void sumarSuperAlleyOop()   { superMeter = std::min(SM_MAX, superMeter + SM_POR_ALLEYOOP); alleyOops++;   }

    bool superLleno() const { return superMeter >= SM_MAX; }
    void gastarSuper()      { superMeter = 0.f; }

    // Regeneración del super con el tiempo (como en el original)
    void actualizar(float dt) {
        for (auto& jj : j) jj.actualizar(dt);

        superRegenTimer += dt;
        if (superRegenTimer >= SM_TIEMPO_REGEN) {
            superRegenTimer = 0.f;
            superMeter = std::min(SM_MAX, superMeter + 1.f);
        }
    }

    void dibujar(sf::RenderWindow& w) {
        (void)w; // Se dibuja desde GameManager por orden Y
    }
};
