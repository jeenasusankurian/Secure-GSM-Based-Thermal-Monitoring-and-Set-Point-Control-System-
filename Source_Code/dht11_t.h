//dht11.h
#ifndef _DHT11_H_
#define _DHT11_H_

#include "types_t.h"

// Function Prototypes

void dht11_request(void);     // Send start signal to DHT11
void dht11_response(void);    // Wait for DHT11 response
u8   dht11_data(void);        // Read one byte from DHT11

void dht11(void);             // Complete read + display function

#endif
