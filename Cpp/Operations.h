#pragma once

#include <cstdint>

enum Operations : uint32_t
{
    plus = 0,
    minus,
    divide,
    multiply,
    sinOp,
    cosOp
};

enum Results : uint32_t
{
    success = 100,
    fail
};