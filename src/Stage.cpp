#include <Stage.hpp>

using namespace sf;
using namespace std;

int
Stage::load(const std::string& fileName)
{
    ifstream stageFile(fileName);

    if (!stageFile) {
        return EXIT_FAILURE;
    }

    auto until = numeric_limits<streamsize>::max();

    while (!stageFile.eof()) {
        stageFile.ignore(until, '<');

        string command;
        getline(stageFile, command, '>');

        if (command == "WALL") {
            _walls.emplace_back(LinesStrip);
            hn::Vector<b2Vec2> vertices;

            stageFile.ignore(until, '<');

            while (stageFile.peek() != '/') {
                Vector2f vtxPos;

                stageFile >> vtxPos.x;
                stageFile.ignore(until, ',');
                stageFile >> vtxPos.y;
                stageFile.ignore(until, '<');

                _walls.back().append({ vtxPos, WALL_COLOR });
                vertices.emplace_back(vtxPos.x, vtxPos.y);
            }

            b2ChainShape shape;
            shape.CreateChain(vertices.data(), vertices.size());

            b2BodyDef bodyDef;
            _wallBodies.emplace_back(_world.CreateBody(&bodyDef));
            _wallBodies.back()->CreateFixture(&shape, 0.0f);
        } else if (command == "FACTORY") {
            Vector2f factPos;
            double   factRot = 0.0;

            stageFile.ignore(until, '<');
            stageFile >> factPos.x;
            stageFile.ignore(until, ',');
            stageFile >> factPos.y;
            stageFile.ignore(until, ',');
            stageFile >> factRot;
            stageFile.ignore(until, '<');

            _factory.setPosition(factPos);
            _factory.setRotation(factRot);
            _factory.setOrigin(AGENT_RAD + 2, AGENT_RAD + 2);
            _factory.setFillColor(Color::Black);
            _factory.setOutlineColor(FACTORY_COLOR);
            _factory.setOutlineThickness(2.0f);
        } else if (command == "CHECKPOINT") {
            stageFile.ignore(until, '<');

            while (stageFile.peek() != '/') {
                _checkPoints.emplace_back();

                Vector2f vtx1Pos;
                Vector2f vtx2Pos;
                b2Vec2   shVx[2];

                stageFile >> vtx1Pos.x;
                shVx[0].x  = vtx1Pos.x;
                stageFile.ignore(until, ',');
                stageFile >> vtx1Pos.y;
                shVx[0].y  = vtx1Pos.y;
                stageFile.ignore(until, ',');
                stageFile >> vtx2Pos.x;
                shVx[1].x  = vtx2Pos.x;
                stageFile.ignore(until, ',');
                stageFile >> vtx2Pos.y;
                shVx[1].y  = vtx2Pos.y;
                stageFile.ignore(until, ',');
                stageFile >> _checkPoints.back()._value;
                stageFile.ignore(until, '<');

                _checkPoints.back()._line.append({ vtx1Pos, CHECKPNT_COLOR });
                _checkPoints.back()._line.append({ vtx2Pos, CHECKPNT_COLOR });

                _checkPoints.front().calculateSegment(_checkPoints.back());

                b2ChainShape cpShape;
                cpShape.CreateChain(shVx, 2);

                b2FixtureDef cpFixture;
                cpFixture.shape    = &cpShape;
                cpFixture.isSensor = true;

                b2BodyDef cpDef;
                _checkPoints.back()._body = _world.CreateBody(&cpDef);
                _checkPoints.back()._body->CreateFixture(&cpFixture);

                if (_checkPoints.size() > 1) {
                    auto& prev = _checkPoints[_checkPoints.size() - 2];

                    _checkPoints.back().calculateSegment(prev);
                }
            }

            for (auto & i : _checkPoints) {
                i._body->GetFixtureList()->SetUserData(&i);
            }
        } else if (command == "TRAP") {
            stageFile.ignore(until, '<');

            while (stageFile.peek() != '/') {
                sf::Vector2f v1;
                sf::Vector2f v2;

                stageFile >> v1.x;
                stageFile.ignore(until, ',');
                stageFile >> v1.y;
                stageFile.ignore(until, ',');
                stageFile >> v2.x;
                stageFile.ignore(until, ',');
                stageFile >> v2.y;
                stageFile.ignore(until, '<');

                _trapRects.emplace_back(sf::Vector2f(v2.x - v1.x, v2.y - v1.y));
                _trapRects.back().setPosition(v1);
                _trapRects.back().setFillColor({ 128, 32, 32, 128 });

                _traps.emplace_back();
                _traps.back().lowerBound.Set(v1.x, v1.y);
                _traps.back().upperBound.Set(v2.x, v2.y);
            }
        }
    }

    _world.SetContactListener(&_listener);
    _listener._stage = this;

    return EXIT_SUCCESS;
}

void
Stage::update()
{
    _world.Step(_TIME_STEP, _VEL_ITERS, _POS_ITERS);
}

void
Stage::drawOn(RenderWindow& surface) const
{
    for (auto& i : _trapRects) {
        surface.draw(i);
    }

    for (auto& i : _walls) {
        surface.draw(i);
    }

    surface.draw(_factory);

    for (auto& i : _checkPoints) {
        surface.draw(i._line);
    }
}

void
Stage::CheckPoint::calculateSegment(const CheckPoint& prev)
{
    float32 a0X = prev._line[0].position.x;
    float32 a0Y = prev._line[0].position.y;
    float32 a1X = prev._line[1].position.x;
    float32 a1Y = prev._line[1].position.y;

    float32 b0X = _line[0].position.x;
    float32 b0Y = _line[0].position.y;
    float32 b1X = _line[1].position.x;
    float32 b1Y = _line[1].position.y;

    float32 dA0B0 = dist( { a0X, a0Y }, { b0X, b0Y });
    float32 dA0B1 = dist( { a0X, a0Y }, { b1X, b1Y });
    float32 dA1B0 = dist( { a1X, a1Y }, { b0X, b0Y });
    float32 dA1B1 = dist( { a1X, a1Y }, { b1X, b1Y });

    _segment = min(min(dA0B0, dA0B1), min(dA1B0, dA1B1));
}
