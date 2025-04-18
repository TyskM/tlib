
/* EXAMPLE https://github.com/palacaze/sigslot
void f() { std::cout << "free function\n"; }

struct s {
    void m() { std::cout << "member function\n"; }
    static void sm() { std::cout << "static member function\n";  }
};

struct o {
    void operator()() { std::cout << "function object\n"; }
};

int main() {
    s d;
    auto lambda = []() { std::cout << "lambda\n"; };
    auto gen_lambda = [](auto && ...a) { std::cout << "generic lambda\n"; };

    // declare a signal instance with no arguments
    sigslot::signal<> sig;

    // connect slots
    sig.connect(f);
    sig.connect(&s::m, &d);
    sig.connect(&s::sm);
    sig.connect(o());
    sig.connect(lambda);
    sig.connect(gen_lambda);

    // a free connect() function is also available
    sigslot::connect(sig, f);

    // emit a signal
    sig();
}
*/

#pragma once
#include <sigslot/signal.hpp>

// Thread unsafe event
template <typename... T>
using Event = sigslot::signal_st<T...>;

// Thread safe event
template <typename... T>
using EventMt = sigslot::signal<T...>;

using ScopedEventConnection = sigslot::scoped_connection;
