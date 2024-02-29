EXE 	:= bin/NeuroRacers.exe
SOURCES := $(wildcard src/*.cpp)
OBJECTS := $(patsubst src/%.cpp,obj/%.o,$(SOURCES))
DEPS    := $(patsubst %.o,%.d,$(OBJECTS))
CFLAGS  := -c -O3 -Wall -std=c++11 -fno-strict-aliasing -MMD -DSFML_DYNAMIC -Iinclude -IC:/TBB/include -IC:/Projects/Libraries/HyperNeat/include -IC:/Projects/Libraries/HyperNeat/plugins/CppnExplorer/include -IC:/Box2D/include -IC:/SFML/include -IC:/cereal/include
LFLAGS  := -LC:/TBB/bin -LC:/Projects/Libraries/HyperNeat/lib -LC:/Projects/Libraries/HyperNeat/plugins/CppnExplorer/lib -lHyperNeat -lCppnExplorer -LC:/Box2D/lib -LC:/SFML/lib -ltbb -lBox2D.dll -lsfml-graphics -lsfml-window -lsfml-system

all: $(OBJECTS)
	g++ -o $(EXE) $(OBJECTS) $(LFLAGS)

-include $(DEPS)

$(OBJECTS):
	g++ $(CFLAGS) $(patsubst obj/%.o,src/%.cpp,$@) -o $@

rebuild: clean all

clean:
	del obj\*.o
	del obj\*.d
	del bin\*.exe
