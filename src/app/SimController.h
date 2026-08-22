#pragma once

#include "domain/SimEngine.h"

namespace fleetsim::app {

class SimController {
public:
    SimController();

    domain::SimEngine& engine();
    const domain::SimEngine& engine() const;

    void start();
    void pause();
    void stepOnce();
    void tick();

    bool isRunning() const;

private:
    domain::SimEngine engine_;
    bool running_{false};
};

}  // namespace fleetsim::app
