#pragma once

class Node;

class NodeState
{
public:
    virtual ~NodeState() = default;

    virtual void enter(Node& node) = 0;
    virtual void process(Node& node) = 0;
    virtual void exit(Node& node) = 0;
};
