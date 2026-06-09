#pragma once
#include <SFML/Graphics.hpp>
#include <cmath>
#include "Constantes.hpp"

// ================================================================
//  Pelota  -  física arcade: vuela en arco, rebota, rueda
//  Nuevas mecánicas: intercepción de pase, rebote en tablero,
//  spin visual, trail en super shot
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
    float altura      = 0.f;
    bool  esPase      = false;
    bool  esSuper     = false;
    bool  esAlleyOop  = false;   // ¡nuevo! alley-oop en vuelo

    // --- Estado libre en suelo ---
    bool  enJuego     = false;
    bool  enManos     = false;

    // Resultado del arco
    bool  anotoPendiente   = false;
    int   puntosPendientes = 0;
    bool  esEquipoH        = false;

    // --- Trail de super shot ---
    struct TrailPt { sf::Vector2f p; float alpha; };
    std::vector<TrailPt> trail;
    float trailTimer = 0.f;

    // --- Spin visual ---
    float angulo = 0.f;

    // --- Rebote de tablero ---
    bool  reboteTablero = false;
    float timerRebote   = 0.f;

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
        esAlleyOop = false;
        esSuper = superShot;
        enJuego = false;
        enManos = false;
        anotoPendiente = false;
        trail.clear();
        pos = desde;
        vel = {};
        angulo = 0.f;
    }

    // Pase rápido (arco bajo) - puede interceptarse
    void lanzarPase(sf::Vector2f desde, sf::Vector2f hasta, float tiempo) {
        orig    = desde;  dest = hasta;
        durArco = tiempo;
        tArco   = 0.f;
        altura  = 55.f;
        enArco  = true;
        esPase  = true;
        esAlleyOop = false;
        esSuper = false;
        enJuego = false;
        enManos = false;
        anotoPendiente = false;
        trail.clear();
        pos = desde;
        vel = {};
    }

    // Alley-oop: pase alto hacia jugador en el aire
    void lanzarAlleyOop(sf::Vector2f desde, sf::Vector2f hasta, float tiempo) {
        orig    = desde;  dest = hasta;
        durArco = tiempo;
        tArco   = 0.f;
        altura  = 140.f;   // arco medio-alto
        enArco  = true;
        esPase  = true;
        esAlleyOop = true;
        esSuper = false;
        enJuego = false;
        enManos = false;
        anotoPendiente = false;
        trail.clear();
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
        esAlleyOop = false;
        enJuego = true;
        enManos = false;
        anotoPendiente = false;
        trail.clear();
    }

    void tomarla() { enJuego = false; enManos = true; enArco = false; trail.clear(); }

    // Posición interpolada en el arco (para intercepción)
    sf::Vector2f posEnArco(float t_norm) const {
        float t = t_norm;
        float x = orig.x + (dest.x - orig.x) * t;
        float baseY = orig.y + (dest.y - orig.y) * t;
        float y = baseY - altura * 4.f * t * (1.f - t);
        return {x, y};
    }

    void actualizar(float dt) {
        // Spin
        angulo += 180.f * dt;
        if (angulo > 360.f) angulo -= 360.f;

        // Rebote de tablero
        if (reboteTablero) {
            timerRebote -= dt;
            if (timerRebote <= 0.f) reboteTablero = false;
        }

        if (enArco) {
            tArco += dt;
            float t = std::min(tArco / durArco, 1.f);

            pos = posEnArco(t);

            // Trail para super shot
            if (esSuper) {
                trailTimer -= dt;
                if (trailTimer <= 0.f) {
                    trail.push_back({pos, 1.f});
                    trailTimer = 0.03f;
                    if (trail.size() > 12) trail.erase(trail.begin());
                }
            }

            if (t >= 1.f) {
                enArco  = false;
                pos     = dest;
                trail.clear();
                if (!anotoPendiente) {
                    // Rebote en el tablero antes de soltar
                    reboteTablero = true;
                    timerRebote   = 0.18f;
                    float vx = ((float)rand()/RAND_MAX - 0.5f) * 130.f;
                    float vy = ((float)rand()/RAND_MAX - 0.5f) * 90.f;
                    soltarLibre(dest, {vx, vy});
                }
            }
        } else if (enJuego) {
            pos += vel * dt;
            vel *= 0.972f;
            if (std::abs(vel.x) < 0.8f && std::abs(vel.y) < 0.8f)
                vel = {};

            // Bordes de cancha
            if (pos.x - P_RADIO < C_X) {
                pos.x = C_X + P_RADIO;
                vel.x = std::abs(vel.x) * 0.65f;
            }
            if (pos.x + P_RADIO > C_X + C_ANCHO) {
                pos.x = C_X + C_ANCHO - P_RADIO;
                vel.x = -std::abs(vel.x) * 0.65f;
            }
            if (pos.y - P_RADIO < C_Y) {
                pos.y = C_Y + P_RADIO;
                vel.y = std::abs(vel.y) * 0.65f;
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
        // Trail de super shot
        if (esSuper && enArco && !trail.empty()) {
            for (int i = 0; i < (int)trail.size(); i++) {
                float a  = (float)i / trail.size();
                float r  = P_RADIO * (0.3f + a * 0.7f);
                sf::CircleShape t(r);
                t.setOrigin(r, r);
                t.setPosition(trail[i].p);
                sf::Uint8 alpha = (sf::Uint8)(a * 130.f);
                t.setFillColor(sf::Color(255, 130 + (sf::Uint8)(a*80), 0, alpha));
                w.draw(t);
            }
        }

        // Halo super shot
        if (esSuper && enArco) {
            for (int i = 3; i >= 1; i--) {
                sf::CircleShape halo(P_RADIO + i * 6.f);
                halo.setOrigin(P_RADIO + i * 6.f, P_RADIO + i * 6.f);
                halo.setPosition(pos);
                halo.setFillColor(sf::Color(255, 100 + i * 30, 0, 55));
                w.draw(halo);
            }
        }

        // Halo alley-oop
        if (esAlleyOop && enArco) {
            sf::CircleShape halo(P_RADIO + 8.f);
            halo.setOrigin(P_RADIO + 8.f, P_RADIO + 8.f);
            halo.setPosition(pos);
            halo.setFillColor(sf::Color(255, 220, 0, 80));
            w.draw(halo);
        }

        // Rebote en tablero: flash naranja
        if (reboteTablero) {
            sf::CircleShape flash(P_RADIO + 12.f);
            flash.setOrigin(P_RADIO + 12.f, P_RADIO + 12.f);
            flash.setPosition(pos);
            flash.setFillColor(sf::Color(255, 80, 0, 90));
            w.draw(flash);
        }

        w.draw(forma);

        // Costuras (rotadas con el spin)
        float rad = angulo * 3.14159f / 180.f;
        sf::VertexArray ln(sf::Lines, 4);
        float cx = std::cos(rad) * P_RADIO;
        float cy = std::sin(rad) * P_RADIO;
        ln[0].position = {pos.x - cx, pos.y - cy};
        ln[1].position = {pos.x + cx, pos.y + cy};
        ln[2].position = {pos.x - cy, pos.y + cx};
        ln[3].position = {pos.x + cy, pos.y - cx};
        for (int i = 0; i < 4; i++) ln[i].color = sf::Color(140, 55, 5, 160);
        w.draw(ln);
    }

    bool cerca(sf::Vector2f p, float radio) const {
        float dx = pos.x - p.x, dy = pos.y - p.y;
        return std::sqrt(dx*dx+dy*dy) < (P_RADIO + radio);
    }

private:
    sf::CircleShape forma;
};
