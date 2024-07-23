#include <cstddef>
#include <cstdio>
#include <print>
#include <cstring>
#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <game/system/RaceConfig.hh>

#include <egg/core/SceneManager.hh>
#include <game/kart/KartObjectManager.hh>
#include <game/system/KPadDirector.hh>
#include <game/system/RaceManager.hh>
#include <host/SceneCreatorDynamic.hh>

static u8* common;
static size_t commonLen;
static u8* course;
static size_t courseLen;

void setFiles(u8* _common, size_t _commonLen, u8* _course, size_t _courseLen) {
    common = _common;
    commonLen = _commonLen;
    course = _course;
    courseLen = _courseLen;
}

namespace Abstract {
namespace File {
u8 *Load(const char *path, size_t &size) {
    // (fileName[0] === "/") ? files.common : files.course;
    if (path[0] == '/') {
        size = commonLen;
        return common;
    } else {
        size = courseLen;
        return course;
    }
}
}
}

void JSlog(const char* /* msg */){};
namespace Pilz {

static EGG::SceneManager *sceneMgr;
constexpr const auto defaultRaceSelections = System::RaceConfig::RaceSelections{
    .course = Course::N64_DKs_Jungle_Parkway,
    .character = Character::Mario,
    .vehicle = Vehicle::Mach_Bike,
    .type = System::RaceConfig::Player::Type::Local,
};

class Controller: public System::KPadController {
public:
    System::ControlSource controlSource() override { return System::ControlSource::Classic; }

    void reset(bool driftIsAuto) override {
        m_driftIsAuto = driftIsAuto;
        m_raceInputState.reset();
        m_connected = true;
    }

    void setButtons(u16 newButtons, f32 stickX, f32 stickY, System::Trick trick) {
        m_raceInputState.buttons = newButtons;
        m_raceInputState.stick = EGG::Vector2f(stickX, stickY);
        m_raceInputState.trick = trick;
    }
    u16 getButtons() { return m_raceInputState.buttons; }
};

struct PlayerTransform {
    s32 timer;
    std::array<f32, 3> pos;
    std::array<f32, 4> rot;
};


static Controller *controller;
static PlayerTransform *playerTransform;
static std::optional<System::RaceConfig::RaceSelections> selections;

void reinit() {
    sceneMgr->reinitCurrentScene();
}


void updateTransform() {
    const auto player = Kart::KartObjectManager::Instance()->object(0);
    const auto &pos = player->pos();
    const auto &rot = player->mainRot();
    // auto rot = quatToEuler(mainRot);
    // const auto &pose = player->pose();
    playerTransform->timer = System::RaceManager::Instance()->getTimer();
    playerTransform->pos = {pos.x, pos.y, pos.z};
    playerTransform->rot = {rot.v.x, rot.v.y, rot.v.z, rot.w};
}

void init() {
    const bool initialized = sceneMgr && sceneMgr->currentSceneId() != 0;
    if (initialized) {
        JSlog("reinitng");
        reinit();
        return;
    }

    playerTransform = new PlayerTransform;
    controller = new Controller;
    selections = std::nullopt;
    auto *sceneCreator = new Host::SceneCreatorDynamic;
    sceneMgr = new EGG::SceneManager(sceneCreator);
    controller->setDriftIsAuto(false);

    sceneMgr->changeScene(0);
    System::KPadDirector::setController(Pilz::controller);
}

void setButtons(u16 buttons, f32 stickX, f32 stickY, System::Trick trick) {
    controller->setButtons(buttons, stickX, stickY, trick);
}

void setCourse(Course course) {
    if (!selections.has_value()) {
        selections = defaultRaceSelections;
    }
    selections->course = course;
    System::RaceConfig::setSelections(selections.value());
}

PlayerTransform* calc() {
    sceneMgr->calc();
    updateTransform();
    return playerTransform;
}

}

// int main() {
//     setbuf(stdout, NULL);
//     JSlog("Hi there, I'm calling because you recently called main(). Please be aware that we have recently moved to init(), and you should call us there instead. Thank you!");
//     return 0;
// }