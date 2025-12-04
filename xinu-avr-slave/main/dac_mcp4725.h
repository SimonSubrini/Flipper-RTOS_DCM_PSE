/*
 * dac_mcp4725.h
 */

#ifndef DAC_MCP4725_H_
#define DAC_MCP4725_H_

#include "i2c.h"


void dac_init(void);
void tx2dac(unsigned char pcm_value);

#endif /* DAC_MCP4725_H_ */