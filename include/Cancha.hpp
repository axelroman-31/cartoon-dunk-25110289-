#pragma once
#include <SFML/Graphics.hpp>
#include <cmath>
#include "Constantes.hpp"

// ================================================================
//  Cancha  -  imagen de Astros con perspectiva + overlay de líneas
//  La imagen se usa como fondo. Encima se dibujan las líneas 2D
//  del área jugable con un efecto de perspectiva falsa.
// ================================================================
class Cancha {
public:
    sf::Texture texCancha;
    sf::Sprite  sprCancha;
    bool        canchaOk = false;

    // Colores de overlay (líneas sobre la foto)
    sf::Color cLinea  = sf::Color(255, 255, 255, 140);
    sf::Color cZona   = sf::Color(200, 230, 255,  50);
    sf::Color cAro    = sf::Color(255,  80,  10, 230);

    Cancha() {
        canchaOk = texCancha.loadFromFile("assets/imagenes/cancha de astros.png");
        if (canchaOk) {
            sprCancha.setTexture(texCancha);
            // Escalar para cubrir exactamente la ventana
            float sx = (float)W_ANCHO / texCancha.getSize().x;
            float sy = (float)W_ALTO  / texCancha.getSize().y;
            float sc = std::max(sx, sy);
            sprCancha.setScale(sc, sc);
            // Centrar
            float pw = texCancha.getSize().x * sc;
            float ph = texCancha.getSize().y * sc;
            sprCancha.setPosition((W_ANCHO - pw) / 2.f, (W_ALTO - ph) / 2.f);
        }
    }

    void dibujar(sf::RenderWindow& w) {
        // ── 1. Imagen de fondo (cancha de astros) ──
        if (canchaOk) {
            w.draw(sprCancha);
        } else {
            // Fallback: piso madera sólido
            sf::RectangleShape piso({(float)W_ANCHO, (float)W_ALTO});
            piso.setFillColor(sf::Color(185, 130, 55));
            w.draw(piso);
        }

        // ── 2. Overlay oscuro para distinguir el área jugable ──
        sf::RectangleShape ov({(float)W_ANCHO, (float)W_ALTO});
        ov.setFillColor(sf::Color(0, 0, 0, 55));
        w.draw(ov);

        // ── 3. Perspectiva falsa: degradado superior más oscuro ──
        // Simula la visión desde el costado (como Neo Geo)
        sf::VertexArray grad(sf::Quads, 4);
        grad[0].position = {0.f, 0.f};
        grad[1].position = {(float)W_ANCHO, 0.f};
        grad[2].position = {(float)W_ANCHO, C_Y + C_ALTO * 0.45f};
        grad[3].position = {0.f, C_Y + C_ALTO * 0.45f};
        grad[0].color = sf::Color(0, 0, 0, 90);
        grad[1].color = sf::Color(0, 0, 0, 90);
        grad[2].color = sf::Color(0, 0, 0, 0);
        grad[3].color = sf::Color(0, 0, 0, 0);
        w.draw(grad);

        // ── 4. Líneas de la cancha (overlay sobre la imagen) ──
        dibujarLineas(w);

        // ── 5. Aros y tableros ──
        dibujarCanasta(w, {ARO_IZQ_X, ARO_Y}, false);
        dibujarCanasta(w, {ARO_DER_X, ARO_Y}, true);
    }

private:
    // Helper: línea con grosor
    void linea(sf::RenderWindow& w, sf::Vector2f a, sf::Vector2f b, sf::Color c, float grosor=2.f) {
        // Vector perpendicular para dar grosor
        sf::Vector2f d = b - a;
        float len = std::sqrt(d.x*d.x + d.y*d.y);
        if (len < 0.001f) return;
        sf::Vector2f n = {-d.y/len * grosor*0.5f, d.x/len * grosor*0.5f};
        sf::VertexArray q(sf::Quads, 4);
        q[0].position = a + n; q[1].position = a - n;
        q[2].position = b - n; q[3].position = b + n;
        for (int i=0;i<4;i++) q[i].color = c;
        w.draw(q);
    }

    void circulo(sf::RenderWindow& w, sf::Vector2f centro, float r, sf::Color c, float grosor=2.f) {
        sf::CircleShape cs(r);
        cs.setOrigin(r, r);
        cs.setPosition(centro);
        cs.setFillColor(sf::Color::Transparent);
        cs.setOutlineThickness(grosor);
        cs.setOutlineColor(c);
        w.draw(cs);
    }

    // Semicírculo de 3 puntos con perspectiva
    void arco3p(sf::RenderWindow& w, sf::Vector2f centro, float radio,
                sf::Color c, bool derecha) {
        const int N = 40;
        sf::VertexArray va(sf::LinesStrip, N+1);
        for (int i=0;i<=N;i++) {
            float base  = derecha ? 90.f : -90.f;
            float rango = derecha ? -180.f : 180.f;
            float ang   = (base + rango * (i/(float)N)) * 3.14159f / 180.f;
            // Perspectiva leve: aplasta eje Y al 75%
            va[i].position = {centro.x + std::cos(ang)*radio,
                               centro.y + std::sin(ang)*radio*0.75f};
            va[i].color = c;
        }
        w.draw(va);
    }

    void dibujarLineas(sf::RenderWindow& w) {
        // Zona de lanzamiento libre (izquierda y derecha)
        sf::RectangleShape zonaL({115.f, 190.f});
        zonaL.setPosition(C_X, ARO_Y - 95.f);
        zonaL.setFillColor(cZona);
        zonaL.setOutlineThickness(2.f);
        zonaL.setOutlineColor(cLinea);
        w.draw(zonaL);

        sf::RectangleShape zonaR({115.f, 190.f});
        zonaR.setPosition(C_X + C_ANCHO - 115.f, ARO_Y - 95.f);
        zonaR.setFillColor(cZona);
        zonaR.setOutlineThickness(2.f);
        zonaR.setOutlineColor(cLinea);
        w.draw(zonaR);

        // Borde de cancha principal
        sf::RectangleShape borde({C_ANCHO, C_ALTO});
        borde.setPosition(C_X, C_Y);
        borde.setFillColor(sf::Color::Transparent);
        borde.setOutlineThickness(3.f);
        borde.setOutlineColor(sf::Color(255,255,255,100));
        w.draw(borde);

        // Línea central
        linea(w, {C_X+C_ANCHO/2.f, C_Y}, {C_X+C_ANCHO/2.f, C_Y+C_ALTO}, cLinea, 2.f);

        // Círculo central (con perspectiva aplanada)
        sf::CircleShape cc(58.f, 40);
        cc.setOrigin(58.f, 43.f);
        cc.setPosition(C_X+C_ANCHO/2.f, C_Y+C_ALTO/2.f);
        cc.setScale(1.f, 0.75f);
        cc.setFillColor(sf::Color::Transparent);
        cc.setOutlineThickness(2.f);
        cc.setOutlineColor(cLinea);
        w.draw(cc);

        // Líneas de tiro libre
        linea(w, {C_X+115.f, ARO_Y-95.f}, {C_X+115.f, ARO_Y+95.f}, cLinea, 2.f);
        linea(w, {C_X+C_ANCHO-115.f, ARO_Y-95.f}, {C_X+C_ANCHO-115.f, ARO_Y+95.f}, cLinea, 2.f);

        // Semicírculos de tiro libre
        circulo(w, {C_X+115.f, ARO_Y}, 60.f, cLinea, 2.f);
        circulo(w, {C_X+C_ANCHO-115.f, ARO_Y}, 60.f, cLinea, 2.f);

        // Arcos de 3 puntos
        arco3p(w, {ARO_IZQ_CX, ARO_Y}, DIST_3P, sf::Color(255,255,255,160), false);
        arco3p(w, {ARO_DER_CX, ARO_Y}, DIST_3P, sf::Color(255,255,255,160), true);

        // Sombras de profundidad en el suelo (debajo de cada aro)
        for (auto cx : {ARO_IZQ_CX, ARO_DER_CX}) {
            sf::CircleShape sh(35.f, 30);
            sh.setOrigin(35.f, 17.5f);
            sh.setPosition(cx, ARO_Y + 5.f);
            sh.setScale(1.f, 0.5f);
            sh.setFillColor(sf::Color(0,0,0,60));
            w.draw(sh);
        }
    }

    void dibujarCanasta(sf::RenderWindow& w, sf::Vector2f pos, bool derecha) {
        float cx = pos.x + (derecha ? -(ARO_RADIO+8.f) : (ARO_RADIO+8.f));

        // ── Poste (perspectiva: más gordo abajo) ──
        sf::VertexArray poste(sf::Quads, 4);
        poste[0].position = {pos.x - 5.f, C_Y + 10.f};
        poste[1].position = {pos.x + 5.f, C_Y + 10.f};
        poste[2].position = {pos.x + 8.f, ARO_Y + 30.f};
        poste[3].position = {pos.x - 8.f, ARO_Y + 30.f};
        for (int i=0;i<4;i++) poste[i].color = sf::Color(160,160,170,200);
        w.draw(poste);

        // ── Tablero (cuadrado blanco con perspectiva) ──
        float tw = 50.f, th = 38.f;
        float ty = ARO_Y - 35.f;
        sf::VertexArray tab(sf::Quads, 4);
        if (derecha) {
            tab[0].position = {pos.x - tw, ty};
            tab[1].position = {pos.x,      ty + 4.f};
            tab[2].position = {pos.x,      ty + th - 4.f};
            tab[3].position = {pos.x - tw, ty + th};
        } else {
            tab[0].position = {pos.x,      ty + 4.f};
            tab[1].position = {pos.x + tw, ty};
            tab[2].position = {pos.x + tw, ty + th};
            tab[3].position = {pos.x,      ty + th - 4.f};
        }
        for (int i=0;i<4;i++) tab[i].color = sf::Color(230,230,220,200);
        w.draw(tab);

        // Cuadro rojo en el tablero
        sf::VertexArray cuad(sf::Quads, 4);
        float cm = 8.f;
        if (derecha) {
            cuad[0].position = {pos.x - tw + cm, ty + cm};
            cuad[1].position = {pos.x - cm,       ty + cm + 2.f};
            cuad[2].position = {pos.x - cm,       ty + th - cm - 2.f};
            cuad[3].position = {pos.x - tw + cm,  ty + th - cm};
        } else {
            cuad[0].position = {pos.x + cm,       ty + cm + 2.f};
            cuad[1].position = {pos.x + tw - cm,  ty + cm};
            cuad[2].position = {pos.x + tw - cm,  ty + th - cm};
            cuad[3].position = {pos.x + cm,       ty + th - cm - 2.f};
        }
        sf::Color rojo(200,40,20,180);
        for (int i=0;i<4;i++) cuad[i].color = rojo;
        w.draw(cuad);

        // ── Brazo del aro (perspectiva) ──
        float armY = ARO_Y - 5.f;
        linea(w, {pos.x, armY}, {cx, armY}, sf::Color(200,150,30,200), 4.f);

        // ── Aro (elipse aplastada = perspectiva) ──
        sf::CircleShape aro(ARO_RADIO, 30);
        aro.setOrigin(ARO_RADIO, ARO_RADIO * 0.4f);
        aro.setPosition(cx, ARO_Y);
        aro.setScale(1.f, 0.40f);           // aplastado = vista lateral
        aro.setFillColor(sf::Color::Transparent);
        aro.setOutlineThickness(4.f);
        aro.setOutlineColor(cAro);
        w.draw(aro);

        // ── Red (triángulo aplastado debajo del aro) ──
        sf::VertexArray red(sf::Triangles, 3);
        red[0].position = {cx - ARO_RADIO, ARO_Y + 3.f};
        red[1].position = {cx + ARO_RADIO, ARO_Y + 3.f};
        red[2].position = {cx, ARO_Y + 22.f};
        red[0].color = red[1].color = red[2].color = sf::Color(220,220,200,100);
        w.draw(red);

        // Líneas de la red
        for (int i=-2;i<=2;i++) {
            float nx = cx + i * (ARO_RADIO/2.5f);
            linea(w, {nx, ARO_Y + 2.f}, {cx, ARO_Y + 22.f}, sf::Color(200,200,180,130), 1.f);
        }
        linea(w, {cx-ARO_RADIO, ARO_Y+3.f}, {cx+ARO_RADIO, ARO_Y+3.f}, sf::Color(200,200,180,130), 1.f);
        linea(w, {cx-ARO_RADIO*0.7f, ARO_Y+10.f}, {cx+ARO_RADIO*0.7f, ARO_Y+10.f}, sf::Color(200,200,180,80), 1.f);
    }
};
