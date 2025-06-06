#ifndef RANK_OUTPUT_ARRAY_HPP
#define RANK_OUTPUT_ARRAY_HPP

#include <array>

const std::array<std::array<int, 48>, 16> rank = {{
    {2, 16, 16, 6, 6, 8, 8, 8, 3, 3, 11, 16, 16, 16, 16, 16, 15, 15, 15, 2, 2, 2, 1, 16, 16, 16, 8, 8, 15, 15, 2, 2, 2, 2, 10, 10, 8, 8, 8, 3, 3, 3, 3, 10, 6, 6, 6, 3},
    {13, 1, 1, 16, 16, 12, 12, 7, 9, 9, 3, 3, 1, 1, 1, 12, 6, 5, 5, 15, 4, 13, 2, 6, 6, 6, 6, 15, 14, 14, 9, 9, 11, 8, 2, 8, 10, 7, 7, 8, 5, 13, 13, 6, 10, 16, 11, 8},
    {11, 2, 6, 10, 3, 3, 3, 3, 14, 11, 4, 11, 4, 9, 12, 7, 1, 2, 2, 5, 5, 4, 6, 10, 10, 8, 16, 2, 2, 2, 15, 5, 5, 11, 8, 13, 7, 5, 5, 7, 11, 11, 11, 3, 3, 10, 9, 5},
    {3, 6, 10, 1, 8, 16, 7, 12, 7, 4, 9, 4, 9, 12, 7, 9, 16, 6, 4, 4, 15, 1, 10, 2, 8, 10, 15, 14, 13, 9, 14, 11, 9, 5, 11, 2, 13, 14, 14, 5, 13, 9, 9, 13, 8, 9, 16, 4},
    {16, 3, 3, 3, 10, 7, 15, 15, 15, 7, 16, 1, 7, 4, 9, 1, 4, 4, 6, 6, 6, 6, 4, 8, 2, 15, 2, 16, 10, 10, 16, 15, 8, 10, 5, 5, 14, 10, 13, 14, 14, 5, 14, 11, 11, 12, 12, 7},
    {4, 10, 2, 8, 12, 6, 16, 14, 11, 15, 7, 9, 12, 7, 4, 4, 12, 1, 1, 1, 1, 10, 13, 13, 13, 2, 10, 13, 16, 13, 6, 16, 15, 7, 13, 11, 5, 13, 11, 11, 8, 14, 8, 8, 9, 13, 14, 10},
    {9, 11, 11, 2, 1, 15, 14, 2, 12, 16, 6, 7, 3, 5, 5, 15, 9, 3, 3, 10, 7, 5, 15, 1, 4, 13, 13, 10, 6, 16, 5, 6, 16, 16, 7, 7, 11, 11, 10, 13, 7, 7, 6, 4, 13, 7, 13, 13},
    {1, 13, 8, 9, 7, 10, 2, 11, 16, 12, 1, 12, 11, 6, 6, 6, 2, 9, 9, 3, 13, 15, 16, 4, 15, 4, 12, 12, 9, 6, 11, 10, 6, 15, 16, 14, 2, 3, 3, 10, 12, 8, 5, 9, 4, 11, 10, 1},
    {6, 4, 4, 12, 2, 2, 6, 16, 6, 6, 12, 6, 6, 14, 15, 5, 5, 16, 11, 7, 10, 7, 8, 15, 1, 14, 14, 6, 12, 12, 10, 14, 10, 9, 15, 1, 1, 2, 12, 12, 10, 12, 7, 7, 12, 8, 7, 12},
    {8, 8, 13, 7, 15, 14, 11, 6, 1, 1, 15, 14, 14, 3, 14, 3, 3, 12, 12, 13, 3, 16, 5, 5, 14, 12, 4, 9, 8, 3, 12, 3, 7, 13, 1, 3, 3, 12, 2, 9, 9, 10, 10, 5, 7, 4, 3, 14},
    {14, 9, 9, 13, 13, 1, 1, 1, 8, 14, 14, 15, 5, 8, 3, 2, 7, 11, 10, 11, 16, 8, 7, 14, 3, 1, 3, 3, 3, 5, 13, 8, 3, 6, 14, 15, 12, 1, 4, 4, 4, 6, 12, 14, 14, 14, 5, 9},
    {5, 14, 7, 11, 11, 11, 10, 10, 10, 8, 13, 8, 8, 11, 11, 14, 11, 7, 16, 9, 11, 3, 3, 7, 5, 3, 9, 4, 4, 4, 3, 12, 4, 3, 3, 16, 15, 15, 9, 6, 6, 4, 4, 12, 5, 3, 8, 16},
    {15, 7, 12, 15, 14, 13, 13, 13, 4, 13, 8, 5, 13, 10, 8, 11, 10, 10, 7, 12, 14, 14, 14, 3, 9, 9, 1, 1, 5, 11, 4, 4, 1, 1, 9, 12, 4, 4, 1, 2, 15, 15, 15, 16, 16, 5, 4, 6},
    {7, 15, 15, 4, 4, 4, 4, 9, 13, 10, 10, 13, 10, 15, 10, 10, 14, 14, 13, 16, 9, 11, 11, 9, 7, 5, 5, 5, 1, 8, 1, 13, 12, 4, 6, 6, 16, 9, 15, 15, 2, 16, 16, 15, 15, 15, 1, 11},
    {10, 12, 14, 14, 9, 5, 5, 4, 2, 2, 5, 10, 15, 13, 13, 13, 13, 13, 14, 14, 8, 9, 9, 11, 12, 7, 11, 11, 11, 1, 8, 1, 13, 12, 12, 9, 9, 6, 6, 1, 1, 2, 2, 2, 2, 1, 15, 15},
    {12, 5, 5, 5, 5, 9, 9, 5, 5, 5, 2, 2, 2, 2, 2, 8, 8, 8, 8, 8, 12, 12, 12, 12, 11, 11, 7, 7, 7, 7, 7, 7, 14, 14, 4, 4, 6, 16, 16, 16, 16, 1, 1, 1, 1, 2, 2, 2},
}};

#endif // RANK_OUTPUT_ARRAY_HPP
