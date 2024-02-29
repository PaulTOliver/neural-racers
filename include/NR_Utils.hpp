#ifndef __NR_UTILS_HPP__
#define __NR_UTILS_HPP__

#include <sstream>

#include <Box2D.h>
#include <SFML/Graphics.hpp>

constexpr double PI         = 3.14159265;
constexpr double IN_RADIANS = 180.0 / PI;
constexpr double IN_DEGREES = PI / 180.0;

const sf::Uint32 AGENT_RAD      = 5;
const sf::Uint32 AGENT_RES      = 16;
constexpr float  AGENT_SIGHT    = 100.0f;
constexpr float  AGENT_SPEED    = 500.0f;
const sf::Color  AGENT_COLOR    = { 128,   0, 255, 128 };
const sf::Color  SIGHT_COLOR    = { 128, 128, 128,  32 };
const sf::Color  CHAMP_COLOR    = { 204, 153, 255 };
const sf::Color  SELECTED_COLOR = { 204,  53,  55 };
const sf::Color  WALL_COLOR     = { 204, 153, 255 };
const sf::Color  FACTORY_COLOR  = { 204, 153, 255 };
const sf::Color  TEXT_COLOR     = { 204, 153, 255 };
const sf::Color  CHECKPNT_COLOR = { 128, 128, 128, 128 };
const sf::Color  BG_COLOR       = {  16,   0,  32 };

const float _TIME_STEP = 1.0f / 60.0f;
const int32 _VEL_ITERS = 6;
const int32 _POS_ITERS = 2;

const sf::Uint32          WIN_SIZE     = 600;
const sf::VideoMode       VIDEO_MODE   = sf::VideoMode(WIN_SIZE, WIN_SIZE);
const sf::String          WIN_TITLE    = "NeuroRacers v0.1";
const sf::Uint32          WIN_STYLE    = sf::Style::Close;
const sf::ContextSettings WIN_SETTINGS = sf::ContextSettings(0, 0, 4);

ssize_t pMod(ssize_t n, ssize_t d);
float dist(const b2Vec2& a, const b2Vec2 b);
float dotP(const b2Vec2& a, const b2Vec2 b);

class RayCastCallback : public b2RayCastCallback {
public:
    float32 ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float32 fraction);

    const b2Fixture* _fixture  = nullptr;
    float32          _fraction = 0.0f;
};

class QueryCallback : public b2QueryCallback {
public:
    bool ReportFixture(b2Fixture* fixture);

    const b2Fixture* _fixture = nullptr;
};

class TrapCallback : public b2QueryCallback {
public:
    bool ReportFixture(b2Fixture* fixture);

    const b2Fixture* _fixture = nullptr;
};

class Stage;
class ContactListener : public b2ContactListener {
public:
    void BeginContact(b2Contact* contact);

    Stage* _stage = nullptr;
};

template <typename T>
std::string toString(const T& value)
{
    std::stringstream stream;
    stream << value;
    return stream.str();
}

#endif // __NR_UTILS_HPP__
