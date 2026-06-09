// ================================================================
//  CARTOON DUNK
//  Baloncesto 3v3 estilo arcade - inspirado en Street Hoop (1994)
//  Motor: SFML 2.6
//
//  Mecánicas implementadas (basadas en el original):
//   - Tiro 2pts / 3pts con probabilidad por atributo
//   - Pase (rápido, interceptable por rivales)
//   - Dunk en el aire (A+B -> A) con arco alto
//   - Super Shot (meter lleno -> dunk de 10 metros)
//   - Alley-oop (pase en vuelo a compañero cerca del aro)
//   - Bloqueo de tiro (salta y corta el arco)
//   - Intercepción de pase (rival corta en vuelo)
//   - Finta (amaga tiro, engaña a la defensa)
//   - Empuje / shoving estilo calle
//   - Rebote de tablero tras fallo
//   - Super meter regenera con el tiempo + acciones
//   - IA defensiva mejorada con bloqueos e interceptaciones
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
#include <functional>

#include "Constantes.hpp"
#include "Pelota.hpp"
#include "Jugador.hpp"
#include "Equipo.hpp"
#include "Cancha.hpp"
#include "HUD.hpp"

// ────────────────────────────────────────────
//  Helpers matemáticos
// ────────────────────────────────────────────
static float dist2(sf::Vector2f a, sf::Vector2f b) {
    float dx = a.x - b.x, dy = a.y - b.y;
    return std::sqrt(dx*dx + dy*dy);
}
static sf::Vector2f norm2(sf::Vector2f v) {
    float l = std::sqrt(v.x*v.x + v.y*v.y);
    return l < 0.001f ? sf::Vector2f{} : sf::Vector2f{v.x/l, v.y/l};
}
static bool prob(float p) {
    return ((float)rand()/RAND_MAX) < p;
}

// ────────────────────────────────────────────
//  Definición de equipos
// ────────────────────────────────────────────
static DatosEquipo equipoCartoon() {
    DatosEquipo d;
    d.nombre = "CARTOON";
    d.sprites[0] = "assets/imagenes/goku.png";
    d.sprites[1] = "assets/imagenes/Pocoyo.png";
    d.sprites[2] = "assets/imagenes/bugs bunny.png";
    d.color  = sf::Color(255, 120, 0);
    d.dunk = 8; d.tres = 6; d.vel = 7; d.def = 5;
    return d;
}
static DatosEquipo equipoRival() {
    DatosEquipo d;
    d.nombre = "RIVALES";
    d.sprites[0] = "assets/imagenes/mistico.png";
    d.sprites[1] = "assets/imagenes/mistico.png";
    d.sprites[2] = "assets/imagenes/mistico.png";
    d.color  = sf::Color(80, 160, 255);
    d.dunk = 6; d.tres = 8; d.vel = 6; d.def = 8;
    return d;
}

// ────────────────────────────────────────────
//  GameManager
// ────────────────────────────────────────────
class GameManager {
    sf::RenderWindow ventana;
    Equipo   eH, eCPU;
    Pelota   pelota;
    Cancha   cancha;
    HUD      hud;
    sf::Music musica;

    EstadoJuego estado = EstadoJuego::SELECCION;
    float tiempo  = TIEMPO_MITAD;
    int   mitad   = 1;

    sf::RectangleShape fondoRect;

    // ── Controles dunk en aire ──
    bool  esperandoDunkEnAire = false;
    float timerSalto          = 0.f;

    // ── Alley-oop pendiente ──
    bool  alleyOopActivado    = false;
    int   alleyOopReceptor    = -1;
    float timerAlleyOop       = 0.f;    // ventana para recibir el pase

    // ── Cooldowns ──
    float cdTiro  = 0.f;
    float cdPase  = 0.f;
    float cdRobo  = 0.f;
    float cdEmpuje = 0.f;

    // ── Post gol ──
    float pausaPostGol = 0.f;

    // ── Selección ──
    int equipoSeleccionado = 0;

    // ── Indicador de bloqueo exitoso ──
    bool  bloqueadoReciente = false;
    float timerBloqueado    = 0.f;

    // ── Intercepción reciente ──
    bool  interceptadoReciente = false;
    float timerInterceptado     = 0.f;

public:
    GameManager()
        : ventana(sf::VideoMode(W_ANCHO, W_ALTO), "CARTOON DUNK - Street Hoop Style")
    {
        ventana.setFramerateLimit(60);
        srand((unsigned)time(nullptr));

        fondoRect.setSize({(float)W_ANCHO, (float)W_ALTO});
        fondoRect.setFillColor(sf::Color(22, 12, 4));

        if (musica.openFromFile("assets/musica/Gang$tazz.ogg")) {
            musica.setLoop(true);
            musica.setVolume(55.f);
            musica.play();
        }
    }

    // ─── Inicializar partido ───────────────────
    void iniciarPartida() {
        estado  = EstadoJuego::JUGANDO;
        tiempo  = TIEMPO_MITAD;
        mitad   = 1;
        cdTiro  = cdPase = cdRobo = cdEmpuje = 0.f;
        pausaPostGol = 0.f;
        esperandoDunkEnAire = false;
        alleyOopActivado    = false;
        alleyOopReceptor    = -1;

        DatosEquipo dH = equipoCartoon();
        DatosEquipo dCPU = equipoRival();
        if (equipoSeleccionado == 1) {
            std::swap(dH, dCPU);
        }

        eH.configurar(dH, true);
        eCPU.configurar(dCPU, false);

        eH.puntos = eCPU.puntos = 0;
        eH.superMeter = eCPU.superMeter = 0.f;
        eH.asistencias = eH.robos = eH.bloqueos = eH.alleyOops = 0;
        eCPU.asistencias = eCPU.robos = eCPU.bloqueos = eCPU.alleyOops = 0;

        sf::Vector2f posH[3] = {
            {C_X + 210.f, ARO_Y},
            {C_X + 265.f, ARO_Y - 95.f},
            {C_X + 265.f, ARO_Y + 95.f}
        };
        sf::Vector2f posCPU[3] = {
            {C_X + C_ANCHO - 210.f, ARO_Y},
            {C_X + C_ANCHO - 265.f, ARO_Y - 95.f},
            {C_X + C_ANCHO - 265.f, ARO_Y + 95.f}
        };
        for (int i = 0; i < 3; i++) {
            eH.j[i].pos  = posH[i];
            eCPU.j[i].pos = posCPU[i];
            eH.j[i].tienePelota  = false;
            eCPU.j[i].tienePelota = false;
            eH.j[i].estado  = EstadoJ::IDLE;
            eCPU.j[i].estado = EstadoJ::IDLE;
            eH.j[i].estamina = eCPU.j[i].estamina = 100.f;
        }
        eH.activo = eCPU.activo = 0;

        eH.j[0].tienePelota = true;
        pelota.pos = eH.j[0].pos;
        pelota.enManos = true;
        pelota.enJuego = false;
        pelota.enArco  = false;
    }

    // ─── Reiniciar posesión ────────────────────
    void reiniciarPosesion(bool humanoCogePelota) {
        eH.quitarPelota();
        eCPU.quitarPelota();
        pelota.enArco  = false;
        pelota.enJuego = false;
        pelota.enManos = true;
        pelota.esSuper = false;
        alleyOopActivado = false;
        alleyOopReceptor = -1;

        sf::Vector2f centro = {C_X + C_ANCHO/2.f, C_Y + C_ALTO/2.f};
        pelota.pos = centro;

        if (humanoCogePelota) {
            eH.j[0].pos = centro;
            eH.darPelota(0);
        } else {
            eCPU.j[0].pos = centro;
            eCPU.darPelota(0);
        }
    }

    // ─── Calcular resultado del tiro ─────────────
    int evalEnceste(bool esEquipoH, float distAlAro) {
        Jugador& j = esEquipoH ? eH.j[eH.activo] : eCPU.j[eCPU.activo];
        bool esTres = distAlAro > DIST_3P;
        float p = j.probTiro(distAlAro, esTres);
        if (pelota.esSuper) p = std::min(p + 0.40f, 0.97f);
        if (!prob(p)) return 0;
        return esTres ? 3 : 2;
    }

    // ─── Anotar ───────────────────────────────
    void anotar(bool equipoH, int pts, sf::Vector2f /*posAro*/) {
        std::string msg;
        if (pelota.esSuper)        msg = "!SUPER DUNK!";
        else if (pelota.esAlleyOop) msg = "!ALLEY-OOP!";
        else if (pts == 3)         msg = "!TRIPLE!";
        else                       msg = "CANASTA!";

        sf::Color c = equipoH ? sf::Color(255,200,0) : sf::Color(100,200,255);

        if (equipoH) {
            eH.sumarPuntos(pts);
            hud.flash(sf::Color(255,200,0));
        } else {
            eCPU.sumarPuntos(pts);
            hud.flash(sf::Color(80,120,255));
        }
        hud.mensaje(msg, c, 2.5f);
        pausaPostGol = 1.3f;
        reiniciarPosesion(!equipoH);
    }

    // ─── Lanzar tiro normal ───────────────────
    void lanzarTiroNormal(bool esH) {
        Jugador& act = esH ? eH.j[eH.activo] : eCPU.j[eCPU.activo];
        Equipo&  eq  = esH ? eH : eCPU;
        if (!act.tienePelota) return;

        sf::Vector2f aroPos = esH
            ? sf::Vector2f{ARO_DER_CX, ARO_Y}
            : sf::Vector2f{ARO_IZQ_CX, ARO_Y};

        float d    = dist2(act.pos, aroPos);
        bool super = eq.superLleno();
        if (super) eq.gastarSuper();

        act.tienePelota = false;
        act.setEstado(super ? EstadoJ::SUPER_SHOT : EstadoJ::LANZANDO, 0.6f);
        act.fintaActiva = false;

        float tVuelo = 0.45f + d / 1300.f;
        pelota.lanzarTiro(act.pos, aroPos, tVuelo, super);
        pelota.esEquipoH = esH;

        int pts = evalEnceste(esH, d);
        pelota.anotoPendiente  = (pts > 0);
        pelota.puntosPendientes = pts;

        cdTiro = 0.85f;
        esperandoDunkEnAire = false;
        alleyOopActivado    = false;
        if (esH) cdTiro = 0.85f;
    }

    // ─── Lanzar dunk en aire ──────────────────
    void lanzarDunk(bool esH) {
        Jugador& act = esH ? eH.j[eH.activo] : eCPU.j[eCPU.activo];
        Equipo&  eq  = esH ? eH : eCPU;
        if (!act.tienePelota) return;

        sf::Vector2f aroPos = esH
            ? sf::Vector2f{ARO_DER_CX, ARO_Y}
            : sf::Vector2f{ARO_IZQ_CX, ARO_Y};

        float d    = dist2(act.pos, aroPos);
        bool super = eq.superLleno();
        if (super) eq.gastarSuper();

        act.tienePelota = false;
        act.setEstado(EstadoJ::DUNKEANDO, 0.55f);

        float tVuelo = 0.5f + d / 1100.f;
        pelota.lanzarTiro(act.pos, aroPos, tVuelo, super);
        pelota.esEquipoH = esH;

        // Dunk: +15% de probabilidad
        float d2   = dist2(act.pos, aroPos);
        bool  esTres = d2 > DIST_3P;
        float p = act.probTiro(d2, esTres) + 0.15f;
        if (super) p = std::min(p + 0.40f, 0.97f);
        p = std::min(p, 0.95f);
        if (!prob(p)) {
            pelota.anotoPendiente   = false;
            pelota.puntosPendientes = 0;
        } else {
            pelota.anotoPendiente   = true;
            pelota.puntosPendientes = esTres ? 3 : 2;
        }

        cdTiro = 0.9f;
        esperandoDunkEnAire = false;
    }

    // ─── Finta ────────────────────────────────
    void activarFinta() {
        Jugador& act = eH.j[eH.activo];
        if (!act.tienePelota || act.fintaActiva) return;
        act.fintaActiva = true;
        act.timerFinta  = 0.45f;
        act.setEstado(EstadoJ::FINTANDO, 0.45f);
        // La finta puede provocar que la IA pierda posición defensiva
        hud.mensaje("FINTA!", sf::Color(255, 160, 0), 0.8f);
    }

    // ─── Pase entre compañeros ─────────────────
    void lanzarPase() {
        Jugador& act = eH.j[eH.activo];
        if (!act.tienePelota) return;

        sf::Vector2f aro{ARO_DER_CX, ARO_Y};

        // ── Si hay un receptor de alley-oop esperando, hacer alley-oop ──
        if (alleyOopActivado && alleyOopReceptor >= 0 &&
            eH.j[alleyOopReceptor].esperandoAlleyOop) {
            int r = alleyOopReceptor;
            act.tienePelota = false;
            act.setEstado(EstadoJ::PASANDO, 0.35f);
            float tVuelo = dist2(act.pos, eH.j[r].pos) / 650.f + 0.12f;
            pelota.lanzarAlleyOop(act.pos, eH.j[r].pos, tVuelo);
            pelota.esEquipoH = true;
            eH.receptorAlleyOop = r;
            eH.receptorPase  = -1;
            eH.sumarSuperAlleyOop();
            alleyOopActivado = false;
            cdPase = 0.65f;
            hud.mensaje("ALLEY-OOP!", sf::Color(255, 220, 0), 1.2f);
            return;
        }

        // Pase normal al compañero más libre (más cerca del aro)
        int receptor = -1;
        float minD   = 1e9f;
        for (int i = 0; i < 3; i++) {
            if (i == eH.activo) continue;
            float d2 = dist2(eH.j[i].pos, aro);
            if (d2 < minD) { minD = d2; receptor = i; }
        }
        if (receptor < 0) return;

        act.tienePelota = false;
        act.setEstado(EstadoJ::PASANDO, 0.35f);

        float tVuelo = dist2(act.pos, eH.j[receptor].pos) / 720.f + 0.08f;
        pelota.lanzarPase(act.pos, eH.j[receptor].pos, tVuelo);
        pelota.esEquipoH = true;
        eH.receptorPase  = receptor;
        eH.receptorAlleyOop = -1;
        eH.sumarSuperPase();
        cdPase = 0.6f;
    }

    // ─── Alley-oop: J+K cuando NO tienes pelota ──
    void activarAlleyOop() {
        Jugador& act = eH.j[eH.activo];
        if (act.tienePelota) return;
        // El jugador activo salta esperando el pase
        act.setEstado(EstadoJ::ALLEYOOP_VUELO, 0.9f);
        act.esperandoAlleyOop = true;
        act.timerAlleyOop     = 0.9f;
        alleyOopActivado  = true;
        alleyOopReceptor  = eH.activo;
    }

    // ─── Bloqueo humano ───────────────────────
    void intentarBloqueo() {
        Jugador& act = eH.j[eH.activo];
        act.setEstado(EstadoJ::BLOQUEANDO, 0.38f);
        cdRobo = 0.45f;

        if (!pelota.enArco || pelota.esPase || pelota.esEquipoH) return;

        // La pelota está en vuelo hacia el aro humano
        float d2 = dist2(act.pos, pelota.pos);
        if (d2 < RADIO_BLOQUEO) {
            float p = act.probBloqueo();
            if (prob(p)) {
                // Bloqueo exitoso: la pelota sale disparada libre
                pelota.enArco  = false;
                pelota.anotoPendiente = false;
                float vx = (eH.activo < 1 ? 1.f : -1.f) * 90.f;
                float vy = ((float)rand()/RAND_MAX - 0.5f) * 60.f;
                pelota.soltarLibre(pelota.pos, {vx, vy});
                eH.sumarSuperBloqueo();
                bloqueadoReciente = true;
                timerBloqueado    = 0.f;
                act.timerFlash    = 0.45f;
                hud.mensaje("!BLOQUEADO!", sf::Color(100,255,100), 1.5f);
            }
        }
    }

    // ─── Robo humano ──────────────────────────
    void intentarRoboHumano() {
        Jugador& act = eH.j[eH.activo];
        act.setEstado(EstadoJ::ROBANDO, 0.35f);

        for (auto& jcpu : eCPU.j) {
            if (!jcpu.tienePelota) continue;
            float d2 = dist2(act.pos, jcpu.pos);
            if (d2 < RADIO_ROBO + 8.f) {
                float p = act.probRobo() * 0.55f;
                if (prob(p)) {
                    jcpu.tienePelota = false;
                    act.tienePelota  = true;
                    pelota.pos  = act.pos;
                    pelota.enManos = true;
                    pelota.enJuego = false;
                    eH.sumarSuperRobo();
                    hud.mensaje("!ROBO!", sf::Color(255,255,100), 1.5f);
                }
                break;
            }
        }
    }

    // ─── Empuje estilo Street Hoop ────────────
    void intentarEmpuje() {
        if (cdEmpuje > 0.f) return;
        Jugador& act = eH.j[eH.activo];
        cdEmpuje = 0.7f;
        act.setEstado(EstadoJ::EMPUJANDO, 0.25f);

        for (auto& jcpu : eCPU.j) {
            float d2 = dist2(act.pos, jcpu.pos);
            if (d2 < J_RADIO * 2.6f) {
                sf::Vector2f dir = norm2(jcpu.pos - act.pos);
                jcpu.recibirEmpuje(dir, 180.f);

                // Probabilidad de perder la pelota al recibir empujón
                if (jcpu.tienePelota && prob(0.30f)) {
                    jcpu.tienePelota = false;
                    pelota.soltarLibre(jcpu.pos,
                        {dir.x * 80.f + ((float)rand()/RAND_MAX - 0.5f)*40.f,
                         dir.y * 60.f + ((float)rand()/RAND_MAX - 0.5f)*40.f});
                }
                break;
            }
        }
    }

    // ─── Interceptar pase en vuelo ────────────
    void verificarIntercepciones() {
        if (!pelota.enArco || !pelota.esPase) return;

        // Humano intercepta pase de CPU
        if (!pelota.esEquipoH) {
            for (int i = 0; i < 3; i++) {
                Jugador& jh = eH.j[i];
                if (jh.timerAturdido > 0.f) continue;
                float d2 = dist2(jh.pos, pelota.pos);
                if (d2 < J_RADIO + P_RADIO + 18.f) {
                    float p = jh.probInterceptar();
                    if (prob(p * 0.5f)) {
                        eH.darPelota(i);
                        pelota.tomarla();
                        pelota.pos = jh.pos;
                        eCPU.receptorPase = -1;
                        eCPU.receptorAlleyOop = -1;
                        eH.sumarSuperRobo();
                        interceptadoReciente = true;
                        timerInterceptado    = 0.f;
                        hud.mensaje("INTERCEPCION!", sf::Color(100,255,200), 1.5f);
                    }
                    return;
                }
            }
        }
        // CPU intercepta pase del humano
        else {
            for (int i = 0; i < 3; i++) {
                Jugador& jcpu = eCPU.j[i];
                if (jcpu.timerAturdido > 0.f) continue;
                float d2 = dist2(jcpu.pos, pelota.pos);
                if (d2 < J_RADIO + P_RADIO + 14.f) {
                    float p = jcpu.probInterceptar() * 0.38f;
                    if (prob(p)) {
                        eCPU.darPelota(i);
                        pelota.tomarla();
                        pelota.pos = jcpu.pos;
                        eH.receptorPase = -1;
                        eH.receptorAlleyOop = -1;
                        eCPU.sumarSuperRobo();
                        hud.mensaje("Pase cortado!", sf::Color(100,180,255), 1.2f);
                    }
                    return;
                }
            }
        }
    }

    // ─── Bloqueo de la IA ─────────────────────
    void verificarBloqueoIA() {
        if (!pelota.enArco || pelota.esPase) return;
        if (!pelota.esEquipoH) return;  // la pelota va hacia el aro de la CPU

        // Si la pelota va hacia el aro de la CPU, un defensor puede bloquearla
        for (int i = 0; i < 3; i++) {
            Jugador& def = eCPU.j[i];
            float d2 = dist2(def.pos, pelota.pos);
            if (d2 < RADIO_BLOQUEO + 10.f && def.timerAturdido <= 0.f) {
                float p = def.probBloqueo() * 0.30f;  // más difícil para la IA
                if (prob(p * (1.f/60.f) * 120.f)) {  // ajustado por frame
                    pelota.enArco = false;
                    pelota.anotoPendiente = false;
                    def.setEstado(EstadoJ::BLOQUEANDO, 0.38f);
                    float vx = -90.f;
                    float vy = ((float)rand()/RAND_MAX - 0.5f) * 60.f;
                    pelota.soltarLibre(pelota.pos, {vx, vy});
                    eCPU.sumarSuperBloqueo();
                    hud.mensaje("BLOQUEO!", sf::Color(100,200,255), 1.2f);
                    return;
                }
            }
        }
    }

    // ─── INPUT HUMANO ─────────────────────────
    void inputHumano(float dt) {
        if (estado != EstadoJuego::JUGANDO || pausaPostGol > 0.f) return;

        Jugador& act = eH.porActivo();
        bool tieneB = act.tienePelota;

        cdTiro  = std::max(0.f, cdTiro  - dt);
        cdPase  = std::max(0.f, cdPase  - dt);
        cdRobo  = std::max(0.f, cdRobo  - dt);
        cdEmpuje = std::max(0.f, cdEmpuje - dt);

        // ── Movimiento WASD ──
        sf::Vector2f dir{};
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) dir.x -= 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) dir.x += 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) dir.y -= 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) dir.y += 1.f;
        bool sprint = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift);

        // No moverse si está en estados fijos
        bool puedeMove = (act.estado == EstadoJ::IDLE ||
                          act.estado == EstadoJ::CORRIENDO ||
                          act.estado == EstadoJ::FINTANDO);
        if (puedeMove) act.mover(dir, dt, sprint);

        // Sincronizar pelota con portador
        if (tieneB && !pelota.enArco) pelota.pos = act.pos;

        // ── Auto-switch cuando no tenemos pelota ──
        if (!tieneB && !pelota.enArco && pelota.enJuego && !alleyOopActivado) {
            int mejorI = eH.activo;
            float minD = dist2(eH.j[eH.activo].pos, pelota.pos);
            for (int i = 0; i < 3; i++) {
                if (i == eH.activo) continue;
                float d2 = dist2(eH.j[i].pos, pelota.pos);
                if (d2 < minD - 30.f) { minD = d2; mejorI = i; }
            }
            if (mejorI != eH.activo) {
                eH.j[eH.activo].esHumano = false;
                eH.activo = mejorI;
                eH.j[eH.activo].esHumano = true;
            }
        }

        bool jP = sf::Keyboard::isKeyPressed(sf::Keyboard::J);
        bool kP = sf::Keyboard::isKeyPressed(sf::Keyboard::K);
        bool lP = sf::Keyboard::isKeyPressed(sf::Keyboard::L);

        // ── OFENSIVA ──────────────────────────
        if (tieneB) {
            // J+K → saltar para dunk
            if (jP && kP && cdTiro <= 0.f && !esperandoDunkEnAire) {
                act.setEstado(EstadoJ::EN_AIRE, 0.55f);
                esperandoDunkEnAire = true;
                cdTiro = 0.3f;
            }
            // En el aire → J para dunk
            else if (esperandoDunkEnAire && act.estado == EstadoJ::EN_AIRE
                     && jP && cdTiro <= 0.f) {
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
            // L solo → finta
            else if (lP && cdTiro <= 0.f && !act.fintaActiva && !esperandoDunkEnAire) {
                activarFinta();
                cdTiro = 0.5f;
            }
            // Cancelar salto sin dunk
            if (esperandoDunkEnAire && act.estado != EstadoJ::EN_AIRE) {
                esperandoDunkEnAire = false;
                lanzarTiroNormal(true);
            }
        }
        // ── DEFENSIVA ─────────────────────────
        else {
            // J → bloqueo (si hay tiro en vuelo) o empuje (si hay rival cerca)
            if (jP && cdRobo <= 0.f) {
                if (pelota.enArco && !pelota.esPase && !pelota.esEquipoH) {
                    intentarBloqueo();
                } else {
                    intentarEmpuje();
                }
            }
            // K → robo
            if (kP && cdRobo <= 0.f) {
                intentarRoboHumano();
                cdRobo = 0.6f;
            }
            // J+K → alley-oop receptor (salta esperando pase del compañero)
            if (jP && kP && cdTiro <= 0.f && !alleyOopActivado) {
                activarAlleyOop();
                cdTiro = 1.f;
            }
            // L → empuje directo
            if (lP && cdEmpuje <= 0.f) {
                intentarEmpuje();
            }
        }
    }

    // ─── IA DEL CPU ───────────────────────────
    void updateIA(float dt) {
        if (estado != EstadoJuego::JUGANDO || pausaPostGol > 0.f) return;

        Jugador* conPelota = eCPU.conPelota();
        sf::Vector2f aroObj{ARO_IZQ_CX, ARO_Y};

        if (conPelota) {
            float distAro = dist2(conPelota->pos, aroObj);

            sf::Vector2f d2 = norm2(aroObj - conPelota->pos);
            if (conPelota->timerAturdido <= 0.f)
                conPelota->mover(d2, dt * 0.88f);
            pelota.pos = conPelota->pos;

            bool enZona3 = distAro < DIST_3P + 35.f && distAro > DIST_3P - 15.f;
            bool enZona2 = distAro < 155.f;

            // ── IA: Super dunk cuando tiene el meter lleno ──
            if (eCPU.superLleno() && distAro < 220.f && prob(0.028f * (dt * 60.f))) {
                // Primero salta
                conPelota->setEstado(EstadoJ::EN_AIRE, 0.55f);
                // Luego ejecuta dunk (en siguiente frame lo toma updateIA)
                lanzarDunk(false);
                return;
            }

            // ── Tiro normal ──
            if ((enZona2 || enZona3) && prob((enZona2 ? 0.030f : 0.022f) * (dt * 60.f))) {
                lanzarTiroNormal(false);
                return;
            }

            // ── Pase a compañero más libre ──
            if (distAro > 260.f && prob(0.012f * (dt * 60.f))) {
                int mejor = -1;
                float mDist = distAro;
                for (int i = 0; i < 3; i++) {
                    if (eCPU.j[i].tienePelota) continue;
                    float dc = dist2(eCPU.j[i].pos, aroObj);
                    if (dc < mDist - 50.f) { mDist = dc; mejor = i; }
                }
                if (mejor >= 0) {
                    conPelota->tienePelota = false;
                    conPelota->setEstado(EstadoJ::PASANDO, 0.35f);
                    float tV = dist2(conPelota->pos, eCPU.j[mejor].pos) / 720.f + 0.08f;
                    pelota.lanzarPase(conPelota->pos, eCPU.j[mejor].pos, tV);
                    pelota.esEquipoH = false;
                    eCPU.receptorPase = mejor;
                    eCPU.sumarSuperPase();
                    return;
                }
            }

            // ── Alley-oop de la IA ──
            if (distAro > 120.f && distAro < 250.f && prob(0.008f * (dt * 60.f))) {
                int recep = eCPU.mejorReceptorAlleyOop(aroObj);
                if (recep >= 0 && dist2(eCPU.j[recep].pos, aroObj) < 130.f) {
                    // Receptor salta
                    eCPU.j[recep].setEstado(EstadoJ::ALLEYOOP_VUELO, 0.75f);
                    eCPU.j[recep].esperandoAlleyOop = true;
                    eCPU.j[recep].timerAlleyOop     = 0.75f;
                    // Pase alley-oop
                    conPelota->tienePelota = false;
                    conPelota->setEstado(EstadoJ::PASANDO, 0.35f);
                    float tV = dist2(conPelota->pos, eCPU.j[recep].pos) / 650.f + 0.12f;
                    pelota.lanzarAlleyOop(conPelota->pos, eCPU.j[recep].pos, tV);
                    pelota.esEquipoH = false;
                    eCPU.receptorAlleyOop = recep;
                    eCPU.sumarSuperAlleyOop();
                    hud.mensaje("Alley-oop rival!", sf::Color(100,180,255), 1.2f);
                    return;
                }
            }

            // Compañeros sin pelota: posicionarse
            for (int i = 0; i < 3; i++) {
                if (eCPU.j[i].tienePelota) continue;
                if (eCPU.j[i].timerAturdido > 0.f) continue;
                sf::Vector2f obj = {aroObj.x + 80.f + i * 55.f,
                                    aroObj.y + (float)((i-1) * 110)};
                obj.x = std::max(C_X + J_RADIO, std::min(obj.x, C_X + C_ANCHO - J_RADIO));
                obj.y = std::max(C_Y + J_RADIO, std::min(obj.y, C_Y + C_ALTO  - J_RADIO));
                sf::Vector2f d3 = norm2(obj - eCPU.j[i].pos);
                if (dist2(eCPU.j[i].pos, obj) > 12.f)
                    eCPU.j[i].mover(d3, dt * 0.65f);
            }
        }
        // ── CPU no tiene pelota: defender ──
        else {
            for (int i = 0; i < 3; i++) {
                Jugador& defensor = eCPU.j[i];
                if (defensor.timerAturdido > 0.f) continue;
                Jugador& objetivo = eH.j[i % 3];

                sf::Vector2f posDefensa = objetivo.pos;
                sf::Vector2f dAro = norm2(aroObj - objetivo.pos);
                posDefensa += dAro * 42.f;

                sf::Vector2f dir = norm2(posDefensa - defensor.pos);
                if (dist2(defensor.pos, posDefensa) > 20.f)
                    defensor.mover(dir, dt * 0.80f);

                // Intento de robo
                if (objetivo.tienePelota) {
                    float d2 = dist2(defensor.pos, objetivo.pos);
                    // Finta hace que la IA se equivoque
                    float multFinta = objetivo.fintaActiva ? 0.3f : 1.f;
                    if (d2 < RADIO_ROBO && prob(defensor.probRobo() * dt * 1.8f * multFinta)) {
                        objetivo.tienePelota  = false;
                        defensor.tienePelota  = true;
                        pelota.pos    = defensor.pos;
                        pelota.enManos = true;
                        pelota.enJuego = false;
                        eCPU.activo   = i;
                        eCPU.sumarSuperRobo();
                        esperandoDunkEnAire = false;
                        alleyOopActivado    = false;
                        hud.mensaje("Robo del rival", sf::Color(100,200,255), 1.2f);
                    }
                }

                // Empuje de la IA (ocasional)
                if (!objetivo.tienePelota && prob(0.004f * (dt * 60.f))) {
                    float d2 = dist2(defensor.pos, objetivo.pos);
                    if (d2 < J_RADIO * 2.5f) {
                        sf::Vector2f dirE = norm2(objetivo.pos - defensor.pos);
                        objetivo.recibirEmpuje(dirE, 140.f);
                        if (objetivo.tienePelota && prob(0.20f)) {
                            objetivo.tienePelota = false;
                            pelota.soltarLibre(objetivo.pos,
                                {dirE.x * 70.f, dirE.y * 50.f});
                        }
                    }
                }
            }

            // ¿CPU puede recoger pelota libre?
            if (pelota.enJuego) {
                for (int i = 0; i < 3; i++) {
                    if (dist2(eCPU.j[i].pos, pelota.pos) < J_RADIO + P_RADIO + 5.f) {
                        eH.quitarPelota();
                        eCPU.darPelota(i);
                        pelota.tomarla();
                        break;
                    }
                }
            }
        }
    }

    // ─── Verificar pelota libre ───────────────
    void verificarPelotaLibre() {
        if (!pelota.enJuego) return;
        Jugador& actH = eH.j[eH.activo];
        if (!actH.tienePelota && dist2(actH.pos, pelota.pos) < J_RADIO + P_RADIO + 8.f) {
            eH.darPelota(eH.activo);
            pelota.tomarla();
        }
    }

    // ─── Verificar llegada de pase (normal) ──
    void verificarPase() {
        if (!pelota.enArco || !pelota.esPase || pelota.esAlleyOop) return;
        if (pelota.progreso() < 0.88f) return;

        if (pelota.esEquipoH && eH.receptorPase >= 0) {
            int r = eH.receptorPase;
            eH.j[eH.activo].esHumano = false;
            eH.darPelota(r);
            eH.j[r].esHumano = true;
            pelota.tomarla();
            pelota.pos = eH.j[r].pos;
            eH.receptorPase = -1;
        }
        else if (!pelota.esEquipoH && eCPU.receptorPase >= 0) {
            int r = eCPU.receptorPase;
            eCPU.darPelota(r);
            pelota.tomarla();
            pelota.pos = eCPU.j[r].pos;
            eCPU.receptorPase = -1;
        }
    }

    // ─── Verificar alley-oop ──────────────────
    void verificarAlleyOop() {
        if (!pelota.enArco || !pelota.esPase || !pelota.esAlleyOop) return;
        if (pelota.progreso() < 0.85f) return;

        if (pelota.esEquipoH && eH.receptorAlleyOop >= 0) {
            int r = eH.receptorAlleyOop;
            Jugador& receptor = eH.j[r];
            // Receptor ejecuta dunk automático
            receptor.esperandoAlleyOop = false;
            receptor.tienePelota = true;
            pelota.tomarla();
            pelota.pos = receptor.pos;
            eH.activo  = r;
            eH.j[r].esHumano = true;
            eH.receptorAlleyOop = -1;
            // Ejecutar dunk (el receptor ya está en el aire)
            lanzarDunk(true);
        }
        else if (!pelota.esEquipoH && eCPU.receptorAlleyOop >= 0) {
            int r = eCPU.receptorAlleyOop;
            Jugador& receptor = eCPU.j[r];
            receptor.esperandoAlleyOop = false;
            receptor.tienePelota = true;
            pelota.tomarla();
            pelota.pos = receptor.pos;
            eCPU.activo = r;
            eCPU.receptorAlleyOop = -1;
            lanzarDunk(false);
        }
    }

    // ─── Verificar llegada de tiro ────────────
    void verificarTiro() {
        if (!pelota.enArco || pelota.esPase) return;
        if (pelota.progreso() < 0.96f) return;

        if (pelota.anotoPendiente) {
            anotar(pelota.esEquipoH, pelota.puntosPendientes, pelota.dest);
            pelota.anotoPendiente = false;
        }
    }

    // ─── Colisiones entre jugadores ───────────
    void resolverColisiones() {
        std::vector<Jugador*> todos;
        for (auto& jj : eH.j)   todos.push_back(&jj);
        for (auto& jj : eCPU.j) todos.push_back(&jj);
        float minSep = J_RADIO * 2.f - 2.f;
        for (int a = 0; a < (int)todos.size(); a++) {
            for (int b2 = a+1; b2 < (int)todos.size(); b2++) {
                sf::Vector2f dv = todos[a]->pos - todos[b2]->pos;
                float dl = std::sqrt(dv.x*dv.x + dv.y*dv.y);
                if (dl < minSep && dl > 0.001f) {
                    sf::Vector2f push = (dv / dl) * (minSep - dl) * 0.5f;
                    todos[a]->pos += push;
                    todos[b2]->pos -= push;
                    todos[a]->clampCancha();
                    todos[b2]->clampCancha();
                }
            }
        }
    }

    // ─── UPDATE PRINCIPAL ─────────────────────
    void update(float dt) {
        hud.actualizar(dt);
        eH.actualizar(dt);
        eCPU.actualizar(dt);

        if (estado == EstadoJuego::SELECCION) return;
        if (estado == EstadoJuego::MEDIO_TIEMPO) return;
        if (estado == EstadoJuego::FIN) return;

        if (pausaPostGol > 0.f) {
            pausaPostGol -= dt;
            return;
        }

        // Cronómetro
        tiempo -= dt;
        if (tiempo <= 0.f) {
            if (mitad == 1) {
                mitad = 2;
                tiempo = TIEMPO_MITAD;
                estado = EstadoJuego::MEDIO_TIEMPO;
                hud.mensaje("MEDIO TIEMPO", sf::Color(255,220,50), 3.f);
                reiniciarPosesion(false);
            } else {
                estado = EstadoJuego::FIN;
                std::string res = eH.puntos > eCPU.puntos ? "!GANASTE!" :
                                  eH.puntos < eCPU.puntos ? "PERDISTE"  : "EMPATE";
                hud.mensaje(res, sf::Color(255,220,50), 999.f);
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
        for (auto& jj : eH.j)   if (jj.tienePelota && !pelota.enArco) pelota.pos = jj.pos;
        for (auto& jj : eCPU.j) if (jj.tienePelota && !pelota.enArco) pelota.pos = jj.pos;
    }

    // ─── DIBUJAR ──────────────────────────────
    void dibujar() {
        ventana.clear();
        ventana.draw(fondoRect);

        if (estado == EstadoJuego::SELECCION) {
            dibujarSeleccion();
            ventana.display();
            return;
        }

        cancha.dibujar(ventana);

        // Ordenar todos los jugadores por Y (profundidad)
        std::vector<Jugador*> todos;
        for (auto& jj : eH.j)   todos.push_back(&jj);
        for (auto& jj : eCPU.j) todos.push_back(&jj);
        std::sort(todos.begin(), todos.end(), [](Jugador* a, Jugador* b){
            return a->pos.y < b->pos.y;
        });

        // Sombras primero
        for (auto* jj : todos) ventana.draw(jj->sombra);

        // Pelota en suelo
        if (pelota.enJuego) pelota.dibujar(ventana);

        // Jugadores
        for (auto* jj : todos) jj->dibujar(ventana);

        // Pelota en arco o en manos (encima)
        if (!pelota.enJuego) pelota.dibujar(ventana);

        // Indicador de alley-oop activo (flecha)
        if (alleyOopActivado && alleyOopReceptor >= 0) {
            Jugador& rec = eH.j[alleyOopReceptor];
            sf::Text t;
            if (hud.fuenteOk) t.setFont(hud.fuente);
            t.setString("ALLEY!");
            t.setCharacterSize(16);
            t.setFillColor(sf::Color(255,220,0,200));
            t.setOutlineThickness(1.f);
            t.setOutlineColor(sf::Color::Black);
            sf::FloatRect b = t.getLocalBounds();
            t.setOrigin(b.width/2.f, b.height/2.f);
            t.setPosition(rec.pos.x, rec.pos.y - 75.f - rec.alturaSalto);
            ventana.draw(t);
        }

        // HUD
        hud.dibujar(ventana, eH.puntos, eCPU.puntos, tiempo, mitad,
                    eH.superMeter, eCPU.superMeter, eH.nombre, eCPU.nombre);

        if (estado == EstadoJuego::MEDIO_TIEMPO) {
            overlayTexto("MEDIO TIEMPO", "Pulsa ENTER para la 2a mitad");
        }
        if (estado == EstadoJuego::FIN) {
            std::string res = eH.puntos > eCPU.puntos ? "!GANASTE!" :
                              eH.puntos < eCPU.puntos ? "PERDISTE"   : "EMPATE";
            std::ostringstream ss;
            ss << eH.puntos << " - " << eCPU.puntos
               << "  |  R = Revanche  |  ESC = Salir";
            overlayTexto(res, ss.str());

            // Estadísticas del partido
            dibujarEstadisticas();
        }

        ventana.display();
    }

    // ─── Estadísticas al final ────────────────
    void dibujarEstadisticas() {
        auto txtC = [&](const std::string& s, int sz, sf::Color c, float x, float y) {
            sf::Text t;
            if (hud.fuenteOk) t.setFont(hud.fuente);
            t.setString(s); t.setCharacterSize(sz); t.setFillColor(c);
            t.setOutlineThickness(1.f); t.setOutlineColor(sf::Color::Black);
            sf::FloatRect b = t.getLocalBounds();
            t.setOrigin(b.width/2.f, 0.f);
            t.setPosition(x, y); ventana.draw(t);
        };

        float y0 = W_ALTO/2.f + 90.f;
        txtC("Robos: " + std::to_string(eH.robos) +
             "  Bloqueos: " + std::to_string(eH.bloqueos) +
             "  Asistencias: " + std::to_string(eH.asistencias) +
             "  Alley-oops: " + std::to_string(eH.alleyOops),
             14, sf::Color(255,190,80), W_ANCHO/2.f, y0);
    }

    // ─── Pantalla de selección ────────────────
    void dibujarSeleccion() {
        sf::Font& f = hud.fuente;
        bool fok    = hud.fuenteOk;

        auto txt = [&](const std::string& s, int sz, sf::Color c,
                        float x, float y, bool c2 = true) {
            sf::Text t;
            if (fok) t.setFont(f);
            t.setString(s); t.setCharacterSize(sz); t.setFillColor(c);
            t.setOutlineThickness(2.f); t.setOutlineColor(sf::Color::Black);
            sf::FloatRect b = t.getLocalBounds();
            if (c2) t.setOrigin(b.width/2.f, b.height/2.f);
            t.setPosition(x, y); ventana.draw(t);
        };

        txt("CARTOON DUNK", 52, sf::Color(255,200,0), W_ANCHO/2.f, 80.f);
        txt("Street Basketball 3v3  -  Street Hoop Style", 18,
            sf::Color(200,200,200), W_ANCHO/2.f, 140.f);
        txt("Elige tu equipo:", 26, sf::Color::White, W_ANCHO/2.f, 190.f);

        float px = 230.f, py = 255.f;
        sf::RectangleShape box1({280.f, 200.f});
        box1.setPosition(px - 140.f, py - 20.f);
        box1.setFillColor(equipoSeleccionado == 0
            ? sf::Color(255,140,0,180) : sf::Color(50,30,10,120));
        box1.setOutlineThickness(3.f);
        box1.setOutlineColor(sf::Color(255,180,50));
        ventana.draw(box1);

        txt("CARTOON DUNK", 20, sf::Color(255,220,80), px, py);
        txt("Goku / Pocoyo / Bugs", 14, sf::Color(220,220,200), px, py + 33.f);
        txt("Dunk:  ████████░", 14, sf::Color(255,180,0), px, py + 60.f);
        txt("3pts:  ██████░░░", 14, sf::Color(255,180,0), px, py + 80.f);
        txt("Vel:   ███████░░", 14, sf::Color(255,180,0), px, py + 100.f);
        txt("Def:   █████░░░░", 14, sf::Color(255,180,0), px, py + 120.f);

        float px2 = W_ANCHO - 230.f;
        sf::RectangleShape box2({280.f, 200.f});
        box2.setPosition(px2 - 140.f, py - 20.f);
        box2.setFillColor(equipoSeleccionado == 1
            ? sf::Color(60,120,255,180) : sf::Color(20,30,60,120));
        box2.setOutlineThickness(3.f);
        box2.setOutlineColor(sf::Color(80,180,255));
        ventana.draw(box2);

        txt("RIVALES", 20, sf::Color(100,200,255), px2, py);
        txt("Mistico x3", 14, sf::Color(200,220,255), px2, py + 33.f);
        txt("Dunk:  ██████░░░", 14, sf::Color(100,180,255), px2, py + 60.f);
        txt("3pts:  ████████░", 14, sf::Color(100,180,255), px2, py + 80.f);
        txt("Vel:   ██████░░░", 14, sf::Color(100,180,255), px2, py + 100.f);
        txt("Def:   ████████░", 14, sf::Color(100,180,255), px2, py + 120.f);

        std::string flecha = equipoSeleccionado == 0 ? "◄" : "►";
        txt(flecha, 32, sf::Color::Yellow, W_ANCHO/2.f, py + 60.f);

        txt("← → para elegir   ENTER para jugar", 18,
            sf::Color(180,180,180), W_ANCHO/2.f, 490.f);

        // Controles extendidos
        txt("WASD:Mover  LShift:Sprint  J:Tiro/Dunk-Aire  K:Pase  L:Robo/Finta",
            12, sf::Color(130,130,130), W_ANCHO/2.f, 522.f);
        txt("J+K(con balón):Saltar para Dunk  |  J+K(sin balón):Alley-oop receptor",
            12, sf::Color(130,130,130), W_ANCHO/2.f, 540.f);
        txt("J(def,tiro en vuelo):Bloquear  |  L(def):Empujar  |  Tab:Cambiar jugador",
            12, sf::Color(130,130,130), W_ANCHO/2.f, 558.f);
    }

    // ─── Overlay de estado ────────────────────
    void overlayTexto(const std::string& titulo, const std::string& sub) {
        sf::RectangleShape ov({(float)W_ANCHO, (float)W_ALTO});
        ov.setFillColor(sf::Color(0,0,0,150));
        ventana.draw(ov);

        sf::Text t, t2;
        if (hud.fuenteOk) { t.setFont(hud.fuente); t2.setFont(hud.fuente); }
        t.setString(titulo);  t.setCharacterSize(50);
        t.setFillColor(sf::Color(255,220,50));
        t.setOutlineThickness(3.f); t.setOutlineColor(sf::Color::Black);
        sf::FloatRect b = t.getLocalBounds();
        t.setOrigin(b.width/2.f, b.height/2.f);
        t.setPosition(W_ANCHO/2.f, W_ALTO/2.f - 30.f);
        ventana.draw(t);

        t2.setString(sub); t2.setCharacterSize(20);
        t2.setFillColor(sf::Color::White);
        t2.setOutlineThickness(2.f); t2.setOutlineColor(sf::Color::Black);
        sf::FloatRect b2 = t2.getLocalBounds();
        t2.setOrigin(b2.width/2.f, 0.f);
        t2.setPosition(W_ANCHO/2.f, W_ALTO/2.f + 30.f);
        ventana.draw(t2);
    }

    // ─── EVENTOS ──────────────────────────────
    void evento(sf::Event& e) {
        if (e.type == sf::Event::Closed) ventana.close();
        if (e.type != sf::Event::KeyPressed) return;

        if (estado == EstadoJuego::SELECCION) {
            if (e.key.code == sf::Keyboard::Left  || e.key.code == sf::Keyboard::A)
                equipoSeleccionado = 0;
            if (e.key.code == sf::Keyboard::Right || e.key.code == sf::Keyboard::D)
                equipoSeleccionado = 1;
            if (e.key.code == sf::Keyboard::Return) iniciarPartida();
            if (e.key.code == sf::Keyboard::Escape) ventana.close();
            return;
        }

        if (estado == EstadoJuego::MEDIO_TIEMPO && e.key.code == sf::Keyboard::Return) {
            estado = EstadoJuego::JUGANDO;
            hud.mensaje("", sf::Color::White, 0.f);
            return;
        }

        if (estado == EstadoJuego::FIN) {
            if (e.key.code == sf::Keyboard::R) {
                estado = EstadoJuego::SELECCION;
                hud.mensaje("", sf::Color::White, 0.f);
            }
            if (e.key.code == sf::Keyboard::Escape) ventana.close();
            return;
        }

        if (estado == EstadoJuego::JUGANDO) {
            if (e.key.code == sf::Keyboard::Tab) {
                int sig = (eH.activo + 1) % 3;
                if (!eH.j[sig].tienePelota) {
                    eH.j[eH.activo].esHumano = false;
                    eH.activo = sig;
                    eH.j[eH.activo].esHumano = true;
                }
            }
            if (e.key.code == sf::Keyboard::Escape) ventana.close();
        }
    }

    // ─── LOOP PRINCIPAL ───────────────────────
    void run() {
        sf::Clock reloj;
        while (ventana.isOpen()) {
            float dt = reloj.restart().asSeconds();
            dt = std::min(dt, 0.05f);

            sf::Event e;
            while (ventana.pollEvent(e)) evento(e);

            update(dt);
            dibujar();
        }
    }
};

// ────────────────────────────────────────────
int main() {
    GameManager gm;
    gm.run();
    return 0;
}
