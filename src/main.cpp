// ============================================================
//  Cartoon Dunk  –  Street Hoop estilo 3v3
//  Desarrollado con SFML
//  Mecánicas basadas en Data East Street Slam (1994)
// ============================================================
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <string>
#include <sstream>

#include "Constantes.hpp"
#include "Pelota.hpp"
#include "Jugador.hpp"
#include "EquipoJuego.hpp"
#include "Cancha.hpp"
#include "HUD.hpp"

// ─────────────────────────────────────────
//  Helpers geométricos
// ─────────────────────────────────────────
float distancia(sf::Vector2f a, sf::Vector2f b) {
    float dx = a.x - b.x, dy = a.y - b.y;
    return std::sqrt(dx*dx + dy*dy);
}

sf::Vector2f normalizar(sf::Vector2f v) {
    float len = std::sqrt(v.x*v.x + v.y*v.y);
    if (len < 0.001f) return {0.f, 0.f};
    return {v.x/len, v.y/len};
}

// ─────────────────────────────────────────
//  Configuración de Equipos
// ─────────────────────────────────────────
DatosEquipo equipoCartoon() {
    DatosEquipo d;
    d.nombre = "CARTOON";
    d.sprites[0] = "assets/imagenes/goku.png";
    d.sprites[1] = "assets/imagenes/Pocoyo.png";
    d.sprites[2] = "assets/imagenes/bugs bunny.png";
    d.color  = sf::Color(255, 120, 0);
    d.dunk   = 8; d.tresP = 6; d.vel = 7; d.def = 5;
    return d;
}

DatosEquipo equipoRival() {
    DatosEquipo d;
    d.nombre = "RIVAL";
    d.sprites[0] = "assets/imagenes/mistico.png";
    d.sprites[1] = "assets/imagenes/mistico.png";
    d.sprites[2] = "assets/imagenes/mistico.png";
    d.color  = sf::Color(80, 160, 255);
    d.dunk   = 6; d.tresP = 7; d.vel = 6; d.def = 8;
    return d;
}

// ─────────────────────────────────────────
//  GameManager
// ─────────────────────────────────────────
class GameManager {
public:
    sf::RenderWindow ventana;
    EquipoJuego      equipoH;   // humano
    EquipoJuego      equipoCPU; // CPU
    Pelota           pelota;
    Cancha           cancha;
    HUD              hud;

    // Música
    sf::Music musica;

    // Estado
    EstadoJuego estadoJuego = EstadoJuego::JUGANDO;
    float       tiempo      = TIEMPO_MITAD;
    int         mitad       = 1;

    // Control de tiro
    bool tirandoHumano    = false;
    float timerLanzamiento = 0.f;

    // Control de pase
    bool pasandoHumano    = false;
    int  destinoPase      = 1; // índice del compañero destino

    // Mensajes de puntuación / eventos
    float timerMensajeGol = 0.f;
    int   puntosUltimoGol = 0;

    // Animación de enceste
    bool  animEnceste   = false;
    float timerEnceste  = 0.f;
    sf::Vector2f posAro;
    int          equipoAnotador = 0; // 0=humano, 1=CPU

    // Fondo (gradiente simple)
    sf::RectangleShape fondo;
    sf::RectangleShape barraTop;
    sf::RectangleShape barraBot;

    GameManager() : ventana(sf::VideoMode(ANCHO_VENTANA, ALTO_VENTANA), "CARTOON DUNK") {
        ventana.setFramerateLimit(60);
        srand((unsigned)time(nullptr));

        // Fondo
        fondo.setSize({(float)ANCHO_VENTANA, (float)ALTO_VENTANA});
        fondo.setFillColor(sf::Color(30, 15, 5));
        barraTop.setSize({(float)ANCHO_VENTANA, CANCHA_Y});
        barraTop.setFillColor(sf::Color(20, 10, 5, 220));
        barraBot.setSize({(float)ANCHO_VENTANA, ALTO_VENTANA - CANCHA_Y - CANCHA_ALTO});
        barraBot.setPosition(0.f, CANCHA_Y + CANCHA_ALTO);
        barraBot.setFillColor(sf::Color(20, 10, 5, 220));

        iniciarPartida();

        // Música
        if (musica.openFromFile("assets/musica/Gang$tazz.ogg")) {
            musica.setLoop(true);
            musica.setVolume(60.f);
            musica.play();
        }
    }

    void iniciarPartida() {
        tiempo = TIEMPO_MITAD;
        mitad  = 1;
        equipoH.puntos   = 0;
        equipoCPU.puntos = 0;

        sf::Vector2f posH[3] = {
            {CANCHA_X + 200.f, ARO_Y},
            {CANCHA_X + 250.f, ARO_Y - 90.f},
            {CANCHA_X + 250.f, ARO_Y + 90.f}
        };
        sf::Vector2f posCPU[3] = {
            {CANCHA_X + CANCHA_ANCHO - 200.f, ARO_Y},
            {CANCHA_X + CANCHA_ANCHO - 250.f, ARO_Y - 90.f},
            {CANCHA_X + CANCHA_ANCHO - 250.f, ARO_Y + 90.f}
        };

        equipoH.configurar(equipoCartoon(), true, posH);
        equipoCPU.configurar(equipoRival(), false, posCPU);

        pelota.posicion = equipoH.activo().posicion;
        pelota.enJuego  = false;
        equipoH.activo().tienePelota = true;
    }

    // ─────────────────────────────────────
    //  Comprueba si la pelota entra en un aro
    // ─────────────────────────────────────
    void verificarEnceste() {
        if (!pelota.enArco) return;

        // Aro izquierdo (ataca CPU) → aro derecho (ataca humano)
        sf::Vector2f aroL = {ARO_IZQUIERDA_X + CANASTA_RADIO + 5.f, ARO_Y};
        sf::Vector2f aroR = {ARO_DERECHA_X   - CANASTA_RADIO - 5.f, ARO_Y};

        // Destino de la pelota cerca del aro izquierdo → CPU anota
        if (distancia(pelota.destinoArco, aroL) < CANASTA_RADIO + 8.f) {
            float prob = equipoCPU.jugadores[equipoCPU.jugadorActivo]
                         .probTiro(distancia(equipoCPU.activo().posicion, aroL), pelota.superShot);
            if ((float)rand()/RAND_MAX < prob) {
                int pts = (distancia(equipoCPU.activo().posicion, aroL) > DIST_TRES_PUNTOS) ? 3 : 2;
                equipoCPU.agregarPuntos(pts);
                activarAnimEnceste(aroL, 1, pts);
            }
        }
        // Destino cerca del aro derecho → Humano anota
        if (distancia(pelota.destinoArco, aroR) < CANASTA_RADIO + 8.f) {
            float prob = equipoH.jugadores[equipoH.jugadorActivo]
                         .probTiro(distancia(equipoH.activo().posicion, aroR), pelota.superShot);
            if ((float)rand()/RAND_MAX < prob) {
                int pts = (distancia(equipoH.activo().posicion, aroR) > DIST_TRES_PUNTOS) ? 3 : 2;
                equipoH.agregarPuntos(pts);
                activarAnimEnceste(aroR, 0, pts);
            }
        }
    }

    void activarAnimEnceste(sf::Vector2f aro, int equipo, int pts) {
        animEnceste    = true;
        timerEnceste   = 1.8f;
        posAro         = aro;
        equipoAnotador = equipo;
        puntosUltimoGol = pts;

        std::string msg = (pts == 3) ? "TRIPLE!" :
                          (pts == 2) ? "CANASTA!" : "GOL!";
        if (pelota.superShot) msg = "SUPER DUNK!!!";
        hud.mostrarMensaje(msg, 2.5f);

        // Reiniciar posesión al otro equipo
        reinicarPosesion(equipo == 0 ? equipoCPU : equipoH, equipo == 0);
    }

    void reinicarPosesion(EquipoJuego& equipoConPelota, bool humanoPierde) {
        equipoH.quitarPelota();
        equipoCPU.quitarPelota();

        // Pelota aparece en el centro
        pelota.posicion = {CANCHA_X + CANCHA_ANCHO / 2.f, CANCHA_Y + CANCHA_ALTO / 2.f};
        pelota.velocidad = {0.f, 0.f};
        pelota.enArco    = false;
        pelota.enJuego   = false;
        pelota.superShot = false;

        equipoConPelota.jugadorActivo = 0;
        equipoConPelota.jugadores[0].tienePelota = true;
        equipoConPelota.jugadores[0].posicion = pelota.posicion;
    }

    // ─────────────────────────────────────
    //  Input Humano
    // ─────────────────────────────────────
    void procesarInputHumano(float dt) {
        Jugador& activo = equipoH.activo();
        bool tieneB = activo.tienePelota;

        // Movimiento WASD
        sf::Vector2f dir = {0.f, 0.f};
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) dir.x -= 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) dir.x += 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) dir.y -= 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) dir.y += 1.f;

        bool sprint = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift);
        activo.mover(dir, dt, sprint);

        // Mover pelota con el jugador que la tiene
        if (tieneB && !pelota.enArco) {
            pelota.posicion = activo.posicion;
        }

        // Rotar jugador activo con Tab
        // (se maneja en evento de teclado)

        // Tiro  →  Tecla J (equivale al botón A original)
        // Solo dispara al soltarlo después de mantener
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::J) && tieneB) {
            tirandoHumano = true;
        }

        // Pase  →  Tecla K (botón B original)
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::K) && tieneB) {
            pasandoHumano = true;
        }

        // Defensa / Robo  →  Tecla L
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::L) && !tieneB) {
            activo.estado = EstadoJugador::BLOQUEANDO;
            activo.timerEstado = 0.15f;
            // Intentar robar a un jugador CPU cercano
            for (auto& jcpu : equipoCPU.jugadores) {
                if (jcpu.tienePelota) {
                    float d = distancia(activo.posicion, jcpu.posicion);
                    if (d < RADIO_ROBO + 10.f) {
                        float prob = activo.probRobo() * 0.4f;
                        if ((float)rand()/RAND_MAX < prob) {
                            jcpu.tienePelota = false;
                            activo.tienePelota = true;
                            pelota.posicion = activo.posicion;
                            pelota.enJuego = false;
                            equipoH.aumentarSuperPorRobo();
                            hud.mostrarMensaje("ROBO!", 1.5f);
                        }
                        break;
                    }
                }
            }
        }
    }

    void ejecutarTiroHumano() {
        Jugador& activo = equipoH.activo();
        if (!activo.tienePelota) return;

        sf::Vector2f aroDestino = {ARO_DERECHA_X - CANASTA_RADIO - 5.f, ARO_Y};
        float dist = distancia(activo.posicion, aroDestino);

        bool esSuper = equipoH.tieneSuper();

        if (esSuper) {
            equipoH.gastarSuper();
            activo.estado = EstadoJugador::SUPER_SHOT;
            pelota.superShot = true;
        } else {
            activo.estado = EstadoJugador::LANZANDO;
        }
        activo.tienePelota  = false;
        activo.timerEstado  = 0.6f;

        float tiempoVuelo = 0.5f + dist / 1200.f;
        pelota.lanzarEnArco(activo.posicion, aroDestino, tiempoVuelo, esSuper);

        tirandoHumano = false;
    }

    void ejecutarPaseHumano() {
        Jugador& activo = equipoH.activo();
        if (!activo.tienePelota) return;

        // Elegir compañero más cercano al aro
        int mejor = -1;
        float menorDist = 99999.f;
        sf::Vector2f aro = {ARO_DERECHA_X - CANASTA_RADIO - 5.f, ARO_Y};
        for (int i = 0; i < 3; i++) {
            if (i == equipoH.jugadorActivo) continue;
            float d = distancia(equipoH.jugadores[i].posicion, aro);
            if (d < menorDist) { menorDist = d; mejor = i; }
        }
        if (mejor < 0) { pasandoHumano = false; return; }

        activo.tienePelota = false;
        activo.estado      = EstadoJugador::PASANDO;
        activo.timerEstado = 0.4f;

        Jugador& receptor = equipoH.jugadores[mejor];
        float tiempoVuelo = distancia(activo.posicion, receptor.posicion) / 700.f + 0.1f;
        pelota.lanzarPase(activo.posicion, receptor.posicion, tiempoVuelo);

        // Al llegar el pase: pend. receptor toma pelota (se resuelve en update)
        destinoPase = mejor;
        equipoH.aumentarSuperPorPase();
        pasandoHumano = false;
    }

    // ─────────────────────────────────────
    //  IA  CPU  (estilo arcade: agresiva pero predecible)
    // ─────────────────────────────────────
    void updateIA(float dt) {
        Jugador* conPelota = equipoCPU.conPelota();
        sf::Vector2f aroL  = {ARO_IZQUIERDA_X + CANASTA_RADIO + 5.f, ARO_Y};

        if (conPelota) {
            // Moverse hacia el aro
            sf::Vector2f dir = normalizar(aroL - conPelota->posicion);
            conPelota->mover(dir, dt * 0.9f);

            float dist = distancia(conPelota->posicion, aroL);

            // Decidir si tira
            bool tiroLibre = dist < 150.f;
            bool tiroTres  = dist < DIST_TRES_PUNTOS + 40.f && dist > DIST_TRES_PUNTOS - 20.f;
            if (tiroLibre || tiroTres) {
                float prob = conPelota->probTiro(dist, tiroTres) * dt * 2.5f;
                if ((float)rand()/RAND_MAX < prob) {
                    bool esSuper = equipoCPU.tieneSuper();
                    if (esSuper) equipoCPU.gastarSuper();
                    conPelota->tienePelota = false;
                    conPelota->estado      = esSuper ? EstadoJugador::SUPER_SHOT : EstadoJugador::LANZANDO;
                    conPelota->timerEstado = 0.6f;
                    float tv = 0.5f + dist / 1200.f;
                    pelota.lanzarEnArco(conPelota->posicion, aroL, tv, esSuper);
                    return;
                }
            }

            // Pasar a un compañero más libre
            float probPase = 0.008f * dt * 60.f;
            if (dist > 250.f && (float)rand()/RAND_MAX < probPase) {
                for (int i = 0; i < 3; i++) {
                    if (&equipoCPU.jugadores[i] == conPelota) continue;
                    float dc = distancia(equipoCPU.jugadores[i].posicion, aroL);
                    if (dc < dist - 60.f) {
                        conPelota->tienePelota = false;
                        conPelota->estado = EstadoJugador::PASANDO;
                        float tv = distancia(conPelota->posicion, equipoCPU.jugadores[i].posicion) / 700.f + 0.1f;
                        pelota.lanzarPase(conPelota->posicion, equipoCPU.jugadores[i].posicion, tv);
                        equipoCPU.jugadorActivo = i;
                        equipoCPU.aumentarSuperPorPase();
                        break;
                    }
                }
            }

            // Mover compañeros sin pelota a posiciones ofensivas
            for (int i = 0; i < 3; i++) {
                Jugador& j = equipoCPU.jugadores[i];
                if (j.tienePelota) continue;
                sf::Vector2f objetivo = {
                    aroL.x + 80.f + (float)(i * 60),
                    aroL.y + (float)((i - 1) * 120)
                };
                sf::Vector2f d = normalizar(objetivo - j.posicion);
                if (distancia(j.posicion, objetivo) > 10.f) j.mover(d, dt * 0.7f);
            }

        } else {
            // CPU no tiene pelota: defender
            for (int i = 0; i < 3; i++) {
                Jugador& j = equipoCPU.jugadores[i];
                Jugador& objetivo = equipoH.jugadores[i % 3];
                sf::Vector2f dir = normalizar(objetivo.posicion - j.posicion);
                if (distancia(j.posicion, objetivo.posicion) > 45.f)
                    j.mover(dir, dt * 0.75f);

                // Intentar robar
                if (objetivo.tienePelota) {
                    float dist = distancia(j.posicion, objetivo.posicion);
                    if (dist < RADIO_ROBO) {
                        float prob = j.probRobo() * dt * 1.5f;
                        if ((float)rand()/RAND_MAX < prob) {
                            objetivo.tienePelota = false;
                            j.tienePelota = true;
                            pelota.posicion = j.posicion;
                            pelota.enJuego  = false;
                            equipoCPU.jugadorActivo = i;
                            equipoCPU.aumentarSuperPorRobo();
                            tirandoHumano = false;
                            pasandoHumano = false;
                            hud.mostrarMensaje("ROBO del RIVAL!", 1.5f);
                        }
                    }
                }
            }
        }
    }

    // ─────────────────────────────────────
    //  Actualización del pase (destino recibe pelota)
    // ─────────────────────────────────────
    void verificarLlegadaPase() {
        if (!pelota.enArco) return;
        float t = pelota.tiempoArco / pelota.duracionArco;
        if (t < 0.85f) return;

        // Pase al equipo humano
        for (int i = 0; i < 3; i++) {
            if (equipoH.jugadores[i].tienePelota) return; // ya tiene
        }
        // Si el destino del arco es un jugador humano (t ~= 1) y no es tiro
        float dH = distancia(pelota.destinoArco, equipoH.jugadores[destinoPase].posicion);
        if (dH < 80.f && !pelota.superShot) {
            // Podría ser un pase a humano
            if (distancia(pelota.destinoArco, {ARO_DERECHA_X - CANASTA_RADIO - 5.f, ARO_Y}) > 60.f &&
                distancia(pelota.destinoArco, {ARO_IZQUIERDA_X + CANASTA_RADIO + 5.f, ARO_Y}) > 60.f) {
                // Es pase, no tiro
            }
        }

        // Pase CPU: el receptor activo toma la pelota cuando llega
        if (!pelota.enJuego) return;
        Jugador& receptorCPU = equipoCPU.jugadores[equipoCPU.jugadorActivo];
        if (distancia(pelota.posicion, receptorCPU.posicion) < 50.f && !receptorCPU.tienePelota) {
            receptorCPU.tienePelota = true;
            pelota.enJuego = false;
        }
    }

    // ─────────────────────────────────────
    //  Cuando la pelota termina su arco libre
    // ─────────────────────────────────────
    void verificarPelotaLibre() {
        if (pelota.enArco || !pelota.enJuego) return;

        // Pelota sin dueño: el jugador humano activo puede recogerla
        Jugador& actH = equipoH.activo();
        if (!actH.tienePelota && distancia(actH.posicion, pelota.posicion) < JUGADOR_RADIO + PELOTA_RADIO + 5.f) {
            actH.tienePelota = true;
            pelota.enJuego   = false;
            pelota.velocidad = {0.f,0.f};
        }
        // CPU
        for (auto& jcpu : equipoCPU.jugadores) {
            if (distancia(jcpu.posicion, pelota.posicion) < JUGADOR_RADIO + PELOTA_RADIO + 5.f) {
                equipoCPU.quitarPelota();
                equipoH.quitarPelota();
                jcpu.tienePelota = true;
                pelota.enJuego   = false;
                pelota.velocidad = {0.f,0.f};
                break;
            }
        }
    }

    // ─────────────────────────────────────
    //  Update principal
    // ─────────────────────────────────────
    void update(float dt) {
        if (estadoJuego != EstadoJuego::JUGANDO) return;

        // Tiempo
        tiempo -= dt;
        if (tiempo <= 0.f) {
            if (mitad == 1) {
                mitad = 2;
                tiempo = TIEMPO_MITAD;
                estadoJuego = EstadoJuego::MEDIO_TIEMPO;
                hud.mostrarMensaje("MEDIO TIEMPO!", 3.f);
            } else {
                estadoJuego = EstadoJuego::FIN_JUEGO;
            }
        }

        // Ejecutar acciones acumuladas
        if (tirandoHumano) ejecutarTiroHumano();
        if (pasandoHumano) ejecutarPaseHumano();

        // Sincronizar pelota con portador
        Jugador* conPelotaH = equipoH.conPelota();
        if (conPelotaH && !pelota.enArco) pelota.posicion = conPelotaH->posicion;
        Jugador* conPelotaC = equipoCPU.conPelota();
        if (conPelotaC && !pelota.enArco) pelota.posicion = conPelotaC->posicion;

        // Input humano
        procesarInputHumano(dt);

        // IA CPU
        updateIA(dt);

        // Pelota
        pelota.actualizar(dt);

        // Verificar enceste cuando la pelota llega al destino
        if (pelota.enArco && pelota.tiempoArco / pelota.duracionArco > 0.9f) {
            static bool verificado = false;
            if (!verificado) { verificarEnceste(); verificado = true; }
        } else { /* reset handled by the bool above */ }

        // Pase llegó
        verificarLlegadaPase();

        // Pelota libre en suelo
        verificarPelotaLibre();

        // Update entidades
        equipoH.actualizar(dt);
        equipoCPU.actualizar(dt);
        hud.actualizar(dt);
    }

    // ─────────────────────────────────────
    //  Dibujo
    // ─────────────────────────────────────
    void dibujar() {
        ventana.clear(sf::Color(20, 10, 5));

        // Fondo
        ventana.draw(fondo);

        // Cancha
        cancha.dibujar(ventana);

        // Barras superior e inferior
        ventana.draw(barraTop);
        ventana.draw(barraBot);

        // Indicador de zona 3pts (línea de distancia)
        // ya está en la cancha dibujada

        // Jugadores (ordenar por Y para profundidad)
        std::vector<Jugador*> todos;
        for (auto& j : equipoH.jugadores)   todos.push_back(&j);
        for (auto& j : equipoCPU.jugadores) todos.push_back(&j);
        std::sort(todos.begin(), todos.end(), [](Jugador* a, Jugador* b){
            return a->posicion.y < b->posicion.y;
        });
        for (auto* j : todos) j->dibujar(ventana);

        // Pelota
        pelota.dibujar(ventana);

        // HUD
        hud.dibujar(ventana,
                    equipoH.puntos, equipoCPU.puntos,
                    tiempo, mitad,
                    equipoH.superMeter, equipoCPU.superMeter,
                    equipoH.nombre, equipoCPU.nombre);

        // Pantalla de fin
        if (estadoJuego == EstadoJuego::FIN_JUEGO) {
            sf::RectangleShape overlay({(float)ANCHO_VENTANA, (float)ALTO_VENTANA});
            overlay.setFillColor(sf::Color(0,0,0,160));
            ventana.draw(overlay);
            sf::Text txFin;
            if (hud.fuenteOk) txFin.setFont(hud.fuente);
            txFin.setCharacterSize(48);
            txFin.setOutlineThickness(3.f);
            txFin.setOutlineColor(sf::Color::Black);
            std::string resultado;
            if (equipoH.puntos > equipoCPU.puntos)       resultado = "GANASTE!";
            else if (equipoH.puntos < equipoCPU.puntos)  resultado = "PERDISTE!";
            else                                           resultado = "EMPATE!";
            txFin.setString(resultado);
            txFin.setFillColor(sf::Color(255, 220, 50));
            sf::FloatRect b = txFin.getLocalBounds();
            txFin.setOrigin(b.width/2.f, b.height/2.f);
            txFin.setPosition(ANCHO_VENTANA/2.f, ALTO_VENTANA/2.f - 30.f);
            ventana.draw(txFin);

            sf::Text txScore;
            if (hud.fuenteOk) txScore.setFont(hud.fuente);
            txScore.setCharacterSize(28);
            txScore.setFillColor(sf::Color::White);
            txScore.setOutlineThickness(2.f);
            txScore.setOutlineColor(sf::Color::Black);
            std::ostringstream ss;
            ss << equipoH.puntos << " - " << equipoCPU.puntos;
            txScore.setString(ss.str());
            sf::FloatRect bs = txScore.getLocalBounds();
            txScore.setOrigin(bs.width/2.f, 0.f);
            txScore.setPosition(ANCHO_VENTANA/2.f, ALTO_VENTANA/2.f + 30.f);
            ventana.draw(txScore);

            sf::Text txR;
            if (hud.fuenteOk) txR.setFont(hud.fuente);
            txR.setCharacterSize(18);
            txR.setFillColor(sf::Color(200,200,200));
            txR.setString("Presiona R para jugar de nuevo  |  ESC para salir");
            sf::FloatRect br = txR.getLocalBounds();
            txR.setOrigin(br.width/2.f, 0.f);
            txR.setPosition(ANCHO_VENTANA/2.f, ALTO_VENTANA/2.f + 80.f);
            ventana.draw(txR);
        }

        // Pantalla medio tiempo
        if (estadoJuego == EstadoJuego::MEDIO_TIEMPO) {
            sf::RectangleShape overlay({(float)ANCHO_VENTANA, (float)ALTO_VENTANA});
            overlay.setFillColor(sf::Color(0,0,0,140));
            ventana.draw(overlay);
            sf::Text tx;
            if (hud.fuenteOk) tx.setFont(hud.fuente);
            tx.setCharacterSize(40);
            tx.setFillColor(sf::Color(255,200,0));
            tx.setOutlineThickness(3.f);
            tx.setOutlineColor(sf::Color::Black);
            tx.setString("MEDIO TIEMPO");
            sf::FloatRect b = tx.getLocalBounds();
            tx.setOrigin(b.width/2.f, b.height/2.f);
            tx.setPosition(ANCHO_VENTANA/2.f, ALTO_VENTANA/2.f - 20.f);
            ventana.draw(tx);

            sf::Text txC;
            if (hud.fuenteOk) txC.setFont(hud.fuente);
            txC.setCharacterSize(20);
            txC.setFillColor(sf::Color::White);
            txC.setString("Presiona Enter para continuar");
            sf::FloatRect bc = txC.getLocalBounds();
            txC.setOrigin(bc.width/2.f, 0.f);
            txC.setPosition(ANCHO_VENTANA/2.f, ALTO_VENTANA/2.f + 30.f);
            ventana.draw(txC);
        }
    }

    // ─────────────────────────────────────
    //  Eventos de teclado
    // ─────────────────────────────────────
    void procesarEvento(sf::Event& e) {
        if (e.type == sf::Event::Closed) ventana.close();

        if (e.type == sf::Event::KeyPressed) {
            // Cambiar jugador activo
            if (e.key.code == sf::Keyboard::Tab && estadoJuego == EstadoJuego::JUGANDO) {
                // Elegir compañero más cercano a la pelota
                int mejor = equipoH.jugadorActivo;
                float menorD = 99999.f;
                for (int i = 0; i < 3; i++) {
                    if (i == equipoH.jugadorActivo) continue;
                    float d = distancia(equipoH.jugadores[i].posicion, pelota.posicion);
                    if (d < menorD) { menorD = d; mejor = i; }
                }
                if (!equipoH.jugadores[mejor].tienePelota)
                    equipoH.jugadorActivo = mejor;
            }
            // Continuar desde medio tiempo
            if (e.key.code == sf::Keyboard::Return && estadoJuego == EstadoJuego::MEDIO_TIEMPO) {
                estadoJuego = EstadoJuego::JUGANDO;
                // Reiniciar posiciones (ahora el equipo que perdió la mitad 1 tiene balón)
                sf::Vector2f posH[3] = {
                    {CANCHA_X + 200.f, ARO_Y},
                    {CANCHA_X + 250.f, ARO_Y - 90.f},
                    {CANCHA_X + 250.f, ARO_Y + 90.f}
                };
                sf::Vector2f posCPU[3] = {
                    {CANCHA_X + CANCHA_ANCHO - 200.f, ARO_Y},
                    {CANCHA_X + CANCHA_ANCHO - 250.f, ARO_Y - 90.f},
                    {CANCHA_X + CANCHA_ANCHO - 250.f, ARO_Y + 90.f}
                };
                for (int i = 0; i < 3; i++) {
                    equipoH.jugadores[i].setPosicion(posH[i]);
                    equipoH.jugadores[i].tienePelota = false;
                    equipoCPU.jugadores[i].setPosicion(posCPU[i]);
                    equipoCPU.jugadores[i].tienePelota = false;
                }
                pelota.posicion = {ANCHO_VENTANA/2.f, ARO_Y};
                pelota.enArco = false;
                pelota.enJuego = false;
                equipoH.jugadores[0].tienePelota = true;
                equipoH.jugadorActivo = 0;
            }
            // Reiniciar
            if (e.key.code == sf::Keyboard::R && estadoJuego == EstadoJuego::FIN_JUEGO) {
                iniciarPartida();
                estadoJuego = EstadoJuego::JUGANDO;
            }
            // Salir
            if (e.key.code == sf::Keyboard::Escape) ventana.close();
        }
    }

    // ─────────────────────────────────────
    //  Loop principal
    // ─────────────────────────────────────
    void run() {
        sf::Clock reloj;
        while (ventana.isOpen()) {
            float dt = reloj.restart().asSeconds();
            if (dt > 0.05f) dt = 0.05f; // cap a 50ms

            sf::Event evento;
            while (ventana.pollEvent(evento)) {
                procesarEvento(evento);
            }

            update(dt);

            dibujar();
            ventana.display();
        }
    }
};

// ─────────────────────────────────────────
//  main()
// ─────────────────────────────────────────
int main() {
    GameManager juego;
    juego.run();
    return 0;
}
