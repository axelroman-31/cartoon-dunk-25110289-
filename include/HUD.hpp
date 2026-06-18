#pragma once
#include <SFML/Graphics.hpp>
#include <sstream>
#include <iomanip>
#include <string>
#include <cmath>
#include "Constantes.hpp"

// ================================================================
//  HUD  -  marcador, tiempo, super meters, mensajes flash
// ================================================================
class HUD {
public:
    sf::Font fuente;
    bool     fuenteOk = false;

    // Mensaje flotante
    std::string  msgTexto;
    float        msgTimer = 0.f;
    sf::Color    msgColor = sf::Color::Yellow;

    // Animación de flash al anotar
    float flashTimer = 0.f;
    sf::Color flashColor;

    HUD() {
        fuenteOk = fuente.loadFromFile("assets/front/texto.ttf");
    }

    void flash(sf::Color c, float dur = 0.4f) {
        flashColor = c;
        flashTimer = dur;
    }

    void mensaje(const std::string& txt, sf::Color c = sf::Color::Yellow, float dur = 2.2f) {
        msgTexto = txt;
        msgColor = c;
        msgTimer = dur;
    }

    void actualizar(float dt) {
        if (msgTimer   > 0.f) msgTimer   -= dt;
        if (flashTimer > 0.f) flashTimer -= dt;
    }

    void dibujar(sf::RenderWindow& w,
                 int ptsH, int ptsCPU,
                 float tiempo, int mitad,
                 float superH, float superCPU,
                 const std::string& nombreH,
                 const std::string& nombreCPU) {

        // Flash de pantalla al anotar
        if (flashTimer > 0.f) {
            sf::RectangleShape fsh({(float)W_ANCHO, (float)W_ALTO});
            float a = (flashTimer / 0.4f) * 80.f;
            sf::Color fc = flashColor;
            fc.a = (sf::Uint8)a;
            fsh.setFillColor(fc);
            w.draw(fsh);
        }

        // Panel HUD superior (semi-transparente)
        sf::RectangleShape panel({(float)W_ANCHO, C_Y - 2.f});
        panel.setFillColor(sf::Color(10, 8, 5, 210));
        w.draw(panel);

        // Marcador
        auto txt = [&](const std::string& s, int sz, sf::Color c,
                       float x, float y, bool centrado = true) {
            sf::Text t;
            if (fuenteOk) t.setFont(fuente);
            t.setString(s);
            t.setCharacterSize(sz);
            t.setFillColor(c);
            t.setOutlineThickness(2.f);
            t.setOutlineColor(sf::Color::Black);
            sf::FloatRect b = t.getLocalBounds();
            if (centrado) t.setOrigin(b.width/2.f, b.height/2.f);
            t.setPosition(x, y);
            w.draw(t);
        };

        // Nombres equipos
        txt(nombreH,   18, sf::Color(255,160,60),  160.f, 18.f);
        txt(nombreCPU, 18, sf::Color(100,180,255), 864.f, 18.f);

        // Puntos
        txt(std::to_string(ptsH),   38, sf::Color::White, 240.f, 44.f);
        txt("-",                    30, sf::Color(180,180,180), W_ANCHO/2.f, 44.f);
        txt(std::to_string(ptsCPU), 38, sf::Color::White, 784.f, 44.f);

        // Tiempo y mitad (centro)
        int mn = (int)tiempo / 60;
        int sg = (int)tiempo % 60;
        std::ostringstream oss;
        oss << mn << ":" << std::setfill('0') << std::setw(2) << sg;
        txt(oss.str(), 30, sf::Color(255,220,70), W_ANCHO/2.f, 30.f);
        txt("Mitad " + std::to_string(mitad), 14, sf::Color(180,180,180), W_ANCHO/2.f, 60.f);

        // Super meters
        dibujarSuperMeter(w, superH,   20.f,  W_ALTO - 34.f, sf::Color(255,170,0), sf::Color(255,80,0));
        dibujarSuperMeter(w, superCPU, W_ANCHO - 185.f, W_ALTO - 34.f,
                          sf::Color(70,170,255), sf::Color(0,210,255));

        // Etiquetas SUPER
        txt("SUPER", 13, sf::Color(255,200,0,200),   102.f, W_ALTO - 52.f, false);
        txt("SUPER", 13, sf::Color(70,200,255,200), W_ANCHO - 185.f, W_ALTO - 52.f, false);

        // Mensaje flash
        if (msgTimer > 0.f) {
            float a = std::min(msgTimer, 0.8f) / 0.8f;
            float sc = 1.f + (1.f - a) * 0.3f;
            sf::Text t;
            if (fuenteOk) t.setFont(fuente);
            t.setString(msgTexto);
            t.setCharacterSize(44);
            t.setFillColor(sf::Color(msgColor.r, msgColor.g, msgColor.b, (sf::Uint8)(a*255)));
            t.setOutlineThickness(3.f);
            t.setOutlineColor(sf::Color(0,0,0,(sf::Uint8)(a*200)));
            sf::FloatRect b = t.getLocalBounds();
            t.setOrigin(b.width/2.f, b.height/2.f);
            t.setScale(sc, sc);
            t.setPosition(W_ANCHO/2.f, W_ALTO/2.f - 50.f);
            w.draw(t);
        }

        // Indicador controles (abajo)
        sf::RectangleShape panelBot({(float)W_ANCHO, (float)(W_ALTO - C_Y - C_ALTO)});
        panelBot.setPosition(0, C_Y + C_ALTO);
        panelBot.setFillColor(sf::Color(10,8,5,210));
        w.draw(panelBot);

        txt("WASD:Mover  J:Tiro  K:Pase  L:Robo/Bloqueo  J+K:Super Dunk  Tab:Cambiar jugador",
            11, sf::Color(150,150,150), W_ANCHO/2.f, C_Y + C_ALTO + 10.f);
    }

private:
    void dibujarSuperMeter(sf::RenderWindow& w, float val,
                           float x, float y, sf::Color cLlena, sf::Color cFull) {
        float bw = 165.f, bh = 14.f;
        // Fondo
        sf::RectangleShape bg({bw, bh});
        bg.setPosition(x, y);
        bg.setFillColor(sf::Color(30,30,30,190));
        bg.setOutlineThickness(1.f);
        bg.setOutlineColor(sf::Color(80,80,80));
        w.draw(bg);
        // Llena
        float fill = bw * (val / SM_MAX);
        if (fill > 0.f) {
            sf::RectangleShape llen({fill, bh});
            llen.setPosition(x, y);
            bool lleno = val >= SM_MAX;
            llen.setFillColor(lleno ? cFull : cLlena);
            if (lleno) {
                // Efecto pulso (parpadeo simple)
                static float t = 0.f; t += 0.05f;
                float alpha = 180.f + std::sin(t * 8.f) * 75.f;
                sf::Color pul = cFull;
                pul.a = (sf::Uint8)alpha;
                llen.setFillColor(pul);
            }
            w.draw(llen);
        }
    }
};
