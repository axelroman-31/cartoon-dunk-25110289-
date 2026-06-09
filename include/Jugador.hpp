#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <cmath>
#include "Constantes.hpp"
#include "Pelota.hpp"

// ================================================================
//  Jugador  -  personaje con sprite escalado, stats, lógica
// ================================================================
class Jugador {
public:
    std::string  nombre;
    sf::Vector2f pos;
    sf::Color    colorEquipo;

    // Atributos 1-9 (escala original de Street Slam)
    int aDunk = 5;
    int aTres = 5;
    int aVel  = 5;
    int aDef  = 5;

    bool tienePelota  = false;
    bool esHumano     = false;

    EstadoJ estado    = EstadoJ::IDLE;
    float   timerE    = 0.f;   // duración del estado actual

    // --- Sprite ---
    sf::Texture  tex;
    sf::Sprite   spr;
    bool         sprOk = false;

    // --- Formas auxiliares ---
    sf::CircleShape sombra;
    sf::CircleShape indicador;   // estrella "tiene pelota"
    sf::CircleShape circulo;     // fallback si no hay sprite

    // --- Salto para dunk ---
    float  alturaSalto  = 0.f;   // pixels arriba (0 = en suelo)
    bool   botonAireAct = false; // esperando A en el aire para dunk

    Jugador() {
        sombra.setRadius(17.f);
        sombra.setOrigin(17.f, 8.f);
        sombra.setFillColor(sf::Color(0,0,0,55));

        indicador.setRadius(7.f);
        indicador.setOrigin(7.f, 7.f);
        indicador.setFillColor(sf::Color(255,230,0,220));
        indicador.setOutlineThickness(2.f);
        indicador.setOutlineColor(sf::Color::White);

        circulo.setRadius(J_RADIO);
        circulo.setOrigin(J_RADIO, J_RADIO);
        circulo.setOutlineThickness(2.f);
        circulo.setOutlineColor(sf::Color::White);
    }

    bool cargarSprite(const std::string& ruta) {
        if (!tex.loadFromFile(ruta)) return false;
        sprOk = true;
        spr.setTexture(tex);
        sf::FloatRect b = spr.getLocalBounds();
        spr.setOrigin(b.width / 2.f, b.height);
        float esc = 72.f / b.height;
        spr.setScale(esc, esc);
        return true;
    }

    // Velocidad real según atributo
    float velReal(bool sprint = false) const {
        float v = VEL_BASE + (aVel - 1) * 11.f;
        return sprint ? VEL_SPRINT + (aVel - 1) * 5.f : v;
    }

    // Prob. de enceste (0–1)
    float probTiro(float dist, bool esTres) const {
        float base = esTres ? (aTres / 9.f) : (aDunk / 9.f);
        float penal = std::min(dist / 650.f, 0.45f);
        return std::max(0.12f, base - penal);
    }

    // Prob. de robo por frame
    float probRobo() const {
        return 0.08f + (aDef / 9.f) * 0.32f;
    }

    void mover(sf::Vector2f dir, float dt, bool sprint = false) {
        float len = std::sqrt(dir.x*dir.x + dir.y*dir.y);
        if (len > 0.f) {
            dir /= len;
            pos += dir * velReal(sprint) * dt;
            if (estado == EstadoJ::IDLE || estado == EstadoJ::CORRIENDO)
                estado = EstadoJ::CORRIENDO;
        } else {
            if (estado == EstadoJ::CORRIENDO) estado = EstadoJ::IDLE;
        }
        clampCancha();
    }

    void clampCancha() {
        pos.x = std::max(C_X + J_RADIO, std::min(pos.x, C_X + C_ANCHO - J_RADIO));
        pos.y = std::max(C_Y + J_RADIO, std::min(pos.y, C_Y + C_ALTO  - J_RADIO));
    }

    void setEstado(EstadoJ e, float dur) {
        estado = e;
        timerE = dur;
    }

    void actualizar(float dt) {
        // Timer de estado
        if (timerE > 0.f) {
            timerE -= dt;
            if (timerE <= 0.f && estado != EstadoJ::IDLE && estado != EstadoJ::CORRIENDO) {
                timerE = 0.f;
                estado = EstadoJ::IDLE;
            }
        }

        // Animación de salto (arco sinusoidal simple)
        if (estado == EstadoJ::EN_AIRE || estado == EstadoJ::DUNKEANDO) {
            alturaSalto = 55.f * std::sin(3.14159f * (1.f - timerE / 0.5f));
        } else {
            alturaSalto = 0.f;
        }

        // Actualizar shapes
        sombra.setPosition(pos.x, pos.y + 18.f - alturaSalto * 0.1f);
        sombra.setScale(1.f - alturaSalto * 0.007f, 1.f - alturaSalto * 0.007f);

        if (sprOk) spr.setPosition(pos.x, pos.y + 18.f - alturaSalto);
        circulo.setPosition(pos.x, pos.y - alturaSalto);
        circulo.setFillColor(colorEquipo);
        indicador.setPosition(pos.x, pos.y - 50.f - alturaSalto);
    }

    void dibujar(sf::RenderWindow& w) {
        w.draw(sombra);
        if (sprOk) {
            w.draw(spr);
        } else {
            w.draw(circulo);
        }
        if (tienePelota)           w.draw(indicador);
        if (estado == EstadoJ::SUPER_SHOT) {
            sf::CircleShape glow(J_RADIO + 14.f);
            glow.setOrigin(J_RADIO + 14.f, J_RADIO + 14.f);
            glow.setPosition(pos.x, pos.y - alturaSalto);
            glow.setFillColor(sf::Color(255, 200, 0, 70));
            w.draw(glow);
        }
        // Resaltar jugador activo (humano)
        if (esHumano && (estado == EstadoJ::IDLE || estado == EstadoJ::CORRIENDO)) {
            sf::CircleShape ring(J_RADIO + 4.f);
            ring.setOrigin(J_RADIO + 4.f, J_RADIO + 4.f);
            ring.setPosition(pos.x, pos.y + 15.f);
            ring.setFillColor(sf::Color::Transparent);
            ring.setOutlineThickness(2.f);
            ring.setOutlineColor(sf::Color(255,255,100,160));
            w.draw(ring);
        }
    }
};
