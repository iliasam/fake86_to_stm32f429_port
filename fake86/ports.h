#ifndef _PORTS_H
#define _PORTS_H

void portout (uint16_t portnum, uint8_t value);
uint8_t portin (uint16_t portnum);
void set_port_write_redirector (uint16_t startport, uint16_t endport, void *callback);
void set_port_read_redirector (uint16_t startport, uint16_t endport, void *callback);
void portout16 (uint16_t portnum, uint16_t value);
uint16_t portin16 (uint16_t portnum);



#endif

