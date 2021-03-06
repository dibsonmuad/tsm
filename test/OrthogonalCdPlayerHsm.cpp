#include "CdPlayerHsm.h"
#include "Observer.h"
#include "OrthogonalHsm.h"

#include <catch2/catch.hpp>

using tsm::BlockingObserver;
using tsm::Event;
using tsm::OrthogonalHsm;
using tsm::SingleThreadedExecutionPolicy;

using tsmtest::CdPlayerController;
using tsmtest::CdPlayerHsm;
using tsmtest::ErrorHsm;

using OrthogonalCdPlayerHsm =
  OrthogonalHsm<CdPlayerHsm<CdPlayerController>, ErrorHsm>;

/// A "Blocking" Observer with Async Execution Policy
template<typename StateType>
using AsyncBlockingObserver =
  tsm::AsyncExecWithObserver<StateType, BlockingObserver>;

using OrthogonalCdPlayerHsmSeparateThread =
  AsyncBlockingObserver<OrthogonalCdPlayerHsm>;

using OrthogonalCdPlayerHsmSingleThread =
  SingleThreadedExecutionPolicy<OrthogonalCdPlayerHsm>;

TEST_CASE("TestOrthogonalCdPlayerHsm - testOrthogonalHsmSeparateThread")
{
    auto sm = std::make_shared<OrthogonalCdPlayerHsmSeparateThread>();

    auto* cdPlayerHsm = &std::get<0>(sm->sms_);

    auto* Playing = &cdPlayerHsm->Playing;

    auto* errorHsm = &std::get<1>(sm->sms_);

    REQUIRE(Playing->getParent() == cdPlayerHsm);
    REQUIRE(sm.get() == cdPlayerHsm->getParent());
    REQUIRE(sm.get() == errorHsm->getParent());

    sm->startSM();

    sm->wait();
    REQUIRE(errorHsm->getCurrentState() == &errorHsm->AllOk);

    sm->sendEvent(cdPlayerHsm->cd_detected);
    sm->wait();
    REQUIRE(cdPlayerHsm->getCurrentState() == &cdPlayerHsm->Stopped);

    sm->sendEvent(cdPlayerHsm->play);
    sm->wait();
    REQUIRE(cdPlayerHsm->getCurrentState() == Playing);

    REQUIRE(Playing->getCurrentState() == &Playing->Song1);

    sm->sendEvent(Playing->next_song);
    sm->wait();
    REQUIRE(cdPlayerHsm->getCurrentState() == Playing);
    REQUIRE(Playing->getCurrentState() == &Playing->Song2);

    sm->sendEvent(cdPlayerHsm->pause);
    sm->wait();
    REQUIRE(cdPlayerHsm->getCurrentState() == &cdPlayerHsm->Paused);
    REQUIRE(Playing->getCurrentState() == &Playing->Song2);

    sm->sendEvent(cdPlayerHsm->end_pause);
    sm->wait();
    REQUIRE(cdPlayerHsm->getCurrentState() == &cdPlayerHsm->Playing);
    REQUIRE(Playing->getCurrentState() == &Playing->Song2);

    sm->sendEvent(cdPlayerHsm->stop_event);
    sm->wait();
    REQUIRE(cdPlayerHsm->getCurrentState() == &cdPlayerHsm->Stopped);
    REQUIRE(Playing->getCurrentState() == nullptr);

    // same as TestCdPlayerHsm::testTransitions upto this point
    sm->sendEvent(errorHsm->error);
    sm->wait();
    REQUIRE(cdPlayerHsm->getCurrentState() == &cdPlayerHsm->Stopped);
    REQUIRE(errorHsm->getCurrentState() == &errorHsm->ErrorMode);

    sm->sendEvent(errorHsm->recover);
    sm->wait();
    REQUIRE(cdPlayerHsm->getCurrentState() == &cdPlayerHsm->Stopped);
    REQUIRE(errorHsm->getCurrentState() == &errorHsm->AllOk);

    sm->stopSM();
}

TEST_CASE("TestOrthogonalCdPlayerHsm - testOrthogonalHsmSingleThread")
{

    auto sm = std::make_shared<OrthogonalCdPlayerHsmSingleThread>();

    auto* cdPlayerHsm = &std::get<0>(sm->sms_);

    auto* Playing = &cdPlayerHsm->Playing;

    auto* errorHsm = &std::get<1>(sm->sms_);

    REQUIRE(Playing->getParent() == cdPlayerHsm);
    REQUIRE(sm.get() == cdPlayerHsm->getParent());
    REQUIRE(sm.get() == errorHsm->getParent());

    sm->startSM();

    REQUIRE(errorHsm->getCurrentState() == &errorHsm->AllOk);

    sm->sendEvent(cdPlayerHsm->cd_detected);
    sm->step();
    REQUIRE(cdPlayerHsm->getCurrentState() == &cdPlayerHsm->Stopped);

    sm->sendEvent(cdPlayerHsm->play);
    sm->step();
    REQUIRE(cdPlayerHsm->getCurrentState() == Playing);

    REQUIRE(Playing->getCurrentState() == &Playing->Song1);

    sm->sendEvent(Playing->next_song);
    sm->step();
    REQUIRE(cdPlayerHsm->getCurrentState() == Playing);
    REQUIRE(Playing->getCurrentState() == &Playing->Song2);

    // Should get dropped
    Event randomEvent;
    sm->sendEvent(randomEvent);
    sm->step();
    // Everything should remain as is
    REQUIRE(cdPlayerHsm->getCurrentState() == Playing);
    REQUIRE(Playing->getCurrentState() == &Playing->Song2);

    sm->sendEvent(cdPlayerHsm->pause);
    sm->step();
    REQUIRE(cdPlayerHsm->getCurrentState() == &cdPlayerHsm->Paused);
    REQUIRE(Playing->getCurrentState() == &Playing->Song2);

    sm->sendEvent(cdPlayerHsm->end_pause);
    sm->step();
    REQUIRE(cdPlayerHsm->getCurrentState() == &cdPlayerHsm->Playing);
    REQUIRE(Playing->getCurrentState() == &Playing->Song2);

    sm->sendEvent(cdPlayerHsm->stop_event);
    sm->step();
    REQUIRE(cdPlayerHsm->getCurrentState() == &cdPlayerHsm->Stopped);
    REQUIRE(Playing->getCurrentState() == nullptr);

    // same as TestCdPlayerHsm::testTransitions upto this point
    sm->sendEvent(errorHsm->error);
    sm->step();
    REQUIRE(cdPlayerHsm->getCurrentState() == &cdPlayerHsm->Stopped);
    REQUIRE(errorHsm->getCurrentState() == &errorHsm->ErrorMode);

    sm->sendEvent(errorHsm->recover);
    sm->step();
    REQUIRE(cdPlayerHsm->getCurrentState() == &cdPlayerHsm->Stopped);
    REQUIRE(errorHsm->getCurrentState() == &errorHsm->AllOk);

    sm->stopSM();
}
