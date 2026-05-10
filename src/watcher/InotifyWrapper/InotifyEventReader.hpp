
#pragma once

#include <linux/limits.h>
#include <sys/inotify.h>
#include <array>
#include <cstddef>
#include <iterator>
#include <memory>
#include <optional>

#include "./InotifyInstance.hpp"
#include "./InotifyEvent.hpp"
#include "./InotifyWatch.hpp"

class InotifyEventReader
{
    public:
        struct Iterator {
            using iter_category = std::forward_iterator_tag;
            using val_type = InotifyEvent;
            using dif_type = std::ptrdiff_t;

            const char *ptr;

            InotifyEvent operator*() { return InotifyEvent{reinterpret_cast<const inotify_event*>(ptr)}; }

            Iterator& operator++() {
                const auto *e = reinterpret_cast<const inotify_event*>(ptr);
                ptr += sizeof(inotify_event) + e->len;
                return *this;
            }

            bool operator!=(const Iterator& other) const { return ptr != other.ptr; }
        };

    using EventRange = std::pair<Iterator, Iterator>;

    explicit InotifyEventReader(const InotifyInstance& instance) noexcept; 

    std::optional<EventRange> read();

    private:
        int _fd;
        static constexpr std::size_t _buffsize = sizeof(inotify_event) + NAME_MAX + 1; 

        std::array<char, _buffsize> _BUFFER = {};
};
