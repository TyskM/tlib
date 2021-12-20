#pragma once

#include <typeinfo>
#include <functional>
#include <stdexcept>
#include <memory>
#include <atomic>
#include <cstring>

///
//
// CSharp style events.
// I forgot where I found this class...
// but it works.
// 
// Use the bind method for class members.
// Use the += operator for lambdas.
//
/// Usage examples:
//
// Event<yourReturnType(yourArgument)> yourEvent;
// 
// Event<void()> event1;
// Event<void(int, double)> event2;
//
// event1.bind(&Test::foo, &t1);
// event2 += [](int arg1, double arg2) { cout << "Lambda(" << arg1 << ", " << arg2 << ") " << endl; };
// event2.bind(&Test::foo1, &t1);
// event2.unbind(&Test::foo1, &t1);
// function<void(int, double)> stdfunction = [](int arg1, double arg2) { cout << "stdfunction(" << arg1 << ", " << arg2 << ") " << endl;  };
// event2 += stdfunction;
// event2 -= stdfunction;
// event2 = nullptr;
//
///

template <typename TFunc>
class Event;

template <class RetType, class... Args>
class Event<RetType(Args ...)> final
{
private:
    typedef typename std::function<RetType(Args ...)> Closure;

    struct ComparableClosure
    {
        Closure Callable;
        void* Object;
        uint8_t* Functor;
        int FunctorSize;

        ComparableClosure(const ComparableClosure&) = delete;

        ComparableClosure() : Object(nullptr), Functor(nullptr), FunctorSize(0) { }

        ComparableClosure(Closure&& closure) : Callable(std::move(closure)), Object(nullptr), Functor(nullptr), FunctorSize(0) { }

        ~ComparableClosure()
        {
            if (Functor != nullptr)
                delete[] Functor;
        }

        ComparableClosure& operator=(const ComparableClosure& closure)
        {
            Callable = closure.Callable;
            Object = closure.Object;
            FunctorSize = closure.FunctorSize;
            if (closure.FunctorSize == 0)
            {
                Functor = nullptr;
            }
            else
            {
                Functor = new uint8_t[closure.FunctorSize];
                std::memcpy(Functor, closure.Functor, closure.FunctorSize);
            }

            return *this;
        }

        bool operator==(const ComparableClosure& closure)
        {
            if (Object == nullptr && closure.Object == nullptr)
            {
                return Callable.target_type() == closure.Callable.target_type();
            }
            else
            {
                return Object == closure.Object && FunctorSize == closure.FunctorSize
                    && std::memcmp(Functor, closure.Functor, FunctorSize) == 0;
            }
        }
    };

    struct ClosureList
    {
        ComparableClosure* Closures;
        int Count;

        ClosureList(ComparableClosure* closures, int count)
        {
            Closures = closures;
            Count = count;
        }

        ~ClosureList()
        {
            delete[] Closures;
        }
    };

    typedef std::shared_ptr<ClosureList> ClosureListPtr;

private:
    ClosureListPtr m_events;

private:
    bool addClosure(const ComparableClosure& closure)
    {
        auto events = std::atomic_load(&m_events);
        int count;
        ComparableClosure* closures;
        if (events == nullptr)
        {
            count = 0;
            closures = nullptr;
        }
        else
        {
            count = events->Count;
            closures = events->Closures;
        }

        auto newCount = count + 1;
        auto newClosures = new ComparableClosure[newCount];
        if (count != 0)
        {
            for (int i = 0; i < count; i++)
                newClosures[i] = closures[i];
        }

        newClosures[count] = closure;
        auto newEvents = ClosureListPtr(new ClosureList(newClosures, newCount));
        if (std::atomic_compare_exchange_weak(&m_events, &events, newEvents))
            return true;

        return false;
    }

    bool removeClosure(const ComparableClosure& closure)
    {
        auto events = std::atomic_load(&m_events);
        if (events == nullptr)
            return true;

        int index = -1;
        auto count = events->Count;
        auto closures = events->Closures;
        for (int i = 0; i < count; i++)
        {
            if (closures[i] == closure)
            {
                index = i;
                break;
            }
        }

        if (index == -1)
            return true;

        auto newCount = count - 1;
        ClosureListPtr newEvents;
        if (newCount == 0)
        {
            newEvents = nullptr;
        }
        else
        {
            auto newClosures = new ComparableClosure[newCount];
            for (int i = 0; i < index; i++)
                newClosures[i] = closures[i];

            for (int i = index + 1; i < count; i++)
                newClosures[i - 1] = closures[i];

            newEvents = ClosureListPtr(new ClosureList(newClosures, newCount));
        }

        if (std::atomic_compare_exchange_weak(&m_events, &events, newEvents))
            return true;

        return false;
    }

public:
    Event()
    {
        std::atomic_store(&m_events, ClosureListPtr());
    }

    Event(const Event& event)
    {
        std::atomic_store(&m_events, std::atomic_load(&event.m_events));
    }

    ~Event()
    {
        (*this) = nullptr;
    }

    void operator =(const Event& event)
    {
        std::atomic_store(&m_events, std::atomic_load(&event.m_events));
    }

    void operator=(nullptr_t nullpointer)
    {
        while (true)
        {
            auto events = std::atomic_load(&m_events);
            if (!std::atomic_compare_exchange_weak(&m_events, &events, ClosureListPtr()))
                continue;

            break;
        }
    }

    bool operator==(nullptr_t nullpointer)
    {
        auto events = std::atomic_load(&m_events);
        return events == nullptr;
    }

    bool operator!=(nullptr_t nullpointer)
    {
        auto events = std::atomic_load(&m_events);
        return events != nullptr;
    }

    void operator +=(Closure f)
    {
        ComparableClosure closure(std::move(f));
        while (true)
        {
            if (addClosure(closure))
                break;
        }
    }

    void operator -=(Closure f)
    {
        ComparableClosure closure(std::move(f));
        while (true)
        {
            if (removeClosure(closure))
                break;
        }
    }

    template <typename TObject>
    void bind(RetType(TObject::* function)(Args...), TObject* object)
    {
        ComparableClosure closure;
        closure.Callable = [object, function](Args&&...args)
        {
            return (object->*function)(std::forward<Args>(args)...);
        };
        closure.FunctorSize = sizeof(function);
        closure.Functor = new uint8_t[closure.FunctorSize];
        std::memcpy(closure.Functor, (void*)&function, sizeof(function));
        closure.Object = object;

        while (true)
        {
            if (addClosure(closure))
                break;
        }
    }

    template <typename TObject>
    void unbind(RetType(TObject::* function)(Args...), TObject* object)
    {
        ComparableClosure closure;
        closure.FunctorSize = sizeof(function);
        closure.Functor = new uint8_t[closure.FunctorSize];
        std::memcpy(closure.Functor, (void*)&function, sizeof(function));
        closure.Object = object;

        while (true)
        {
            if (removeClosure(closure))
                break;
        }
    }

    void operator()()
    {
        auto events = std::atomic_load(&m_events);
        if (events == nullptr)
            return;

        auto count = events->Count;
        auto closures = events->Closures;
        for (int i = 0; i < count; i++)
            closures[i].Callable();
    }

    template <typename TArg0, typename ...Args2>
    void operator()(TArg0 a1, Args2... tail)
    {
        auto events = std::atomic_load(&m_events);
        if (events == nullptr)
            return;

        auto count = events->Count;
        auto closures = events->Closures;
        for (int i = 0; i < count; i++)
            closures[i].Callable(a1, tail...);
    }
};