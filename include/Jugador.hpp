#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <cmath>
#include "Constantes.hpp"
#include "Pelota.hpp"

// ================================================================
//  Jugador  -  personaje con sprite escalado, stats, lógica
//  Mecánicas nuevas: finta, alley-oop receptor, empuje, aturdido
// ================================================================
class Jugador {
public:
    std::string  nombre;
    sf::Vector2f pos;
    sf::Color    colorEquipo;

    // Atributos 1-9 (escala de Street Slam)
    int aDunk = 5;
    int aTres = 5;
    int aVel  = 5;
    int aDef  = 5;

    bool tienePelota  = false;
    bool esHumano     = false;

    EstadoJ estado    = EstadoJ::IDLE;
    float   timerE    = 0.f;

    // --- Sprite ---
    sf::Texture  tex;
    sf::Sprite   spr;
    bool         sprOk = false;
    bool         mirando_izq = false;  // flip horizontal

    // --- Formas auxiliares ---
    sf::CircleShape sombra;
    sf::CircleShape indicador;
    sf::CircleShape circulo;

    // --- Animación de salto ---
    float alturaSalto   = 0.f;
    bool  botonAireAct  = false;

    // --- Finta ---
    bool  fintaActiva   = false;
    float timerFinta    = 0.f;

    // --- Alley-oop receptor ---
    bool  esperandoAlleyOop = false;
    float timerAlleyOop     = 0.f;

    // --- Empuje / aturdido ---
    float timerAturdido = 0.f;
    sf::Vector2f velEmpuje;

    // --- Animación de parpadeo (tras robo/bloqueo) ---
    float timerFlash    = 0.f;
    bool  visible       = true;

    // --- Estamina (se agota al sprintar, se recupera en idle) ---
    float estamina      = 100.f;

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

    // Velocidad real según atributo y estamina
    float velReal(bool sprint = false) const {
        float mult = 0.6f + (estamina / 100.f) * 0.4f;  // baja si cansado
        if (sprint) {
            return (VEL_SPRINT + (aVel - 1) * 5.f) * mult;
        }
        return (VEL_BASE + (aVel - 1) * 11.f) * mult;
    }

    // Prob. de enceste
    float probTiro(float dist, bool esTres) const {
        float base = esTres ? (aTres / 9.f) : (aDunk / 9.f);
        float penal = std::min(dist / 650.f, 0.45f);
        // Penalización por estar aturdido
        if (timerAturdido > 0.f) base *= 0.5f;
        return std::max(0.10f, base - penal);
    }

    float probBloqueo() const {
        return 0.10f + (aDef / 9.f) * 0.38f;
    }

    float probRobo() const {
        return 0.07f + (aDef / 9.f) * 0.30f;
    }

    // Prob de intercepción de pase
    float probInterceptar() const {
        return 0.04f + (aDef / 9.f) * 0.22f;
    }

    void mover(sf::Vector2f dir, float dt, bool sprint = false) {
        if (timerAturdido > 0.f) {
            // Movimiento de empuje (sin control)
            pos += velEmpuje * dt;
            velEmpuje *= 0.88f;
            clampCancha();
            return;
        }
        float len = std::sqrt(dir.x*dir.x + dir.y*dir.y);
        if (len > 0.f) {
            dir /= len;
            // Flip de sprite según dirección
            if (dir.x < -0.1f)       mirando_izq = true;
            else if (dir.x > 0.1f)   mirando_izq = false;

            float v = velReal(sprint);
            pos += dir * v * dt;

            // Gestión de estamina
            if (sprint) {
                estamina = std::max(0.f, estamina - 22.f * dt);
            }

            if (estado == EstadoJ::IDLE || estado == EstadoJ::CORRIENDO)
                estado = EstadoJ::CORRIENDO;
        } else {
            // Recuperar estamina en reposo
            estamina = std::min(100.f, estamina + 28.f * dt);
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

    // Recibir empujón
    void recibirEmpuje(sf::Vector2f dir, float fuerza) {
        velEmpuje    = dir * fuerza;
        timerAturdido = 0.30f;
        if (tienePelota) {
            // Hay probabilidad de perder la pelota al ser empujado
            // (se maneja desde fuera)
        }
        setEstado(EstadoJ::ATURDIDO, 0.30f);
    }

    void actualizar(float dt) {
        // Timer de estado
        if (timerE > 0.f) {
            timerE -= dt;
            if (timerE <= 0.f && estado != EstadoJ::IDLE && estado != EstadoJ::CORRIENDO) {
                timerE = 0.f;
                estado = EstadoJ::IDLE;
                fintaActiva = false;
                esperandoAlleyOop = false;
            }
        }

        // Aturdido
        if (timerAturdido > 0.f) {
            timerAturdido -= dt;
            if (timerAturdido <= 0.f) {
                timerAturdido = 0.f;
                velEmpuje = {};
                if (estado == EstadoJ::ATURDIDO) estado = EstadoJ::IDLE;
            }
        }

        // Finta
        if (fintaActiva) {
            timerFinta -= dt;
            if (timerFinta <= 0.f) fintaActiva = false;
        }

        // Alley-oop esperando
        if (esperandoAlleyOop) {
            timerAlleyOop -= dt;
            if (timerAlleyOop <= 0.f) {
                esperandoAlleyOop = false;
                // Si no llegó el pase, aterrizar
                if (estado == EstadoJ::ALLEYOOP_VUELO) estado = EstadoJ::IDLE;
            }
        }

        // Parpadeo tras eventos
        if (timerFlash > 0.f) {
            timerFlash -= dt;
            visible = (int)(timerFlash * 12.f) % 2 == 0;
        } else {
            visible = true;
        }

        // Animación de salto
        if (estado == EstadoJ::EN_AIRE || estado == EstadoJ::DUNKEANDO) {
            float progN = timerE > 0.f ? (1.f - timerE / 0.55f) : 1.f;
            alturaSalto = 55.f * std::sin(3.14159f * progN);
        } else if (estado == EstadoJ::ALLEYOOP_VUELO) {
            alturaSalto = 70.f;  // en el aire esperando
        } else if (estado == EstadoJ::BLOQUEANDO) {
            float progN = timerE > 0.f ? (1.f - timerE / 0.35f) : 1.f;
            alturaSalto = 45.f * std::sin(3.14159f * progN);
        } else {
            alturaSalto = 0.f;
        }

        // Actualizar shapes
        sombra.setPosition(pos.x, pos.y + 18.f - alturaSalto * 0.1f);
        float ss = 1.f - alturaSalto * 0.007f;
        sombra.setScale(ss, ss);

        float yDibujo = pos.y + 18.f - alturaSalto;
        if (sprOk) {
            spr.setPosition(pos.x, yDibujo);
            float ex = mirando_izq ? -std::abs(spr.getScale().x) : std::abs(spr.getScale().x);
            spr.setScale(ex, std::abs(spr.getScale().y));
        }
        circulo.setPosition(pos.x, pos.y - alturaSalto);
        circulo.setFillColor(colorEquipo);
        indicador.setPosition(pos.x, pos.y - 50.f - alturaSalto);
    }

    void dibujar(sf::RenderWindow& w) {
        if (!visible) return;

        w.draw(sombra);

        // Halo de alley-oop receptor esperando
        if (esperandoAlleyOop) {
            float pulse = 0.5f + 0.5f * std::sin(timerAlleyOop * 10.f);
            sf::CircleShape ring(J_RADIO + 10.f + pulse * 6.f);
            ring.setOrigin(ring.getRadius(), ring.getRadius());
            ring.setPosition(pos.x, pos.y - alturaSalto);
            ring.setFillColor(sf::Color::Transparent);
            ring.setOutlineThickness(3.f);
            ring.setOutlineColor(sf::Color(255, 220, 0, (sf::Uint8)(160 + pulse * 80)));
            w.draw(ring);
        }

        // Halo de finta
        if (fintaActiva) {
            sf::CircleShape ring(J_RADIO + 7.f);
            ring.setOrigin(ring.getRadius(), ring.getRadius());
            ring.setPosition(pos.x, pos.y - alturaSalto);
            ring.setFillColor(sf::Color::Transparent);
            ring.setOutlineThickness(2.f);
            ring.setOutlineColor(sf::Color(255, 100, 0, 180));
            w.draw(ring);
        }

        if (sprOk) {
            w.draw(spr);
        } else {
            w.draw(circulo);
        }

        if (tienePelota) w.draw(indicador);

        // Super shot glow
        if (estado == EstadoJ::SUPER_SHOT) {
            sf::CircleShape glow(J_RADIO + 14.f);
            glow.setOrigin(J_RADIO + 14.f, J_RADIO + 14.f);
            glow.setPosition(pos.x, pos.y - alturaSalto);
            glow.setFillColor(sf::Color(255, 200, 0, 70));
            w.draw(glow);
        }

        // Resaltar jugador activo
        if (esHumano && (estado == EstadoJ::IDLE || estado == EstadoJ::CORRIENDO
                         || estado == EstadoJ::FINTANDO)) {
            sf::CircleShape ring(J_RADIO + 4.f);
            ring.setOrigin(J_RADIO + 4.f, J_RADIO + 4.f);
            ring.setPosition(pos.x, pos.y + 15.f);
            ring.setFillColor(sf::Color::Transparent);
            ring.setOutlineThickness(2.f);
            ring.setOutlineColor(sf::Color(255,255,100,160));
            w.draw(ring);
        }

        // Indicador de aturdido (estrellas)
        if (timerAturdido > 0.f) {
            for (int i = 0; i < 3; i++) {
                float ang = (timerAturdido * 8.f + i * 2.094f);
                sf::CircleShape star(4.f, 5);
                star.setOrigin(4.f, 4.f);
                star.setPosition(pos.x + std::cos(ang) * 22.f,
                                 pos.y - alturaSalto - 30.f + std::sin(ang) * 8.f);
                star.setFillColor(sf::Color(255, 255, 50, 200));
                w.draw(star);
            }
        }
    }
};
