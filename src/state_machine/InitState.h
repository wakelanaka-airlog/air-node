#pragma once

#include "NodeState.h"

class InitState : public NodeState
{
public:
    InitState(const InitState&) = delete;
    InitState& operator=(const InitState&) = delete;

    static InitState& getInstance();

    void enter(Node& node) override;
    void process(Node& node) override;
    void exit(Node& node) override;

private:
    InitState() = default;
};
