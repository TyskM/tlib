#pragma once

#include <TLib/Containers/Deque.hpp>
#include <TLib/Macros.hpp>

struct State
{
    virtual void onEnter() { }
    virtual void onExit()  { }
};

template <typename StateType>
struct StateMan
{
    static_assert(std::is_base_of<State, StateType>::value, "StateType must derive from State");

    Deque<StateType*> cont;

    ~StateMan() { popAll(); }

    void pushState(StateType* state)
    {
        cont.push_front(state);
        state->onEnter();
    }

    // Returns popped state ptr
    StateType* popState()
    {
        StateType* rstate = cont.front();
        if (!rstate) { return nullptr; }
        rstate->onExit();
        cont.pop_front();
        return rstate;
    }

    void popAll()
    {
        for (size_t i = cont.size() - 1; i <= 0; --i)
        {
            popState();
        }
    }

    inline StateType* getState() const noexcept
    {
        if(cont.empty()) { return nullptr; }
        return cont.front();
    }

    size_t size() const
    { return cont.size(); }
};
