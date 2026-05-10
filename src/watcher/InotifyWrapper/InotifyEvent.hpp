
#pragma once

#include <sys/inotify.h>
#include <cstdint>

class InotifyEvent
{
    public:
        explicit InotifyEvent(const inotify_event *raw) noexcept : _raw_event(raw) {}

        [[nodiscard]] uint32_t mask() const noexcept { return _raw_event->mask; }
        [[nodiscard]] int wd() const noexcept { return _raw_event->wd; }
        [[nodiscard]] uint32_t len() const noexcept { return _raw_event->len; }

        [[nodiscard]] bool isModified() const noexcept  { return _raw_event->mask & IN_MODIFY; }
        [[nodiscard]] bool isClosedWrite() const noexcept { return _raw_event->mask & IN_CLOSE_WRITE; }
        [[nodiscard]] bool isDeleted() const noexcept { return _raw_event->mask & IN_DELETE; }

    private:
        const inotify_event *_raw_event;
};
