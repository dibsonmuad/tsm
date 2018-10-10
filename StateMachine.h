#pragma once

#include "Event.h"
#include "State.h"
#include "StateMachineDef.h"

namespace tsm {
template<typename HSMDef>
struct StateMachine : public HSMDef
{
    using Transition = typename StateMachineDef<HSMDef>::Transition;

    StateMachine(IHsmDef* parent = nullptr)
      : HSMDef(parent)
    {}

    virtual ~StateMachine() = default;

    void startSM() { this->onEntry(Event::dummy_event); }

    void stopSM() { this->onExit(Event::dummy_event); }

    // traverse the hsm hierarchy down.
    IHsmDef* dispatch(IHsmDef* state) const
    {
        IHsmDef* parent = state;
        IHsmDef* kid = dynamic_cast<IHsmDef*>(parent->getCurrentState());
        while (kid && kid->getParent()) {
            parent = kid;
            kid = dynamic_cast<IHsmDef*>(kid->getCurrentState());
        }
        return parent;
    }

    void execute(Event const& nextEvent) override
    {
        DLOG(INFO) << "Current State:" << this->currentState_->name
                   << " Event:" << nextEvent.id;

        Transition* t = this->next(*this->currentState_, nextEvent);

        if (!t) {
            // If transition does not exist, pass event to parent HSM
            if (this->parent_) {
                // TODO(sriram) : should call onExit? UML spec seems to say yes
                // invoking onExit() here will not work for Orthogonal state
                // machines
                // this->onExit(nextEvent);
                this->parent_->execute(nextEvent);
            } else {
                DLOG(ERROR) << "Reached top level HSM. Cannot handle event";
            }
        } else {
            // Evaluate guard if it exists
            bool result = t->guard && (this->*(t->guard))();

            if (!(t->guard) || result) {
                // Perform entry and exit actions in the doTransition function.
                // If just an internal transition, Entry and exit actions are
                // not performed
                t->template doTransition<HSMDef>(this);
                this->currentState_ = &t->toState;
                DLOG(INFO) << "Next State:" << this->currentState_->name;

                if (!dynamic_cast<IHsmDef*>(this->currentState_)) {
                    this->currentState_->execute(nextEvent);
                }

            } else {
                DLOG(INFO) << "Guard prevented transition";
            }
            if (this->currentState_ == this->getStopState()) {
                DLOG(INFO) << this->name << " Reached stop state. Exiting... ";
                this->onExit(Event::dummy_event);
            }
        }
    }
};

} // namespace tsm
