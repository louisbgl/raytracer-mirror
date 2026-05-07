#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include "../DataTypes/Vec3.hpp"

enum class MessageType : uint8_t {
    READY     = 0x01,
    ASSIGN    = 0x02,
    HEARTBEAT = 0x03,
    PIXELS    = 0x04,
    ACK       = 0x05,
    ABORT     = 0x06,
    DONE      = 0x07
};

struct AssignPayload {
    std::string sceneContent;
    int firstRow;
    int lastRow;
    int width;
    int height;
};

struct Message {
    MessageType type;
    std::vector<uint8_t> payload;

    static Message makeReady();
    static Message makeAssign(const AssignPayload& data);
    static Message makeHeartbeat(int progressPercent);
    static Message makePixels(const std::vector<Vec3>& pixels);
    static Message makeAck();
    static Message makeAbort();
    static Message makeDone();

    std::vector<uint8_t> serialize() const;
    static Message deserialize(const std::vector<uint8_t>& data);

    AssignPayload parseAssign() const;
    int parseHeartbeat() const;
    std::vector<Vec3> parsePixels() const;
};
