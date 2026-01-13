//
// Created by scion on 1/12/2026.
//

#pragma once
#include <type_traits>

template<typename ENUM_T>
bool CheckEnumFlags(ENUM_T mask, ENUM_T e)
{
    return static_cast<std::underlying_type_t<ENUM_T>>(mask) & static_cast<std::underlying_type_t<ENUM_T>>(e);
}
