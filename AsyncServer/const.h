#pragma once
#ifndef _CONST_H_
#define _CONST_H_
#include<iostream>

#define MAX_SEND_QUEUE 1000
constexpr std::size_t HEAD_ID_LENGTH = 2;
constexpr std::size_t HEAD_DATA_LENGTH = 2;
constexpr std::size_t HEAD_TOTAL_LENGTH = 4;
constexpr std::size_t MAX_LENGTH = 2 * 1024;

#endif