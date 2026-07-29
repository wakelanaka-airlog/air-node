#pragma once

#include "NodeState.h"

class ErrorState : public NodeState
{
public:
    ErrorState(const ErrorState&) = delete;
    ErrorState& operator=(const ErrorState&) = delete;

    static ErrorState& getInstance();

    void enter(Node& node) override;
    void process(Node& node) override;
    void exit(Node& node) override;

private:
    ErrorState() = default;
};
