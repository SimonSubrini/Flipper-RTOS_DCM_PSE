/**********************************************************************
 *
 * serial.h: interfaz del driver serial
 *
 **********************************************************************/

#ifndef _SERIAL_H
#define _SERIAL_H

void serial_init(void);
char serial_get_char(void);
void serial_put_str_flash(const char *str); // Prototipo para strings en Flash

#endif /* _SERIAL_H */