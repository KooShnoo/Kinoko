#pragma once

#include "game/system/GhostFile.hh"

#include <functional>
#include <unordered_map>
#include <vector>

namespace System {

/// @addr{0x809BD728}
/// @brief Initializes the player with parameters specified in the provided ghost file.
/// @details In the base game, this class is responsible for managing the race and menu scenarios.
/// The menu scenario mostly pertains to character and vehicle selection in the menus prior to
/// starting a race. In Kinoko, we don't have these menus, so we initialize the race directly
/// through this class.
class RaceConfig : EGG::Disposer {
public:
    struct Player {
    public:
        enum class Type {
            Local = 0, // Inputs managed by ML algorithm
            Ghost = 3, // Inputs managed by ghost
            None = 5,
        };

        Character character;
        Vehicle vehicle;
        Type type;
        bool driftIsAuto;

        // size_t ghostIdx = 0; ///< for multiplayer testing; valid only if Type::Ghost; index of which rawghostfile to use
        size_t playerInputIdx = 0; // todo(ks)
    };

    struct Scenario {
    public:
        enum class GameMode {
            Time_Trial = 2,
            Ghost_Race = 5,
        };

        void init();

        std::vector<Player> players;
        Course course;
    };

    typedef std::function<void(RaceConfig *, void *)> InitCallback;

    void init();
    void initRace();
    void initControllers();
    void initGhost(Player &player, RawGhostFile rawGhost);

    [[nodiscard]] const Scenario &raceScenario() const {
        return m_raceScenario;
    }

    [[nodiscard]] Scenario &raceScenario() {
        return m_raceScenario;
    }

    void setGhost(const u8 *rkg, size_t playerIdx) {
        m_ghosts[playerIdx] = RawGhostFile(rkg);
    }

    static void RegisterInitCallback(const InitCallback &callback, void *arg);

    static RaceConfig *CreateInstance();
    static void DestroyInstance();
    [[nodiscard]] static RaceConfig *Instance();

private:
    RaceConfig();
    ~RaceConfig() override;

    Scenario m_raceScenario;
    std::unordered_map<size_t, RawGhostFile> m_ghosts;

    static RaceConfig *s_instance; ///< @addr{0x809BD728}
    static InitCallback s_onInitCallback;
    static void *s_onInitCallbackArg;
};

} // namespace System
