#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <cmath>
#include "Constantes.hpp"
#include "Pelota.hpp"

// ================================================================
//  SpriteSheet  -  maneja animaciones desde un sprite sheet PNG
//  Filas por personaje (5 filas uniformes):
//    Row 0 = WALK     (caminar)
//    Row 1 = RUN      (correr)
//    Row 2 = DRIBBLE  (regate / jump preparacion)
//    Row 3 = SHOOT    (tiro / salto)
//    Row 4 = DUNK     (slam dunk)
// ================================================================
struct SpriteSheet {
    sf::Texture tex;
    sf::Sprite  spr;
    bool        ok = false;

    int frameW = 0, frameH = 0;
    int cols   = 8;   // frames por fila
    int rows   = 5;   // filas de animación

    // Animación actual
    int  rowActual  = 0;
    int  frameActual = 0;
    int  maxFrames  = 8;   // frames en la fila actual
    float timerAnim = 0.f;
    float velAnim   = 0.10f;  // segundos por frame

    bool mirando_izq = false;

    bool cargar(const std::string& ruta, int c, int r) {
        if (!tex.loadFromFile(ruta)) return false;
        ok   = true;
        cols = c; rows = r;
        frameW = (int)tex.getSize().x / c;
        frameH = (int)tex.getSize().y / r;
        spr.setTexture(tex);
        spr.setTextureRect(sf::IntRect(0, 0, frameW, frameH));
        spr.setOrigin(frameW / 2.f, (float)frameH);
        return true;
    }

    // row: 0=walk,1=run,2=dribble,3=shoot,4=dunk
    void setAnimacion(int row, int nFrames, float fps = 10.f) {
        if (row == rowActual && maxFrames == nFrames) return;
        rowActual   = row;
        maxFrames   = nFrames;
        frameActual = 0;
        timerAnim   = 0.f;
        velAnim     = 1.f / fps;
    }

    void actualizar(float dt) {
        if (!ok) return;
        timerAnim += dt;
        if (timerAnim >= velAnim) {
            timerAnim -= velAnim;
            frameActual = (frameActual + 1) % maxFrames;
        }
        int fx = frameActual * frameW;
        int fy = rowActual   * frameH;
        spr.setTextureRect(sf::IntRect(fx, fy, frameW, frameH));
    }

    void dibujar(sf::RenderWindow& w, sf::Vector2f pos, float escala, float offsetY) {
        if (!ok) return;
        float ex = mirando_izq ? -escala : escala;
        spr.setScale(ex, escala);
        spr.setPosition(pos.x, pos.y + offsetY);
        w.draw(spr);
    }

    float altoEscalado(float escala) const {
        return frameH * escala;
    }
};

// ================================================================
//  Jugador  -  personaje con sprite sheet animado, stats, física
//  Row mapping:  0=walk  1=run  2=dribble/jump  3=shoot  4=dunk
// ================================================================
class Jugador {
public:
    std::string  nombre;
    sf::Vector2f pos;
    sf::Color    colorEquipo;

    // Atributos 1-9 (escala de Street Slam)
    int aDunk = 5, aTres = 5, aVel = 5, aDef = 5;

    bool tienePelota = false;
    bool esHumano    = false;

    EstadoJ estado  = EstadoJ::IDLE;
    float   timerE  = 0.f;

    // Sprite sheet animado
    SpriteSheet sheet;
    float       escalaSprite = 1.f;

    // Shapes auxiliares
    sf::CircleShape sombra, indicador, circulo;

    // Física de salto
    float alturaSalto   = 0.f;
    bool  botonAireAct  = false;

    // Finta
    bool  fintaActiva = false;
    float timerFinta  = 0.f;

    // Alley-oop
    bool  esperandoAlleyOop = false;
    float timerAlleyOop     = 0.f;

    // Empuje / aturdido
    float        timerAturdido = 0.f;
    sf::Vector2f velEmpuje;

    // Flash
    float timerFlash = 0.f;
    bool  visible    = true;

    // Estamina
    float estamina = 100.f;

    // Número de frames por animación para cada personaje
    // (se configura al cargar el sheet)
    int framesWalk   = 8;
    int framesRun    = 8;
    int framesDribble= 6;
    int framesShoot  = 4;
    int framesDunk   = 4;

    Jugador() {
        sombra.setRadius(17.f);
        sombra.setOrigin(17.f, 8.f);
        sombra.setFillColor(sf::Color(0,0,0,60));

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

    // Carga el sprite sheet con parámetros del personaje
    bool cargarSprite(const std::string& ruta,
                      int cols=8, int rows=5,
                      int fw=8, int fr=8, int fd=6, int fs=4, int fdk=4,
                      float altoPx=80.f) {
        if (!sheet.cargar(ruta, cols, rows)) return false;
        framesWalk    = fw;
        framesRun     = fr;
        framesDribble = fd;
        framesShoot   = fs;
        framesDunk    = fdk;
        // Escala para que el personaje mida altoPx px de alto
        escalaSprite  = altoPx / sheet.frameH;
        return true;
    }

    // Sprite estático (imagen única - fallback)
    sf::Texture texStatic;
    sf::Sprite  sprStatic;
    bool staticOk = false;
    bool cargarSpriteEstatico(const std::string& ruta, float altoPx=80.f) {
        if (!texStatic.loadFromFile(ruta)) return false;
        staticOk = true;
        sprStatic.setTexture(texStatic);
        sf::FloatRect b = sprStatic.getLocalBounds();
        sprStatic.setOrigin(b.width/2.f, b.height);
        float esc = altoPx / b.height;
        sprStatic.setScale(esc, esc);
        return true;
    }

    // ─── Stats ───────────────────────────────────────
    float velReal(bool sprint=false) const {
        float mult = 0.6f + (estamina/100.f)*0.4f;
        if (sprint) return (VEL_SPRINT + (aVel-1)*5.f)  * mult;
        return          (VEL_BASE  + (aVel-1)*11.f) * mult;
    }
    float probTiro(float d, bool esTres) const {
        float base = esTres ? (aTres/9.f) : (aDunk/9.f);
        float pen  = std::min(d/650.f, 0.45f);
        if (timerAturdido > 0.f) base *= 0.5f;
        return std::max(0.10f, base-pen);
    }
    float probBloqueo()    const { return 0.10f + (aDef/9.f)*0.38f; }
    float probRobo()       const { return 0.07f + (aDef/9.f)*0.30f; }
    float probInterceptar()const { return 0.04f + (aDef/9.f)*0.22f; }

    // ─── Movimiento ──────────────────────────────────
    void mover(sf::Vector2f dir, float dt, bool sprint=false) {
        if (timerAturdido > 0.f) {
            pos += velEmpuje * dt;
            velEmpuje *= 0.88f;
            clampCancha(); return;
        }
        float len = std::sqrt(dir.x*dir.x + dir.y*dir.y);
        if (len > 0.f) {
            dir /= len;
            if (dir.x < -0.1f)     sheet.mirando_izq = true;
            else if (dir.x > 0.1f) sheet.mirando_izq = false;
            pos += dir * velReal(sprint) * dt;
            if (sprint) estamina = std::max(0.f, estamina - 22.f*dt);
            if (estado==EstadoJ::IDLE||estado==EstadoJ::CORRIENDO)
                estado = EstadoJ::CORRIENDO;
        } else {
            estamina = std::min(100.f, estamina + 28.f*dt);
            if (estado==EstadoJ::CORRIENDO) estado = EstadoJ::IDLE;
        }
        clampCancha();
    }
    void clampCancha() {
        pos.x = std::max(C_X+J_RADIO, std::min(pos.x, C_X+C_ANCHO-J_RADIO));
        pos.y = std::max(C_Y+J_RADIO, std::min(pos.y, C_Y+C_ALTO -J_RADIO));
    }
    void setEstado(EstadoJ e, float dur) { estado=e; timerE=dur; }
    void recibirEmpuje(sf::Vector2f dir, float fuerza) {
        velEmpuje=dir*fuerza; timerAturdido=0.30f;
        setEstado(EstadoJ::ATURDIDO, 0.30f);
    }

    // ─── Elegir animación según estado ───────────────
    void actualizarAnimacion(float dt) {
        if (!sheet.ok) return;
        switch (estado) {
            case EstadoJ::IDLE:
                sheet.setAnimacion(0, framesWalk, 6.f); break;
            case EstadoJ::CORRIENDO:
                sheet.setAnimacion(1, framesRun,  10.f); break;
            case EstadoJ::LANZANDO:
            case EstadoJ::SUPER_SHOT:
                sheet.setAnimacion(3, framesShoot, 9.f); break;
            case EstadoJ::EN_AIRE:
                sheet.setAnimacion(2, framesDribble, 8.f); break;
            case EstadoJ::DUNKEANDO:
                sheet.setAnimacion(4, framesDunk, 10.f); break;
            case EstadoJ::PASANDO:
                sheet.setAnimacion(3, framesShoot, 9.f); break;
            case EstadoJ::BLOQUEANDO:
                sheet.setAnimacion(3, framesShoot, 8.f); break;
            case EstadoJ::ROBANDO:
            case EstadoJ::EMPUJANDO:
                sheet.setAnimacion(2, framesDribble, 9.f); break;
            case EstadoJ::FINTANDO:
                sheet.setAnimacion(2, framesDribble, 12.f); break;
            case EstadoJ::ALLEYOOP_VUELO:
                sheet.setAnimacion(4, framesDunk, 8.f); break;
            case EstadoJ::ATURDIDO:
                sheet.setAnimacion(0, framesWalk, 5.f); break;
            default:
                sheet.setAnimacion(0, framesWalk, 6.f); break;
        }
        sheet.actualizar(dt);
    }

    // ─── Update ──────────────────────────────────────
    void actualizar(float dt) {
        // Timer de estado
        if (timerE > 0.f) {
            timerE -= dt;
            if (timerE <= 0.f && estado!=EstadoJ::IDLE && estado!=EstadoJ::CORRIENDO) {
                timerE=0.f; estado=EstadoJ::IDLE;
                fintaActiva=false; esperandoAlleyOop=false;
            }
        }
        if (timerAturdido>0.f) {
            timerAturdido -= dt;
            if (timerAturdido<=0.f) { timerAturdido=0.f; velEmpuje={}; if(estado==EstadoJ::ATURDIDO) estado=EstadoJ::IDLE; }
        }
        if (fintaActiva) { timerFinta-=dt; if(timerFinta<=0.f) fintaActiva=false; }
        if (esperandoAlleyOop) {
            timerAlleyOop-=dt;
            if(timerAlleyOop<=0.f){esperandoAlleyOop=false; if(estado==EstadoJ::ALLEYOOP_VUELO) estado=EstadoJ::IDLE;}
        }
        if (timerFlash>0.f){ timerFlash-=dt; visible=(int)(timerFlash*12.f)%2==0; }
        else visible=true;

        // Altura de salto
        if (estado==EstadoJ::EN_AIRE||estado==EstadoJ::DUNKEANDO) {
            float p = timerE>0.f?(1.f-timerE/0.55f):1.f;
            alturaSalto = 55.f*std::sin(3.14159f*p);
        } else if (estado==EstadoJ::ALLEYOOP_VUELO) {
            alturaSalto = 70.f;
        } else if (estado==EstadoJ::BLOQUEANDO) {
            float p = timerE>0.f?(1.f-timerE/0.35f):1.f;
            alturaSalto = 45.f*std::sin(3.14159f*p);
        } else {
            alturaSalto = 0.f;
        }

        actualizarAnimacion(dt);

        // Sombra (achicarse con la altura del salto)
        float ss = 1.f - alturaSalto*0.007f;
        sombra.setPosition(pos.x, pos.y+18.f-alturaSalto*0.1f);
        sombra.setScale(ss, ss);
        circulo.setPosition(pos.x, pos.y-alturaSalto);
        circulo.setFillColor(colorEquipo);
        indicador.setPosition(pos.x, pos.y-50.f-alturaSalto);
    }

    // ─── Dibujo ──────────────────────────────────────
    void dibujar(sf::RenderWindow& w) {
        if (!visible) return;

        // Halo alley-oop
        if (esperandoAlleyOop) {
            float pulse = 0.5f+0.5f*std::sin(timerAlleyOop*10.f);
            sf::CircleShape ring(J_RADIO+10.f+pulse*6.f);
            ring.setOrigin(ring.getRadius(),ring.getRadius());
            ring.setPosition(pos.x, pos.y-alturaSalto);
            ring.setFillColor(sf::Color::Transparent);
            ring.setOutlineThickness(3.f);
            ring.setOutlineColor(sf::Color(255,220,0,(sf::Uint8)(160+pulse*80)));
            w.draw(ring);
        }
        // Halo finta
        if (fintaActiva) {
            sf::CircleShape ring(J_RADIO+7.f);
            ring.setOrigin(ring.getRadius(),ring.getRadius());
            ring.setPosition(pos.x, pos.y-alturaSalto);
            ring.setFillColor(sf::Color::Transparent);
            ring.setOutlineThickness(2.f);
            ring.setOutlineColor(sf::Color(255,100,0,180));
            w.draw(ring);
        }

        // Sprite (sheet animado o estático como fallback)
        if (sheet.ok) {
            sheet.dibujar(w, pos, escalaSprite, 18.f - alturaSalto);
        } else if (staticOk) {
            float ex = sheet.mirando_izq ? -std::abs(sprStatic.getScale().x)
                                         :  std::abs(sprStatic.getScale().x);
            sprStatic.setScale(ex, std::abs(sprStatic.getScale().y));
            sprStatic.setPosition(pos.x, pos.y+18.f-alturaSalto);
            w.draw(sprStatic);
        } else {
            w.draw(circulo);
        }

        if (tienePelota) w.draw(indicador);

        // Super shot glow
        if (estado==EstadoJ::SUPER_SHOT) {
            sf::CircleShape glow(J_RADIO+14.f);
            glow.setOrigin(J_RADIO+14.f,J_RADIO+14.f);
            glow.setPosition(pos.x, pos.y-alturaSalto);
            glow.setFillColor(sf::Color(255,200,0,70));
            w.draw(glow);
        }

        // Resaltar jugador activo
        if (esHumano && (estado==EstadoJ::IDLE||estado==EstadoJ::CORRIENDO||estado==EstadoJ::FINTANDO)) {
            sf::CircleShape ring(J_RADIO+4.f);
            ring.setOrigin(J_RADIO+4.f,J_RADIO+4.f);
            ring.setPosition(pos.x, pos.y+15.f);
            ring.setFillColor(sf::Color::Transparent);
            ring.setOutlineThickness(2.f);
            ring.setOutlineColor(sf::Color(255,255,100,160));
            w.draw(ring);
        }

        // Estrellas de aturdido
        if (timerAturdido>0.f) {
            for (int i=0;i<3;i++) {
                float ang = timerAturdido*8.f + i*2.094f;
                sf::CircleShape star(4.f,5);
                star.setOrigin(4.f,4.f);
                star.setPosition(pos.x+std::cos(ang)*22.f,
                                 pos.y-alturaSalto-30.f+std::sin(ang)*8.f);
                star.setFillColor(sf::Color(255,255,50,200));
                w.draw(star);
            }
        }
    }
};
