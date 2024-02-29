#ifndef __AGENT_HPP__
#define __AGENT_HPP__

#include <Stage.hpp>
#include <SFML/Graphics.hpp>
#include <HyperNeat/Organism.hpp>
#include <HyperNeat/NeuralNet.hpp>
#include <HyperNeat/NoveltyMetric.hpp>

namespace hyperneat {
    class NeuralNetPrms;
    class Genome;
    class Population;
}

namespace hn = hyperneat;

class Agent {
public:
    void create(Stage& stage);
    void recreate(const Stage& stage, bool createNNet);
    void setPoints(const Stage& stage, bool passToOrg);
    void setBehavior(const Stage& stage);
    void update(const Stage& stage, bool withNS);
    void drawOn(sf::RenderWindow& surface);
    void incrementCheckPoint(Stage& stage);

private:
    double distanceTravelled(const Stage& stage);
    float distanceToCheckpoint(const Stage::CheckPoint& cp) const;

    hn::Organism* _organism = nullptr;
    bool          _isOld    = false;

    sf::CircleShape          _shape;
    b2Body*                  _body = nullptr;
    hn::Vector<sf::Vector2f> _eyeSight;

    const Stage::CheckPoint* _currCheckPoint = nullptr;
    const Stage::CheckPoint* _nextCheckPoint = nullptr;

    size_t _completedCircuits = 0;
    size_t _lifetime          = 0;
    double _distance          = 0.0;
    bool   _isOnTrap          = false;

    static size_t _currentLifetime;

    friend class ContactListener;
    friend class TrapCallback;
    friend class App;
};

#endif // __AGENT_HPP__
