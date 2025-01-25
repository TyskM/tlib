#pragma once
#include <sigslot/signal.hpp>

// Thread unsafe event
template <typename... T>
using Event = sigslot::signal_st<T...>;

// Thread safe event
template <typename... T>
using EventMt = sigslot::signal<T...>;

using ScopedEventConnection = sigslot::scoped_connection;
