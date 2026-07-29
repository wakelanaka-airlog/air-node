#pragma once

#include "NodeState.h"

class ReadingState : public NodeState
{
public:
    ReadingState(const ReadingState&) = delete;
    ReadingState& operator=(const ReadingState&) = delete;

    static ReadingState& getInstance();

    void enter(Node& node) override;
    void process(Node& node) override;
    void exit(Node& node) override;

private:
    ReadingState() = default;
};
