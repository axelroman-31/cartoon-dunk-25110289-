#pragma once
#include <SFML/Graphics.hpp>
#include <cmath>
#include "Constantes.hpp"

// ================================================================
//  Cancha  -  vista top-down estilo arcade
// ================================================================
class Cancha {
public:
    // Paleta de colores (cancha callejera)
    sf::Color cPiso   = sf::Color(185, 130, 55);
    sf::Color cLinea  = sf::Color(235, 195, 95);
    sf::Color cZona   = sf::Color(170, 110, 45);
    sf::Color cAro    = sf::Color(255, 55, 15);
    sf::Color cBorde  = sf::Color(145, 90, 25);

    void dibujar(sf::RenderWindow& w) {
        // Piso principal
        rect(w, C_X, C_Y, C_ANCHO, C_ALTO, cPiso, cBorde, 4.f);

        // Zona de lanzamiento libre (pintada) - izquierda
        rect(w, C_X, ARO_Y - 95.f, 115.f, 190.f, cZona, cLinea, 2.f);
        // Zona de lanzamiento libre - derecha
        rect(w, C_X + C_ANCHO - 115.f, ARO_Y - 95.f, 115.f, 190.f, cZona, cLinea, 2.f);

        // Línea central
        linea(w, {C_X + C_ANCHO/2.f, C_Y}, {C_X + C_ANCHO/2.f, C_Y + C_ALTO}, cLinea, 2.f);

        // Círculo central
        circulo(w, {C_X + C_ANCHO/2.f, C_Y + C_ALTO/2.f}, 58.f, cLinea, 2.f);

        // Arco de 3 puntos izquierdo
        arco3p(w, {ARO_IZQ_CX, ARO_Y}, DIST_3P, cLinea, false);
        // Arco de 3 puntos derecho
        arco3p(w, {ARO_DER_CX, ARO_Y}, DIST_3P, cLinea, true);

        // Tableros y aros
        canasta(w, {ARO_IZQ_X, ARO_Y}, false);
        canasta(w, {ARO_DER_X, ARO_Y}, true);
    }

private:
    void rect(sf::RenderWindow& w, float x, float y, float aw, float ah,
              sf::Color fill, sf::Color outline, float ow) {
        sf::RectangleShape r({aw, ah});
        r.setPosition(x, y);
        r.setFillColor(fill);
        r.setOutlineThickness(ow);
        r.setOutlineColor(outline);
        w.draw(r);
    }

    void linea(sf::RenderWindow& w, sf::Vector2f a, sf::Vector2f b,
               sf::Color c, float) {
        sf::VertexArray v(sf::Lines, 2);
        v[0].position = a; v[0].color = c;
        v[1].position = b; v[1].color = c;
        w.draw(v);
    }

    void circulo(sf::RenderWindow& w, sf::Vector2f centro, float r,
                 sf::Color c, float ow) {
        sf::CircleShape cs(r);
        cs.setOrigin(r, r);
        cs.setPosition(centro);
        cs.setFillColor(sf::Color::Transparent);
        cs.setOutlineThickness(ow);
        cs.setOutlineColor(c);
        w.draw(cs);
    }

    // Semicírculo de 3 puntos
    void arco3p(sf::RenderWindow& w, sf::Vector2f centro, float radio,
                sf::Color c, bool derecha) {
        const int N = 36;
        sf::VertexArray va(sf::LinesStrip, N + 1);
        for (int i = 0; i <= N; i++) {
            float base = derecha ? 90.f : -90.f;
            float rango = derecha ? -180.f : 180.f;
            float ang = (base + rango * (i / (float)N)) * 3.14159f / 180.f;
            va[i].position = {centro.x + std::cos(ang)*radio,
                               centro.y + std::sin(ang)*radio};
            va[i].color = c;
        }
        w.draw(va);
    }

    void canasta(sf::RenderWindow& w, sf::Vector2f pos, bool derecha) {
        // Tablero (bloque lateral)
        sf::RectangleShape tablero({8.f, 58.f});
        tablero.setOrigin(4.f, 29.f);
        tablero.setPosition(pos);
        tablero.setFillColor(sf::Color(215, 215, 195));
        tablero.setOutlineThickness(2.f);
        tablero.setOutlineColor(sf::Color(170, 170, 150));
        w.draw(tablero);

        // Cuadrado de enceste en el tablero
        sf::RectangleShape cuad({8.f, 22.f});
        cuad.setOrigin(4.f, 11.f);
        cuad.setPosition(pos);
        cuad.setFillColor(sf::Color::Transparent);
        cuad.setOutlineThickness(2.f);
        cuad.setOutlineColor(sf::Color(200, 60, 60));
        w.draw(cuad);

        // Aro
        float cx = pos.x + (derecha ? -(ARO_RADIO + 8.f) : (ARO_RADIO + 8.f));
        sf::CircleShape aro(ARO_RADIO);
        aro.setOrigin(ARO_RADIO, ARO_RADIO);
        aro.setPosition(cx, pos.y);
        aro.setFillColor(sf::Color::Transparent);
        aro.setOutlineThickness(4.f);
        aro.setOutlineColor(cAro);
        w.draw(aro);

        // Poste de conexión tablero-aro
        linea(w, pos, {cx, pos.y}, sf::Color(160,80,20), 2.f);
    }
};
