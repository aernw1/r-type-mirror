/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Test EventBus
*/

#include "Core/EventBus.hpp"
#include <iostream>
#include <cassert>

struct TestEvent {
    int value;
};

struct AnotherEvent {
    std::string message;
};

int main() {
    RType::Core::EventBus eventBus;
    bool handlerCalled = false;
    int receivedValue = 0;

    // Test Subscription
    eventBus.Subscribe<TestEvent>([&](const TestEvent& event) {
        handlerCalled = true;
        receivedValue = event.value;
    });

    // Test Publish
    TestEvent event{42};
    eventBus.Publish(event);

    assert(handlerCalled && "Handler should be called");
    assert(receivedValue == 42 && "Value should be 42");

    // Test multiple handlers
    bool handler2Called = false;
    eventBus.Subscribe<TestEvent>([&](const TestEvent&) {
        handler2Called = true;
    });

    handlerCalled = false;
    handler2Called = false;
    eventBus.Publish(TestEvent{100});

    assert(handlerCalled && "Handler 1 should be called");
    assert(handler2Called && "Handler 2 should be called");

    // Test different event type
    bool stringHandlerCalled = false;
    eventBus.Subscribe<AnotherEvent>([&](const AnotherEvent& event) {
        stringHandlerCalled = true;
        assert(event.message == "Hello");
    });

    eventBus.Publish(AnotherEvent{"Hello"});
    assert(stringHandlerCalled && "String handler should be called");

    // Test Clear
    eventBus.Clear();
    handlerCalled = false;
    eventBus.Publish(TestEvent{1});
    assert(!handlerCalled && "Handler should NOT be called after Clear()");

    std::cout << "EventBus tests passed!" << std::endl;
    return 0;
}
