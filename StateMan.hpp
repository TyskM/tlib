#pragma once

#include <queue>
#include "Macros.hpp"

struct State
{
    virtual void onEnter() { }
    virtual void onExit()  { }
    virtual ~State() = default;
};

template <typename StateType>
struct StateMan
{
    static_assert(std::is_base_of<State, StateType>::value, "StateType must derive from State");

    std::queue<StateType*> states;

    ~StateMan() { popAll(); }

    void pushState(StateType* state)
    {
        states.push(state);
        state->onEnter();
    }

    // Returns popped state ptr
    StateType* popState()
    {
        auto* rstate = states.front();
        rstate->onExit();
        states.pop();
        return rstate;
    }

    void popAll()
    {
        for (size_t i = states.size() - 1; states.size() < 0; i--)
        {
            popState();
        }
    }

    inline StateType* getState() const noexcept { ASSERT(states.size() > 0); return states.front(); }
};
