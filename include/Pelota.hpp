#pragma once
#include <SFML/Graphics.hpp>
#include <cmath>
#include "Constantes.hpp"

// ================================================================
//  Pelota  -  física arcade: vuela en arco, rebota, rueda
// ================================================================
class Pelota {
public:
    sf::Vector2f pos;
    sf::Vector2f vel;

    // --- Arco parabólico ---
    bool  enArco      = false;
    float tArco       = 0.f;
    float durArco     = 0.f;
    sf::Vector2f orig, dest;
    float altura      = 0.f;   // peak height
    bool  esPase      = false; // pase = arco bajo, tiro = arco alto
    bool  esSuper     = false;

    // --- Estado libre en suelo ---
    bool  enJuego     = false; // true = libre rodando, nadie la tiene
    bool  enManos     = false; // alguien la tiene

    // Resultado del arco: llegó al aro? (se evalúa al terminar el arco)
    bool  anotoPendiente = false;
    int   puntosPendientes = 0;
    bool  esEquipoH   = false; // quién tiró

    Pelota() {
        pos = { W_ANCHO / 2.f, ARO_Y };
        forma.setRadius(P_RADIO);
        forma.setOrigin(P_RADIO, P_RADIO);
        forma.setFillColor(sf::Color(230, 115, 20));
        forma.setOutlineThickness(2.f);
        forma.setOutlineColor(sf::Color(140, 55, 5));
    }

    // Disparar en arco (tiro o super shot)
    void lanzarTiro(sf::Vector2f desde, sf::Vector2f hasta,
                    float tiempoVuelo, bool superShot = false) {
        orig    = desde;  dest = hasta;
        durArco = tiempoVuelo;
        tArco   = 0.f;
        altura  = 220.f;
        enArco  = true;
        esPase  = false;
        esSuper = superShot;
        enJuego = false;
        enManos = false;
        anotoPendiente = false;
        pos = desde;
        vel = {};
    }

    // Pase rápido (arco bajo)
    void lanzarPase(sf::Vector2f desde, sf::Vector2f hasta, float tiempo) {
        orig    = desde;  dest = hasta;
        durArco = tiempo;
        tArco   = 0.f;
        altura  = 55.f;
        enArco  = true;
        esPase  = true;
        esSuper = false;
        enJuego = false;
        enManos = false;
        anotoPendiente = false;
        pos = desde;
        vel = {};
    }

    // Pelota libre en el suelo (rebote de fallo)
    void soltarLibre(sf::Vector2f desde, sf::Vector2f velInicial) {
        pos     = desde;
        vel     = velInicial;
        enArco  = false;
        esPase  = false;
        esSuper = false;
        enJuego = true;
        enManos = false;
        anotoPendiente = false;
    }

    void tomarla() { enJuego = false; enManos = true; enArco = false; }

    void actualizar(float dt) {
        if (enArco) {
            tArco += dt;
            float t = std::min(tArco / durArco, 1.f);

            pos.x = orig.x + (dest.x - orig.x) * t;
            float baseY = orig.y + (dest.y - orig.y) * t;
            pos.y = baseY - altura * 4.f * t * (1.f - t);

            if (t >= 1.f) {
                enArco  = false;
                pos     = dest;
                // Si no anotó → rebote libre
                if (!anotoPendiente) {
                    soltarLibre(dest, {((float)rand()/RAND_MAX - 0.5f) * 120.f,
                                       ((float)rand()/RAND_MAX - 0.5f) * 80.f});
                }
            }
        } else if (enJuego) {
            pos += vel * dt;
            // Fricción
            vel *= 0.978f;
            if (std::abs(vel.x) < 1.f && std::abs(vel.y) < 1.f)
                vel = {};

            // Bordes de cancha
            if (pos.x - P_RADIO < C_X) {
                pos.x = C_X + P_RADIO;
                vel.x =  std::abs(vel.x) * 0.65f;
            }
            if (pos.x + P_RADIO > C_X + C_ANCHO) {
                pos.x = C_X + C_ANCHO - P_RADIO;
                vel.x = -std::abs(vel.x) * 0.65f;
            }
            if (pos.y - P_RADIO < C_Y) {
                pos.y = C_Y + P_RADIO;
                vel.y =  std::abs(vel.y) * 0.65f;
            }
            if (pos.y + P_RADIO > C_Y + C_ALTO) {
                pos.y = C_Y + C_ALTO - P_RADIO;
                vel.y = -std::abs(vel.y) * 0.65f;
            }
        }
        forma.setPosition(pos);
    }

    float progreso() const {
        if (durArco <= 0.f) return 1.f;
        return std::min(tArco / durArco, 1.f);
    }

    void dibujar(sf::RenderWindow& w) {
        if (esSuper && enArco) {
            // Halo de fuego
            for (int i = 3; i >= 1; i--) {
                sf::CircleShape halo(P_RADIO + i * 6.f);
                halo.setOrigin(P_RADIO + i * 6.f, P_RADIO + i * 6.f);
                halo.setPosition(pos);
                halo.setFillColor(sf::Color(255, 100 + i * 30, 0, 60));
                w.draw(halo);
            }
        }
        w.draw(forma);
        // Costuras
        sf::VertexArray ln(sf::Lines, 4);
        ln[0].position = {pos.x - P_RADIO, pos.y};
        ln[1].position = {pos.x + P_RADIO, pos.y};
        ln[2].position = {pos.x, pos.y - P_RADIO};
        ln[3].position = {pos.x, pos.y + P_RADIO};
        for (int i=0;i<4;i++) ln[i].color = sf::Color(140,55,5,160);
        w.draw(ln);
    }

    bool cerca(sf::Vector2f p, float radio) const {
        float dx = pos.x - p.x, dy = pos.y - p.y;
        return std::sqrt(dx*dx+dy*dy) < (P_RADIO + radio);
    }

private:
    sf::CircleShape forma;
};
