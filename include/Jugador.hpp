#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <cmath>
#include "Constantes.hpp"
#include "Pelota.hpp"

// ============================================================
//  Jugador  – personaje de cancha con sprite y mecánicas
// ============================================================
class Jugador {
public:
    std::string nombre;
    sf::Vector2f posicion;
    sf::Vector2f velocidad;
    sf::Color    colorEquipo;

    // Atributos (1-9, escala como Street Slam)
    int atk_dunk;
    int atk_tres;
    int atk_vel;
    int atk_def;

    bool tienePelota   = false;
    bool esJugadorHumano = false;
    bool esCPU         = false;

    // Super meter individual (comparte el del equipo, pero lo referencia)
    float* superMeter  = nullptr;

    EstadoJugador estado = EstadoJugador::IDLE;
    float timerEstado    = 0.f;

    // Sprite
    sf::Texture textura;
    sf::Sprite  sprite;
    bool        texturaOk = false;

    // Sombra
    sf::CircleShape sombra;

    // Indicador "tiene pelota"
    sf::CircleShape indicador;

    Jugador() {
        sombra.setRadius(18.f);
        sombra.setOrigin(18.f, 4.f);
        sombra.setFillColor(sf::Color(0,0,0,60));

        indicador.setRadius(8.f);
        indicador.setOrigin(8.f, 8.f);
        indicador.setFillColor(sf::Color(255,220,0,200));
        indicador.setOutlineThickness(2.f);
        indicador.setOutlineColor(sf::Color::White);
    }

    void cargarTextura(const std::string& ruta) {
        if (textura.loadFromFile(ruta)) {
            texturaOk = true;
            sprite.setTexture(textura);
            // Centrar y escalar el sprite para que quepa en ~55x75px
            sf::FloatRect bounds = sprite.getLocalBounds();
            sprite.setOrigin(bounds.width / 2.f, bounds.height);
            float escala = 75.f / bounds.height;
            sprite.setScale(escala, escala);
        }
    }

    void setPosicion(sf::Vector2f pos) {
        posicion = pos;
    }

    float velocidadReal() const {
        return VELOCIDAD_BASE + (atk_vel - 1) * 10.f;
    }

    void mover(sf::Vector2f dir, float dt, bool sprint = false) {
        float speed = sprint ? VELOCIDAD_SPRINT : velocidadReal();
        float len = std::sqrt(dir.x*dir.x + dir.y*dir.y);
        if (len > 0.f) {
            dir /= len;
            posicion += dir * speed * dt;
            velocidad = dir;
            estado = EstadoJugador::CORRIENDO;
        } else {
            estado = EstadoJugador::IDLE;
            velocidad = {0.f, 0.f};
        }
        // Clamp dentro de cancha
        posicion.x = std::max(CANCHA_X + JUGADOR_RADIO, std::min(posicion.x, CANCHA_X + CANCHA_ANCHO - JUGADOR_RADIO));
        posicion.y = std::max(CANCHA_Y + JUGADOR_RADIO, std::min(posicion.y, CANCHA_Y + CANCHA_ALTO - JUGADOR_RADIO));
    }

    // Probabilidad de éxito de tiro (0.0–1.0) según atributo y distancia al aro
    float probTiro(float distAro, bool esTres) const {
        float base = esTres ? (atk_tres / 9.f) : (atk_dunk / 9.f);
        // Penalización por distancia
        float penalDist = std::min(distAro / 600.f, 0.5f);
        return std::max(0.1f, base - penalDist);
    }

    // Probabilidad de robo según defensa
    float probRobo() const {
        return 0.1f + (atk_def / 9.f) * 0.35f;
    }

    bool intentarRobo(Pelota& pelota, float dt) {
        if (estado == EstadoJugador::BLOQUEANDO) return false;
        float dx = pelota.posicion.x - posicion.x;
        float dy = pelota.posicion.y - posicion.y;
        float dist = std::sqrt(dx*dx + dy*dy);
        if (dist < RADIO_ROBO && pelota.enJuego) {
            // Ventana de tiempo muy corta para equilibrar
            float prob = probRobo() * dt * 3.f;
            return ((float)rand() / RAND_MAX) < prob;
        }
        return false;
    }

    void actualizar(float dt) {
        timerEstado -= dt;
        if (timerEstado < 0.f) timerEstado = 0.f;

        if (timerEstado <= 0.f && estado != EstadoJugador::IDLE
            && estado != EstadoJugador::CORRIENDO) {
            estado = EstadoJugador::IDLE;
        }

        // Actualizar posiciones de dibujado
        sombra.setPosition(posicion.x, posicion.y + 15.f);
        if (texturaOk) sprite.setPosition(posicion.x, posicion.y + 20.f);
        indicador.setPosition(posicion.x, posicion.y - 45.f);
    }

    void dibujar(sf::RenderWindow& w) {
        w.draw(sombra);
        if (texturaOk) {
            w.draw(sprite);
        } else {
            sf::CircleShape c(JUGADOR_RADIO);
            c.setOrigin(JUGADOR_RADIO, JUGADOR_RADIO);
            c.setPosition(posicion);
            c.setFillColor(colorEquipo);
            c.setOutlineThickness(2.f);
            c.setOutlineColor(sf::Color::White);
            w.draw(c);
        }
        if (tienePelota) {
            w.draw(indicador);
        }
        // Estado superShot: brillo amarillo alrededor
        if (estado == EstadoJugador::SUPER_SHOT) {
            sf::CircleShape glow(JUGADOR_RADIO + 10.f);
            glow.setOrigin(JUGADOR_RADIO + 10.f, JUGADOR_RADIO + 10.f);
            glow.setPosition(posicion);
            glow.setFillColor(sf::Color(255,220,0,80));
            w.draw(glow);
        }
    }
};
