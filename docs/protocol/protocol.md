# Cluster Rendering Protocol

TCP-based binary protocol between a **coordinator** and one or more **workers**.
The coordinator listens on port **6767**. Workers connect, receive row chunks to render, and send back pixels.

---

## Message Format

Every message shares the same wire format:

```
+--------+--------------------+---------------------+
| type   | payload_length     | payload             |
| 1 byte | 4 bytes (big-end.) | payload_length bytes|
+--------+--------------------+---------------------+
```
Total header: **5 bytes**. Payload length can be 0.

---

## Message Types

| Type     | Opcode | Direction          | Payload |
|----------|--------|--------------------|---------|
| READY    | `0x01` | worker → coordinator | none |
| ASSIGN   | `0x02` | coordinator → worker | scene content + row range |
| PIXELS   | `0x04` | worker → coordinator | rendered pixel colors |
| ABORT    | `0x06` | coordinator → worker | none |
| CHUNK    | `0x07` | coordinator → worker | row range only |
| FINISH   | `0x08` | coordinator → worker | none |

---

## Payload Layouts

### ASSIGN (`0x02`)
Sent once per worker as its **first** assignment. Carries the full scene so workers don't need the file on disk.

```
[4] firstRow      (int, big-endian)
[4] lastRow       (int, big-endian)
[4] width         (int, big-endian)
[4] height        (int, big-endian)
[4] contentLen    (int, big-endian)
[N] sceneContent  (UTF-8 text, N = contentLen bytes)
```

### CHUNK (`0x07`)
Sent for every **subsequent** assignment. Lightweight — no scene retransmission.

```
[4] firstRow  (int, big-endian)
[4] lastRow   (int, big-endian)
```

### PIXELS (`0x04`)
Sent by the worker after finishing a chunk, in batches of up to **1000 pixels**.

```
[4]      count           (int, big-endian)
[count × 24]  pixels    (3 × double per pixel, IEEE 754, memcpy)
```

Each pixel is 3 doubles (R, G, B), 8 bytes each = 24 bytes/pixel.
Pixels are sent in row-major order starting from `firstRow`.

---

## Session Flow

```
Worker                          Coordinator
  |                                  |
  |---- TCP connect ---------------→ |
  |---- READY ----------------------→|  "I'm available"
  |                                  |
  |←----------------------- ASSIGN --|  first chunk + full scene
  |  [render rows firstRow..lastRow] |
  |---- PIXELS (batch 1) ----------→ |
  |---- PIXELS (batch 2) ----------→ |  (up to 1000px per message)
  |---- PIXELS (last batch) -------→ |
  |                                  |
  |←------------------------ CHUNK --|  next chunk (8 bytes only)
  |  [render ...]                    |
  |---- PIXELS ... ----------------→ |
  |                                  |
  |←----------------------- FINISH --|  no more work
  |  [disconnect]                    |
```

If more workers connect than there are chunks, excess workers receive FINISH immediately after ASSIGN.

---

## Timeout & Fault Tolerance

The coordinator uses `poll()` with a **12-second timeout** per cycle.
After **2 consecutive missed timeouts**, the worker is considered dead and its current chunk is re-queued for another worker (or the coordinator itself).

---

## Coordinator as Worker

The coordinator also renders chunks locally, competing fairly with remote workers via the same shared chunk queue. This avoids wasting the coordinator machine's CPU.

---
