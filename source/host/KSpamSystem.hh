#pragma once

#include "host/KSystem.hh"

#include <egg/core/SceneManager.hh>

#include <game/system/RaceConfig.hh>

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