#pragma once
#include <SFML/Graphics.hpp>
#include <sstream>
#include <iomanip>
#include "Constantes.hpp"

// ============================================================
//  HUD  –  marcador, tiempo, super meter, mensajes
// ============================================================
class HUD {
public:
    sf::Font fuente;
    bool     fuenteOk = false;

    // Textos
    sf::Text txtMarcador;
    sf::Text txtTiempo;
    sf::Text txtMensaje;
    sf::Text txtMitad;
    sf::Text txtSuperEtiqueta;

    // Barras de Super Meter
    sf::RectangleShape barFondoL, barLlenaL;   // equipo izquierda
    sf::RectangleShape barFondoR, barLlenaR;   // equipo derecha

    float tiempoMensaje = 0.f;

    HUD() {
        // Fuente de respaldo si no carga el archivo
        if (fuente.loadFromFile("assets/front/texto.ttf")) {
            fuenteOk = true;
        }

        auto initText = [&](sf::Text& t, int size, sf::Color color) {
            if (fuenteOk) t.setFont(fuente);
            t.setCharacterSize(size);
            t.setFillColor(color);
            t.setOutlineThickness(2.f);
            t.setOutlineColor(sf::Color::Black);
        };

        initText(txtMarcador,    32, sf::Color::White);
        initText(txtTiempo,      26, sf::Color(255,220,80));
        initText(txtMensaje,     36, sf::Color(255,80,80));
        initText(txtMitad,       22, sf::Color(200,200,200));
        initText(txtSuperEtiqueta,14, sf::Color(255,200,0));

        // Barras super meter
        float barW = 160.f, barH = 14.f;
        float margen = 20.f;
        // Izquierda
        barFondoL.setSize({barW, barH});
        barFondoL.setFillColor(sf::Color(40,40,40,180));
        barFondoL.setPosition(margen, ALTO_VENTANA - 36.f);
        barLlenaL.setSize({barW, barH});
        barLlenaL.setPosition(margen, ALTO_VENTANA - 36.f);
        barLlenaL.setFillColor(sf::Color(255,180,0));
        // Derecha
        barFondoR.setSize({barW, barH});
        barFondoR.setFillColor(sf::Color(40,40,40,180));
        barFondoR.setPosition(ANCHO_VENTANA - margen - barW, ALTO_VENTANA - 36.f);
        barLlenaR.setSize({barW, barH});
        barLlenaR.setPosition(ANCHO_VENTANA - margen - barW, ALTO_VENTANA - 36.f);
        barLlenaR.setFillColor(sf::Color(80,180,255));
    }

    void mostrarMensaje(const std::string& msg, float duracion = 2.f) {
        txtMensaje.setString(msg);
        sf::FloatRect b = txtMensaje.getLocalBounds();
        txtMensaje.setOrigin(b.width / 2.f, b.height / 2.f);
        txtMensaje.setPosition(ANCHO_VENTANA / 2.f, ALTO_VENTANA / 2.f - 60.f);
        tiempoMensaje = duracion;
    }

    void actualizar(float dt) {
        tiempoMensaje -= dt;
    }

    void dibujar(sf::RenderWindow& w,
                 int puntosL, int puntosR,
                 float tiempo,
                 int mitad,
                 float superL, float superR,
                 const std::string& nombreL, const std::string& nombreR) {
        // Marcador
        std::ostringstream ss;
        ss << nombreL << "  " << puntosL << " : " << puntosR << "  " << nombreR;
        txtMarcador.setString(ss.str());
        sf::FloatRect bm = txtMarcador.getLocalBounds();
        txtMarcador.setOrigin(bm.width / 2.f, 0.f);
        txtMarcador.setPosition(ANCHO_VENTANA / 2.f, 8.f);
        w.draw(txtMarcador);

        // Tiempo
        int min = (int)tiempo / 60;
        int seg = (int)tiempo % 60;
        std::ostringstream st;
        st << std::setfill('0') << std::setw(1) << min << ":"
           << std::setfill('0') << std::setw(2) << seg;
        txtTiempo.setString(st.str());
        sf::FloatRect bt = txtTiempo.getLocalBounds();
        txtTiempo.setOrigin(bt.width / 2.f, 0.f);
        txtTiempo.setPosition(ANCHO_VENTANA / 2.f, 44.f);
        w.draw(txtTiempo);

        // Mitad
        txtMitad.setString("Mitad " + std::to_string(mitad));
        sf::FloatRect bmi = txtMitad.getLocalBounds();
        txtMitad.setOrigin(bmi.width / 2.f, 0.f);
        txtMitad.setPosition(ANCHO_VENTANA / 2.f, 68.f);
        w.draw(txtMitad);

        // Super meters
        float barW = 160.f;
        barLlenaL.setSize({barW * (superL / SUPER_MAX), 14.f});
        // Color cambia cuando está lleno
        barLlenaL.setFillColor(superL >= SUPER_MAX
            ? sf::Color(255, 80, 0) : sf::Color(255, 180, 0));
        w.draw(barFondoL);
        if (superL > 0.f) w.draw(barLlenaL);

        barLlenaR.setSize({barW * (superR / SUPER_MAX), 14.f});
        barLlenaR.setFillColor(superR >= SUPER_MAX
            ? sf::Color(0, 200, 255) : sf::Color(80, 180, 255));
        w.draw(barFondoR);
        if (superR > 0.f) w.draw(barLlenaR);

        // Etiquetas super
        txtSuperEtiqueta.setString("SUPER");
        txtSuperEtiqueta.setPosition(20.f, ALTO_VENTANA - 52.f);
        txtSuperEtiqueta.setFillColor(sf::Color(255, 200, 0));
        w.draw(txtSuperEtiqueta);
        txtSuperEtiqueta.setPosition(ANCHO_VENTANA - 70.f, ALTO_VENTANA - 52.f);
        txtSuperEtiqueta.setFillColor(sf::Color(80, 200, 255));
        w.draw(txtSuperEtiqueta);

        // Mensaje flotante
        if (tiempoMensaje > 0.f) {
            float alpha = std::min(tiempoMensaje, 1.f) * 255.f;
            txtMensaje.setFillColor(sf::Color(255, 80, 80, (sf::Uint8)alpha));
            w.draw(txtMensaje);
        }
    }
};
