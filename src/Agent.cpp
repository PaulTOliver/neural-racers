#include <Agent.hpp>

#include <memory>

#include <HyperNeat/Cppn.hpp>
#include <HyperNeat/Population.hpp>

using namespace hn;
using namespace sf;
using namespace std;

size_t
Agent::_currentLifetime = 1500;

void
Agent::create(Stage& stage)
{
    _eyeSight.resize(5);

    _currCheckPoint = &stage._checkPoints.back();
    _nextCheckPoint = &stage._checkPoints.front();

    _shape.setRadius(AGENT_RAD);
    _shape.setOrigin(AGENT_RAD, AGENT_RAD);
    _shape.setFillColor(AGENT_COLOR);

    b2CircleShape agentShape;
    agentShape.m_radius = AGENT_RAD;

    b2FixtureDef agentFixture;
    agentFixture.shape             = &agentShape;
    agentFixture.density           = 1.0f;
    agentFixture.friction          = 0.3f;
    agentFixture.filter.groupIndex = -1;
    agentFixture.userData          = this;

    b2BodyDef agentDef;
    agentDef.type = b2_dynamicBody;

    _body = stage._world.CreateBody(&agentDef);
    _body->CreateFixture(&agentFixture);

    recreate(stage, false);
}

void
Agent::recreate(const Stage& stage, bool createNNet)
{
    _currCheckPoint = &stage._checkPoints.back();
    _nextCheckPoint = &stage._checkPoints.front();

    _completedCircuits = 0;
    _lifetime          = 0;
    _distance          = 0.0;
    _isOnTrap          = false;
    _isOld             = false;

    auto   factoryPos = stage._factory.getPosition();
    double factoryRot = stage._factory.getRotation() / IN_RADIANS;

    _body->SetTransform({factoryPos.x, factoryPos.y}, factoryRot);
    _body->SetLinearVelocity( { 0.0f, 0.0f });
    _body->SetAngularVelocity(0.0f);

    float pX    = _body->GetPosition().x;
    float pY    = _body->GetPosition().y;
    float angle = _body->GetAngle() / IN_DEGREES;

    Transform tr;
    tr.translate(pX, pY);
    tr.rotate(angle);

    float sight  = AGENT_SIGHT;
    float iSight = AGENT_SIGHT * 0.7071f;

    _eyeSight[0] = tr.transformPoint({ -sight,   0.0f});
    _eyeSight[1] = tr.transformPoint({-iSight, iSight});
    _eyeSight[2] = tr.transformPoint({   0.0f,  sight});
    _eyeSight[3] = tr.transformPoint({ iSight, iSight});
    _eyeSight[4] = tr.transformPoint({  sight,   0.0f});

    if (createNNet) {
        _organism->createNeuralNet();
    }
}

void
Agent::setPoints(const Stage& stage, bool passToOrg)
{
    if (!_organism->isOld()) {
        return;
    }

    // Calculate fitness
    double dist = distanceTravelled(stage);

    if (passToOrg) {
        _organism->_fitness = (dist / static_cast<double>(max(_lifetime, (size_t)1)));
    }
}

void
Agent::setBehavior(const Stage& stage)
{
    _organism->getBehavior().at(0) = _body->GetPosition().x;
    _organism->getBehavior().at(1) = _body->GetPosition().y;
    _organism->getBehavior().at(2) = (distanceTravelled(stage) / static_cast<double>(max(_lifetime, (size_t)1)));
}

void
Agent::update(const Stage& stage, bool withNS)
{
    if (withNS) {
        if (_organism->getLifetime() >= (_currentLifetime - 1)) {
            if (!_isOld) {
                _isOld = true;
            }
        } else if (_organism->getLifetime() < (_currentLifetime - 1)) {
            if (_isOld) {
                _isOld = false;
            }
        }
    }

    ++_lifetime;

    if (_isOnTrap) {
        return;
    }

    float pX    = _body->GetPosition().x;
    float pY    = _body->GetPosition().y;
    float angle = _body->GetAngle() / IN_DEGREES;

    Transform tr, rot;
    tr.translate(pX, pY);
    tr.rotate(angle);
    rot.rotate(angle);

    float sight  = AGENT_SIGHT;
    float iSight = AGENT_SIGHT * 0.7071f;

    _eyeSight[0] = tr.transformPoint({ -sight,   0.0f});
    _eyeSight[1] = tr.transformPoint({-iSight, iSight});
    _eyeSight[2] = tr.transformPoint({   0.0f,  sight});
    _eyeSight[3] = tr.transformPoint({ iSight, iSight});
    _eyeSight[4] = tr.transformPoint({  sight,   0.0f});

    for (size_t i = 0; i < 5; ++i) {
        RayCastCallback callback;
        b2Vec2 b2EyeSight = { _eyeSight[i].x, _eyeSight[i].y };
        stage._world.RayCast(&callback, _body->GetPosition(), b2EyeSight);
        _organism->_neuralNet->inputAt(i) = callback._fraction;
    }

    _organism->_neuralNet->cycle();

    Vector2f velocity
        = rot.transformPoint({ 0.0f, static_cast<float>(_organism->_neuralNet->outputAt(1)) * AGENT_SPEED });
    float rotation
        = (((_organism->_neuralNet->outputAt(0) + _organism->_neuralNet->outputAt(2)) * 2.0f - 1.0f) / IN_RADIANS) *
        AGENT_SPEED;

    _body->SetLinearVelocity({velocity.x, velocity.y});
    _body->SetAngularVelocity(rotation);
}

void
Agent::drawOn(sf::RenderWindow& surface)
{
    float pX = _body->GetPosition().x;
    float pY = _body->GetPosition().y;

    _shape.setPosition(pX, pY);

    VertexArray eyeSightLines(Lines);

    for (auto & i : _eyeSight) {
        eyeSightLines.append({{pX, pY}, SIGHT_COLOR});
        eyeSightLines.append({       i, SIGHT_COLOR});
    }

    surface.draw(_shape);
    surface.draw(eyeSightLines);
}

void
Agent::incrementCheckPoint(Stage& stage)
{
    _currCheckPoint = _nextCheckPoint;

    if (_currCheckPoint == &stage._checkPoints.back()) {
        _nextCheckPoint = &stage._checkPoints.front();
    } else {
        ++_nextCheckPoint;
    }

    if (_currCheckPoint == &stage._checkPoints.back()) {
        ++_completedCircuits;
    }
}

double
Agent::distanceTravelled(const Stage& stage)
{
    double pCurr = _currCheckPoint->_value;
    double pLoop = stage._checkPoints.back()._value;

    if (_currCheckPoint == &stage._checkPoints.back()) {
        pCurr -= pLoop;
    }

    double dNext = distanceToCheckpoint(*_nextCheckPoint);
    double sNext = _nextCheckPoint->_segment;

    if (dNext > sNext) {
        _distance = pCurr + (pLoop * _completedCircuits);
    } else {
        double pNext = _nextCheckPoint->_value;
        double f1    = dNext / sNext;
        double f2    = 1.0 - f1;
        _distance    = (pCurr * f1) + (pNext * f2) + (pLoop * _completedCircuits);
    }

    if (_distance < 1.0) {
        _distance = 1.0;
    }

    return _distance;
}

float
Agent::distanceToCheckpoint(const Stage::CheckPoint& cp) const
{
    b2Vec2 p = _body->GetPosition();
    b2Vec2 v = {cp._line[0].position.x, cp._line[0].position.y};
    b2Vec2 w = {cp._line[1].position.x, cp._line[1].position.y};

    float l2 = (w - v).LengthSquared();

    if (l2 == 0.0f) {
        return dist(p, v);
    }

    float t = dotP(p - v, w - v) / l2;

    if (t < 0.0f) {
        return dist(p, v);
    } else if (t > 1.0f) {
        return dist(p, w);
    }

    b2Vec2 projection = v + t * (w - v);

    return dist(p, projection);
}
