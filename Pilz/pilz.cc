#include "Common.hh"
#include "game/system/KPadController.hh"
#include <cassert>
#include <cstddef>
#include <egg/math/Matrix.hh>
#include <egg/math/Quat.hh>
#include <game/kart/KartObjectProxy.hh>
#include <game/system/RaceConfig.hh>
#include <optional>

#include <array>
#include <cstdio>
#include <cstdlib>
#include <egg/core/SceneManager.hh>
#include <egg/math/Vector.hh>
#include <game/kart/KartObjectManager.hh>
#include <game/system/KPadDirector.hh>
#include <game/system/RaceManager.hh>
#include <host/SceneCreatorDynamic.hh>


void JSlog(const char* msg);
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

    void setButtons(u16 newButtons, f32 stickX, f32 stickY) {
        m_raceInputState.buttons = newButtons;
        m_raceInputState.stick = EGG::Vector2f(stickX, stickY);
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

__attribute__((export_name("init")))
void init() {
    const bool initialized = sceneMgr->currentSceneId() != 0;
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



__attribute__((export_name("alloc")))
u8* alloc(u32 size) { return new u8[size]; }

__attribute__((export_name("setButtons")))
void setButtons(u16 buttons, f32 stickX, f32 stickY) { controller->setButtons(buttons, stickX, stickY); }

__attribute__((export_name("setCourse")))
void setCourse(Course course) {
    if (!selections.has_value()) {
        selections = defaultRaceSelections;
    }
    selections->course = course;
    System::RaceConfig::setSelections(selections.value());
}

__attribute__((export_name("calc")))
void* calc() {
    sceneMgr->calc();
    updateTransform();
    return playerTransform;
}

}

int main() {
    setbuf(stdout, NULL);
    JSlog("Hi there, I'm calling because you recently called main(). Please be aware that we have recently moved to init(), and you should call us there instead. Thank you!");
    return 0;
}