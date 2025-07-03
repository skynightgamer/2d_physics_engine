#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <random>

// physicsobjekt (gibt nur kugeln)
class Kugel {
public:
    sf::Vector2f pos;
    sf::Vector2f geschwindigkeit;
    sf::Vector2f beschleunigung;
    float radius;
    float masse;
    sf::Color farbe;
    bool istStatisch;

    Kugel(float x, float y, float r, float m, sf::Color c = sf::Color::White, bool statisch = false)
        : pos(x, y), geschwindigkeit(0, 0), beschleunigung(0, 0), 
          radius(r), masse(m), farbe(c), istStatisch(statisch) {}

    void kraftAnwenden(const sf::Vector2f& kraft) {
        if (!istStatisch) {
            beschleunigung += kraft / masse;
        }
    }

    void update(float deltaZeit) {
        if (!istStatisch) {
            geschwindigkeit += beschleunigung * deltaZeit;
            pos += geschwindigkeit * deltaZeit;
            beschleunigung = sf::Vector2f(0, 0);
        }
    }

    void zeichnen(sf::RenderWindow& fenster) const {
        sf::CircleShape form(radius);
        form.setPosition(pos.x - radius, pos.y - radius);
        form.setFillColor(farbe);
        form.setOutlineThickness(2);
        form.setOutlineColor(sf::Color::Black);
        fenster.draw(form);
    }
};

// ENGiNE
class PhysikEngine {
private:
    std::vector<Kugel> kugeln;
    bool gravityOn;
    float schwerkraft;
    float breite, hoehe;
    float daempfung;
    float rueckprall;
    float airres; 
    int kollisionscounter;
    bool counterAktiv;
    bool rechteWandAus;

public:
    PhysikEngine(float b, float h) 
        : breite(b), hoehe(h), gravityOn(true), schwerkraft(500.0f), 
          daempfung(0.98f), rueckprall(0.8f), airres(0.99f),
          kollisionscounter(0), counterAktiv(false), rechteWandAus(false) {}

    void kugelHinzufuegen(const Kugel& kugel) {
        kugeln.push_back(kugel);
    }

    void alleLoeschen() {
        kugeln.clear();
        kollisionscounter = 0;
    }

    void schwerkraftUmschalten() {
        gravityOn = !gravityOn;
    }
    
    void setupWelt(float grav, float daemp, float rueck, float luft, bool gravAn = true, bool keineRechteWand = false) {
        schwerkraft = grav;
        daempfung = daemp;
        rueckprall = rueck;
        airres = luft;
        gravityOn = gravAn;
        rechteWandAus = keineRechteWand;
    }
    
    void counterAktivieren(bool aktiv) {
        counterAktiv = aktiv;
        if (aktiv) kollisionscounter = 0;
    }

    

    void update(float deltaZeit) {
        if (gravityOn) {
            for (auto& kugel : kugeln) {
                if (!kugel.istStatisch) {
                    kugel.kraftAnwenden(sf::Vector2f(0, kugel.masse * schwerkraft));
                }
            }
        }

        for (auto& kugel : kugeln) {
            kugel.update(deltaZeit);
        }

        kollisionenPruefen();
        wandKollisionenPruefen();

        for (auto& kugel : kugeln) {
            if (!kugel.istStatisch) {
                kugel.geschwindigkeit *= airres;
            }
        }
    }

    void zeichnen(sf::RenderWindow& fenster) {
        for (const auto& kugel : kugeln) {
            kugel.zeichnen(fenster);
        }
    }

    bool istgravityOn() const { return gravityOn; }
    int getKugelAnzahl() const { return kugeln.size(); }
    int getKollisionen() const { return kollisionscounter; }

private:
    void kollisionenPruefen() {
        for (size_t i = 0; i < kugeln.size(); i++) {
            for (size_t j = i + 1; j < kugeln.size(); j++) {
                Kugel& a = kugeln[i];
                Kugel& b = kugeln[j];

                sf::Vector2f delta = b.pos - a.pos;
                float distanz = std::sqrt(delta.x * delta.x + delta.y * delta.y);

                if (distanz < a.radius + b.radius && distanz > 0.0f) {
                    sf::Vector2f normale = delta / distanz;
                    float ueberlappung = a.radius + b.radius - distanz;

                    if (!a.istStatisch && !b.istStatisch) {
                        a.pos -= normale * (ueberlappung * 0.5f);
                        b.pos += normale * (ueberlappung * 0.5f);
                    } else if (!a.istStatisch) {
                        a.pos -= normale * ueberlappung;
                    } else if (!b.istStatisch) {
                        b.pos += normale * ueberlappung;
                    }

                    sf::Vector2f relativeGeschw = b.geschwindigkeit - a.geschwindigkeit;
                    float geschwindigkeitAufNormale = relativeGeschw.x * normale.x + relativeGeschw.y * normale.y;

                    if (geschwindigkeitAufNormale > 0) continue;

                    float impuls = 2 * geschwindigkeitAufNormale / (1/a.masse + 1/b.masse);
                    impuls *= -rueckprall;

                    sf::Vector2f impulsVektor = normale * impuls;
                    if (!a.istStatisch) {
                        a.geschwindigkeit -= impulsVektor / a.masse;
                    }
                    if (!b.istStatisch) {
                        b.geschwindigkeit += impulsVektor / b.masse;
                    }
                    
                    if (counterAktiv) {
                        kollisionscounter++;
                    }
                }
            }
        }
    }

    void wandKollisionenPruefen() {
        for (auto& kugel : kugeln) {
            if (kugel.istStatisch) continue;

            //
            if (kugel.pos.x - kugel.radius < 0) {
                kugel.pos.x = kugel.radius;
                kugel.geschwindigkeit.x = -kugel.geschwindigkeit.x * rueckprall;
                if (counterAktiv) kollisionscounter++;
            }
            // rechts (welt2!)
            if (!rechteWandAus && kugel.pos.x + kugel.radius > breite) {
                kugel.pos.x = breite - kugel.radius;
                kugel.geschwindigkeit.x = -kugel.geschwindigkeit.x * rueckprall;
                if (counterAktiv) kollisionscounter++;
            }
            // 
            if (kugel.pos.y - kugel.radius < 0) {
                kugel.pos.y = kugel.radius;
                kugel.geschwindigkeit.y = -kugel.geschwindigkeit.y * rueckprall;
                if (counterAktiv) kollisionscounter++;
            }
            // 
            if (kugel.pos.y + kugel.radius > hoehe) {
                kugel.pos.y = hoehe - kugel.radius;
                kugel.geschwindigkeit.y = -kugel.geschwindigkeit.y * rueckprall;
                if (counterAktiv) kollisionscounter++;
            }
        }
    }
};

// ==== Welten-Manager ====
class WeltenManager {
public:
    static void loadworld1(PhysikEngine& engine, float breite, float hoehe) {
        engine.alleLoeschen();
        engine.setupWelt(500.0f, 0.98f, 0.95f, 0.99f);
        engine.counterAktivieren(false);
        
        
        std::vector<sf::Color> farben = {sf::Color::Red, sf::Color::Blue, sf::Color::Green, 
                                         sf::Color::Yellow, sf::Color::Magenta};
        for (int i = 0; i < 5; i++) {
            Kugel ball(100 + i * 200, 100 + i * 50, 30, 3, farben[i]);
            ball.geschwindigkeit = sf::Vector2f(rand() % 200 - 100, rand() % 200);
            engine.kugelHinzufuegen(ball);
        }
    }
    
    static void loadworld2(PhysikEngine& engine, float breite, float hoehe) {
        engine.alleLoeschen();
        engine.setupWelt(0.0f, 1.0f, 1.0f, 1.0f, false, true);  
        engine.counterAktivieren(true);
        
        // small ball
        Kugel kleineKugel(100, hoehe/2, 20, 1, sf::Color::White);
        kleineKugel.geschwindigkeit = sf::Vector2f(0, 0);
        engine.kugelHinzufuegen(kleineKugel);
        
        // big ball
        Kugel grosseKugel(300, hoehe/2, 40, 100, sf::Color::Red);
        grosseKugel.geschwindigkeit = sf::Vector2f(-100, 0);  
        engine.kugelHinzufuegen(grosseKugel);
    }
    
    static void loadworld3(PhysikEngine& engine, float breite, float hoehe) {
        engine.alleLoeschen();
        engine.setupWelt(300.0f, 0.95f, 0.7f, 0.98f);
        engine.counterAktivieren(false);

        Kugel wand(breite/2,hoehe * 2, 1000, 1000, sf::Color(100, 100, 100), true);
        engine.kugelHinzufuegen(wand);
    }
    
    static void loadbilliard(PhysikEngine& engine, float breite, float hoehe) {
        engine.alleLoeschen();
        engine.setupWelt(0.0f, 0.99f, 0.95f, 0.99f, false);
        engine.counterAktivieren(false);
        
        // Billard Dreieck AI USE (OpanAI chatgpt)
        float startX = breite * 0.65f;
        float startY = hoehe / 2;
        float ballRadius = 15;
        
        int ballNummer = 0;
        
        for (int reihe = 0; reihe < 5; reihe++) {
            for (int pos = 0; pos <= reihe; pos++) {
                float x = startX + reihe * (ballRadius * 2 - 2);
                float y = startY + (pos - reihe/2.0f) * (ballRadius * 2 - 2);
                Kugel ball(x, y, ballRadius, 1, sf::Color::Blue);
                engine.kugelHinzufuegen(ball);
                ballNummer++;
            }
        }
        
        // Spielball
        Kugel spielball(breite * 0.25f, hoehe / 2, ballRadius, 1, sf::Color::White);
        spielball.geschwindigkeit = sf::Vector2f(500, 0);
        engine.kugelHinzufuegen(spielball);
    }
};

// Hauptprogramm (AI USAGE: teile des renderings (Openai chatgpt); repetetive codeabschnitte (openai chatgpt))
int main() {
    const float BREITE = 1200.0f;
    const float HOEHE = 800.0f;

    sf::RenderWindow fenster(sf::VideoMode(BREITE, HOEHE), "Physik Simulation");
    fenster.setFramerateLimit(60);

    PhysikEngine engine(BREITE, HOEHE);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> radiusGen(15.0f, 40.0f);
    std::uniform_real_distribution<> farbGen(0.0f, 255.0f);

    // automatisch welt1 laden
    WeltenManager::loadworld1(engine, BREITE, HOEHE);
    int aktuelleWelt = 1;

    sf::Clock uhr;
    sf::Font schrift;
    bool schriftGeladen = schrift.loadFromFile("arial.ttf");

    sf::Text infoText;
    if (schriftGeladen) {
        infoText.setFont(schrift);
        infoText.setCharacterSize(16);
        infoText.setFillColor(sf::Color::White);
        infoText.setPosition(10, 10);
    }
    bool pausiert = false;

    while (fenster.isOpen()) {
        sf::Event ereignis;
        while (fenster.pollEvent(ereignis)) {
            if (ereignis.type == sf::Event::Closed)
                fenster.close();
            else if (ereignis.type == sf::Event::KeyPressed) {
                switch (ereignis.key.code) {
                    case sf::Keyboard::Num1:
                        WeltenManager::loadworld1(engine, BREITE, HOEHE);
                        aktuelleWelt = 1;
                        break;
                    case sf::Keyboard::Num2:
                        WeltenManager::loadworld2(engine, BREITE, HOEHE);
                        aktuelleWelt = 2;
                        break;
                    case sf::Keyboard::Num3:
                        WeltenManager::loadworld3(engine, BREITE, HOEHE);
                        aktuelleWelt = 3;
                        break;
                    case sf::Keyboard::Num4:
                        WeltenManager::loadbilliard(engine, BREITE, HOEHE);
                        aktuelleWelt = 4;
                        break;
                    case sf::Keyboard::G:
                        engine.schwerkraftUmschalten();
                        break;
                    case sf::Keyboard::P:
                        pausiert = !pausiert;
                        break;
                    case sf::Keyboard::Space:
                        {
                            sf::Vector2i mausPos = sf::Mouse::getPosition(fenster);
                            float radius = radiusGen(gen);
                            float masse = radius * 0.1f;
                            sf::Color farbe(farbGen(gen), farbGen(gen), farbGen(gen));
                            Kugel neueKugel(mausPos.x, mausPos.y, radius, masse, farbe);
                            engine.kugelHinzufuegen(neueKugel);
                        }
                        break;
                    case sf::Keyboard::S:
                        {
                            sf::Vector2i mausPos = sf::Mouse::getPosition(fenster);
                            Kugel wand(mausPos.x, mausPos.y, 50, 1000, sf::Color(100, 100, 100), true);
                            engine.kugelHinzufuegen(wand);
                        }
                        break;
                    case sf::Keyboard::C:
                        engine.alleLoeschen();
                        break;
                }
            } else if (ereignis.type == sf::Event::MouseButtonPressed) {
                if (ereignis.mouseButton.button == sf::Mouse::Left) {
                    float radius = radiusGen(gen);
                    float masse = radius * 0.1f;
                    sf::Color farbe(farbGen(gen), farbGen(gen), farbGen(gen));
                    Kugel kugel(ereignis.mouseButton.x, ereignis.mouseButton.y, radius, masse, farbe);
                    kugel.geschwindigkeit = sf::Vector2f(200, -300);
                    engine.kugelHinzufuegen(kugel);
                }
            }
        }

        float deltaZeit = uhr.restart().asSeconds();
        if (!pausiert) {
            engine.update(deltaZeit);
        }

        fenster.clear(sf::Color::Black);
        engine.zeichnen(fenster);

        if (schriftGeladen) {
            std::string schwerkraftStatus = engine.istgravityOn() ? "ON" : "OFF";
            std::string pauseStatus = pausiert ? "PAUSED" : "RUNNING";
            std::string weltName;
            
            switch (aktuelleWelt) {
                case 1: weltName = "Baellebad"; break;
                case 2: weltName = "Pi-Kollisionen"; break;
                case 3: weltName = "Leer (Rutschen baun)"; break;
                case 4: weltName = "Billard"; break;
            }
            
            std::string infoString = 
                "Welt: " + weltName + " | Objekte: " + std::to_string(engine.getKugelAnzahl()) + "\n"
                "Commands:\n"
                "1-4: Welt wechseln | G: Gravity (" + schwerkraftStatus + ") | P: Pause (" + pauseStatus + ")\n"
                "Linksklick: Ball mit speed | Space: Ball at mouse | S: fester Ball | C: Clear all";
            
            if (aktuelleWelt == 2) {
                infoString += "\nKollisionen: " + std::to_string(engine.getKollisionen());
            }
            
            infoText.setString(infoString);
            fenster.draw(infoText);
        }

        fenster.display();
    }

    return 0;
}
