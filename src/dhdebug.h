/**
 *  \file   dhdebug.h
 *  \brief    Print debug text.
 *  \author   Nikolay Khabarov
 *  \date   2015
 *  \copyright  DeviceHive MIT
 */

#ifndef _DHDEBUG_H_
#define _DHDEBUG_H_

#include <Arduino.h>

#define dhdebug(...) Serial.printf( __VA_ARGS__ )

#ifndef dhdebug
#define dhdebug(...)
#endif

#ifndef dhdebug_direct
#define dhdebug_direct(...)
#endif

#endif /* _DHDEBUG_H_ */

