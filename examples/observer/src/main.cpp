#include "ibn_observer.h"

#include <bn_core.h>
#include <bn_log.h>
#include <bn_point.h>
#include <bn_string_view.h>

// No need for destructors.
// Both `ibn::observer::~observer()` and `ibn::subject::~subject()` will handle the unsubscription.

struct events
{
    ibn::subject<void(bn::string_view, bn::string_view)> chat;
    ibn::subject<void(bn::string_view, const bn::point&, const bn::point&)> position_moved;
};

struct logger
{
    bn::string_view tag;

    ibn::observer<void(bn::string_view, bn::string_view)> chat;
    decltype(events::position_moved)::observer_t position_moved;

    logger(bn::string_view logger_tag, events& evs)
        : tag(logger_tag),
          chat([this](bn::string_view user_name, bn::string_view message) { this->log_chat(user_name, message); }),
          position_moved([this](bn::string_view user_name, const bn::point& prev_pos, const bn::point& next_pos) {
              log_position_moved(user_name, prev_pos, next_pos);
          })
    {
        evs.chat.attach(chat);
        evs.position_moved.attach(position_moved);
    }

    void log_chat(bn::string_view user_name, bn::string_view message)
    {
        BN_LOG("[", tag, "] ", user_name, " said \"", message, "\"!");
    }

    void log_position_moved(bn::string_view user_name, const bn::point& prev_pos, const bn::point& next_pos)
    {
        BN_LOG("[", tag, "] ", user_name, " moved from (", prev_pos.x(), ", ", prev_pos.y(), ") to (", next_pos.x(),
               ", ", next_pos.y(), ")");
    }
};

int main()
{
    bn::core::init();

    events evs;

    {
        logger logger_a("A", evs);

        evs.chat.notify("Bob", "Hello, world!");
        evs.position_moved.notify("Bob", bn::point(1, 2), bn::point(3, 4));

        {
            logger logger_b("B", evs);

            evs.chat.notify("Alice", "Goodbye, world!");
            evs.position_moved.notify("Alice", bn::point(5, 6), bn::point(7, 8));

        } // `logger_b` goes out of scope, unsubscribed.

        evs.chat.notify("Tom", "Nay, world!");
        evs.position_moved.notify("Tom", bn::point(-99, -99), bn::point(77, 77));

    } // `logger_a` goes out of scope, unsubscribed.

    evs.chat.notify("Nobody", "Nobody's home!");
    evs.position_moved.notify("Nobody", bn::point(0, 0), bn::point(0, 1));

    while (true)
        bn::core::update();
}
