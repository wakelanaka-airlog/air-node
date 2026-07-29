#pragma once

#include "NodeState.h"

class SendingState : public NodeState
{
public:
    SendingState(const SendingState&) = delete;
    SendingState& operator=(const SendingState&) = delete;

    static SendingState& getInstance();

    void enter(Node& node) override;
    void process(Node& node) override;
    void exit(Node& node) override;

private:
    SendingState() = default;
};
