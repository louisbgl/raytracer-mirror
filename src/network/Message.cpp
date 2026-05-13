#include "Message.hpp"
#include <cstring>
#include <stdexcept>

// helpers to write/read integers into byte buffers
static void writeInt(std::vector<uint8_t>& buf, int value)
{
    uint32_t v = static_cast<uint32_t>(value);
    buf.push_back((v >> 24) & 0xFF);
    buf.push_back((v >> 16) & 0xFF);
    buf.push_back((v >>  8) & 0xFF);
    buf.push_back((v >>  0) & 0xFF);
}

static int readInt(const uint8_t* data)
{
    uint32_t v = (static_cast<uint32_t>(data[0]) << 24)
               | (static_cast<uint32_t>(data[1]) << 16)
               | (static_cast<uint32_t>(data[2]) <<  8)
               | (static_cast<uint32_t>(data[3]) <<  0);
    return static_cast<int>(v);
}

static void writeDouble(std::vector<uint8_t>& buf, double value)
{
    uint8_t bytes[sizeof(double)];
    std::memcpy(bytes, &value, sizeof(double));
    for (auto b : bytes)
        buf.push_back(b);
}

static double readDouble(const uint8_t* data)
{
    double value;
    std::memcpy(&value, data, sizeof(double));
    return value;
}

// builders
Message Message::makeReady()
{
    return {MessageType::READY, {}};
}

Message Message::makeAssign(const AssignPayload& data)
{
    std::vector<uint8_t> payload;
    writeInt(payload, data.firstRow);
    writeInt(payload, data.lastRow);
    writeInt(payload, data.width);
    writeInt(payload, data.height);
    writeInt(payload, static_cast<int>(data.sceneContent.size()));
    for (char c : data.sceneContent)
        payload.push_back(static_cast<uint8_t>(c));
    return {MessageType::ASSIGN, payload};
}

Message Message::makeHeartbeat(int progressPercent)
{
    std::vector<uint8_t> payload;
    writeInt(payload, progressPercent);
    return {MessageType::HEARTBEAT, payload};
}

Message Message::makePixels(const std::vector<Vec3>& pixels)
{
    std::vector<uint8_t> payload;
    writeInt(payload, static_cast<int>(pixels.size()));
    for (const Vec3& p : pixels) {
        writeDouble(payload, p.x());
        writeDouble(payload, p.y());
        writeDouble(payload, p.z());
    }
    return {MessageType::PIXELS, payload};
}

Message Message::makeAbort()
{
    return {MessageType::ABORT, {}};
}

Message Message::makeChunk(const ChunkPayload& data)
{
    std::vector<uint8_t> payload;
    writeInt(payload, data.firstRow);
    writeInt(payload, data.lastRow);
    return {MessageType::CHUNK, payload};
}

Message Message::makeFinish()
{
    return {MessageType::FINISH, {}};
}

// wire format: [type: 1 byte][length: 4 bytes][payload: N bytes]
std::vector<uint8_t> Message::serialize() const
{
    std::vector<uint8_t> out;
    out.push_back(static_cast<uint8_t>(type));
    writeInt(out, static_cast<int>(payload.size()));
    out.insert(out.end(), payload.begin(), payload.end());
    return out;
}

Message Message::deserialize(const std::vector<uint8_t>& data)
{
    if (data.size() < 5)
        throw std::runtime_error("Message::deserialize: too short");
    Message msg;
    msg.type = static_cast<MessageType>(data[0]);
    int length = readInt(data.data() + 1);
    if (static_cast<int>(data.size()) < 5 + length)
        throw std::runtime_error("Message::deserialize: truncated payload");
    msg.payload.assign(data.begin() + 5, data.begin() + 5 + length);
    return msg;
}

// payload parsers
AssignPayload Message::parseAssign() const
{
    if (payload.size() < 20)
        throw std::runtime_error("parseAssign: payload too short");
    AssignPayload data;
    data.firstRow  = readInt(payload.data() + 0);
    data.lastRow   = readInt(payload.data() + 4);
    data.width     = readInt(payload.data() + 8);
    data.height    = readInt(payload.data() + 12);
    int contentLen = readInt(payload.data() + 16);
    if (static_cast<int>(payload.size()) < 20 + contentLen)
        throw std::runtime_error("parseAssign: scene content truncated");
    data.sceneContent.assign(payload.begin() + 20, payload.begin() + 20 + contentLen);
    return data;
}

int Message::parseHeartbeat() const
{
    if (payload.size() < 4)
        throw std::runtime_error("parseHeartbeat: payload too short");
    return readInt(payload.data());
}

std::vector<Vec3> Message::parsePixels() const
{
    if (payload.size() < 4)
        throw std::runtime_error("parsePixels: payload too short");
    int count = readInt(payload.data());
    if (static_cast<int>(payload.size()) < 4 + count * 3 * static_cast<int>(sizeof(double)))
        throw std::runtime_error("parsePixels: payload truncated");
    std::vector<Vec3> pixels(count);
    for (int i = 0; i < count; ++i) {
        double x = readDouble(payload.data() + 4 + i * 3 * sizeof(double));
        double y = readDouble(payload.data() + 4 + i * 3 * sizeof(double) + sizeof(double));
        double z = readDouble(payload.data() + 4 + i * 3 * sizeof(double) + sizeof(double) * 2);
        pixels[i] = Vec3(x, y, z);
    }
    return pixels;
}

ChunkPayload Message::parseChunk() const
{
    if (payload.size() < 8)
        throw std::runtime_error("parseChunk: payload too short");
    ChunkPayload data;
    data.firstRow = readInt(payload.data() + 0);
    data.lastRow  = readInt(payload.data() + 4);
    return data;
}
