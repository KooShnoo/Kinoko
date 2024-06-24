#include "Common.hh"
#include "game/system/KPadController.hh"
#include <game/kart/KartObjectProxy.hh>
#include <string>

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

class PilzController: public System::KPadController {
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

    static PilzController *Instance() { return static_cast<PilzController*>(System::KPadDirector::Instance()->m_controller); }
    // u16 JSButtons;
};

__attribute__((export_name("alloc")))
u8* alloc(u32 size) { return new u8[size]; }

__attribute__((export_name("setButtons")))
void setButtons(u16 buttons, f32 stickX, f32 stickY) { PilzController::Instance()->setButtons(buttons, stickX, stickY); }

__attribute__((export_name("logRots")))
const char* logRots() {
    const auto &fullRot = Kart::KartObjectManager::Instance()->object(0)->fullRot();
    const auto &mainRot = Kart::KartObjectManager::Instance()->object(0)->mainRot();
    const auto msg = std::format("fullRot:{:08.2f} {:08.2f} {:08.2f} {:08.2f}\nmainRot: {:08.2f} {:08.2f} {:08.2f} {:08.2f}",
        fullRot.v.x,
        fullRot.v.y,
        fullRot.v.z,
        fullRot.w,
        mainRot.v.x,
        mainRot.v.y,
        mainRot.v.z,
        mainRot.w
    );
    const auto str = new std::string(msg);
    return str->c_str();
}

static EGG::SceneManager *sceneMgr;
struct PilzPlayerTransform {
    PilzPlayerTransform(): pos({0}), timer(0) {}

    std::array<f32, 3> pos;
    s32 timer;
    std::array<f32, 3> rot;
    static PilzPlayerTransform *instance;

    static void updateTransform() {
        const auto &pos = Kart::KartObjectManager::Instance()->object(0)->pos();
        PilzPlayerTransform::instance->pos = {pos.x, pos.y, pos.z};
        // const auto front = Kart::KartObjectManager::Instance()->object(0)->bodyFront();
        // PilzPlayerTransform::instance->rot = {front.x, front.y, front.z};
        PilzPlayerTransform::instance->timer = System::RaceManager::Instance()->getTimer();
    }
};

__attribute__((export_name("calc")))
void* calc() {
    sceneMgr->calc();
    PilzPlayerTransform::updateTransform();
    return PilzPlayerTransform::instance;
}


int main() {
    auto *sceneCreator = new Host::SceneCreatorDynamic;
    sceneMgr = new EGG::SceneManager(sceneCreator);
    sceneMgr->changeScene(0);

    auto controller = new PilzController;
    controller->setDriftIsAuto(false);
    System::KPadDirector::SetController(controller);

    PilzPlayerTransform::instance = new PilzPlayerTransform();

    setbuf(stdout, NULL);
    JSlog("initialized!");
    return 0;
}