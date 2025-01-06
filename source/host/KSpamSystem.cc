

#include <Common.hh>
#include <cstdio>
#include <cstdlib>
#include <egg/core/SceneManager.hh>

#include <filesystem>
#include <format>
#include <fstream>
#include <game/system/GhostFile.hh>
#include <game/system/RaceConfig.hh>
#include <game/system/RaceManager.hh>
#include <host/SceneCreatorDynamic.hh>
#include <host/KSpamSystem.hh>
#include <print>
#include <ranges>
#include <sstream>

auto formatTimer(const System::Timer &timer) {

    std::ostringstream oss;
    oss << std::setw(2) << std::setfill('0') << timer.min << ":" << std::setw(2)
        << std::setfill('0') << timer.sec << "." << std::setw(3)
        << std::setfill('0') << timer.mil;
    return oss.str();
}

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

        auto rawGhost = System::RawGhostFile(rkg);
        auto ghost = System::GhostFile(rawGhost);

        auto ghostTimers = GhostTimers{
            .lapTimers =
                {
                    ghost.lapTimer(0),
                    ghost.lapTimer(1),
                    ghost.lapTimer(2),
                },
            .raceTimer = ghost.raceTimer(),
            .ghostPath = path.path(),
        };
        s_ghostTimers.emplace_back(ghostTimers);

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

        // verify
        bool desynced = false;
        auto players = System::RaceConfig::Instance()->raceScenario().players;
        using namespace std::ranges::views;
        for (auto [i, correctTimers] : ENUMERATE(s_ghostTimers)) {
            auto player = System::RaceManager::Instance()->player(i);

            if (!player.didFinishLapThisFrame) {
                continue;
            }

            if (player.m_bFinished && player.raceTimer() != correctTimers.raceTimer) {
                desynced = true;
                std::println("race timer desync for player {} {}: \n{} "
                           "measured,\n{} expected.",
                           i, correctTimers.ghostPath,
                           formatTimer(player.raceTimer()),
                           formatTimer(correctTimers.raceTimer));
                continue;
            }

            auto newlap = player.m_currentLap;
            const auto &correctTimer = correctTimers.lapTimers[newlap];
            const auto &actualTimer = player.lapTimers()[newlap];
            if (actualTimer != correctTimer) {
                desynced = true;
                std::println("lap timer {} desync for player {} {}: \n{} "
                            "measured,\n{} expected.",
                            newlap,
                            i, correctTimers.ghostPath,
                            formatTimer(actualTimer),
                            formatTimer(correctTimer));
            }
        }

        if (desynced) {
            return false;
        }
    }

    return true;
}

KSpamSystem *KSpamSystem::CreateInstance() {
    ASSERT(!s_instance);
    s_instance = new KSpamSystem;
    return static_cast<KSpamSystem *>(s_instance);
}
