#pragma once
#include <SFML/Graphics.hpp>
#include "Constantes.hpp"

// ============================================================
//  Cancha  –  dibuja la cancha vista top-down estilo arcade
// ============================================================
class Cancha {
public:
    sf::Color colorPiso    = sf::Color(200, 145, 60);
    sf::Color colorLineas  = sf::Color(240, 200, 100);
    sf::Color colorAros    = sf::Color(255, 60, 20);

    void dibujar(sf::RenderWindow& w) {
        // Fondo de cancha
        sf::RectangleShape fondo({CANCHA_ANCHO, CANCHA_ALTO});
        fondo.setPosition(CANCHA_X, CANCHA_Y);
        fondo.setFillColor(colorPiso);
        fondo.setOutlineThickness(4.f);
        fondo.setOutlineColor(sf::Color(160, 100, 30));
        w.draw(fondo);

        // Lineas de cancha (color madera oscuro)
        sf::Color lc = colorLineas;

        // Línea central
        dibujarLinea(w, {CANCHA_X + CANCHA_ANCHO/2.f, CANCHA_Y},
                        {CANCHA_X + CANCHA_ANCHO/2.f, CANCHA_Y + CANCHA_ALTO}, lc, 2.f);

        // Círculo central
        dibujarCirculo(w, {CANCHA_X + CANCHA_ANCHO/2.f, CANCHA_Y + CANCHA_ALTO/2.f},
                       60.f, lc, 2.f);

        // Zonas de 3 puntos (semicírculos)
        // Izquierda
        dibujarSemicirculo(w, {ARO_IZQUIERDA_X, ARO_Y},
                           DIST_TRES_PUNTOS, lc, 2.f, false);
        // Derecha
        dibujarSemicirculo(w, {ARO_DERECHA_X, ARO_Y},
                           DIST_TRES_PUNTOS, lc, 2.f, true);

        // Zonas de tiro libre (rectangulares)
        float zw = 120.f, zh = 200.f;
        // Izquierda
        sf::RectangleShape zonaL({zw, zh});
        zonaL.setPosition(CANCHA_X, ARO_Y - zh/2.f);
        zonaL.setFillColor(sf::Color(180, 120, 50));
        zonaL.setOutlineThickness(2.f);
        zonaL.setOutlineColor(lc);
        w.draw(zonaL);
        // Derecha
        sf::RectangleShape zonaR({zw, zh});
        zonaR.setPosition(CANCHA_X + CANCHA_ANCHO - zw, ARO_Y - zh/2.f);
        zonaR.setFillColor(sf::Color(180, 120, 50));
        zonaR.setOutlineThickness(2.f);
        zonaR.setOutlineColor(lc);
        w.draw(zonaR);

        // Tableros y aros
        dibujarCanasta(w, {ARO_IZQUIERDA_X, ARO_Y}, false);
        dibujarCanasta(w, {ARO_DERECHA_X,   ARO_Y}, true);
    }

private:
    void dibujarLinea(sf::RenderWindow& w, sf::Vector2f a, sf::Vector2f b,
                      sf::Color c, float grosor) {
        sf::VertexArray v(sf::Lines, 2);
        v[0].position = a; v[0].color = c;
        v[1].position = b; v[1].color = c;
        w.draw(v);
    }

    void dibujarCirculo(sf::RenderWindow& w, sf::Vector2f centro,
                        float radio, sf::Color c, float grosor) {
        sf::CircleShape circulo(radio);
        circulo.setOrigin(radio, radio);
        circulo.setPosition(centro);
        circulo.setFillColor(sf::Color::Transparent);
        circulo.setOutlineThickness(grosor);
        circulo.setOutlineColor(c);
        w.draw(circulo);
    }

    void dibujarSemicirculo(sf::RenderWindow& w, sf::Vector2f centro,
                             float radio, sf::Color c, float grosor, bool miraDerecha) {
        const int N = 32;
        sf::VertexArray va(sf::LinesStrip, N + 1);
        for (int i = 0; i <= N; i++) {
            float ang = (miraDerecha ? 90.f : -90.f) +
                        (miraDerecha ? -180.f : 180.f) * (i / (float)N);
            float rad = ang * 3.14159f / 180.f;
            va[i].position = {centro.x + std::cos(rad) * radio,
                               centro.y + std::sin(rad) * radio};
            va[i].color = c;
        }
        w.draw(va);
    }

    void dibujarCanasta(sf::RenderWindow& w, sf::Vector2f pos, bool derecha) {
        // Tablero
        float tw = 10.f, th = 60.f;
        sf::RectangleShape tablero({tw, th});
        tablero.setOrigin(tw / 2.f, th / 2.f);
        tablero.setPosition(pos);
        tablero.setFillColor(sf::Color(220, 220, 200));
        tablero.setOutlineThickness(2.f);
        tablero.setOutlineColor(sf::Color(180, 180, 160));
        w.draw(tablero);

        // Aro
        sf::CircleShape aro(CANASTA_RADIO);
        aro.setOrigin(CANASTA_RADIO, CANASTA_RADIO);
        aro.setPosition(pos.x + (derecha ? -CANASTA_RADIO - 5.f : CANASTA_RADIO + 5.f), pos.y);
        aro.setFillColor(sf::Color::Transparent);
        aro.setOutlineThickness(4.f);
        aro.setOutlineColor(colorAros);
        w.draw(aro);
    }
};
