#ifndef BLE_SYSTEM_H_
#define BLE_SYSTEM_H

/*
#define HAL_IO_RADIO_CSN       SS
#define HAL_IO_RADIO_REQN      SS
#define HAL_IO_RADIO_RDY       3
#define HAL_IO_RADIO_SCK       SCK
#define HAL_IO_RADIO_MOSI      MOSI
#define HAL_IO_RADIO_MISO      MISO
#define HAL_IO_RADIO_RESET     9
#define HAL_IO_RADIO_ACTIVE    8

//#define HAL_IO_LED0          2
//#define HAL_IO_LED1          6
*/

#define ENABLE_INTERRUPTS()   sei()
#define DISABLE_INTERRUPTS()  cli()
#define ARE_INTERRUPTS_ENABLED() ((SREG & 0x80) == 0x80)

#endif
