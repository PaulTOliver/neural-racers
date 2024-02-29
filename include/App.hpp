#ifndef __APP_HPP__
#define __APP_HPP__

#include <Stage.hpp>
#include <Agent.hpp>
#include <CppnExplorer.hpp>
#include <HyperNeat/Population.hpp>
#include <HyperNeat/NeuralNetPrms.hpp>
#include <HyperNeat/NoveltyMetric.hpp>
#include <HyperNeat/PopulationPrms.hpp>

namespace hn = hyperneat;

class App {
public:
    int startup();
    int execute();
    void update();
    void draw();
    void shutdown();

private:
    bool _draw           = true;
    bool _runningWOGfx   = true;
    bool _isPaused       = true;
    bool _isVSyncEnabled = true;

    sf::RenderWindow _window;

    Stage             _stage;
    hn::Vector<Agent> _agents;
    ssize_t           _selectedIdx  = -1;
    sf::Clock         _clock;
    sf::Clock         _globalClock;
    float             _secondsToRun = 0.0f;
    sf::CircleShape   _champ;
    sf::CircleShape   _selected;

    hn::CppnExplorer _cppnEx;

    sf::Font _font;

    sf::Text _updates;
    sf::Text _champFitness;
    sf::Text _avergFitness;

    hn::NeuralNetPrms  _nnPrms;
    hn::PopulationPrms _popPrms;
    hn::Population     _population;

    bool                        _doNoveltySearch   = false;
    size_t                      _completedCircuits = 0;
    hn::NoveltyMetricPrms       _noveltyPrms;
    hn::Vector<sf::CircleShape> _noveltyMarks;
};

#endif // __APP_HPP__
