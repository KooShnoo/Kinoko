

#include <Common.hh>
#include <cstdio>
#include <egg/core/SceneManager.hh>

#include <filesystem>
#include <fstream>
#include <game/system/RaceConfig.hh>
#include <host/SceneCreatorDynamic.hh>
#include <host/KSpamSystem.hh>
#include <ranges>


void KSpamSystem::OnInit(System::RaceConfig *config, void *arg) {
    (void)arg;

    config->raceScenario().course = Course::Luigi_Circuit;

    for (const auto [i, path] : ENUMERATE(std::filesystem::directory_iterator("../rkgs"))) {

        if (i > 40) {
            return;
        }

        auto fileSize = path.file_size();
        auto file = std::ifstream(path.path());
        
        auto rkg = new u8[fileSize];
        file.read((char *) rkg, fileSize);
        file.close();

        config->setGhost(rkg, i);
        delete[] rkg;

        config->raceScenario().players.emplace_back();
        config->raceScenario().players.back().type = System::RaceConfig::Player::Type::Ghost;
    }
}

/// @brief Executes a run.
/// @return Whether the run was successful or not.
bool KSpamSystem::run() {
    setbuf(stdout, NULL);
    auto *sceneCreator = new Host::SceneCreatorDynamic;
    auto *sceneMgr = new EGG::SceneManager(sceneCreator);
    System::RaceConfig::RegisterInitCallback(OnInit, nullptr);

    // Creates the root scene, which creates the race scene, which is a game scene
    sceneMgr->changeScene(0);

    for (auto _ : std::ranges::views::iota(5000)) {
        sceneMgr->calc();
    }

    return true;
}

KSpamSystem *KSpamSystem::CreateInstance() {
    ASSERT(!s_instance);
    s_instance = new KSpamSystem;
    return static_cast<KSpamSystem *>(s_instance);
}
