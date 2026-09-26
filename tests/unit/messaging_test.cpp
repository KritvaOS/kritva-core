//==============================================================================
// Kritva Core — Messaging Contract Tests
// SPDX-License-Identifier: Apache-2.0
//
// Requirements:
//   CORE-MSG-001 : MessageHeader
//   CORE-MSG-002 : Topic
//
// These tests validate only the contracts exposed by the current Messaging
// headers. Transport, queues, publishers, subscribers, threading, and
// serialization are intentionally outside this test.
//==============================================================================

#include <cassert>
#include <type_traits>
#include <utility>

#include "kritva/core/messaging/message.hpp"
#include "kritva/core/messaging/topic.hpp"

using namespace kritva::core;
using namespace kritva::core::messaging;

namespace {

void test_topic_constructs_with_name() {
    const Topic topic("robot/status");

    assert(topic.name() == "robot/status");
}

void test_topic_preserves_empty_name() {
    const Topic topic("");

    assert(topic.name().empty());
}

void test_topic_preserves_name_exactly() {
    const std::string name = "kritva/core/joint/command";

    const Topic topic(name);

    assert(topic.name() == name);
}

void test_topic_move_construction() {
    std::string name = "kritva/sense/imu";
    Topic topic(std::move(name));

    assert(topic.name() == "kritva/sense/imu");
}

void test_topic_copy_semantics() {
    const Topic original("kritva/motion/state");
    const Topic copy = original;

    assert(copy.name() == original.name());
}

void test_topic_const_access() {
    const Topic topic("kritva/mind/event");

    const std::string& name = topic.name();

    assert(name == "kritva/mind/event");
}

void test_message_header_defaults() {
    const MessageHeader header;

    assert(!header.message_id.valid());
    assert(!header.source_id.valid());
    assert(header.timestamp.nanoseconds() == 0);
    assert(header.timestamp.domain() == ClockDomain::MONOTONIC);
}

void test_message_header_message_id() {
    MessageHeader header;
    const Id message_id{101};

    header.message_id = message_id;

    assert(header.message_id == message_id);
    assert(header.message_id.valid());
}

void test_message_header_source_id() {
    MessageHeader header;
    const Id source_id{202};

    header.source_id = source_id;

    assert(header.source_id == source_id);
    assert(header.source_id.valid());
}

void test_message_header_timestamp() {
    MessageHeader header;

    const Timestamp timestamp{
        123456789LL,
        ClockDomain::MONOTONIC
    };

    header.timestamp = timestamp;

    assert(header.timestamp == timestamp);
    assert(header.timestamp.nanoseconds() == 123456789LL);
    assert(header.timestamp.domain() == ClockDomain::MONOTONIC);
}

void test_message_header_realtime_timestamp() {
    MessageHeader header;

    const Timestamp timestamp{
        1700000000000000000LL,
        ClockDomain::REALTIME
    };

    header.timestamp = timestamp;

    assert(header.timestamp.domain() == ClockDomain::REALTIME);
    assert(header.timestamp.nanoseconds() == 1700000000000000000LL);
}

void test_message_header_complete_envelope() {
    MessageHeader header;

    const Id message_id{1001};
    const Id source_id{2002};
    const Timestamp timestamp{
        9876543210LL,
        ClockDomain::MONOTONIC
    };

    header.message_id = message_id;
    header.source_id = source_id;
    header.timestamp = timestamp;

    assert(header.message_id == message_id);
    assert(header.source_id == source_id);
    assert(header.timestamp == timestamp);
}

void test_message_header_copy_semantics() {
    MessageHeader original;

    original.message_id = Id{3001};
    original.source_id = Id{4002};
    original.timestamp = Timestamp{
        555000LL,
        ClockDomain::REALTIME
    };

    const MessageHeader copy = original;

    assert(copy.message_id == original.message_id);
    assert(copy.source_id == original.source_id);
    assert(copy.timestamp == original.timestamp);
}

} // namespace

int main() {
    // CORE-MSG-002 — Topic
    test_topic_constructs_with_name();
    test_topic_preserves_empty_name();
    test_topic_preserves_name_exactly();
    test_topic_move_construction();
    test_topic_copy_semantics();
    test_topic_const_access();

    // CORE-MSG-001 — MessageHeader
    test_message_header_defaults();
    test_message_header_message_id();
    test_message_header_source_id();
    test_message_header_timestamp();
    test_message_header_realtime_timestamp();
    test_message_header_complete_envelope();
    test_message_header_copy_semantics();

    return 0;
}
