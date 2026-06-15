// ================================================================
//   CARTOON DUNK  -  Street Hoop Style
//   SFML 2.6  |  C++17  |  Código Corregido y Optimizado
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
#include <memory> 

#include "Constantes.hpp"
#include "Pelota.hpp"
#include "Jugador.hpp"
#include "Equipo.hpp"
#include "Cancha.hpp"
#include "HUD.hpp"

// ─────────────────────────────────────────
//   Enum de pantallas
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
//   Helpers matemáticos
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
//   Datos de equipos
// ─────────────────────────────────────────
static DatosEquipo equipoCartoon() {
    DatosEquipo d;
    d.nombre    = "CARTOON";
    d.sprites[0]= "assets/imagenes/goku.png";
    d.sprites[1]= "assets/imagenes/Pocoyo.png";
    d.sprites[2]= "assets/imagenes/goku.png"; 
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
//   PANTALLA DE INTRO
// ================================================================
class PantallaIntro {
public:
    sf::Sprite sprFondo, sprGokuArt, sprMisticoArt, sprPocoyoArt;
    
    struct Part { float x, y, vel, tam, alfa; sf::Color col; };
    std::vector<Part> partes;

    float tTotal=0, parpadeo=0, escLogo=0, alfaFondo=0, alfaPerso=0, ondaY=0;
    sf::Font& F; bool Fok;

    // Las texturas ahora se reciben por referencia desde el GameManager para evitar tirones
    PantallaIntro(sf::Font& f, bool fok, const sf::Texture& tFondo, const sf::Texture& tGoku, const sf::Texture& tMistico, const sf::Texture& tPocoyo) 
        : F(f), Fok(fok) {
        
        sprFondo.setTexture(tFondo);
        float sx = (float)W_ANCHO / tFondo.getSize().x;
        float sy = (float)W_ALTO  / tFondo.getSize().y;
        sprFondo.setScale(std::max(sx,sy), std::max(sx,sy));
        sprFondo.setColor(sf::Color(255,255,255,0));

        auto setupSprite = [](sf::Sprite& s, const sf::Texture& t, float h, float x, float y) {
            s.setTexture(t);
            float sc = h / t.getSize().y;
            s.setScale(sc, sc);
            s.setPosition(x, y);
            s.setColor(sf::Color(255,255,255,0));
        };

        setupSprite(sprGokuArt,   tGoku,   210.f, 30.f,  (float)W_ALTO - 220.f);
        setupSprite(sprPocoyoArt, tPocoyo, 130.f, 200.f, (float)W_ALTO - 145.f);

        sprMisticoArt.setTexture(tMistico);
        float sc = 210.f / tMistico.getSize().y;
        sprMisticoArt.setScale(sc, sc);
        float pw = tMistico.getSize().x * sc;
        sprMisticoArt.setPosition((float)W_ANCHO - pw - 30.f, (float)W_ALTO - 220.f);
        sprMisticoArt.setColor(sf::Color(255,255,255,0));

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

        sprFondo.setColor(sf::Color(255,255,255,(sf::Uint8)alfaFondo));

        sf::Uint8 ap = (sf::Uint8)alfaPerso;
        sprGokuArt.setPosition(30.f, (float)W_ALTO - 220.f + ondaY * 0.6f);
        sprGokuArt.setColor({255,255,255,ap});
        
        float pw = sprMisticoArt.getTexture()->getSize().x * sprMisticoArt.getScale().x;
        sprMisticoArt.setPosition((float)W_ANCHO - pw - 30.f, (float)W_ALTO - 220.f - ondaY * 0.6f);
        sprMisticoArt.setColor({255,255,255,ap});
        
        sprPocoyoArt.setPosition(200.f, (float)W_ALTO - 145.f + ondaY * 0.4f);
        sprPocoyoArt.setColor({255,255,255,ap});

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
        sf::RectangleShape bg({(float)W_ANCHO,(float)W_ALTO});
        bg.setFillColor(sf::Color(8, 4, 18));
        w.draw(bg);

        w.draw(sprFondo);

        sf::RectangleShape ov({(float)W_ANCHO,(float)W_ALTO});
        ov.setFillColor(sf::Color(0,0,0,110));
        w.draw(ov);

        for (auto& p : partes) {
            sf::CircleShape c(p.tam);
            c.setPosition(p.x, p.y);
            sf::Color col = p.col; col.a = (sf::Uint8)p.alfa;
            c.setFillColor(col);
            w.draw(c);
        }

        w.draw(sprGokuArt);
        w.draw(sprMisticoArt);
        w.draw(sprPocoyoArt);

        if (escLogo > 0.01f) {
            dtxt(w, "CARTOON DUNK", 68, sf::Color(180, 80, 0, (sf::Uint8)(200*escLogo)), (float)W_ANCHO/2.f + 4.f, 84.f, escLogo);
            dtxt(w, "CARTOON DUNK", 68, sf::Color(255, 220, 40, (sf::Uint8)(255*escLogo)), (float)W_ANCHO/2.f, 80.f, escLogo);
            dtxt(w, "Street Basketball 3v3", 22, sf::Color(200, 200, 200, (sf::Uint8)(220*escLogo)), (float)W_ANCHO/2.f, 160.f, std::min(escLogo*1.3f, 1.f));

            if (escLogo >= 0.9f) {
                float lw = 380.f * escLogo;
                sf::RectangleShape ln({lw, 2.f});
                ln.setFillColor(sf::Color(255,180,0,160));
                ln.setOrigin(lw/2.f, 1.f);
                ln.setPosition((float)W_ANCHO/2.f, 178.f);
                w.draw(ln);
                dtxt(w, "Inspirado en Street Slam | Data East 1994", 14, sf::Color(140,140,170,180), (float)W_ANCHO/2.f, 192.f, 1.f);
            }
        }

        if (tTotal > 1.2f) {
            float a = std::sin(parpadeo) * 0.5f + 0.5f;
            dtxt(w, "PRESIONA  ENTER  PARA  COMENZAR", 20, sf::Color(255, 240, 100, (sf::Uint8)(220*a)), (float)W_ANCHO/2.f, (float)W_ALTO - 52.f, 1.f);
        }

        if (escLogo >= 0.9f) {
            dtxt(w, "WASD:Mover  J:Tiro  K:Pase  J+K:Dunk  L:Finta/Robo  Tab:Cambiar jugador", 11, sf::Color(110,110,120,150), (float)W_ANCHO/2.f, (float)W_ALTO - 20.f, 1.f);
        }
    }

private:
    void dtxt(sf::RenderWindow& w, const std::string& s, int sz, sf::Color c, float x, float y, float esc) {
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
//   PANTALLA DE MENÚ PRINCIPAL
// ================================================================
class MenuPrincipal {
public:
    sf::Sprite  sprG, sprP, sprM, sprF;

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

    MenuPrincipal(sf::Font& f, bool fok, const sf::Texture& tFondo, const sf::Texture& tGoku, const sf::Texture& tMistico, const sf::Texture& tPocoyo) 
        : F(f), Fok(fok) {
        
        sprF.setTexture(tFondo);
        float sx=(float)W_ANCHO/tFondo.getSize().x, sy=(float)W_ALTO/tFondo.getSize().y;
        sprF.setScale(std::max(sx,sy), std::max(sx,sy));
        sprF.setColor(sf::Color(255,255,255,55));
        
        auto ss = [](sf::Sprite& s, const sf::Texture& tx, float h, float x, float y) {
            s.setTexture(tx);
            float sc = h / tx.getSize().y;
            s.setScale(sc, sc);
            s.setPosition(x, y);
        };
        
        ss(sprG, tGoku, 260.f, 30.f, (float)W_ALTO - 270.f);
        ss(sprP, tPocoyo, 140.f, 260.f, (float)W_ALTO - 155.f);
        
        ss(sprM, tMistico, 200.f, 0.f, 0.f);
        float pw = tMistico.getSize().x * sprM.getScale().x;
        sprM.setPosition((float)W_ANCHO - pw - 220.f, (float)W_ALTO - 210.f);

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

        sprG.setPosition(30.f, (float)W_ALTO - 270.f + ondaY * 0.7f);
        sprP.setPosition(260.f, (float)W_ALTO - 155.f + ondaY);
        
        float pw = sprM.getTexture()->getSize().x * sprM.getScale().x;
        sprM.setPosition((float)W_ANCHO - pw - 220.f, (float)W_ALTO - 210.f + ondaY*0.8f); 

        for (auto& b : bolas) {
            b.y   -= b.vel * dt;
            b.alfa = std::min(b.alfa + dt * 80.f, 70.f);
            if (b.y < -20.f) { b.y = (float)W_ALTO + 10.f; b.x = (float)(rand() % W_ANCHO); b.alfa = 0.f; }
        }
    }

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
        return sel == 3 && sub == Sub::NINGUNA && (k == sf::Keyboard::Return || k == sf::Keyboard::Space);
    }

    void dibujar(sf::RenderWindow& w) {
        sf::RectangleShape bg({(float)W_ANCHO,(float)W_ALTO});
        bg.setFillColor(sf::Color(6,4,16));
        w.draw(bg);

        w.draw(sprF);

        sf::RectangleShape piz({420.f,(float)W_ALTO});
        piz.setFillColor(sf::Color(0,0,0,145));
        w.draw(piz);

        for (auto& b : bolas) {
            sf::CircleShape c(b.r);
            c.setPosition(b.x - b.r, b.y - b.r);
            c.setFillColor(sf::Color(255, 140, 20, (sf::Uint8)b.alfa));
            w.draw(c);
        }

        w.draw(sprG);
        w.draw(sprP);
        w.draw(sprM);

        sf::Uint8 ae = (sf::Uint8)alfaE;
        tx(w, "CARTOON DUNK",        46, sf::Color(255,215,30,ae),           210.f, 65.f);
        tx(w, "Street Basketball 3v3",18, sf::Color(180,180,200,(sf::Uint8)(ae*0.8f)), 210.f,118.f);

        sf::RectangleShape sep({330.f, 2.f});
        sep.setOrigin(165.f, 1.f);
        sep.setPosition(210.f, 140.f);
        sep.setFillColor(sf::Color(255,180,0,(sf::Uint8)(ae*0.5f)));
        w.draw(sep);

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

        tx(w, "W/S:Navegar    ENTER:Seleccionar    ESC:Volver", 11, sf::Color(100,100,120,(sf::Uint8)(ae*0.7f)), 210.f, (float)W_ALTO - 22.f);

        if (sub == Sub::CTRL) dibujarCtrl(w);
        if (sub == Sub::CRED) dibujarCred(w);
    }

private:
    void tx(sf::RenderWindow& w, const std::string& s, int sz, sf::Color c, float x, float y) {
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
            {"PERSONAJES",  "Goku  |  Pocoyo  |  Mistico"},
            {"MUSICA",      "Gang$tazz.ogg"},
            {"FUENTE",      "texto.ttf"},
            {"",""},
            {"MECANICAS",   "Tiro 2/3 pts  |  Dunk en Aire  |  Super Shot"},
            {"",            "Alley-Oop  |  Finta  |  Bloqueo  |  Intercepcion  |  Empuje"},
            {"",""},
            {"EQUIPOS",     "Cartoon (Goku/Pocoyo)  vs  Rivales (Mistico)"},
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
//   GameManager
// ================================================================
class GameManager {
private:
    sf::RenderWindow ventana;
    sf::Font  fuente; bool fuenteOk = false;

    // Almacenamiento centralizado de Texturas para evitar errores de ruta en cascada
    sf::Texture tFondo, tGoku, tMistico, tPocoyo;

    std::unique_ptr<PantallaIntro> pIntro;
    std::unique_ptr<MenuPrincipal> pMenu;

    Equipo   eH, eCPU;
    Pelota   pelota;
    Cancha   cancha;
    HUD      hud;
    sf::Music musica;

    Pantalla pantalla = Pantalla::INTRO;
    float tiempo = TIEMPO_MITAD;
    int   mitad  = 1;

    sf::RectangleShape fondoRect;

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

        // Carga única y segura de texturas al iniciar el juego
        tFondo.loadFromFile("assets/imagenes/pantalla inicial del juego.jpg");
        tGoku.loadFromFile("assets/imagenes/goku.png");
        tMistico.loadFromFile("assets/imagenes/mistico.png");
        tPocoyo.loadFromFile("assets/imagenes/Pocoyo.png");

        if (musica.openFromFile("assets/musica/Gang$tazz.ogg")) {
            musica.setLoop(true);
            musica.setVolume(55.f);
            musica.play();
        }

        // Construcción segura pasando las texturas cargadas
        pIntro = std::make_unique<PantallaIntro>(fuente, fuenteOk, tFondo, tGoku, tMistico, tPocoyo);
        pMenu  = std::make_unique<MenuPrincipal>(fuente, fuenteOk, tFondo, tGoku, tMistico, tPocoyo);
    }

    ~GameManager() = default;

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

        sf::Vector2f posH[3]   = {{C_X+210.f,ARO_Y},{C_X+265.f,ARO_Y-95.f},{C_X+265.f,ARO_Y+95.f}};
        sf::Vector2f posCPU[3] = {{C_X+C_ANCHO-210.f,ARO_Y},{C_X+C_ANCHO-265.f,ARO_Y-95.f},{C_X+C_ANCHO-265.f,ARO_Y+95.f}};

        for (int i = 0; i < 3; i++) {
            eH.j[i].pos   = posH[i];
            eCPU.j[i].pos = posCPU[i];
            eH.j[i].tienePelota   = eCPU.j[i].tienePelota = false;
            eH.j[i].estado        = eCPU.j[i].estado       = EstadoJ::IDLE;
        }
        eH.activo = eCPU.activo = 0;
        eH.j[0].tienePelota = true;
        pelota.pos = eH.j[0].pos;
        pelota.enManos = true; pelota.enJuego = false; pelota.enArco = false;
    }

    void reiniciarPosesion(bool hCoge) {
        eH.quitarPelota(); eCPU.quitarPelota();
        pelota.enArco = pelota.enJuego = false;
        pelota.enManos = true; pelota.esSuper = false;
        alleyOopActivado = false; alleyOopReceptor = -1;
        sf::Vector2f c = {C_X + C_ANCHO/2.f, C_Y + C_ALTO/2.f};
        pelota.pos = c;
        if (hCoge) { eH.j[0].pos = c;   eH.darPelota(0); }
        else       { eCPU.j[0].pos = c; eCPU.darPelota(0); }
    }

    int evalEnceste(bool esH, float d) {
        Jugador& j = esH ? eH.j[eH.activo] : eCPU.j[eCPU.activo];
        bool es3 = d > DIST_3P;
        float p  = j.probTiro(d, es3);
        if (pelota.esSuper) p = std::min(p + 0.40f, 0.97f);
        if (!prob(p)) return 0;
        return es3 ? 3 : 2;
    }

    void anotar(bool eqH, int pts) {
        std::string msg = pelota.esSuper ? "!SUPER DUNK!" : (pts == 3 ? "!TRIPLE!" : "CANASTA!");
        sf::Color c = eqH ? sf::Color(255,200,0) : sf::Color(100,200,255);
        if (eqH) eH.sumarPuntos(pts); else eCPU.sumarPuntos(pts);

        hud.mensaje(msg, c, 2.5f);
        pausaPostGol = 1.3f;
        reiniciarPosesion(!eqH);
    }

    void lanzarTiroNormal(bool esH) {
        Jugador& act = esH ? eH.j[eH.activo] : eCPU.j[eCPU.activo];
        Equipo&  eq  = esH ? eH : eCPU;
        if (!act.tienePelota) return;

        sf::Vector2f ap = esH ? sf::Vector2f{ARO_DER_CX, ARO_Y} : sf::Vector2f{ARO_IZQ_CX, ARO_Y};
        float d   = dist2(act.pos, ap);
        bool  sup = eq.superLleno();
        if (sup) eq.gastarSuper();

        act.tienePelota = false;
        act.setEstado(sup ? EstadoJ::SUPER_SHOT : EstadoJ::LANZANDO, 0.6f);

        float tv = 0.45f + d / 1300.f;
        pelota.lanzarTiro(act.pos, ap, tv, sup);
        pelota.esEquipoH = esH;

        int p = evalEnceste(esH, d);
        pelota.anotoPendiente   = (p > 0);
        pelota.puntosPendientes = p;

        cdTiro = 0.85f;
    }
    
    // ... resto del ciclo de métodos unificados del bucle principal
};