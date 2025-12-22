// SPDX-FileCopyrightText: Copyright 2021-2025 Guyeon Yu <copyrat90@gmail.com>
// SPDX-License-Identifier: Zlib

#pragma once

#include "ibn_observer_fwd.h"

#include "ibn_function.h"

#include <bn_intrusive_list.h>

#include <utility>

namespace ibn
{

template <typename Ret, typename... Args>
class subject<Ret(Args...)>
{
public:
    using observer_t = observer<Ret(Args...)>;

public:
    ~subject()
    {
        // Safe iteration
        auto cur = _observers.begin();
        while (cur != _observers.end())
        {
            // Store next iterator beforehand
            auto next = cur;
            ++next;

            cur->unsubscribe();

            cur = next;
        }
    }

    subject() = default;

    subject(const subject&) = delete;
    subject& operator=(const subject&) = delete;

public:
    void attach(observer_t& obser)
    {
        // Unsubscribe from previous subject
        obser.unsubscribe();

        _observers.push_back(obser);
        obser.set_owner(this);
    }

    void detach(observer_t& obser)
    {
        _observers.erase(obser);
        obser.set_owner(nullptr);
    }

    template <typename... CallArgs>
    void notify(CallArgs&&... args)
    {
        // Safe iteration (callback might detach itself)
        auto cur = _observers.begin();
        while (cur != _observers.end())
        {
            // Store next iterator beforehand
            auto next = cur;
            ++next;

            cur->_callback(std::forward<CallArgs>(args)...);

            cur = next;
        }
    }

private:
    bn::intrusive_list<observer_t> _observers;
};

template <typename Ret, typename... Args>
class observer<Ret(Args...)> : public bn::intrusive_list_node_type
{
public:
    using subject_t = subject<Ret(Args...)>;

public:
    ~observer()
    {
        unsubscribe();
    }

    observer(const function<Ret(Args...)>& callback) : _callback(callback)
    {
    }

    observer(function<Ret(Args...)>&& callback) : _callback(std::move(callback))
    {
    }

    observer(const observer&) = delete;
    observer& operator=(const observer&) = delete;

public:
    void unsubscribe()
    {
        if (_owner)
        {
            _owner->detach(*this);
            set_owner(nullptr);
        }
    }

private:
    friend class subject<Ret(Args...)>;

    void set_owner(subject_t* owner)
    {
        _owner = owner;
    }

private:
    function<Ret(Args...)> _callback;

    subject_t* _owner = nullptr;
};

} // namespace ibn
