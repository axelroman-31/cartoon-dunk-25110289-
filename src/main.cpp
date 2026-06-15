// ================================================================
//  CARTOON DUNK  -  Street Hoop Style
//  SFML 2.6  |  C++17
//  PANTALLAS: INTRO -> MENU -> SELECCION -> JUGANDO -> FIN
// ================================================================
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <string>
#include <sstream>
#include <array>

#include "Constantes.hpp"
#include "Pelota.hpp"
#include "Jugador.hpp"
#include "Equipo.hpp"
#include "Cancha.hpp"
#include "HUD.hpp"

// ─────────────────────────────────────────
//  Enum de pantallas (nuevo flujo completo)
// ─────────────────────────────────────────
enum class Pantalla {
    INTRO,
    MENU,
    SELECCION,
    JUGANDO,
    MEDIO_TIEMPO,
    FIN
};

// ─────────────────────────────────────────
//  Helpers matemáticos
// ─────────────────────────────────────────
static float dist2(sf::Vector2f a, sf::Vector2f b) {
    float dx = a.x - b.x, dy = a.y - b.y;
    return std::sqrt(dx*dx + dy*dy);
}
static sf::Vector2f norm2(sf::Vector2f v) {
    float l = std::sqrt(v.x*v.x + v.y*v.y);
    return l < 0.001f ? sf::Vector2f{} : sf::Vector2f{v.x/l, v.y/l};
}
static bool prob(float p) { return ((float)rand()/RAND_MAX) < p; }

// ─────────────────────────────────────────
//  Datos de equipos
// ─────────────────────────────────────────
static DatosEquipo equipoCartoon() {
    DatosEquipo d;
    d.nombre    = "CARTOON";
    d.sprites[0]= "assets/imagenes/goku.png";
    d.sprites[1]= "assets/imagenes/Pocoyo.png";
    d.sprites[2]= "assets/imagenes/bugs bunny.png";
    d.color     = sf::Color(255, 120, 0);
    d.dunk = 8; d.tres = 6; d.vel = 7; d.def = 5;
    return d;
}
static DatosEquipo equipoRival() {
    DatosEquipo d;
    d.nombre    = "RIVALES";
    d.sprites[0]= "assets/imagenes/mistico.png";
    d.sprites[1]= "assets/imagenes/mistico.png";
    d.sprites[2]= "assets/imagenes/mistico.png";
    d.color     = sf::Color(80, 160, 255);
    d.dunk = 6; d.tres = 8; d.vel = 6; d.def = 8;
    return d;
}

// ================================================================
//  PANTALLA DE INTRO  -  fondo con arte, personajes, partículas
// ================================================================
class PantallaIntro {
public:
    sf::Texture texFondo, texGokuArt, texMisticoArt, texPocoyoArt, texPixelBugs;
    sf::Sprite  sprFondo, sprGokuArt, sprMisticoArt, sprPocoyoArt, sprPixelBugs;
    bool okFondo=false, okGA=false, okMA=false, okPA=false, okPB=false;

    struct Part { float x, y, vel, tam, alfa; sf::Color col; };
    std::vector<Part> partes;

    float tTotal=0, parpadeo=0, escLogo=0, alfaFondo=0, alfaPerso=0, ondaY=0;
    sf::Font& F; bool Fok;

    PantallaIntro(sf::Font& f, bool fok) : F(f), Fok(fok) {
        okFondo = texFondo.loadFromFile("assets/imagenes/pantalla inicial del juego.jpg");
        okGA    = texGokuArt.loadFromFile("assets/imagenes/goku art.jpg");
        okMA    = texMisticoArt.loadFromFile("assets/imagenes/mistico art.png");
        okPA    = texPocoyoArt.loadFromFile("assets/imagenes/Pocoyo art.webp");
        okPB    = texPixelBugs.loadFromFile("assets/imagenes/pixel bugs.png");

        // Configurar fondo
        if (okFondo) {
            sprFondo.setTexture(texFondo);
            float sx = (float)W_ANCHO / texFondo.getSize().x;
            float sy = (float)W_ALTO  / texFondo.getSize().y;
            sprFondo.setScale(std::max(sx,sy), std::max(sx,sy));
            sprFondo.setColor(sf::Color(255,255,255,0));
        }

        // Helper para sprites de personajes
        auto setupSprite = [](sf::Sprite& s, sf::Texture& t, float h, float x, float y) {
            s.setTexture(t);
            float sc = h / t.getSize().y;
            s.setScale(sc, sc);
            s.setPosition(x, y);
            s.setColor(sf::Color(255,255,255,0));
        };

        if (okGA) setupSprite(sprGokuArt,   texGokuArt,   210.f, 30.f,  (float)W_ALTO - 220.f);
        if (okPA) setupSprite(sprPocoyoArt, texPocoyoArt, 130.f, 200.f, (float)W_ALTO - 145.f);

        if (okMA) {
            sprMisticoArt.setTexture(texMisticoArt);
            float sc = 210.f / texMisticoArt.getSize().y;
            sprMisticoArt.setScale(sc, sc);
            float pw = texMisticoArt.getSize().x * sc;
            sprMisticoArt.setPosition((float)W_ANCHO - pw - 30.f, (float)W_ALTO - 220.f);
            sprMisticoArt.setColor(sf::Color(255,255,255,0));
        }
        if (okPB) {
            sprPixelBugs.setTexture(texPixelBugs);
            float sc = 100.f / texPixelBugs.getSize().y;
            sprPixelBugs.setScale(sc, sc);
            float pw = texPixelBugs.getSize().x * sc;
            sprPixelBugs.setPosition((float)W_ANCHO - pw - 210.f, (float)W_ALTO - 120.f);
            sprPixelBugs.setColor(sf::Color(255,255,255,0));
        }

        // Partículas de fondo
        for (int i = 0; i < 60; i++) {
            Part p;
            p.x   = (float)(rand() % W_ANCHO);
            p.y   = (float)(rand() % W_ALTO);
            p.vel = 18.f + (rand() % 40);
            p.tam = 2.f  + (rand() % 4);
            p.alfa = 0.f;
            int c = rand() % 4;
            if      (c == 0) p.col = sf::Color(255, 200, 20);
            else if (c == 1) p.col = sf::Color(80,  200, 255);
            else if (c == 2) p.col = sf::Color(255, 60,  120);
            else             p.col = sf::Color(120, 255, 100);
            partes.push_back(p);
        }
    }

    void actualizar(float dt) {
        tTotal  += dt;
        ondaY    = std::sin(tTotal * 1.8f) * 6.f;
        parpadeo += dt * 2.8f;
        alfaFondo  = std::min(alfaFondo  + dt * 140.f, 200.f);
        if (tTotal > 0.3f) escLogo  = std::min(escLogo  + dt * 2.2f, 1.f);
        if (tTotal > 0.7f) alfaPerso = std::min(alfaPerso + dt * 200.f, 255.f);

        if (okFondo) sprFondo.setColor(sf::Color(255,255,255,(sf::Uint8)alfaFondo));

        sf::Uint8 ap = (sf::Uint8)alfaPerso;
        if (okGA) {
            sprGokuArt.setPosition(30.f, (float)W_ALTO - 220.f + ondaY * 0.6f);
            sprGokuArt.setColor({255,255,255,ap});
        }
        if (okMA) {
            float pw = texMisticoArt.getSize().x * sprMisticoArt.getScale().x;
            sprMisticoArt.setPosition((float)W_ANCHO - pw - 30.f, (float)W_ALTO - 220.f - ondaY * 0.6f);
            sprMisticoArt.setColor({255,255,255,ap});
        }
        if (okPA) {
            sprPocoyoArt.setPosition(200.f, (float)W_ALTO - 145.f + ondaY * 0.4f);
            sprPocoyoArt.setColor({255,255,255,ap});
        }
        if (okPB) {
            float pw = texPixelBugs.getSize().x * sprPixelBugs.getScale().x;
            sprPixelBugs.setPosition((float)W_ANCHO - pw - 210.f, (float)W_ALTO - 120.f - ondaY * 0.4f);
            sprPixelBugs.setColor({255,255,255,ap});
        }

        for (auto& p : partes) {
            p.y   -= p.vel * dt;
            p.alfa = std::min(p.alfa + dt * 120.f, 180.f);
            if (p.y < -10.f) {
                p.y   = (float)W_ALTO + 5.f;
                p.x   = (float)(rand() % W_ANCHO);
                p.alfa = 0.f;
            }
        }
    }

    void dibujar(sf::RenderWindow& w) {
        // Fondo negro
        sf::RectangleShape bg({(float)W_ANCHO,(float)W_ALTO});
        bg.setFillColor(sf::Color(8, 4, 18));
        w.draw(bg);

        if (okFondo) w.draw(sprFondo);

        // Overlay oscuro sobre el fondo
        sf::RectangleShape ov({(float)W_ANCHO,(float)W_ALTO});
        ov.setFillColor(sf::Color(0,0,0,110));
        w.draw(ov);

        // Partículas
        for (auto& p : partes) {
            sf::CircleShape c(p.tam);
            c.setPosition(p.x, p.y);
            sf::Color col = p.col; col.a = (sf::Uint8)p.alfa;
            c.setFillColor(col);
            w.draw(c);
        }

        // Arte de personajes
        if (okGA) w.draw(sprGokuArt);
        if (okMA) w.draw(sprMisticoArt);
        if (okPA) w.draw(sprPocoyoArt);
        if (okPB) w.draw(sprPixelBugs);

        // Logo
        if (escLogo > 0.01f) {
            dtxt(w, "CARTOON DUNK", 68,
                 sf::Color(180, 80, 0, (sf::Uint8)(200*escLogo)),
                 (float)W_ANCHO/2.f + 4.f, 84.f, escLogo);
            dtxt(w, "CARTOON DUNK", 68,
                 sf::Color(255, 220, 40, (sf::Uint8)(255*escLogo)),
                 (float)W_ANCHO/2.f, 80.f, escLogo);
            dtxt(w, "Street Basketball 3v3", 22,
                 sf::Color(200, 200, 200, (sf::Uint8)(220*escLogo)),
                 (float)W_ANCHO/2.f, 160.f, std::min(escLogo*1.3f, 1.f));

            if (escLogo >= 0.9f) {
                float lw = 380.f * escLogo;
                sf::RectangleShape ln({lw, 2.f});
                ln.setFillColor(sf::Color(255,180,0,160));
                ln.setOrigin(lw/2.f, 1.f);
                ln.setPosition((float)W_ANCHO/2.f, 178.f);
                w.draw(ln);
                dtxt(w, "Inspirado en Street Slam | Data East 1994", 14,
                     sf::Color(140,140,170,180), (float)W_ANCHO/2.f, 192.f, 1.f);
            }
        }

        // "Press Enter"
        if (tTotal > 1.2f) {
            float a = std::sin(parpadeo) * 0.5f + 0.5f;
            dtxt(w, "PRESIONA  ENTER  PARA  COMENZAR", 20,
                 sf::Color(255, 240, 100, (sf::Uint8)(220*a)),
                 (float)W_ANCHO/2.f, (float)W_ALTO - 52.f, 1.f);
        }

        // Controles en pie de página
        if (escLogo >= 0.9f) {
            dtxt(w, "WASD:Mover  J:Tiro  K:Pase  J+K:Dunk  L:Finta/Robo  Tab:Cambiar jugador",
                 11, sf::Color(110,110,120,150),
                 (float)W_ANCHO/2.f, (float)W_ALTO - 20.f, 1.f);
        }
    }

private:
    void dtxt(sf::RenderWindow& w, const std::string& s, int sz,
              sf::Color c, float x, float y, float esc) {
        sf::Text t;
        if (Fok) t.setFont(F);
        t.setString(s);
        t.setCharacterSize(sz);
        t.setFillColor(c);
        t.setOutlineThickness(2.5f);
        t.setOutlineColor(sf::Color(0, 0, 0, (sf::Uint8)(c.a * 0.7f)));
        sf::FloatRect b = t.getLocalBounds();
        t.setOrigin(b.width/2.f, b.height/2.f);
        t.setScale(esc, esc);
        t.setPosition(x, y);
        w.draw(t);
    }
};

// ================================================================
//  MENÚ PRINCIPAL  -  opciones, paneles, submenús
// ================================================================
class MenuPrincipal {
public:
    sf::Texture texG, texP, texB, texM, texF;
    sf::Sprite  sprG, sprP, sprB, sprM, sprF;
    bool okG=false, okP=false, okB=false, okM=false, okFondo=false;

    struct Opcion { std::string txt, desc; sf::Color cn, cs; };
    std::vector<Opcion> ops = {
        {"JUGAR",      "Elige tu equipo e inicia la partida",   {200,200,200},{255,220,50}},
        {"COMO JUGAR", "Ver controles y mecanicas del juego",   {200,200,200},{100,220,255}},
        {"CREDITOS",   "Acerca de Cartoon Dunk",                {200,200,200},{180,255,120}},
        {"SALIR",      "Cerrar el juego",                       {200,200,200},{255,90,90}}
    };
    int sel = 0;

    enum class Sub { NINGUNA, CTRL, CRED } sub = Sub::NINGUNA;

    float t=0, alfaE=0, ondaY=0;

    struct Bola { float x, y, vel, r, alfa; };
    std::vector<Bola> bolas;

    sf::Font& F; bool Fok;

    MenuPrincipal(sf::Font& f, bool fok) : F(f), Fok(fok) {
        okFondo = texF.loadFromFile("assets/imagenes/pantalla inicial del juego.jpg");
        okG     = texG.loadFromFile("assets/imagenes/goku.png");
        okP     = texP.loadFromFile("assets/imagenes/Pocoyo.png");
        okB     = texB.loadFromFile("assets/imagenes/bugs bunny.png");
        okM     = texM.loadFromFile("assets/imagenes/mistico.png");

        if (okFondo) {
            sprF.setTexture(texF);
            float sx=(float)W_ANCHO/texF.getSize().x, sy=(float)W_ALTO/texF.getSize().y;
            sprF.setScale(std::max(sx,sy), std::max(sx,sy));
            sprF.setColor(sf::Color(255,255,255,55));
        }
        auto ss = [](sf::Sprite& s, sf::Texture& tx, float h, float x, float y) {
            s.setTexture(tx);
            float sc = h / tx.getSize().y;
            s.setScale(sc, sc);
            s.setPosition(x, y);
        };
        if (okG) ss(sprG, texG, 260.f, 30.f, (float)W_ALTO - 270.f);
        if (okP) ss(sprP, texP, 140.f, 260.f, (float)W_ALTO - 155.f);
        if (okB) {
            ss(sprB, texB, 180.f, 0.f, 0.f);
            float pw = texB.getSize().x * sprB.getScale().x;
            sprB.setPosition((float)W_ANCHO - pw - 30.f, (float)W_ALTO - 190.f);
        }
        if (okM) {
            ss(sprM, texM, 200.f, 0.f, 0.f);
            float pw = texM.getSize().x * sprM.getScale().x;
            sprM.setPosition((float)W_ANCHO - pw - 220.f, (float)W_ALTO - 210.f);
        }

        for (int i = 0; i < 18; i++) {
            Bola b;
            b.x   = (float)(rand() % W_ANCHO);
            b.y   = (float)(rand() % W_ALTO);
            b.vel = 12.f + (rand() % 25);
            b.r   = 4.f  + (rand() % 8);
            b.alfa = 0.f;
            bolas.push_back(b);
        }
    }

    void resetear() { alfaE = 0.f; sel = 0; sub = Sub::NINGUNA; }

    void actualizar(float dt) {
        t += dt;
        ondaY = std::sin(t * 1.4f) * 7.f;
        alfaE = std::min(alfaE + dt * 280.f, 255.f);

        if (okG) sprG.setPosition(30.f,           (float)W_ALTO - 270.f + ondaY * 0.7f);
        if (okP) sprP.setPosition(260.f,           (float)W_ALTO - 155.f + ondaY);
        if (okB) { float pw = texB.getSize().x * sprB.getScale().x;
                   sprB.setPosition((float)W_ANCHO - pw - 30.f,  (float)W_ALTO - 190.f - ondaY*0.5f); }
        if (okM) { float pw = texM.getSize().x * sprM.getScale().x;
                   sprM.setPosition((float)W_ANCHO - pw - 220.f, (float)W_ALTO - 210.f + ondaY*0.8f); }

        for (auto& b : bolas) {
            b.y   -= b.vel * dt;
            b.alfa = std::min(b.alfa + dt * 80.f, 70.f);
            if (b.y < -20.f) { b.y = (float)W_ALTO + 10.f; b.x = (float)(rand() % W_ANCHO); b.alfa = 0.f; }
        }
    }

    // Devuelve true si el jugador eligió JUGAR
    bool procesarTecla(sf::Keyboard::Key k) {
        if (sub != Sub::NINGUNA) {
            if (k == sf::Keyboard::Escape || k == sf::Keyboard::Return || k == sf::Keyboard::BackSpace)
                sub = Sub::NINGUNA;
            return false;
        }
        if (k == sf::Keyboard::Up   || k == sf::Keyboard::W) sel = (sel - 1 + 4) % 4;
        if (k == sf::Keyboard::Down || k == sf::Keyboard::S) sel = (sel + 1) % 4;
        if (k == sf::Keyboard::Return || k == sf::Keyboard::Space) {
            if (sel == 0) return true;
            if (sel == 1) sub = Sub::CTRL;
            if (sel == 2) sub = Sub::CRED;
        }
        return false;
    }

    bool quiereSalir(sf::Keyboard::Key k) {
        return sel == 3 && sub == Sub::NINGUNA &&
               (k == sf::Keyboard::Return || k == sf::Keyboard::Space);
    }

    void dibujar(sf::RenderWindow& w) {
        sf::RectangleShape bg({(float)W_ANCHO,(float)W_ALTO});
        bg.setFillColor(sf::Color(6,4,16));
        w.draw(bg);

        if (okFondo) w.draw(sprF);

        // Panel izquierdo oscuro para los botones
        sf::RectangleShape piz({420.f,(float)W_ALTO});
        piz.setFillColor(sf::Color(0,0,0,145));
        w.draw(piz);

        // Pelotas decorativas flotando
        for (auto& b : bolas) {
            sf::CircleShape c(b.r);
            c.setPosition(b.x - b.r, b.y - b.r);
            c.setFillColor(sf::Color(255, 140, 20, (sf::Uint8)b.alfa));
            w.draw(c);
        }

        // Sprites de personajes
        if (okG) w.draw(sprG);
        if (okP) w.draw(sprP);
        if (okB) w.draw(sprB);
        if (okM) w.draw(sprM);

        sf::Uint8 ae = (sf::Uint8)alfaE;

        // Título
        tx(w, "CARTOON DUNK",        46, sf::Color(255,215,30,ae),           210.f, 65.f);
        tx(w, "Street Basketball 3v3",18, sf::Color(180,180,200,(sf::Uint8)(ae*0.8f)), 210.f,118.f);

        // Separador
        sf::RectangleShape sep({330.f, 2.f});
        sep.setOrigin(165.f, 1.f);
        sep.setPosition(210.f, 140.f);
        sep.setFillColor(sf::Color(255,180,0,(sf::Uint8)(ae*0.5f)));
        w.draw(sep);

        // Botones
        float yB = 182.f, paso = 68.f;
        for (int i = 0; i < 4; i++) {
            bool s = (i == sel);
            float yO = yB + i * paso;
            if (s) {
                float pu = std::sin(t * 4.f) * 0.04f + 1.f;
                sf::RectangleShape bx({310.f * pu, 50.f});
                bx.setOrigin(310.f*pu/2.f, 25.f);
                bx.setPosition(210.f, yO + 8.f);
                sf::Color cf = ops[i].cs; cf.a = (sf::Uint8)(ae * 0.18f);
                bx.setFillColor(cf);
                bx.setOutlineThickness(2.f);
                sf::Color cb = ops[i].cs; cb.a = (sf::Uint8)(ae * 0.7f);
                bx.setOutlineColor(cb);
                w.draw(bx);
                tx(w, ">>", 22, sf::Color(ops[i].cs.r, ops[i].cs.g, ops[i].cs.b, ae), 60.f, yO + 3.f);
                tx(w, ops[i].desc, 13, sf::Color(180,180,180,(sf::Uint8)(ae*0.85f)), 210.f, yB + 4*paso + 14.f);
            }
            sf::Color col = s ? ops[i].cs : ops[i].cn;
            col.a = ae;
            tx(w, ops[i].txt, s ? 30 : 26, col, 210.f, yO);
        }

        tx(w, "W/S:Navegar    ENTER:Seleccionar    ESC:Volver",
           11, sf::Color(100,100,120,(sf::Uint8)(ae*0.7f)), 210.f, (float)W_ALTO - 22.f);

        if (sub == Sub::CTRL) dibujarCtrl(w);
        if (sub == Sub::CRED) dibujarCred(w);
    }

private:
    void tx(sf::RenderWindow& w, const std::string& s, int sz,
            sf::Color c, float x, float y) {
        sf::Text t;
        if (Fok) t.setFont(F);
        t.setString(s);
        t.setCharacterSize(sz);
        t.setFillColor(c);
        t.setOutlineThickness(2.f);
        t.setOutlineColor(sf::Color(0,0,0,(sf::Uint8)(c.a*0.6f)));
        sf::FloatRect b = t.getLocalBounds();
        t.setOrigin(b.width/2.f, b.height/2.f);
        t.setPosition(x, y);
        w.draw(t);
    }

    void panel(sf::RenderWindow& w, sf::Color borde) {
        sf::RectangleShape p({(float)W_ANCHO - 120.f, (float)W_ALTO - 80.f});
        p.setPosition(60.f, 40.f);
        p.setFillColor(sf::Color(8, 5, 20, 232));
        p.setOutlineThickness(3.f);
        p.setOutlineColor(borde);
        w.draw(p);
    }

    void linTxt(sf::RenderWindow& w, const std::string& key, const std::string& accion, float& y) {
        sf::Text tk, ta;
        if (Fok) { tk.setFont(F); ta.setFont(F); }
        tk.setString(key);    tk.setCharacterSize(13); tk.setFillColor(sf::Color(150,220,255)); tk.setPosition(155.f, y); w.draw(tk);
        ta.setString(accion); ta.setCharacterSize(13); ta.setFillColor(sf::Color(210,210,210)); ta.setPosition(520.f, y); w.draw(ta);
        y += 18.f;
    }

    void dibujarCtrl(sf::RenderWindow& w) {
        panel(w, sf::Color(100,200,255,200));
        tx(w, "COMO JUGAR", 36, sf::Color(100,220,255), (float)W_ANCHO/2.f, 82.f);

        struct Seccion { std::string titulo; std::vector<std::pair<std::string,std::string>> lineas; };
        std::vector<Seccion> secs = {
            {"MOVIMIENTO", {
                {"W / A / S / D",  "Mover al jugador activo"},
                {"LShift",         "Sprint (corre mas rapido)"},
                {"Tab",            "Cambiar jugador activo"}
            }},
            {"OFENSIVA (con balon)", {
                {"J",              "Tiro normal (2 o 3 pts segun distancia)"},
                {"K",              "Pasar al companero mas libre"},
                {"J + K",          "Saltar para dunk (combo del original)"},
                {"J (en el aire)", "Ejecutar el dunk"},
                {"L",              "Finta: amaga tiro sin soltar la pelota"},
                {"Super lleno+J",  "Super Shot garantizado"}
            }},
            {"DEFENSIVA (sin balon)", {
                {"J",              "Bloquear tiro rival / Empujar rival"},
                {"K",              "Intentar robo de balon"},
                {"L",              "Empuje directo"},
                {"J + K (sin bal)","Alley-oop: salta esperando el pase"}
            }},
            {"SUPER METER", {
                {"Se llena con:",  "Encestes, pases, robos, bloqueos, alley-oops"},
                {"Al llenarse:",   "El proximo tiro es un Super Shot espectacular"}
            }}
        };

        float y = 120.f;
        for (auto& sec : secs) {
            sf::Text ts; if (Fok) ts.setFont(F);
            ts.setString(sec.titulo); ts.setCharacterSize(15);
            ts.setFillColor(sf::Color(255,200,50)); ts.setPosition(140.f, y);
            w.draw(ts); y += 22.f;
            for (auto& l : sec.lineas) { linTxt(w, l.first, l.second, y); }
            y += 8.f;
        }
        tx(w, "ENTER / ESC para volver", 15, sf::Color(150,150,160), (float)W_ANCHO/2.f, (float)W_ALTO - 50.f);
    }

    void dibujarCred(sf::RenderWindow& w) {
        panel(w, sf::Color(180,255,120,200));
        tx(w, "CREDITOS", 36, sf::Color(180,255,120), (float)W_ANCHO/2.f, 95.f);

        struct C { std::string r, n; };
        std::vector<C> cs = {
            {"JUEGO",       "Cartoon Dunk"},
            {"INSPIRADO EN","Street Slam / Street Hoop (Data East, 1994)"},
            {"MOTOR",       "SFML 2.6  |  C++17"},
            {"",""},
            {"PERSONAJES",  "Goku  |  Pocoyo  |  Bugs Bunny  |  Mistico"},
            {"MUSICA",      "Gang$tazz.ogg"},
            {"FUENTE",      "texto.ttf"},
            {"",""},
            {"MECANICAS",   "Tiro 2/3 pts  |  Dunk en Aire  |  Super Shot"},
            {"",            "Alley-Oop  |  Finta  |  Bloqueo  |  Intercepcion  |  Empuje"},
            {"",""},
            {"EQUIPOS",     "Cartoon (Goku/Pocoyo/Bugs)  vs  Rivales (Mistico)"},
            {"MODO",        "1 Jugador vs CPU  |  3v3  |  2 mitades de 2 minutos"}
        };
        float y = 148.f;
        for (auto& c : cs) {
            if (c.r.empty() && c.n.empty()) { y += 8.f; continue; }
            sf::Text tr, tn;
            if (Fok) { tr.setFont(F); tn.setFont(F); }
            tr.setString(c.r); tr.setCharacterSize(13); tr.setFillColor(sf::Color(255,200,60)); tr.setPosition(120.f, y); w.draw(tr);
            tn.setString(c.n); tn.setCharacterSize(13); tn.setFillColor(sf::Color(210,215,225)); tn.setPosition(340.f, y); w.draw(tn);
            y += 20.f;
        }
        tx(w, "ENTER / ESC para volver", 15, sf::Color(150,150,160), (float)W_ANCHO/2.f, (float)W_ALTO - 55.f);
    }
};

// ================================================================
//  GameManager  -  motor principal del juego
// ================================================================
class GameManager {
    sf::RenderWindow ventana;
    sf::Font   fuente; bool fuenteOk = false;

    PantallaIntro*  pIntro = nullptr;
    MenuPrincipal*  pMenu  = nullptr;

    Equipo   eH, eCPU;
    Pelota   pelota;
    Cancha   cancha;
    HUD      hud;
    sf::Music musica;

    Pantalla pantalla = Pantalla::INTRO;
    float tiempo = TIEMPO_MITAD;
    int   mitad  = 1;

    sf::RectangleShape fondoRect;

    // Estado de mecánicas
    bool  esperandoDunkEnAire = false;
    bool  alleyOopActivado    = false;
    int   alleyOopReceptor    = -1;
    float cdTiro=0, cdPase=0, cdRobo=0, cdEmpuje=0;
    float pausaPostGol = 0;
    int   equipoSeleccionado = 0;

public:
    GameManager()
        : ventana(sf::VideoMode(W_ANCHO, W_ALTO), "CARTOON DUNK - Street Hoop Style")
    {
        ventana.setFramerateLimit(60);
        srand((unsigned)time(nullptr));

        fuenteOk = fuente.loadFromFile("assets/front/texto.ttf");
        fondoRect.setSize({(float)W_ANCHO,(float)W_ALTO});
        fondoRect.setFillColor(sf::Color(22,12,4));

        if (musica.openFromFile("assets/musica/Gang$tazz.ogg")) {
            musica.setLoop(true);
            musica.setVolume(55.f);
            musica.play();
        }

        pIntro = new PantallaIntro(fuente, fuenteOk);
        pMenu  = new MenuPrincipal(fuente, fuenteOk);
    }

    ~GameManager() { delete pIntro; delete pMenu; }

    // ─── Iniciar partido ─────────────────────────────
    void iniciarPartida() {
        pantalla = Pantalla::JUGANDO;
        tiempo   = TIEMPO_MITAD;
        mitad    = 1;
        cdTiro = cdPase = cdRobo = cdEmpuje = pausaPostGol = 0.f;
        esperandoDunkEnAire = false;
        alleyOopActivado    = false;
        alleyOopReceptor    = -1;

        DatosEquipo dH   = equipoCartoon();
        DatosEquipo dCPU = equipoRival();
        if (equipoSeleccionado == 1) std::swap(dH, dCPU);

        eH.configurar(dH,   true);
        eCPU.configurar(dCPU, false);
        eH.puntos = eCPU.puntos = 0;
        eH.superMeter = eCPU.superMeter = 0.f;
        eH.asistencias = eH.robos = eH.bloqueos = eH.alleyOops = 0;
        eCPU.asistencias = eCPU.robos = eCPU.bloqueos = eCPU.alleyOops = 0;

        sf::Vector2f posH[3]   = {{C_X+210.f,ARO_Y},{C_X+265.f,ARO_Y-95.f},{C_X+265.f,ARO_Y+95.f}};
        sf::Vector2f posCPU[3] = {{C_X+C_ANCHO-210.f,ARO_Y},{C_X+C_ANCHO-265.f,ARO_Y-95.f},{C_X+C_ANCHO-265.f,ARO_Y+95.f}};

        for (int i = 0; i < 3; i++) {
            eH.j[i].pos   = posH[i];
            eCPU.j[i].pos = posCPU[i];
            eH.j[i].tienePelota   = eCPU.j[i].tienePelota = false;
            eH.j[i].estado        = eCPU.j[i].estado       = EstadoJ::IDLE;
            eH.j[i].estamina      = eCPU.j[i].estamina     = 100.f;
        }
        eH.activo = eCPU.activo = 0;
        eH.j[0].tienePelota = true;
        pelota.pos = eH.j[0].pos;
        pelota.enManos = true; pelota.enJuego = false; pelota.enArco = false;
    }

    // ─── Reiniciar posesión ──────────────────────────
    void reiniciarPosesion(bool hCoge) {
        eH.quitarPelota(); eCPU.quitarPelota();
        pelota.enArco = pelota.enJuego = false;
        pelota.enManos = true; pelota.esSuper = false;
        alleyOopActivado = false; alleyOopReceptor = -1;
        sf::Vector2f c = {C_X + C_ANCHO/2.f, C_Y + C_ALTO/2.f};
        pelota.pos = c;
        if (hCoge) { eH.j[0].pos = c;   eH.darPelota(0); }
        else        { eCPU.j[0].pos = c; eCPU.darPelota(0); }
    }

    // ─── Evaluar enceste ─────────────────────────────
    int evalEnceste(bool esH, float d) {
        Jugador& j = esH ? eH.j[eH.activo] : eCPU.j[eCPU.activo];
        bool es3 = d > DIST_3P;
        float p  = j.probTiro(d, es3);
        if (pelota.esSuper) p = std::min(p + 0.40f, 0.97f);
        if (!prob(p)) return 0;
        return es3 ? 3 : 2;
    }

    // ─── Anotar ─────────────────────────────────────
    void anotar(bool eqH, int pts) {
        std::string msg;
        if (pelota.esSuper)    msg = "!SUPER DUNK!";
        else if (pelota.esAlleyOop) msg = "!ALLEY-OOP!";
        else if (pts == 3)     msg = "!TRIPLE!";
        else                   msg = "CANASTA!";

        sf::Color c = eqH ? sf::Color(255,200,0) : sf::Color(100,200,255);
        if (eqH) { eH.sumarPuntos(pts);   hud.flash(sf::Color(255,200,0)); }
        else     { eCPU.sumarPuntos(pts);  hud.flash(sf::Color(80,120,255)); }

        hud.mensaje(msg, c, 2.5f);
        pausaPostGol = 1.3f;
        reiniciarPosesion(!eqH);
    }

    // ─── Lanzar tiro normal ──────────────────────────
    void lanzarTiroNormal(bool esH) {
        Jugador& act = esH ? eH.j[eH.activo] : eCPU.j[eCPU.activo];
        Equipo&  eq  = esH ? eH : eCPU;
        if (!act.tienePelota) return;

        sf::Vector2f ap = esH ? sf::Vector2f{ARO_DER_CX, ARO_Y}
                              : sf::Vector2f{ARO_IZQ_CX, ARO_Y};
        float d   = dist2(act.pos, ap);
        bool  sup = eq.superLleno();
        if (sup) eq.gastarSuper();

        act.tienePelota = false;
        act.setEstado(sup ? EstadoJ::SUPER_SHOT : EstadoJ::LANZANDO, 0.6f);
        act.fintaActiva = false;

        float tv = 0.45f + d / 1300.f;
        pelota.lanzarTiro(act.pos, ap, tv, sup);
        pelota.esEquipoH = esH;

        int p = evalEnceste(esH, d);
        pelota.anotoPendiente   = (p > 0);
        pelota.puntosPendientes = p;

        cdTiro = 0.85f;
        esperandoDunkEnAire = false;
        alleyOopActivado    = false;
    }

    // ─── Lanzar dunk (combo J+K → J) ────────────────
    void lanzarDunk(bool esH) {
        Jugador& act = esH ? eH.j[eH.activo] : eCPU.j[eCPU.activo];
        Equipo&  eq  = esH ? eH : eCPU;
        if (!act.tienePelota) return;

        sf::Vector2f ap = esH ? sf::Vector2f{ARO_DER_CX, ARO_Y}
                              : sf::Vector2f{ARO_IZQ_CX, ARO_Y};
        float d   = dist2(act.pos, ap);
        bool  sup = eq.superLleno();
        if (sup) eq.gastarSuper();

        act.tienePelota = false;
        act.setEstado(EstadoJ::DUNKEANDO, 0.55f);

        float tv = 0.5f + d / 1100.f;
        pelota.lanzarTiro(act.pos, ap, tv, sup);
        pelota.esEquipoH = esH;

        float p = act.probTiro(d, d > DIST_3P) + 0.15f;
        if (sup) p = std::min(p + 0.40f, 0.97f);
        p = std::min(p, 0.95f);

        pelota.anotoPendiente   = prob(p);
        pelota.puntosPendientes = pelota.anotoPendiente ? (d > DIST_3P ? 3 : 2) : 0;

        cdTiro = 0.9f;
        esperandoDunkEnAire = false;
    }

    // ─── Finta ───────────────────────────────────────
    void activarFinta() {
        Jugador& act = eH.j[eH.activo];
        if (!act.tienePelota || act.fintaActiva) return;
        act.fintaActiva  = true;
        act.timerFinta   = 0.45f;
        act.setEstado(EstadoJ::FINTANDO, 0.45f);
        hud.mensaje("FINTA!", sf::Color(255,160,0), 0.8f);
    }

    // ─── Pase ────────────────────────────────────────
    void lanzarPase() {
        Jugador& act = eH.j[eH.activo];
        if (!act.tienePelota) return;

        // ¿Alley-oop pendiente?
        if (alleyOopActivado && alleyOopReceptor >= 0
            && eH.j[alleyOopReceptor].esperandoAlleyOop)
        {
            int r = alleyOopReceptor;
            act.tienePelota = false;
            act.setEstado(EstadoJ::PASANDO, 0.35f);
            float tv = dist2(act.pos, eH.j[r].pos) / 650.f + 0.12f;
            pelota.lanzarAlleyOop(act.pos, eH.j[r].pos, tv);
            pelota.esEquipoH = true;
            eH.receptorAlleyOop = r; eH.receptorPase = -1;
            eH.sumarSuperAlleyOop();
            alleyOopActivado = false;
            cdPase = 0.65f;
            hud.mensaje("ALLEY-OOP!", sf::Color(255,220,0), 1.2f);
            return;
        }

        // Pase normal al compañero más libre
        sf::Vector2f aro{ARO_DER_CX, ARO_Y};
        int rc = -1; float mD = 1e9f;
        for (int i = 0; i < 3; i++) {
            if (i == eH.activo) continue;
            float d2 = dist2(eH.j[i].pos, aro);
            if (d2 < mD) { mD = d2; rc = i; }
        }
        if (rc < 0) return;

        act.tienePelota = false;
        act.setEstado(EstadoJ::PASANDO, 0.35f);
        float tv = dist2(act.pos, eH.j[rc].pos) / 720.f + 0.08f;
        pelota.lanzarPase(act.pos, eH.j[rc].pos, tv);
        pelota.esEquipoH = true;
        eH.receptorPase = rc; eH.receptorAlleyOop = -1;
        eH.sumarSuperPase();
        cdPase = 0.6f;
    }

    // ─── Alley-oop receptor ──────────────────────────
    void activarAlleyOop() {
        Jugador& act = eH.j[eH.activo];
        if (act.tienePelota) return;
        act.setEstado(EstadoJ::ALLEYOOP_VUELO, 0.9f);
        act.esperandoAlleyOop = true;
        act.timerAlleyOop     = 0.9f;
        alleyOopActivado  = true;
        alleyOopReceptor  = eH.activo;
    }

    // ─── Bloqueo de tiro ─────────────────────────────
    void intentarBloqueo() {
        Jugador& act = eH.j[eH.activo];
        act.setEstado(EstadoJ::BLOQUEANDO, 0.38f);
        cdRobo = 0.45f;
        if (!pelota.enArco || pelota.esPase || pelota.esEquipoH) return;
        if (dist2(act.pos, pelota.pos) < RADIO_BLOQUEO && prob(act.probBloqueo())) {
            pelota.enArco = false; pelota.anotoPendiente = false;
            float vx = (eH.activo < 1 ? 1.f : -1.f) * 90.f;
            float vy = ((float)rand()/RAND_MAX - 0.5f) * 60.f;
            pelota.soltarLibre(pelota.pos, {vx, vy});
            eH.sumarSuperBloqueo();
            act.timerFlash = 0.45f;
            hud.mensaje("!BLOQUEADO!", sf::Color(100,255,100), 1.5f);
        }
    }

    // ─── Robo humano ─────────────────────────────────
    void intentarRoboHumano() {
        Jugador& act = eH.j[eH.activo];
        act.setEstado(EstadoJ::ROBANDO, 0.35f);
        for (auto& jc : eCPU.j) {
            if (!jc.tienePelota) continue;
            if (dist2(act.pos, jc.pos) < RADIO_ROBO + 8.f && prob(act.probRobo() * 0.55f)) {
                jc.tienePelota  = false;
                act.tienePelota = true;
                pelota.pos = act.pos; pelota.enManos = true; pelota.enJuego = false;
                eH.sumarSuperRobo();
                hud.mensaje("!ROBO!", sf::Color(255,255,100), 1.5f);
            }
            break;
        }
    }

    // ─── Empuje humano ───────────────────────────────
    void intentarEmpuje() {
        if (cdEmpuje > 0.f) return;
        Jugador& act = eH.j[eH.activo];
        cdEmpuje = 0.7f;
        act.setEstado(EstadoJ::EMPUJANDO, 0.25f);
        for (auto& jc : eCPU.j) {
            if (dist2(act.pos, jc.pos) < J_RADIO * 2.6f) {
                sf::Vector2f dir = norm2(jc.pos - act.pos);
                jc.recibirEmpuje(dir, 180.f);
                if (jc.tienePelota && prob(0.30f)) {
                    jc.tienePelota = false;
                    pelota.soltarLibre(jc.pos, {dir.x*80.f + ((float)rand()/RAND_MAX-.5f)*40.f,
                                                dir.y*60.f + ((float)rand()/RAND_MAX-.5f)*40.f});
                }
                break;
            }
        }
    }

    // ─── Interceptar pases ──────────────────────────
    void verificarIntercepciones() {
        if (!pelota.enArco || !pelota.esPase) return;
        if (!pelota.esEquipoH) {
            for (int i = 0; i < 3; i++) {
                Jugador& j = eH.j[i];
                if (j.timerAturdido > 0.f) continue;
                if (dist2(j.pos, pelota.pos) < J_RADIO + P_RADIO + 18.f
                    && prob(j.probInterceptar() * 0.5f))
                {
                    eH.darPelota(i); pelota.tomarla(); pelota.pos = j.pos;
                    eCPU.receptorPase = eCPU.receptorAlleyOop = -1;
                    eH.sumarSuperRobo();
                    hud.mensaje("INTERCEPCION!", sf::Color(100,255,200), 1.5f);
                    return;
                }
            }
        } else {
            for (int i = 0; i < 3; i++) {
                Jugador& j = eCPU.j[i];
                if (j.timerAturdido > 0.f) continue;
                if (dist2(j.pos, pelota.pos) < J_RADIO + P_RADIO + 14.f
                    && prob(j.probInterceptar() * 0.38f))
                {
                    eCPU.darPelota(i); pelota.tomarla(); pelota.pos = j.pos;
                    eH.receptorPase = eH.receptorAlleyOop = -1;
                    eCPU.sumarSuperRobo();
                    hud.mensaje("Pase cortado!", sf::Color(100,180,255), 1.2f);
                    return;
                }
            }
        }
    }

    // ─── Bloqueo IA ──────────────────────────────────
    void verificarBloqueoIA() {
        if (!pelota.enArco || pelota.esPase || !pelota.esEquipoH) return;
        for (int i = 0; i < 3; i++) {
            Jugador& def = eCPU.j[i];
            if (def.timerAturdido > 0.f) continue;
            if (dist2(def.pos, pelota.pos) < RADIO_BLOQUEO + 10.f
                && prob(def.probBloqueo() * 0.30f * (1.f/60.f) * 120.f))
            {
                pelota.enArco = false; pelota.anotoPendiente = false;
                def.setEstado(EstadoJ::BLOQUEANDO, 0.38f);
                pelota.soltarLibre(pelota.pos, {-90.f, ((float)rand()/RAND_MAX-.5f)*60.f});
                eCPU.sumarSuperBloqueo();
                hud.mensaje("BLOQUEO!", sf::Color(100,200,255), 1.2f);
                return;
            }
        }
    }

    // ─── Input del jugador humano ────────────────────
    void inputHumano(float dt) {
        if (pantalla != Pantalla::JUGANDO || pausaPostGol > 0.f) return;

        Jugador& act = eH.porActivo();
        bool tB = act.tienePelota;

        cdTiro  = std::max(0.f, cdTiro  - dt);
        cdPase  = std::max(0.f, cdPase  - dt);
        cdRobo  = std::max(0.f, cdRobo  - dt);
        cdEmpuje= std::max(0.f, cdEmpuje- dt);

        // Movimiento
        sf::Vector2f dir{};
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) dir.x -= 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) dir.x += 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) dir.y -= 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) dir.y += 1.f;
        bool sprint = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift);

        bool puedeMove = (act.estado == EstadoJ::IDLE
                       || act.estado == EstadoJ::CORRIENDO
                       || act.estado == EstadoJ::FINTANDO);
        if (puedeMove) act.mover(dir, dt, sprint);
        if (tB && !pelota.enArco) pelota.pos = act.pos;

        // Auto-switch al jugador más cercano a la pelota libre
        if (!tB && !pelota.enArco && pelota.enJuego && !alleyOopActivado) {
            int mi = eH.activo;
            float md = dist2(eH.j[eH.activo].pos, pelota.pos);
            for (int i = 0; i < 3; i++) {
                if (i == eH.activo) continue;
                float d2 = dist2(eH.j[i].pos, pelota.pos);
                if (d2 < md - 30.f) { md = d2; mi = i; }
            }
            if (mi != eH.activo) {
                eH.j[eH.activo].esHumano = false;
                eH.activo = mi;
                eH.j[eH.activo].esHumano = true;
            }
        }

        bool jP = sf::Keyboard::isKeyPressed(sf::Keyboard::J);
        bool kP = sf::Keyboard::isKeyPressed(sf::Keyboard::K);
        bool lP = sf::Keyboard::isKeyPressed(sf::Keyboard::L);

        if (tB) {
            // J+K → salto para dunk
            if (jP && kP && cdTiro <= 0.f && !esperandoDunkEnAire) {
                act.setEstado(EstadoJ::EN_AIRE, 0.55f);
                esperandoDunkEnAire = true;
                cdTiro = 0.3f;
            }
            // En el aire + J → dunk
            else if (esperandoDunkEnAire && act.estado == EstadoJ::EN_AIRE && jP && cdTiro <= 0.f) {
                lanzarDunk(true);
            }
            // J solo → tiro normal
            else if (jP && !kP && cdTiro <= 0.f && !esperandoDunkEnAire) {
                lanzarTiroNormal(true);
            }
            // K solo → pase
            else if (kP && !jP && cdPase <= 0.f) {
                lanzarPase();
            }
            // L → finta
            else if (lP && cdTiro <= 0.f && !act.fintaActiva && !esperandoDunkEnAire) {
                activarFinta();
                cdTiro = 0.5f;
            }
            // Cancelar salto si no presionó J a tiempo
            if (esperandoDunkEnAire && act.estado != EstadoJ::EN_AIRE) {
                esperandoDunkEnAire = false;
                lanzarTiroNormal(true);
            }
        } else {
            // Sin pelota
            if (jP && cdRobo <= 0.f) {
                if (pelota.enArco && !pelota.esPase && !pelota.esEquipoH) intentarBloqueo();
                else intentarEmpuje();
            }
            if (kP && cdRobo <= 0.f) { intentarRoboHumano(); cdRobo = 0.6f; }
            if (jP && kP && cdTiro <= 0.f && !alleyOopActivado) { activarAlleyOop(); cdTiro = 1.f; }
            if (lP && cdEmpuje <= 0.f) intentarEmpuje();
        }
    }

    // ─── IA del CPU ──────────────────────────────────
    void updateIA(float dt) {
        if (pantalla != Pantalla::JUGANDO || pausaPostGol > 0.f) return;

        Jugador* cp = eCPU.conPelota();
        sf::Vector2f ao{ARO_IZQ_CX, ARO_Y};

        if (cp) {
            float dA = dist2(cp->pos, ao);

            if (cp->timerAturdido <= 0.f)
                cp->mover(norm2(ao - cp->pos), dt * 0.88f);
            pelota.pos = cp->pos;

            // Super dunk
            if (eCPU.superLleno() && dA < 220.f && prob(0.028f * (dt*60.f))) {
                lanzarDunk(false); return;
            }
            // Tiro de 2 o 3
            bool z2 = dA < 155.f, z3 = dA < DIST_3P + 35.f && dA > DIST_3P - 15.f;
            if ((z2 || z3) && prob((z2 ? 0.030f : 0.022f) * (dt*60.f))) {
                lanzarTiroNormal(false); return;
            }
            // Pase a compañero más libre
            if (dA > 260.f && prob(0.012f * (dt*60.f))) {
                int mj = -1; float mD = dA;
                for (int i = 0; i < 3; i++) {
                    if (eCPU.j[i].tienePelota) continue;
                    float dc = dist2(eCPU.j[i].pos, ao);
                    if (dc < mD - 50.f) { mD = dc; mj = i; }
                }
                if (mj >= 0) {
                    cp->tienePelota = false;
                    cp->setEstado(EstadoJ::PASANDO, 0.35f);
                    float tv = dist2(cp->pos, eCPU.j[mj].pos) / 720.f + 0.08f;
                    pelota.lanzarPase(cp->pos, eCPU.j[mj].pos, tv);
                    pelota.esEquipoH = false;
                    eCPU.receptorPase = mj; eCPU.sumarSuperPase();
                    return;
                }
            }
            // Alley-oop de la IA
            if (dA > 120.f && dA < 250.f && prob(0.008f * (dt*60.f))) {
                int rc = eCPU.mejorReceptorAlleyOop(ao);
                if (rc >= 0 && dist2(eCPU.j[rc].pos, ao) < 130.f) {
                    eCPU.j[rc].setEstado(EstadoJ::ALLEYOOP_VUELO, 0.75f);
                    eCPU.j[rc].esperandoAlleyOop = true;
                    eCPU.j[rc].timerAlleyOop     = 0.75f;
                    cp->tienePelota = false;
                    cp->setEstado(EstadoJ::PASANDO, 0.35f);
                    float tv = dist2(cp->pos, eCPU.j[rc].pos) / 650.f + 0.12f;
                    pelota.lanzarAlleyOop(cp->pos, eCPU.j[rc].pos, tv);
                    pelota.esEquipoH = false;
                    eCPU.receptorAlleyOop = rc; eCPU.sumarSuperAlleyOop();
                    hud.mensaje("Alley-oop rival!", sf::Color(100,180,255), 1.2f);
                    return;
                }
            }
            // Posicionar compañeros
            for (int i = 0; i < 3; i++) {
                if (eCPU.j[i].tienePelota || eCPU.j[i].timerAturdido > 0.f) continue;
                sf::Vector2f obj = {ao.x + 80.f + i*55.f, ao.y + (float)((i-1)*110)};
                obj.x = std::max(C_X+J_RADIO, std::min(obj.x, C_X+C_ANCHO-J_RADIO));
                obj.y = std::max(C_Y+J_RADIO, std::min(obj.y, C_Y+C_ALTO-J_RADIO));
                if (dist2(eCPU.j[i].pos, obj) > 12.f)
                    eCPU.j[i].mover(norm2(obj - eCPU.j[i].pos), dt * 0.65f);
            }
        } else {
            // Defensa
            for (int i = 0; i < 3; i++) {
                Jugador& def = eCPU.j[i];
                if (def.timerAturdido > 0.f) continue;
                Jugador& obj = eH.j[i % 3];
                sf::Vector2f pd = obj.pos + norm2(ao - obj.pos) * 42.f;
                if (dist2(def.pos, pd) > 20.f)
                    def.mover(norm2(pd - def.pos), dt * 0.80f);

                if (obj.tienePelota) {
                    float mF = obj.fintaActiva ? 0.3f : 1.f;
                    if (dist2(def.pos, obj.pos) < RADIO_ROBO
                        && prob(def.probRobo() * dt * 1.8f * mF))
                    {
                        obj.tienePelota  = false;
                        def.tienePelota  = true;
                        pelota.pos = def.pos;
                        pelota.enManos = true; pelota.enJuego = false;
                        eCPU.activo = i;
                        eCPU.sumarSuperRobo();
                        esperandoDunkEnAire = false; alleyOopActivado = false;
                        hud.mensaje("Robo del rival", sf::Color(100,200,255), 1.2f);
                    }
                }
                // Empuje aleatorio
                if (!obj.tienePelota && prob(0.004f * (dt*60.f))) {
                    if (dist2(def.pos, obj.pos) < J_RADIO * 2.5f) {
                        sf::Vector2f dE = norm2(obj.pos - def.pos);
                        obj.recibirEmpuje(dE, 140.f);
                        if (obj.tienePelota && prob(0.20f)) {
                            obj.tienePelota = false;
                            pelota.soltarLibre(obj.pos, {dE.x*70.f, dE.y*50.f});
                        }
                    }
                }
            }
            // Recoger pelota libre
            if (pelota.enJuego) {
                for (int i = 0; i < 3; i++) {
                    if (dist2(eCPU.j[i].pos, pelota.pos) < J_RADIO + P_RADIO + 5.f) {
                        eH.quitarPelota(); eCPU.darPelota(i); pelota.tomarla(); break;
                    }
                }
            }
        }
    }

    // ─── Verificaciones de pelota ────────────────────
    void verificarPelotaLibre() {
        if (!pelota.enJuego) return;
        Jugador& a = eH.j[eH.activo];
        if (!a.tienePelota && dist2(a.pos, pelota.pos) < J_RADIO + P_RADIO + 8.f) {
            eH.darPelota(eH.activo); pelota.tomarla();
        }
    }

    void verificarPase() {
        if (!pelota.enArco || !pelota.esPase || pelota.esAlleyOop) return;
        if (pelota.progreso() < 0.88f) return;
        if (pelota.esEquipoH && eH.receptorPase >= 0) {
            int r = eH.receptorPase;
            eH.j[eH.activo].esHumano = false;
            eH.darPelota(r); eH.j[r].esHumano = true;
            pelota.tomarla(); pelota.pos = eH.j[r].pos;
            eH.receptorPase = -1;
        } else if (!pelota.esEquipoH && eCPU.receptorPase >= 0) {
            int r = eCPU.receptorPase;
            eCPU.darPelota(r); pelota.tomarla(); pelota.pos = eCPU.j[r].pos;
            eCPU.receptorPase = -1;
        }
    }

    void verificarAlleyOop() {
        if (!pelota.enArco || !pelota.esPase || !pelota.esAlleyOop) return;
        if (pelota.progreso() < 0.85f) return;
        if (pelota.esEquipoH && eH.receptorAlleyOop >= 0) {
            int r = eH.receptorAlleyOop;
            eH.j[r].esperandoAlleyOop = false;
            eH.j[r].tienePelota = true;
            pelota.tomarla(); pelota.pos = eH.j[r].pos;
            eH.activo = r; eH.j[r].esHumano = true;
            eH.receptorAlleyOop = -1;
            lanzarDunk(true);
        } else if (!pelota.esEquipoH && eCPU.receptorAlleyOop >= 0) {
            int r = eCPU.receptorAlleyOop;
            eCPU.j[r].esperandoAlleyOop = false;
            eCPU.j[r].tienePelota = true;
            pelota.tomarla(); pelota.pos = eCPU.j[r].pos;
            eCPU.activo = r;
            eCPU.receptorAlleyOop = -1;
            lanzarDunk(false);
        }
    }

    void verificarTiro() {
        if (!pelota.enArco || pelota.esPase) return;
        if (pelota.progreso() < 0.96f) return;
        if (pelota.anotoPendiente) {
            anotar(pelota.esEquipoH, pelota.puntosPendientes);
            pelota.anotoPendiente = false;
        }
    }

    // ─── Colisiones físicas ──────────────────────────
    void resolverColisiones() {
        std::vector<Jugador*> t;
        for (auto& j : eH.j)   t.push_back(&j);
        for (auto& j : eCPU.j) t.push_back(&j);
        float ms = J_RADIO * 2.f - 2.f;
        for (int a = 0; a < (int)t.size(); a++) {
            for (int b = a+1; b < (int)t.size(); b++) {
                sf::Vector2f dv = t[a]->pos - t[b]->pos;
                float dl = std::sqrt(dv.x*dv.x + dv.y*dv.y);
                if (dl < ms && dl > 0.001f) {
                    sf::Vector2f push = (dv/dl) * (ms-dl) * 0.5f;
                    t[a]->pos += push; t[b]->pos -= push;
                    t[a]->clampCancha(); t[b]->clampCancha();
                }
            }
        }
    }

    // ─── Update principal ────────────────────────────
    void update(float dt) {
        if (pantalla == Pantalla::INTRO) { pIntro->actualizar(dt); return; }
        if (pantalla == Pantalla::MENU)  { pMenu->actualizar(dt);  return; }

        hud.actualizar(dt);
        eH.actualizar(dt);
        eCPU.actualizar(dt);

        if (pantalla == Pantalla::SELECCION) return;
        if (pausaPostGol > 0.f) { pausaPostGol -= dt; return; }
        if (pantalla != Pantalla::JUGANDO) return;

        // Cronómetro
        tiempo -= dt;
        if (tiempo <= 0.f) {
            if (mitad == 1) {
                mitad = 2; tiempo = TIEMPO_MITAD;
                pantalla = Pantalla::MEDIO_TIEMPO;
                hud.mensaje("MEDIO TIEMPO", sf::Color(255,220,50), 3.f);
                reiniciarPosesion(false);
            } else {
                pantalla = Pantalla::FIN;
                std::string r = eH.puntos > eCPU.puntos ? "!GANASTE!" :
                                eH.puntos < eCPU.puntos ? "PERDISTE"  : "EMPATE";
                hud.mensaje(r, sf::Color(255,220,50), 999.f);
            }
        }

        pelota.actualizar(dt);
        verificarTiro();
        verificarPase();
        verificarAlleyOop();
        verificarPelotaLibre();
        verificarIntercepciones();
        verificarBloqueoIA();
        inputHumano(dt);
        updateIA(dt);
        resolverColisiones();

        // Sincronizar pelota con portador
        for (auto& j : eH.j)   if (j.tienePelota && !pelota.enArco) pelota.pos = j.pos;
        for (auto& j : eCPU.j) if (j.tienePelota && !pelota.enArco) pelota.pos = j.pos;
    }

    // ─── Dibujar ─────────────────────────────────────
    void dibujar() {
        ventana.clear();

        if (pantalla == Pantalla::INTRO) { pIntro->dibujar(ventana); ventana.display(); return; }
        if (pantalla == Pantalla::MENU)  { pMenu->dibujar(ventana);  ventana.display(); return; }

        ventana.draw(fondoRect);

        if (pantalla == Pantalla::SELECCION) {
            dibujarSeleccion(); ventana.display(); return;
        }

        // ── CANCHA ──
        cancha.dibujar(ventana);

        // Ordenar jugadores por Y (profundidad)
        std::vector<Jugador*> t;
        for (auto& j : eH.j)   t.push_back(&j);
        for (auto& j : eCPU.j) t.push_back(&j);
        std::sort(t.begin(), t.end(), [](Jugador* a, Jugador* b){ return a->pos.y < b->pos.y; });

        for (auto* j : t) ventana.draw(j->sombra);
        if (pelota.enJuego) pelota.dibujar(ventana);
        for (auto* j : t) j->dibujar(ventana);
        if (!pelota.enJuego) pelota.dibujar(ventana);

        // Indicador de alley-oop esperando
        if (alleyOopActivado && alleyOopReceptor >= 0) {
            Jugador& r = eH.j[alleyOopReceptor];
            sf::Text tx;
            if (hud.fuenteOk) tx.setFont(hud.fuente);
            tx.setString("ALLEY!");
            tx.setCharacterSize(16);
            tx.setFillColor(sf::Color(255,220,0,200));
            tx.setOutlineThickness(1.f); tx.setOutlineColor(sf::Color::Black);
            sf::FloatRect b = tx.getLocalBounds();
            tx.setOrigin(b.width/2.f, b.height/2.f);
            tx.setPosition(r.pos.x, r.pos.y - 75.f - r.alturaSalto);
            ventana.draw(tx);
        }

        // HUD
        hud.dibujar(ventana, eH.puntos, eCPU.puntos, tiempo, mitad,
                    eH.superMeter, eCPU.superMeter, eH.nombre, eCPU.nombre);

        if (pantalla == Pantalla::MEDIO_TIEMPO)
            overlayTexto("MEDIO TIEMPO", "Pulsa ENTER para la 2a mitad");

        if (pantalla == Pantalla::FIN) {
            std::string r = eH.puntos > eCPU.puntos ? "!GANASTE!" :
                            eH.puntos < eCPU.puntos ? "PERDISTE"  : "EMPATE";
            std::ostringstream ss;
            ss << eH.puntos << " - " << eCPU.puntos << "  |  R=Revanche  |  ESC=Menu";
            overlayTexto(r, ss.str());
            dibujarEstadisticas();
        }

        ventana.display();
    }

    // ─── Helpers de dibujo ───────────────────────────
    void dibujarEstadisticas() {
        auto tc = [&](const std::string& s, int sz, sf::Color c, float x, float y) {
            sf::Text tx;
            if (hud.fuenteOk) tx.setFont(hud.fuente);
            tx.setString(s); tx.setCharacterSize(sz); tx.setFillColor(c);
            tx.setOutlineThickness(1.f); tx.setOutlineColor(sf::Color::Black);
            sf::FloatRect b = tx.getLocalBounds();
            tx.setOrigin(b.width/2.f, 0.f); tx.setPosition(x, y);
            ventana.draw(tx);
        };
        tc("Robos:" + std::to_string(eH.robos) +
           "  Bloqueos:" + std::to_string(eH.bloqueos) +
           "  Asistencias:" + std::to_string(eH.asistencias) +
           "  Alley-oops:" + std::to_string(eH.alleyOops),
           14, sf::Color(255,190,80),
           (float)W_ANCHO/2.f, (float)W_ALTO/2.f + 90.f);
    }

    void dibujarSeleccion() {
        sf::Font& f = hud.fuente; bool fok = hud.fuenteOk;
        auto tx = [&](const std::string& s, int sz, sf::Color c, float x, float y, bool ce=true) {
            sf::Text t;
            if (fok) t.setFont(f);
            t.setString(s); t.setCharacterSize(sz); t.setFillColor(c);
            t.setOutlineThickness(2.f); t.setOutlineColor(sf::Color::Black);
            sf::FloatRect b = t.getLocalBounds();
            if (ce) t.setOrigin(b.width/2.f, b.height/2.f);
            t.setPosition(x, y); ventana.draw(t);
        };

        tx("CARTOON DUNK",                          52, sf::Color(255,200,0),  (float)W_ANCHO/2.f, 80.f);
        tx("Street Basketball 3v3  -  Street Hoop", 18, sf::Color(200,200,200),(float)W_ANCHO/2.f, 140.f);
        tx("Elige tu equipo:",                       26, sf::Color::White,      (float)W_ANCHO/2.f, 190.f);

        float px = 230.f, py = 255.f;
        sf::RectangleShape b1({280.f,200.f}); b1.setPosition(px-140.f, py-20.f);
        b1.setFillColor(equipoSeleccionado==0 ? sf::Color(255,140,0,180) : sf::Color(50,30,10,120));
        b1.setOutlineThickness(3.f); b1.setOutlineColor(sf::Color(255,180,50)); ventana.draw(b1);
        tx("CARTOON DUNK",      20, sf::Color(255,220,80), px, py);
        tx("Goku / Pocoyo / Bugs",14,sf::Color(220,220,200),px, py+33.f);
        tx("Dunk:  ████████░",  14, sf::Color(255,180,0),  px, py+60.f);
        tx("3pts:  ██████░░░",  14, sf::Color(255,180,0),  px, py+80.f);
        tx("Vel:   ███████░░",  14, sf::Color(255,180,0),  px, py+100.f);
        tx("Def:   █████░░░░",  14, sf::Color(255,180,0),  px, py+120.f);

        float px2 = (float)W_ANCHO - 230.f;
        sf::RectangleShape b2({280.f,200.f}); b2.setPosition(px2-140.f, py-20.f);
        b2.setFillColor(equipoSeleccionado==1 ? sf::Color(60,120,255,180) : sf::Color(20,30,60,120));
        b2.setOutlineThickness(3.f); b2.setOutlineColor(sf::Color(80,180,255)); ventana.draw(b2);
        tx("RIVALES",           20, sf::Color(100,200,255),px2, py);
        tx("Mistico x3",        14, sf::Color(200,220,255),px2, py+33.f);
        tx("Dunk:  ██████░░░",  14, sf::Color(100,180,255),px2, py+60.f);
        tx("3pts:  ████████░",  14, sf::Color(100,180,255),px2, py+80.f);
        tx("Vel:   ██████░░░",  14, sf::Color(100,180,255),px2, py+100.f);
        tx("Def:   ████████░",  14, sf::Color(100,180,255),px2, py+120.f);

        tx(equipoSeleccionado==0 ? "◄" : "►", 32, sf::Color::Yellow, (float)W_ANCHO/2.f, py+60.f);
        tx("← → para elegir   ENTER para jugar",      18, sf::Color(180,180,180),(float)W_ANCHO/2.f, 490.f);
        tx("WASD:Mover  J:Tiro  K:Pase  J+K:Dunk  L:Finta",12, sf::Color(130,130,130),(float)W_ANCHO/2.f, 522.f);
        tx("ESC = Volver al Menu",                     13, sf::Color(140,140,160),(float)W_ANCHO/2.f, 580.f);
    }

    void overlayTexto(const std::string& tit, const std::string& sub) {
        sf::RectangleShape ov({(float)W_ANCHO,(float)W_ALTO});
        ov.setFillColor(sf::Color(0,0,0,150)); ventana.draw(ov);
        sf::Text t, t2;
        if (hud.fuenteOk) { t.setFont(hud.fuente); t2.setFont(hud.fuente); }
        t.setString(tit); t.setCharacterSize(50); t.setFillColor(sf::Color(255,220,50));
        t.setOutlineThickness(3.f); t.setOutlineColor(sf::Color::Black);
        sf::FloatRect b = t.getLocalBounds(); t.setOrigin(b.width/2.f, b.height/2.f);
        t.setPosition((float)W_ANCHO/2.f, (float)W_ALTO/2.f - 30.f); ventana.draw(t);
        t2.setString(sub); t2.setCharacterSize(20); t2.setFillColor(sf::Color::White);
        t2.setOutlineThickness(2.f); t2.setOutlineColor(sf::Color::Black);
        sf::FloatRect b2 = t2.getLocalBounds(); t2.setOrigin(b2.width/2.f, 0.f);
        t2.setPosition((float)W_ANCHO/2.f, (float)W_ALTO/2.f + 30.f); ventana.draw(t2);
    }

    // ─── Eventos ─────────────────────────────────────
    void evento(sf::Event& e) {
        if (e.type == sf::Event::Closed) { ventana.close(); return; }

        // INTRO
        if (pantalla == Pantalla::INTRO) {
            if (e.type == sf::Event::KeyPressed &&
                (e.key.code == sf::Keyboard::Return ||
                 e.key.code == sf::Keyboard::Space  ||
                 e.key.code == sf::Keyboard::Escape))
            {
                pantalla = Pantalla::MENU; pMenu->resetear();
            }
            return;
        }

        // MENU
        if (pantalla == Pantalla::MENU) {
            if (e.type == sf::Event::KeyPressed) {
                if (pMenu->quiereSalir(e.key.code)) { ventana.close(); return; }
                if (pMenu->procesarTecla(e.key.code)) pantalla = Pantalla::SELECCION;
            }
            return;
        }

        if (e.type != sf::Event::KeyPressed) return;

        // SELECCION
        if (pantalla == Pantalla::SELECCION) {
            if (e.key.code == sf::Keyboard::Left  || e.key.code == sf::Keyboard::A) equipoSeleccionado = 0;
            if (e.key.code == sf::Keyboard::Right || e.key.code == sf::Keyboard::D) equipoSeleccionado = 1;
            if (e.key.code == sf::Keyboard::Return) iniciarPartida();
            if (e.key.code == sf::Keyboard::Escape) { pantalla = Pantalla::MENU; pMenu->resetear(); }
            return;
        }

        // MEDIO TIEMPO
        if (pantalla == Pantalla::MEDIO_TIEMPO && e.key.code == sf::Keyboard::Return) {
            pantalla = Pantalla::JUGANDO; hud.mensaje("", sf::Color::White, 0.f);
            return;
        }

        // FIN
        if (pantalla == Pantalla::FIN) {
            if (e.key.code == sf::Keyboard::R) {
                pantalla = Pantalla::SELECCION; hud.mensaje("", sf::Color::White, 0.f);
            }
            if (e.key.code == sf::Keyboard::Escape) {
                pantalla = Pantalla::MENU; pMenu->resetear(); hud.mensaje("", sf::Color::White, 0.f);
            }
            return;
        }

        // JUGANDO
        if (pantalla == Pantalla::JUGANDO) {
            if (e.key.code == sf::Keyboard::Tab) {
                int sg = (eH.activo + 1) % 3;
                if (!eH.j[sg].tienePelota) {
                    eH.j[eH.activo].esHumano = false;
                    eH.activo = sg;
                    eH.j[sg].esHumano = true;
                }
            }
            if (e.key.code == sf::Keyboard::Escape) {
                pantalla = Pantalla::MENU; pMenu->resetear();
            }
        }
    }

    // ─── Loop principal ──────────────────────────────
    void run() {
        sf::Clock reloj;
        while (ventana.isOpen()) {
            float dt = std::min(reloj.restart().asSeconds(), 0.05f);
            sf::Event e;
            while (ventana.pollEvent(e)) evento(e);
            update(dt);
            dibujar();
        }
    }
};

// ─────────────────────────────────────────
//  main
// ─────────────────────────────────────────
int main() {
    GameManager gm;
    gm.run();
    return 0;
}
