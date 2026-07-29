#pragma once

#include "NodeState.h"

class IdleState : public NodeState
{
public:
    IdleState(const IdleState&) = delete;
    IdleState& operator=(const IdleState&) = delete;

    static IdleState& getInstance();

    void enter(Node& node) override;
    void process(Node& node) override;
    void exit(Node& node) override;

private:
    IdleState() = default;
};
