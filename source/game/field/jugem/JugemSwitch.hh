#pragma once

#include <Common.hh>

namespace Field {

class JugemSwitch {
public:
    JugemSwitch() : m_isOn(false) {}
    virtual ~JugemSwitch() = default;

    [[nodiscard]] bool isOn() const {
        return m_isOn;
    }

    virtual void init() {
        m_isOn = false;
    }

    virtual void calc() = 0;

protected:
    bool m_isOn;
};

class JugemSwitchReverse : public JugemSwitch {
public:
    JugemSwitchReverse() = default;
    ~JugemSwitchReverse() override = default;

    void init() override {
        m_isOn = false;
        m_activationPercent = 0.0f;
    }

    void calc() override;

private:
    f32 m_activationPercent; ///< Lakitu is activated when this reaches 1.0f
};

} // namespace Field
