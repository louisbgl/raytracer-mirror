#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include "../DataTypes/Vec3.hpp"

enum class MessageType : uint8_t {
    READY     = 0x01,
    ASSIGN    = 0x02,  // first assignment: includes scene content + coords
    PIXELS    = 0x04,
    ABORT     = 0x06,
    CHUNK     = 0x07,  // subsequent assignments: just firstRow/lastRow
    FINISH    = 0x08,  // no more work
};

struct AssignPayload {
    std::string sceneContent;
    int firstRow;
    int lastRow;
    int width;
    int height;
};

struct ChunkPayload {
    int firstRow;
    int lastRow;
};

struct Message {
    MessageType type;
    std::vector<uint8_t> payload;

    static Message makeReady();
    static Message makeAssign(const AssignPayload& data);
    static Message makePixels(const std::vector<Vec3>& pixels);
    static Message makeAbort();
    static Message makeChunk(const ChunkPayload& data);
    static Message makeFinish();

    std::vector<uint8_t> serialize() const;
    static Message deserialize(const std::vector<uint8_t>& data);

    AssignPayload parseAssign() const;
    std::vector<Vec3> parsePixels() const;
    ChunkPayload parseChunk() const;
};
