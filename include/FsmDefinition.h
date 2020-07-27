#pragma once

#include "Event.h"
#include "State.h"
#include "Transition.h"

///
/// Captures 'structural' aspects of the state machine and behavior specific to
/// FsmDef. For e.g. the FsmDef can override the onEntry and onExit behaviors to
/// implement history preserving policy for specific events.
///
namespace tsm {

template<typename FsmDef>
struct FsmDefinition : public State
{
    using StateTransitionTable = StateTransitionTableT<FsmDef>;
    using Transition = typename StateTransitionTableT<FsmDef>::Transition;
    using ActionFn = void (FsmDef::*)();
    using GuardFn = bool (FsmDef::*)();

    FsmDefinition(std::string const& name)
      : State(name)
      , currentState_(nullptr)
    {}

    virtual ~FsmDefinition() = default;

    void add(State& fromState,
             Event const& onEvent,
             State& toState,
             ActionFn action = nullptr,
             GuardFn guard = nullptr)
    {
        table_.add(fromState, onEvent, toState, action, guard);
    }

    Transition* next(State& currentState, Event const& nextEvent)
    {
        return table_.next(currentState, nextEvent);
    }

    State* getCurrentState()
    {
        DLOG(INFO) << "Get Current State: "
                   << ((currentState_) ? currentState_->name : "nullptr");
        return currentState_;
    }

    StateTransitionTable& getTable() const { return table_; }
    std::set<Event> const& getEvents() const { return table_.getEvents(); }

    virtual State* getStartState() = 0;
    virtual State* getStopState() = 0;

  protected:
    StateTransitionTable table_;
    State* currentState_;
};
} // namespace tsm
