#include "GarageDoorSM.h"
#include "Observer.h"

#include <catch2/catch.hpp>

using tsm::AsyncExecWithObserver;
using tsm::BlockingObserver;
using tsm::Hsm;
using tsm::SingleThreadedHsm;

using tsmtest::GarageDoorHsm;

/// A "Blocking" Observer with Async Execution Policy
template<typename StateType>
using AsyncBlockingObserver =
  tsm::AsyncExecWithObserver<StateType, BlockingObserver>;

///
/// GarageDoorHsm is the state machine definition. It has knowledge of the Hsm
/// hierarchy, its states, events and sub-Hsms if any. The relationships between
/// Hsms (parentHsm_) is also setup here. Mix the Async observer and the
/// GarageDoor Hsminition to get an asynchronous garage door state machine
/// that notifies a listener at the end of processing each event.
///
using GarageDoorFsmSeparateThread = AsyncBlockingObserver<GarageDoorHsm>;

TEST_CASE("TestGarageDoorSM - testGarageDoorSeparateThreadPolicy")
{
    auto sm = std::make_shared<GarageDoorFsmSeparateThread>();

    sm->startSM();

    sm->wait();
    REQUIRE(sm->getCurrentState() == &sm->doorClosed);

    sm->sendEvent(sm->click_event);
    sm->wait();
    REQUIRE(sm->getCurrentState() == &sm->doorOpening);

    sm->sendEvent(sm->topSensor_event);
    sm->wait();
    REQUIRE(sm->getCurrentState() == &sm->doorOpen);

    sm->stopSM();
}

TEST_CASE("TestGarageDoorSM - testGarageDoorSingleThreadPolicy")
{
    auto sm = std::make_shared<SingleThreadedHsm<GarageDoorHsm>>();

    sm->sendEvent(sm->click_event);
    sm->sendEvent(sm->topSensor_event);

    sm->startSM();
    REQUIRE(sm->getCurrentState() == &sm->doorClosed);

    sm->step();
    REQUIRE(sm->getCurrentState() == &sm->doorOpening);

    sm->step();
    REQUIRE(sm->getCurrentState() == &sm->doorOpen);

    sm->stopSM();
}
