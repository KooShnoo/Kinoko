#pragma once

#include "host/KSystem.hh"

#include <array>
#include <egg/core/SceneManager.hh>

#include <game/system/RaceConfig.hh>
#include <game/system/TimerManager.hh>
#include <string>
#include <vector>

struct GhostTimers {
    std::array<System::Timer, 3> lapTimers;
    System::Timer raceTimer;
    std::string ghostPath;
};

class KSpamSystem final : public KSystem {
public:
    void init() override {}
    void calc() override {}
    bool run() override;
    void parseOptions(int argc, char **argv) override {
        (void) argc;
        (void) argv;
    }
    static void OnInit(System::RaceConfig *config, void *arg);
    static KSpamSystem *CreateInstance();
};