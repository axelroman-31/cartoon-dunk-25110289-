// ================================================================
//  CARTOON DUNK
//  Baloncesto 3v3 estilo arcade - inspirado en Street Hoop (1994)
//  Motor: SFML 2.6
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
static float dist(sf::Vector2f a, sf::Vector2f b) {
    float dx = a.x - b.x, dy = a.y - b.y;
    return std::sqrt(dx*dx + dy*dy);
}
static sf::Vector2f norm(sf::Vector2f v) {
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
    Equipo   eH, eCPU;   // humano, CPU
    Pelota   pelota;
    Cancha   cancha;
    HUD      hud;
    sf::Music musica;

    EstadoJuego estado = EstadoJuego::SELECCION;
    float tiempo  = TIEMPO_MITAD;
    int   mitad   = 1;

    // Fondo
    sf::RectangleShape fondoRect;

    // ── Mecánica de tiro A+B → dunk en aire ──
    bool aTeclada = false;   // se mantiene en juego
    bool bTeclada = false;
    bool abPresionados = false;   // combo A+B activo
    float timerSalto   = 0.f;    // duración del estado EN_AIRE
    bool  esperandoDunkEnAire = false;

    // ── Cooldowns para evitar spam ──
    float cdTiro  = 0.f;   // cooldown después de disparar
    float cdPase  = 0.f;
    float cdRobo  = 0.f;

    // ── Contador de festejo en enceste ──
    float pausaPostGol = 0.f;

    // ── Pantalla de selección ──
    int equipoSeleccionado = 0;  // 0 = Cartoon, 1 = Rivales

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
        cdTiro  = cdPase = cdRobo = 0.f;
        pausaPostGol = 0.f;
        esperandoDunkEnAire = false;

        eH.configurar(equipoCartoon(), true);
        eCPU.configurar(equipoRival(), false);

        eH.puntos = eCPU.puntos = 0;
        eH.superMeter = eCPU.superMeter = 0.f;

        // Posiciones iniciales
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
        }
        eH.activo = 0;
        eCPU.activo = 0;

        // El humano empieza con la pelota
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
        pelota.enArco = false;
        pelota.enJuego = false;
        pelota.enManos = true;
        pelota.esSuper = false;

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

    // ─── Evaluar enceste ──────────────────────
    // Llamado cuando la pelota llega al destino del arco
    // Retorna puntos anotados (0 = fallo)
    int evalEnceste(bool esEquipoH, float distAlAro) {
        // Al momento del tiro el jugador ya no tiene la pelota;
        // usamos el activo que tiró
        Jugador& j = esEquipoH ? eH.j[eH.activo] : eCPU.j[eCPU.activo];
        bool esTres = distAlAro > DIST_3P;
        float p = j.probTiro(distAlAro, esTres);
        if (pelota.esSuper) p = std::min(p + 0.35f, 0.95f); // super boost
        if (!prob(p)) return 0;
        return esTres ? 3 : 2;
    }

    // ─── Anotar ───────────────────────────────
    void anotar(bool equipoH, int pts, sf::Vector2f posAro) {
        std::string msg = pelota.esSuper ? "!SUPER DUNK!" :
                          (pts == 3) ? "TRIPLE!" : "CANASTA!";
        sf::Color c = equipoH ? sf::Color(255,200,0) : sf::Color(100,200,255);

        if (equipoH) {
            eH.sumarPuntos(pts);
            hud.flash(sf::Color(255,200,0));
        } else {
            eCPU.sumarPuntos(pts);
            hud.flash(sf::Color(80,120,255));
        }
        hud.mensaje(msg, c, 2.5f);
        pausaPostGol = 1.2f;

        // El equipo que RECIBIÓ el gol coge la pelota desde el centro
        reiniciarPosesion(!equipoH);
    }

    // ─── INPUT HUMANO ─────────────────────────
    void inputHumano(float dt) {
        if (estado != EstadoJuego::JUGANDO || pausaPostGol > 0.f) return;

        Jugador& act = eH.porActivo();
        bool tieneB = act.tienePelota;

        // Cooldowns
        cdTiro = std::max(0.f, cdTiro - dt);
        cdPase = std::max(0.f, cdPase - dt);
        cdRobo = std::max(0.f, cdRobo - dt);

        // ── Movimiento WASD ──
        sf::Vector2f dir{};
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) dir.x -= 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) dir.x += 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) dir.y -= 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) dir.y += 1.f;
        bool sprint = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift);
        act.mover(dir, dt, sprint);

        // Sincronizar pelota con portador
        if (tieneB && !pelota.enArco)
            pelota.pos = act.pos;

        // ── Auto-switch: cuando no tenemos pelota, seguir a la pelota ──
        if (!tieneB && !pelota.enArco && pelota.enJuego) {
            // Cambiar automáticamente al más cercano
            int mejorI = eH.activo;
            float minD = dist(eH.j[eH.activo].pos, pelota.pos);
            for (int i = 0; i < 3; i++) {
                if (i == eH.activo) continue;
                float d2 = dist(eH.j[i].pos, pelota.pos);
                if (d2 < minD - 30.f) { minD = d2; mejorI = i; }
            }
            if (mejorI != eH.activo) {
                eH.j[eH.activo].esHumano = false;
                eH.activo = mejorI;
                eH.j[eH.activo].esHumano = true;
            }
        }

        bool jPresionada = sf::Keyboard::isKeyPressed(sf::Keyboard::J);
        bool kPresionada = sf::Keyboard::isKeyPressed(sf::Keyboard::K);

        // ── OFENSIVA ──────────────────────────
        if (tieneB) {
            // COMBO J+K → Saltar para dunk
            if (jPresionada && kPresionada && cdTiro <= 0.f && !esperandoDunkEnAire) {
                act.setEstado(EstadoJ::EN_AIRE, 0.55f);
                esperandoDunkEnAire = true;
                cdTiro = 0.3f;
            }
            // EN AIRE → presionar J para dunk
            else if (esperandoDunkEnAire && act.estado == EstadoJ::EN_AIRE && jPresionada && cdTiro <= 0.f) {
                lanzarDunk(true);
            }
            // J solo → Tiro normal
            else if (jPresionada && !kPresionada && cdTiro <= 0.f && !esperandoDunkEnAire) {
                lanzarTiroNormal(true);
            }
            // K solo → Pase
            else if (kPresionada && !jPresionada && cdPase <= 0.f) {
                lanzarPase();
            }

            // Cancelar salto si pasa el timer sin hacer dunk
            if (esperandoDunkEnAire && act.estado != EstadoJ::EN_AIRE) {
                esperandoDunkEnAire = false;
                // Tiro normal de rescate
                lanzarTiroNormal(true);
            }
        }
        // ── DEFENSIVA ─────────────────────────
        else {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::L) && cdRobo <= 0.f) {
                intentarRoboHumano();
                cdRobo = 0.6f;
            }
            // Bloqueo (J en defensa)
            if (jPresionada && cdRobo <= 0.f) {
                act.setEstado(EstadoJ::BLOQUEANDO, 0.3f);
                cdRobo = 0.4f;
            }
        }
    }

    // ─── Lanzar tiro normal ───────────────────
    void lanzarTiroNormal(bool esH) {
        Jugador& act = esH ? eH.j[eH.activo] : eCPU.j[eCPU.activo];
        Equipo&  eq  = esH ? eH : eCPU;
        if (!act.tienePelota) return;

        sf::Vector2f aroPos = esH
            ? sf::Vector2f{ARO_DER_CX, ARO_Y}
            : sf::Vector2f{ARO_IZQ_CX, ARO_Y};

        float d    = dist(act.pos, aroPos);
        bool super = eq.superLleno();
        if (super) eq.gastarSuper();

        act.tienePelota = false;
        act.setEstado(super ? EstadoJ::SUPER_SHOT : EstadoJ::LANZANDO, 0.6f);

        float tVuelo = 0.45f + d / 1300.f;
        pelota.lanzarTiro(act.pos, aroPos, tVuelo, super);
        pelota.esEquipoH = esH;

        // Calcular si entra (se decide AHORA con dist al lanzar)
        int pts = evalEnceste(esH, d);
        pelota.anotoPendiente  = (pts > 0);
        pelota.puntosPendientes = pts;

        cdTiro = 0.8f;
        esperandoDunkEnAire = false;
        if (esH) cdTiro = 0.8f;
    }

    // ─── Lanzar dunk (A+B combo) ──────────────
    void lanzarDunk(bool esH) {
        Jugador& act = esH ? eH.j[eH.activo] : eCPU.j[eCPU.activo];
        Equipo&  eq  = esH ? eH : eCPU;
        if (!act.tienePelota) return;

        sf::Vector2f aroPos = esH
            ? sf::Vector2f{ARO_DER_CX, ARO_Y}
            : sf::Vector2f{ARO_IZQ_CX, ARO_Y};

        float d    = dist(act.pos, aroPos);
        bool super = eq.superLleno();
        if (super) eq.gastarSuper();

        act.tienePelota = false;
        act.setEstado(EstadoJ::DUNKEANDO, 0.55f);

        // Dunk: arco muy alto, llega al aro
        float tVuelo = 0.5f + d / 1100.f;
        pelota.lanzarTiro(act.pos, aroPos, tVuelo, super);
        pelota.esEquipoH = esH;
        // Dunk tiene bonus de probabilidad
        int pts = evalEnceste(esH, d);
        pelota.anotoPendiente  = (pts > 0);
        pelota.puntosPendientes = pts;

        cdTiro = 0.9f;
        esperandoDunkEnAire = false;
    }

    // ─── Pase entre compañeros ─────────────────
    void lanzarPase() {
        Jugador& act = eH.j[eH.activo];
        if (!act.tienePelota) return;

        // Elegir compañero más libre (más cerca del aro)
        sf::Vector2f aro{ARO_DER_CX, ARO_Y};
        int receptor = -1;
        float minD = 1e9f;
        for (int i = 0; i < 3; i++) {
            if (i == eH.activo) continue;
            float d2 = dist(eH.j[i].pos, aro);
            if (d2 < minD) { minD = d2; receptor = i; }
        }
        if (receptor < 0) return;

        act.tienePelota = false;
        act.setEstado(EstadoJ::PASANDO, 0.35f);

        float tVuelo = dist(act.pos, eH.j[receptor].pos) / 720.f + 0.08f;
        pelota.lanzarPase(act.pos, eH.j[receptor].pos, tVuelo);
        pelota.esEquipoH = true;
        eH.receptorPase = receptor;
        eH.sumarSuperPase();
        cdPase = 0.6f;
    }

    // ─── Robo humano ──────────────────────────
    void intentarRoboHumano() {
        Jugador& act = eH.j[eH.activo];
        act.setEstado(EstadoJ::ROBANDO, 0.35f);

        for (auto& jcpu : eCPU.j) {
            if (!jcpu.tienePelota) continue;
            float d2 = dist(act.pos, jcpu.pos);
            if (d2 < RADIO_ROBO + 8.f) {
                float p = act.probRobo() * 0.55f;
                if (prob(p)) {
                    jcpu.tienePelota = false;
                    act.tienePelota  = true;
                    pelota.pos  = act.pos;
                    pelota.enManos = true;
                    pelota.enJuego = false;
                    eH.sumarSuperRobo();
                    hud.mensaje("¡ROBO!", sf::Color(255,255,100), 1.5f);
                }
                break;
            }
        }
    }

    // ─── IA DEL CPU ───────────────────────────
    void updateIA(float dt) {
        if (estado != EstadoJuego::JUGANDO || pausaPostGol > 0.f) return;

        Jugador* conPelota = eCPU.conPelota();
        sf::Vector2f aroObj{ARO_IZQ_CX, ARO_Y};

        // ── CPU tiene pelota ──
        if (conPelota) {
            float distAro = dist(conPelota->pos, aroObj);

            // Moverse hacia el aro
            sf::Vector2f d2 = norm(aroObj - conPelota->pos);
            conPelota->mover(d2, dt * 0.88f);
            pelota.pos = conPelota->pos;

            // ¿Tira?
            bool enZona3 = distAro < DIST_3P + 35.f && distAro > DIST_3P - 15.f;
            bool enZona2 = distAro < 155.f;

            if ((enZona2 || enZona3) && prob((enZona2 ? 0.030f : 0.022f) * (dt * 60.f))) {
                lanzarTiroNormal(false);
                return;
            }

            // ¿Super shot disponible? Activa dunk
            if (eCPU.superLleno() && distAro < 200.f && prob(0.025f * (dt * 60.f))) {
                lanzarTiroNormal(false);
                return;
            }

            // ¿Pasa a compañero más libre?
            if (distAro > 260.f && prob(0.012f * (dt * 60.f))) {
                int mejor = -1;
                float mDist = distAro;
                for (int i = 0; i < 3; i++) {
                    if (&eCPU.j[i] == conPelota) continue;
                    float dc = dist(eCPU.j[i].pos, aroObj);
                    if (dc < mDist - 50.f) { mDist = dc; mejor = i; }
                }
                if (mejor >= 0) {
                    conPelota->tienePelota = false;
                    conPelota->setEstado(EstadoJ::PASANDO, 0.35f);
                    float tV = dist(conPelota->pos, eCPU.j[mejor].pos) / 720.f + 0.08f;
                    pelota.lanzarPase(conPelota->pos, eCPU.j[mejor].pos, tV);
                    pelota.esEquipoH = false;
                    eCPU.receptorPase = mejor;
                    eCPU.sumarSuperPase();
                    return;
                }
            }

            // Compañeros sin pelota: posicionarse
            for (int i = 0; i < 3; i++) {
                if (eCPU.j[i].tienePelota) continue;
                sf::Vector2f obj = {aroObj.x + 80.f + i * 55.f,
                                    aroObj.y + (float)((i-1) * 110)};
                obj.x = std::max(C_X + J_RADIO, std::min(obj.x, C_X + C_ANCHO - J_RADIO));
                obj.y = std::max(C_Y + J_RADIO, std::min(obj.y, C_Y + C_ALTO  - J_RADIO));
                sf::Vector2f d3 = norm(obj - eCPU.j[i].pos);
                if (dist(eCPU.j[i].pos, obj) > 12.f)
                    eCPU.j[i].mover(d3, dt * 0.65f);
            }
        }
        // ── CPU no tiene pelota: defender ──
        else {
            for (int i = 0; i < 3; i++) {
                Jugador& defensor = eCPU.j[i];
                Jugador& objetivo = eH.j[i % 3];

                // Posición defensiva: entre el jugador rival y el aro propio
                sf::Vector2f posDefensa = objetivo.pos;
                sf::Vector2f dAro = norm(aroObj - objetivo.pos);
                posDefensa += dAro * 45.f;  // 45px por delante hacia el aro

                sf::Vector2f dir = norm(posDefensa - defensor.pos);
                if (dist(defensor.pos, posDefensa) > 20.f)
                    defensor.mover(dir, dt * 0.80f);

                // Intento de robo
                if (objetivo.tienePelota) {
                    float d2 = dist(defensor.pos, objetivo.pos);
                    if (d2 < RADIO_ROBO && prob(defensor.probRobo() * dt * 1.8f)) {
                        objetivo.tienePelota  = false;
                        defensor.tienePelota  = true;
                        pelota.pos    = defensor.pos;
                        pelota.enManos = true;
                        pelota.enJuego = false;
                        eCPU.activo   = i;
                        eCPU.sumarSuperRobo();
                        esperandoDunkEnAire = false;
                        hud.mensaje("ROBO del RIVAL", sf::Color(100,200,255), 1.5f);
                    }
                }
                // Intento de bloquear tiro en arco
                if (pelota.enArco && !pelota.esPase && !pelota.esEquipoH) {
                    // El equipo humano está atacando → CPU defiende = esto no aplica aquí
                }
            }

            // ¿CPU puede recoger pelota libre?
            if (pelota.enJuego) {
                for (int i = 0; i < 3; i++) {
                    if (dist(eCPU.j[i].pos, pelota.pos) < J_RADIO + P_RADIO + 5.f) {
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
        // Humano recoge
        Jugador& actH = eH.j[eH.activo];
        if (!actH.tienePelota && dist(actH.pos, pelota.pos) < J_RADIO + P_RADIO + 8.f) {
            eH.darPelota(eH.activo);
            pelota.tomarla();
            return;
        }
        // CPU recoge (ya se hace en updateIA)
    }

    // ─── Verificar llegada de pase ────────────
    void verificarPase() {
        if (!pelota.enArco || !pelota.esPase) return;
        if (pelota.progreso() < 0.88f) return;

        // Pase del humano
        if (pelota.esEquipoH && eH.receptorPase >= 0) {
            int r = eH.receptorPase;
            eH.j[eH.activo].esHumano = false;
            eH.darPelota(r);
            eH.j[r].esHumano = true;
            pelota.tomarla();
            pelota.pos = eH.j[r].pos;
            eH.receptorPase = -1;
        }
        // Pase de la CPU
        else if (!pelota.esEquipoH && eCPU.receptorPase >= 0) {
            int r = eCPU.receptorPase;
            eCPU.darPelota(r);
            pelota.tomarla();
            pelota.pos = eCPU.j[r].pos;
            eCPU.receptorPase = -1;
        }
    }

    // ─── Verificar llegada de tiro ────────────
    void verificarTiro() {
        if (!pelota.enArco || pelota.esPase) return;
        if (pelota.progreso() < 0.96f) return;

        // La pelota llegó al destino
        if (pelota.anotoPendiente) {
            anotar(pelota.esEquipoH, pelota.puntosPendientes, pelota.dest);
            pelota.anotoPendiente = false;
        }
        // Si no anotó: Pelota.actualizar() la soltará como libre automáticamente
    }

    // ─── UPDATE PRINCIPAL ─────────────────────
    void update(float dt) {
        hud.actualizar(dt);
        eH.actualizar(dt);
        eCPU.actualizar(dt);

        if (estado == EstadoJuego::SELECCION) return;
        if (estado == EstadoJuego::MEDIO_TIEMPO) return;
        if (estado == EstadoJuego::FIN) return;

        // Pausa post gol
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
                // En la 2ª mitad la CPU empieza con pelota
                reiniciarPosesion(false);
            } else {
                estado = EstadoJuego::FIN;
                std::string res = eH.puntos > eCPU.puntos ? "¡GANASTE!" :
                                  eH.puntos < eCPU.puntos ? "PERDISTE"  : "EMPATE";
                hud.mensaje(res, sf::Color(255,220,50), 999.f);
            }
        }

        // Pelota
        pelota.actualizar(dt);

        // Verificaciones
        verificarTiro();
        verificarPase();
        verificarPelotaLibre();

        // Input humano
        inputHumano(dt);

        // IA
        updateIA(dt);

        // Colisiones entre jugadores (empujones estilo Street Hoop)
        resolverColisiones();

        // Sincronizar pelota con portador
        for (auto& jj : eH.j)   if (jj.tienePelota && !pelota.enArco) pelota.pos = jj.pos;
        for (auto& jj : eCPU.j) if (jj.tienePelota && !pelota.enArco) pelota.pos = jj.pos;
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

        // Ordenar todos por Y (simulación de profundidad)
        std::vector<Jugador*> todos;
        for (auto& jj : eH.j)   todos.push_back(&jj);
        for (auto& jj : eCPU.j) todos.push_back(&jj);
        std::sort(todos.begin(), todos.end(), [](Jugador* a, Jugador* b){
            return a->pos.y < b->pos.y;
        });

        // Dibujar sombras primero
        for (auto* jj : todos) {
            sf::CircleShape s = jj->sombra;
            ventana.draw(s);
        }

        // Dibujar pelota si está en el suelo (debajo de los jugadores)
        if (pelota.enJuego) pelota.dibujar(ventana);

        // Dibujar jugadores
        for (auto* jj : todos) jj->dibujar(ventana);

        // Pelota si está en arco o en manos (encima de jugadores)
        if (!pelota.enJuego) pelota.dibujar(ventana);

        // HUD
        hud.dibujar(ventana, eH.puntos, eCPU.puntos, tiempo, mitad,
                    eH.superMeter, eCPU.superMeter, eH.nombre, eCPU.nombre);

        // Overlay medio tiempo
        if (estado == EstadoJuego::MEDIO_TIEMPO) {
            overlayTexto("MEDIO TIEMPO", "Pulsa ENTER para la 2ª mitad");
        }
        // Overlay fin
        if (estado == EstadoJuego::FIN) {
            std::string res = eH.puntos > eCPU.puntos ? "¡GANASTE!" :
                              eH.puntos < eCPU.puntos ? "PERDISTE"  : "EMPATE";
            std::ostringstream ss;
            ss << eH.puntos << " - " << eCPU.puntos << "  |  R = Revanche  |  ESC = Salir";
            overlayTexto(res, ss.str());
        }

        ventana.display();
    }

    // ─── Pantalla de selección ────────────────
    void dibujarSeleccion() {
        sf::Font& f = hud.fuente;
        bool fok    = hud.fuenteOk;

        auto txt = [&](const std::string& s, int sz, sf::Color c, float x, float y, bool c2 = true) {
            sf::Text t;
            if (fok) t.setFont(f);
            t.setString(s); t.setCharacterSize(sz); t.setFillColor(c);
            t.setOutlineThickness(2.f); t.setOutlineColor(sf::Color::Black);
            sf::FloatRect b = t.getLocalBounds();
            if (c2) t.setOrigin(b.width/2.f, b.height/2.f);
            t.setPosition(x, y); ventana.draw(t);
        };

        txt("CARTOON DUNK", 52, sf::Color(255,200,0), W_ANCHO/2.f, 80.f);
        txt("Street Basketball 3v3", 22, sf::Color(200,200,200), W_ANCHO/2.f, 140.f);
        txt("Elige tu equipo:", 26, sf::Color::White, W_ANCHO/2.f, 200.f);

        // Panel equipo 1
        float px = 230.f, py = 260.f;
        sf::RectangleShape box1({280.f, 180.f});
        box1.setPosition(px - 140.f, py - 20.f);
        box1.setFillColor(equipoSeleccionado == 0
            ? sf::Color(255,140,0,180) : sf::Color(50,30,10,120));
        box1.setOutlineThickness(3.f);
        box1.setOutlineColor(sf::Color(255,180,50));
        ventana.draw(box1);

        txt("CARTOON DUNK", 20, sf::Color(255,220,80), px, py);
        txt("Goku / Pocoyo / Bugs", 14, sf::Color(220,220,200), px, py + 35.f);
        txt("Dunk:  ████████░", 14, sf::Color(255,180,0), px, py + 65.f);
        txt("3pts:  ██████░░░", 14, sf::Color(255,180,0), px, py + 85.f);
        txt("Vel:   ███████░░", 14, sf::Color(255,180,0), px, py + 105.f);
        txt("Def:   █████░░░░", 14, sf::Color(255,180,0), px, py + 125.f);

        // Panel equipo 2
        float px2 = W_ANCHO - 230.f;
        sf::RectangleShape box2({280.f, 180.f});
        box2.setPosition(px2 - 140.f, py - 20.f);
        box2.setFillColor(equipoSeleccionado == 1
            ? sf::Color(60,120,255,180) : sf::Color(20,30,60,120));
        box2.setOutlineThickness(3.f);
        box2.setOutlineColor(sf::Color(80,180,255));
        ventana.draw(box2);

        txt("RIVALES", 20, sf::Color(100,200,255), px2, py);
        txt("Místico x3", 14, sf::Color(200,220,255), px2, py + 35.f);
        txt("Dunk:  ██████░░░", 14, sf::Color(100,180,255), px2, py + 65.f);
        txt("3pts:  ████████░", 14, sf::Color(100,180,255), px2, py + 85.f);
        txt("Vel:   ██████░░░", 14, sf::Color(100,180,255), px2, py + 105.f);
        txt("Def:   ████████░", 14, sf::Color(100,180,255), px2, py + 125.f);

        // Flecha de selección
        std::string flecha = equipoSeleccionado == 0 ? "◄" : "►";
        txt(flecha, 32, sf::Color::Yellow, W_ANCHO/2.f, py + 70.f);

        txt("← → para elegir   ENTER para jugar", 18, sf::Color(180,180,180), W_ANCHO/2.f, 490.f);
        txt("Controles:  WASD=Mover  J=Tiro  K=Pase  L=Robo  J+K=SUPER DUNK  Tab=Cambiar", 13,
            sf::Color(130,130,130), W_ANCHO/2.f, 540.f);
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

        // Selección de equipo
        if (estado == EstadoJuego::SELECCION) {
            if (e.key.code == sf::Keyboard::Left  || e.key.code == sf::Keyboard::A)
                equipoSeleccionado = 0;
            if (e.key.code == sf::Keyboard::Right || e.key.code == sf::Keyboard::D)
                equipoSeleccionado = 1;
            if (e.key.code == sf::Keyboard::Return) {
                // Aquí podrías cargar diferentes equipos según selección;
                // por ahora siempre juega con Cartoon Dunk
                iniciarPartida();
            }
            if (e.key.code == sf::Keyboard::Escape) ventana.close();
            return;
        }

        // Medio tiempo → Enter continúa
        if (estado == EstadoJuego::MEDIO_TIEMPO && e.key.code == sf::Keyboard::Return) {
            estado = EstadoJuego::JUGANDO;
            hud.mensaje("", sf::Color::White, 0.f);
            return;
        }

        // Fin → R reinicia, Esc sale
        if (estado == EstadoJuego::FIN) {
            if (e.key.code == sf::Keyboard::R) {
                estado = EstadoJuego::SELECCION;
                hud.mensaje("", sf::Color::White, 0.f);
            }
            if (e.key.code == sf::Keyboard::Escape) ventana.close();
            return;
        }

        // Durante el juego
        if (estado == EstadoJuego::JUGANDO) {
            // Tab: cambiar jugador activo manualmente
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
            dt = std::min(dt, 0.05f);  // cap

            sf::Event e;
            while (ventana.pollEvent(e)) evento(e);

            update(dt);
            dibujar();
        }
    }
};

// ────────────────────────────────────────────
//  main
// ────────────────────────────────────────────
int main() {
    GameManager gm;
    gm.run();
    return 0;
}
