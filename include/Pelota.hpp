#pragma once
#include <SFML/Graphics.hpp>
#include <cmath>
#include "Constantes.hpp"

// ============================================================
//  Pelota  –  física arcade estilo Street Hoop
// ============================================================
class Pelota {
public:
    sf::Vector2f posicion;
    sf::Vector2f velocidad;

    bool enJuego      = true;   // false mientras está en manos de alguien
    bool enArco       = false;  // animación de tiro en arco
    float tiempoArco  = 0.f;
    float duracionArco= 0.f;
    sf::Vector2f origenArco;
    sf::Vector2f destinoArco;
    float alturaArco  = 0.f;    // altura máxima del arco

    bool superShot    = false;  // lleva efecto fuego

    Pelota() {
        posicion  = {ANCHO_VENTANA / 2.f, ARO_Y};
        velocidad = {0.f, 0.f};
        forma.setRadius(PELOTA_RADIO);
        forma.setOrigin(PELOTA_RADIO, PELOTA_RADIO);
        forma.setFillColor(sf::Color(230, 120, 20));
        forma.setOutlineThickness(2.f);
        forma.setOutlineColor(sf::Color(160, 60, 0));
    }

    // Lanzar la pelota en arco hacia un destino
    void lanzarEnArco(sf::Vector2f desde, sf::Vector2f hasta,
                      float tiempoVuelo, bool esSuper = false) {
        origenArco   = desde;
        destinoArco  = hasta;
        duracionArco = tiempoVuelo;
        tiempoArco   = 0.f;
        alturaArco   = 180.f;
        enArco       = true;
        enJuego      = false;
        superShot    = esSuper;
        posicion     = desde;
        velocidad    = {0.f, 0.f};
    }

    void lanzarPase(sf::Vector2f desde, sf::Vector2f hasta, float tiempo) {
        origenArco   = desde;
        destinoArco  = hasta;
        duracionArco = tiempo;
        tiempoArco   = 0.f;
        alturaArco   = 60.f;
        enArco       = true;
        enJuego      = false;
        superShot    = false;
        posicion     = desde;
        velocidad    = {0.f, 0.f};
    }

    void actualizar(float dt) {
        if (enArco) {
            tiempoArco += dt;
            float t = tiempoArco / duracionArco;
            if (t >= 1.f) {
                t = 1.f;
                enArco   = false;
                enJuego  = true;
                posicion = destinoArco;
                velocidad = {0.f, 0.f};
            }
            // Interpolación parabólica
            posicion.x = origenArco.x + (destinoArco.x - origenArco.x) * t;
            float baseY = origenArco.y + (destinoArco.y - origenArco.y) * t;
            float arco  = alturaArco * 4.f * t * (1.f - t); // parábola
            posicion.y  = baseY - arco;
        } else if (enJuego) {
            posicion += velocidad * dt;
            // Rebotes en los bordes de cancha
            if (posicion.x - PELOTA_RADIO < CANCHA_X) {
                posicion.x = CANCHA_X + PELOTA_RADIO;
                velocidad.x = std::abs(velocidad.x) * 0.7f;
            }
            if (posicion.x + PELOTA_RADIO > CANCHA_X + CANCHA_ANCHO) {
                posicion.x = CANCHA_X + CANCHA_ANCHO - PELOTA_RADIO;
                velocidad.x = -std::abs(velocidad.x) * 0.7f;
            }
            if (posicion.y - PELOTA_RADIO < CANCHA_Y) {
                posicion.y = CANCHA_Y + PELOTA_RADIO;
                velocidad.y = std::abs(velocidad.y) * 0.7f;
            }
            if (posicion.y + PELOTA_RADIO > CANCHA_Y + CANCHA_ALTO) {
                posicion.y = CANCHA_Y + CANCHA_ALTO - PELOTA_RADIO;
                velocidad.y = -std::abs(velocidad.y) * 0.7f;
            }
            // Fricción
            velocidad *= 0.985f;
        }
        forma.setPosition(posicion);
    }

    void dibujar(sf::RenderWindow& w) {
        if (superShot) {
            // Halo naranja/fuego
            sf::CircleShape halo(PELOTA_RADIO + 8.f);
            halo.setOrigin(PELOTA_RADIO + 8.f, PELOTA_RADIO + 8.f);
            halo.setPosition(posicion);
            halo.setFillColor(sf::Color(255, 150, 0, 120));
            w.draw(halo);
        }
        w.draw(forma);

        // Líneas de la pelota
        sf::VertexArray lineas(sf::Lines, 4);
        lineas[0].position = {posicion.x - PELOTA_RADIO, posicion.y};
        lineas[1].position = {posicion.x + PELOTA_RADIO, posicion.y};
        lineas[0].color = lineas[1].color = sf::Color(160, 60, 0, 180);
        lineas[2].position = {posicion.x, posicion.y - PELOTA_RADIO};
        lineas[3].position = {posicion.x, posicion.y + PELOTA_RADIO};
        lineas[2].color = lineas[3].color = sf::Color(160, 60, 0, 180);
        w.draw(lineas);
    }

    bool colisionaCon(sf::Vector2f punto, float radio) {
        float dx = posicion.x - punto.x;
        float dy = posicion.y - punto.y;
        return std::sqrt(dx*dx + dy*dy) < (PELOTA_RADIO + radio);
    }

private:
    sf::CircleShape forma;
};
