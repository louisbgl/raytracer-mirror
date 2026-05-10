
#include "./InotifyEventReader.hpp"

#include <sys/types.h>
#include <unistd.h>
#include <cerrno>
#include <optional>


 InotifyEventReader::InotifyEventReader(const InotifyInstance& instance) noexcept : _fd(instance.fd())
{

}

std::optional<InotifyEventReader::EventRange> InotifyEventReader::read()
{
    _BUFFER = {};
    const ssize_t len = ::read(_fd, _BUFFER.data(), _BUFFER.size());

    if (len == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) { return std::nullopt; }
        throw InotifyException("InotifyEventReader read failed");
    }

    return EventRange { 
        Iterator{_BUFFER.data()}, 
        Iterator{_BUFFER.data() + len}
    };
}
