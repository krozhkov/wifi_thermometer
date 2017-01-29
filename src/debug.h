#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <Arduino.h>

#define debug(...) Serial.printf( __VA_ARGS__ )

#ifndef debug
#define debug(...)
#endif

#endif /* _DEBUG_H_ */
