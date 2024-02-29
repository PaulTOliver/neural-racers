#ifndef __STAGE_HPP__
#define __STAGE_HPP__

#include <fstream>
#include <iostream>

#include <NR_Utils.hpp>
#include <Hyperneat/Utils/Vector.hpp>

namespace hn = hyperneat;

class Stage {
public:
    int load(const std::string& fileName);
    void update();
    void drawOn(sf::RenderWindow& surface) const;

private:
    class CheckPoint {
    public:
        void calculateSegment(const CheckPoint& prev);

        sf::VertexArray _line    = sf::VertexArray(sf::Lines);
        double          _value   = 0.0;
        double          _segment = 0.0;
        b2Body*         _body    = nullptr;
    };

    b2World _world = { { 0.0f, 0.0f } };
    ContactListener _listener;

    hn::Vector<sf::VertexArray>    _walls;
    hn::Vector<b2Body*>            _wallBodies;
    hn::Vector<b2AABB>             _traps;
    hn::Vector<sf::RectangleShape> _trapRects;
    hn::Vector<CheckPoint>         _checkPoints;

    sf::CircleShape _factory = sf::CircleShape(AGENT_RAD + 2, AGENT_RES);

    friend class App;
    friend class Agent;
};

#endif // __STAGE_HPP__
