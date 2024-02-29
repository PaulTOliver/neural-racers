#include <App.hpp>
#include <HyperNeat/Cppn.hpp>
#include <tbb/parallel_for.h>
#include <HyperNeat/Utils/Thread.hpp>
#include <HyperNeat/Utils/LoadFile.hpp>
#include <HyperNeat/Utils/SaveFile.hpp>

using namespace hn;
using namespace sf;
using namespace tbb;
using namespace std;

int
App::startup()
{
    ifstream paramsFile("parameters");
    ifstream popFile("current.population");

    if (!paramsFile) {
        return EXIT_FAILURE;
    }

    cout << "Type hours to run sim.: ";
    cin  >> _secondsToRun;
    _secondsToRun *= (60 * 60);

    bool popLoaded = false;

    if (popFile) {
        popLoaded = true;

        LoadFile lFile(popFile);
        lFile.loadPopulation(_population);

        _popPrms         = _population.getPopulationPrms();
        _doNoveltySearch = _population.isNoveltyMetricSet();
    }

    auto until = numeric_limits<streamsize>::max();

    while (!paramsFile.eof()) {
        paramsFile.ignore(until, '<');

        string command;
        getline(paramsFile, command, '>');

        if (!popLoaded && command == "POPULATION_PARAMETERS") {
            paramsFile.ignore(until, '<');

            while (paramsFile.peek() != '/') {
                string subCommand;
                getline(paramsFile, subCommand, '>');

                if (subCommand == "populationSize") {
                    paramsFile >> _popPrms._popSize;
                } else if (subCommand == "seed") {
                    paramsFile >> _popPrms._seed;
                } else if (subCommand == "weightRange") {
                    paramsFile >> _popPrms._weightRange;
                } else if (subCommand == "c1Disjoint") {
                    paramsFile >> _popPrms._c1Disjoint;
                } else if (subCommand == "c3WeightDifference") {
                    paramsFile >> _popPrms._c3WeightDifference;
                } else if (subCommand == "populationSize") {
                    paramsFile >> _popPrms._popSize;
                } else if (subCommand == "initialDistanceThreshold") {
                    paramsFile >> _popPrms._initialDistanceThreshold;
                } else if (subCommand == "distanceThresholdShift") {
                    paramsFile >> _popPrms._distanceThresholdShift;
                } else if (subCommand == "sexualReproductionRate") {
                    paramsFile >> _popPrms._sexualReproductionRate;
                } else if (subCommand == "weightMutationRate") {
                    paramsFile >> _popPrms._weightMutationRate;
                } else if (subCommand == "weightDeviation") {
                    paramsFile >> _popPrms._weightDeviation;
                } else if (subCommand == "interspeciesMatingRate") {
                    paramsFile >> _popPrms._interspeciesMatingRate;
                } else if (subCommand == "geneDisablingRatio") {
                    paramsFile >> _popPrms._geneDisablingRatio;
                } else if (subCommand == "linkMutationRate") {
                    paramsFile >> _popPrms._linkMutationRate;
                } else if (subCommand == "nodeMutationRate") {
                    paramsFile >> _popPrms._nodeMutationRate;
                } else if (subCommand == "targetSpeciesCount") {
                    paramsFile >> _popPrms._targetSpeciesCount;
                } else if (subCommand == "eligibilityRatio") {
                    paramsFile >> _popPrms._eligibilityRatio;
                } else if (subCommand == "minimumLifetime") {
                    paramsFile >> _popPrms._minimumLifetime;
                } else if (subCommand == "replBeforeReorganization") {
                    paramsFile >> _popPrms._replBeforeReorganization;
                }

                paramsFile.ignore(until, '<');
            }
        } else if (command == "NEURAL_NET_PARAMETERS") {
            paramsFile.ignore(until, '<');

            while (paramsFile.peek() != '/') {
                string subCommand;
                getline(paramsFile, subCommand, '>');

                if (subCommand == "testGridLevel") {
                    paramsFile >> _nnPrms._testGridLevel;
                } else if (subCommand == "maxQuadTreeLevel") {
                    paramsFile >> _nnPrms._maxQuadTreeLevel;
                } else if (subCommand == "minQuadTreeLevel") {
                    paramsFile >> _nnPrms._minQuadTreeLevel;
                } else if (subCommand == "bandPruningThreshold") {
                    paramsFile >> _nnPrms._bandPruningThreshold;
                } else if (subCommand == "varianceThreshold") {
                    paramsFile >> _nnPrms._varianceThreshold;
                } else if (subCommand == "divisionThreshold") {
                    paramsFile >> _nnPrms._divisionThreshold;
                } else if (subCommand == "searchIterations") {
                    paramsFile >> _nnPrms._iterations;
                }

                paramsFile.ignore(until, '<');
            }
        } else if (command == "NOVELTY_SEARCH_PARAMETERS") {
            paramsFile.ignore(until, '<');

            while (paramsFile.peek() != '/') {
                string subCommand;
                getline(paramsFile, subCommand, '>');

                if (subCommand == "doNoveltySearch") {
                    paramsFile >> _doNoveltySearch;
                } else if (subCommand == "noveltyThreshold") {
                    paramsFile >> _noveltyPrms._noveltyThreshold;
                } else if (subCommand == "referenceOrganisms") {
                    paramsFile >> _noveltyPrms._referenceOrganisms;
                }

                _noveltyPrms._characterizationSize = 3;

                paramsFile.ignore(until, '<');
            }
        } else if (command == "STAGE_PARAMETERS") {
            paramsFile.ignore(until, '<');

            while (paramsFile.peek() != '/') {
                string subCommand;
                getline(paramsFile, subCommand, '>');

                if (subCommand == "file") {
                    string fileName;
                    paramsFile >> fileName;

                    if (_stage.load(fileName) == EXIT_FAILURE) {
                        return EXIT_FAILURE;
                    }
                } else if (subCommand == "draw") {
                    paramsFile >> _draw;
                }

                paramsFile.ignore(until, '<');
            }
        }
    }

    if (!_draw) {
        _isVSyncEnabled = false;
        _isPaused       = false;
    }

    _nnPrms._inputMap = {
        { -1.0,  -0.5 },
        { -0.7,   0.2 },
        {  0.0,   0.5 },
        {  0.7,   0.2 },
        {  1.0,  -0.5 }
    };

    _nnPrms._outputMap = {
        { -0.5, -0.5 },
        {  0.0,  0.0 },
        {  0.5, -0.5 }
    };

    if (!popLoaded) {
        _population.create(_popPrms, _nnPrms);
    }

    if (_doNoveltySearch && !_population.isNoveltyMetricSet()) {
        _population.setNoveltyMetric(_noveltyPrms);
    } else if (!_doNoveltySearch && _population.isNoveltyMetricSet()) {
        _population.clearNoveltyMetric();
    }

    _agents.resize(_popPrms._popSize, Agent());

    for (size_t i = 0; i < _agents.size(); ++i) {
        _agents[i]._organism = &_population.getOrganism(i);
        _agents[i].create(_stage);
    }

    // Agent::_currentLifetime = (_stage._checkPoints.back()._value / 1.75);

    _champ.setFillColor(Color::Transparent);
    _champ.setOutlineColor(CHAMP_COLOR);
    _champ.setOutlineThickness(2);
    _champ.setRadius(AGENT_RAD + 2);
    _champ.setOrigin(AGENT_RAD + 2, AGENT_RAD + 2);

    _selected = _champ;
    _selected.setOutlineColor(SELECTED_COLOR);

    // Print NoveltyMarks
    if (_population.isNoveltyMetricSet()) {
        for (auto& i : _population.getNoveltyMetric().getArchive()) {
            _noveltyMarks.emplace_back();
            _noveltyMarks.back().setFillColor({ 255, 255, 0, 10 });
            _noveltyMarks.back().setRadius(AGENT_RAD + 2);
            _noveltyMarks.back().setOrigin(AGENT_RAD + 2, AGENT_RAD + 2);
            _noveltyMarks.back().setPosition(i[0], i[1]);
        }
    }

    // Prepare text
    if (!_font.loadFromFile("C:/Windows/Fonts/font_1.ttf")) {
        return EXIT_FAILURE;
    }

    _updates.setFont(_font);
    _champFitness.setFont(_font);
    _avergFitness.setFont(_font);

    _updates.setCharacterSize(11);
    _champFitness.setCharacterSize(11);
    _avergFitness.setCharacterSize(11);

    _updates.setPosition(5, 5);
    _champFitness.setPosition(5, 18);
    _avergFitness.setPosition(5, 31);

    _updates.setColor(TEXT_COLOR);
    _champFitness.setColor(TEXT_COLOR);
    _avergFitness.setColor(TEXT_COLOR);

    return EXIT_SUCCESS;
}

int
App::execute()
{
    if (!_draw) {
        hn::Thread runner([&]() {
            while (_runningWOGfx) {
                update();
            }
        });

        cin.get();
        cin.get();

        _runningWOGfx = false;
        runner.join();

        return EXIT_SUCCESS;
    }

    _window.create(VIDEO_MODE, WIN_TITLE, WIN_STYLE, WIN_SETTINGS);

    _clock.restart();
    _globalClock.restart();

    while (_window.isOpen()) {
        if (!_isPaused) {
            update();
        }

        Event event;

        while (_window.pollEvent(event)) {
            if (event.type == Event::Closed) {
                _cppnEx.shutdown();
                _window.close();
            } else if (event.type == Event::KeyPressed) {
                if (event.key.code == Keyboard::Space) {
                    _isPaused = !_isPaused;
                } else if (event.key.code == Keyboard::V) {
                    _isVSyncEnabled = !_isVSyncEnabled;

                    if (_isVSyncEnabled) {
                        _window.setVerticalSyncEnabled(true);
                    } else {
                        _window.setVerticalSyncEnabled(false);
                    }
                } else if (event.key.code == Keyboard::L) {
                    if (_selectedIdx != -1) {
                        if (_population.isOrganismLocked(_selectedIdx)) {
                            _population.unlockOrganism(_selectedIdx);
                        } else {
                            _population.lockOrganism(_selectedIdx);
                        }
                    }
                } else if (event.key.code == Keyboard::F) {
                    if (_selectedIdx != -1) {
                        if (_population.isOrganismFrozen(_selectedIdx)) {
                            _population.unfreezeOrganism(_selectedIdx);
                        } else {
                            _population.freezeOrganism(_selectedIdx);
                        }
                    }
                } else if (event.key.code == Keyboard::C) {
                    if (_selectedIdx != -1) {
                        auto& genome = _population.getOrganism(_selectedIdx).getGenome();
                        _cppnEx.run(genome, &_nnPrms);
                    }
                }
            } else if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Left) {
                float cX = static_cast<float>(Mouse::getPosition(_window).x);
                float cY = static_cast<float>(Mouse::getPosition(_window).y);

                b2AABB aabb;
                aabb.lowerBound = { cX - 0.5f, cY - 0.5f };
                aabb.upperBound = { cX + 0.5f, cY + 0.5f };

                QueryCallback qCallback;

                _stage._world.QueryAABB(&qCallback, aabb);

                if (qCallback._fixture) {
                    if (qCallback._fixture->GetFilterData().groupIndex == -1) {
                        Agent* agent    = static_cast<Agent*>(qCallback._fixture->GetUserData());
                        size_t agentIdx = 0;

                        while (&_agents[agentIdx] != agent) {
                            ++agentIdx;
                        }

                        _selectedIdx = agentIdx;

                        size_t oldOrg      = 0;
                        double averageDist = 0.0;

                        for (auto& i : _agents) {
                            if (i._organism->isOld()) {
                                ++oldOrg;
                                averageDist += i._distance;
                            }
                        }

                        averageDist /= static_cast<double>(oldOrg);

                        cout.setf(ios::boolalpha);
                        cout << endl;
                        cout << "completedCircuits : " << _agents[agentIdx]._completedCircuits << endl;
                        cout << "lifetime          : " << _agents[agentIdx]._lifetime << endl;
                        cout << "distance          : " << _agents[agentIdx]._distance << endl;
                        cout << "isOnTrap          : " << _agents[agentIdx]._isOnTrap << endl;
                        cout << "isOld             : " << _agents[agentIdx]._organism->isOld() << endl;
                        // cout << "noveltyScore      : " << _agents[agentIdx]._organism->getBehavior().getNoveltyScore() << endl;
                        // cout << "criteriaReached   : " << _agents[agentIdx]._organism->getBehavior()._criteriaReached << endl;
                        cout << "averageDist       : " << averageDist << endl;
                        cout << endl;
                    } else {
                        _selectedIdx = -1;
                    }
                } else {
                    _selectedIdx = -1;
                }
            }
        }

        draw();
    }

    return EXIT_SUCCESS;
}

void
App::update()
{
    parallel_for(size_t(0), _agents.size(), size_t(1), [&](size_t i) {
        if (!_agents[i]._organism->isBeingGenerated()) {
            _agents[i].update(_stage, _doNoveltySearch);
        }
    });

    _population.update([&]() {
        if (!_doNoveltySearch) {
            for (auto& i : _agents) {
                i.setPoints(_stage, true);
            }
        } else {
            for (auto& i : _agents) {
                i.setBehavior(_stage);
            }
        }
    }, [&]() {
        _agents[_population.getLastReplacement()->getIndex()].recreate(_stage, true);
    });

    if (_doNoveltySearch) {
        size_t maxCircuits = 0;

        for (auto& i : _agents) {
            maxCircuits = max(maxCircuits, i._completedCircuits);
        }

        if (maxCircuits != _completedCircuits) {
            _completedCircuits = maxCircuits;
            size_t newLifetime = (_stage._checkPoints.back()._value / 1.75) * (_completedCircuits + 1);

            _population.setMinimumLifetime(newLifetime);
            Agent::_currentLifetime = newLifetime;
        }
    }

    for (auto& i : _stage._traps) {
        TrapCallback trapCallback;
        _stage._world.QueryAABB(&trapCallback, i);
    }

    if (_doNoveltySearch) {
        auto& archive = _population.getNoveltyMetric().getArchive();

        if (archive.size() > _noveltyMarks.size()) {
            for (size_t i = _noveltyMarks.size(); i < archive.size(); ++i) {
                _noveltyMarks.emplace_back();
                _noveltyMarks.back().setFillColor({ 255, 255, 0, 10 });
                _noveltyMarks.back().setRadius(AGENT_RAD + 2);
                _noveltyMarks.back().setOrigin(AGENT_RAD + 2, AGENT_RAD + 2);
                _noveltyMarks.back().setPosition(archive[i][0], archive[i][1]);
            }
        }
    }

    _stage.update();

    if (_population.getUpdates() % 10000 == 0) {
        ofstream popFile("current.population");
        SaveFile sFile(popFile);
        sFile.savePopulation(_population, true);

        using chrono::system_clock;
        system_clock::time_point today = system_clock::now();

        time_t tt = system_clock::to_time_t(today);
        cout << "> Saved on       : " << ctime(&tt);
        cout << "> Champ Fitness  : " << _population.getChampion()._fitness << endl;
        cout << "> Comp. Circuits : " << _completedCircuits << endl;

        if (_population.isNoveltyMetricSet()) {
            cout << "> Archive Size   : " << _population.getNoveltyMetric().getArchive().size() << endl << endl;
        } else {
            cout << endl;
        }
    }

    if (_globalClock.getElapsedTime().asSeconds() > _secondsToRun) {
        _window.close();
        _runningWOGfx = false;
    }
}

void
App::draw()
{
    if (!_isVSyncEnabled) {
        if (_clock.getElapsedTime().asSeconds() < 1.0f / 60.0f) {
            return;
        }

        _clock.restart();
    }

    _window.clear(BG_COLOR);

    _stage.drawOn(_window);

    for (auto& i : _noveltyMarks) {
        _window.draw(i);
    }

    for (size_t i = 0, end = _agents.size(); i < end; ++i) {
        if (!_population.isOrganismBeingGenerated(i)) {
            _agents[i].drawOn(_window);
        }
    }

    // Highlight champ
    double  champFtn      = _population.getChampion()._fitness;
    ssize_t champIdx      = _population.getChampion().getIndex();
    double averageFitness = _population.getAverageFitness();

    if (champIdx != -1) {
        auto cPos = _agents[champIdx]._body->GetPosition();
        _champ.setPosition(cPos.x, cPos.y);
        _window.draw(_champ);
    }

    // Highlight selected
    if (_selectedIdx != -1) {
        auto sPos = _agents[_selectedIdx]._body->GetPosition();
        _selected.setPosition(sPos.x, sPos.y);
        _window.draw(_selected);
    }

    // Print text
    _updates.setString     ("Simulation updates : " + toString(_population.getUpdates()));
    _champFitness.setString("Champ's fitness    : " + toString(champFtn));
    _avergFitness.setString("Average fitness    : " + toString(averageFitness));

    _window.draw(_updates);
    _window.draw(_champFitness);
    _window.draw(_avergFitness);

    _window.display();
}

void
App::shutdown()
{
    _population.shutdown(true);

    ofstream popFile("current.population");
    SaveFile sFile(popFile);
    sFile.savePopulation(_population);

    cout << "SHUTTING DOWN!" << endl;
}

int
main()
{
    App app;

    if (app.startup() == EXIT_FAILURE || app.execute() == EXIT_FAILURE) {
        return EXIT_FAILURE;
    } else {
        app.shutdown();
        return EXIT_SUCCESS;
    }
}
