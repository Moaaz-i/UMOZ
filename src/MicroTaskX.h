/*
 * MicroTaskX Library for Arduino
 * ----------------------------------------------------------------------------
 * Version: 3.1.0 (Modular & High-Performance Edition)
 * ----------------------------------------------------------------------------
 */

#ifndef MICROTASKX_H
#define MICROTASKX_H

#include "MTXKernel.h"
#include "MTXUtils.h"

typedef MicroTaskXKernel<5> MicroTaskX;
typedef MTXUtils MTXUtils;

#define mtx MicroTaskX::getInstance()
#define mtxUtils MTXUtils::getInstance()

#endif
