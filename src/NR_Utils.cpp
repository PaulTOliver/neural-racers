#include <NR_Utils.hpp>
#include <Agent.hpp>
#include <Stage.hpp>

using namespace std;

ssize_t
pMod(ssize_t n, ssize_t d)
{
    return (n % d + d) % d;
}

float
dist(const b2Vec2& a, const b2Vec2 b)
{
    auto x = a.x - b.x;
    auto y = a.y - b.y;

    return sqrt(x * x + y * y);
}

float
dotP(const b2Vec2& a, const b2Vec2 b)
{
    return a.x * b.x + a.y * b.y;
}

float32
RayCastCallback::ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float32 fraction)
{
    if (fixture->GetFilterData().groupIndex == -1 || fixture->IsSensor()) {
        return -1.0f;
    }

    _fixture  = fixture;
    _fraction = 1.0f - fraction;

    return fraction;
}

bool
QueryCallback::ReportFixture(b2Fixture* fixture)
{
    _fixture = fixture;

    return false;
}

bool
TrapCallback::ReportFixture(b2Fixture* fixture)
{
    if (fixture->GetBody()->GetType() != b2_dynamicBody) {
        return true;
    }

    Agent& agent    = *static_cast<Agent*>(fixture->GetUserData());
    agent._isOnTrap = true;
    agent._body->SetLinearVelocity({ 0.0f, 0.0f });
    agent._body->SetAngularVelocity(0.0f);

    return true;
}

void
ContactListener::BeginContact(b2Contact* contact)
{
    b2Fixture* fixA = contact->GetFixtureA();
    b2Fixture* fixB = contact->GetFixtureB();

    if (!fixA->IsSensor() && !fixB->IsSensor()) {
        return;
    }

    Agent*     agent = nullptr;
    b2Fixture* check = nullptr;

    if (fixA->IsSensor()) {
        check = fixA;
        agent = static_cast<Agent*>(fixB->GetUserData());
    } else {
        check = fixB;
        agent = static_cast<Agent*>(fixA->GetUserData());
    }

    if (check->GetUserData() == agent->_nextCheckPoint) {
        agent->incrementCheckPoint(*_stage);
    }
}
