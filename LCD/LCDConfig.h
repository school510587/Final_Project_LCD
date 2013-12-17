#ifndef LCDCONFIG_H
#define LCDCONFIG_H

#include <stdint.h>
#include "main.h"
/*
	LCD Library for STM32Fx
*/

#define LCD_GPIO GPIOE
/* high=data mode Low=command mode */
#define LCD_RS_Pin GPIO_Pin_11
/* high=read low=write */
#define LCD_RW_Pin GPIO_Pin_12
#define LCD_E_Pin GPIO_Pin_13

#define LCD_DB0 GPIO_Pin_3
#define LCD_DB1 GPIO_Pin_4
#define LCD_DB2 GPIO_Pin_5
#define LCD_DB3 GPIO_Pin_6
#define LCD_DB4 GPIO_Pin_7
#define LCD_DB5 GPIO_Pin_8
#define LCD_DB6 GPIO_Pin_9
#define LCD_DB7 GPIO_Pin_10

typedef struct {
	uint16_t RS_Pin;
	uint16_t RW_Pin;
	uint16_t E_Pin;

	/* DB 0 to 7 */
	uint16_t DB_Pins[8];
	GPIO_TypeDef *GPIO;
} LCD_InitTypeDef;

struct LCD_Controller;

typedef struct LCD_Controller LCD_ControllerTypeDef;

struct LCD_Controller{
	/* Which LCD used */
	LCD_InitTypeDef *LCD;
	uint8_t col, row;

	int32_t (*lprintf)(LCD_ControllerTypeDef *, const char *, ...);
};


LCD_ControllerTypeDef new_LCD_Controller(LCD_InitTypeDef *);
void LCD_Init(LCD_InitTypeDef *);
void lwrite(LCD_ControllerTypeDef *, const char *);

/* commands */
enum LCD_CMD {
	LCD_CLEARDISPLAY	=0x01,
	LCD_RETURNHOME		=0x02,
	LCD_ENTRYMODESET	=0x04,
	LCD_DISPLAYCONTROL	=0x08,
	LCD_CURSORSHIFT		=0x10,
	LCD_FUNCTIONSET		=0x20,
	LCD_SETCGRAMADDR	=0x40,
	LCD_SETDDRAMADDR	=0x80
};

/* flags for display entry mode */
enum LCD_ENTRY_FLAGS {
	LCD_ENTRYRIGHT		=0x00,
	LCD_ENTRYLEFT		=0x02,
	LCD_ENTRYSHIFTINCREMENT	=0x01,
	LCD_ENTRYSHIFTDECREMENT	=0x00
};

/* flags for display on/off control */
enum LCD_DISPLAY {
	LCD_DISPLAYON		=0x04,
	LCD_DISPLAYOFF		=0x00,
	LCD_CURSORON		=0x02,
	LCD_CURSOROFF		=0x00,
	LCD_BLINKON		=0x01,
	LCD_BLINKOFF		=0x00
};

/* flags for display/cursor shift */
enum LCD_MOVE {
	LCD_DISPLAYMOVE		=0x08,
	LCD_CURSORMOVE		=0x00,
	LCD_MOVERIGHT		=0x04,
	LCD_MOVELEFT		=0x00
};

/* flags for function set */
enum LCD_FUNSET {
	LCD_8BITMODE		=0x10,
	LCD_4BITMODE		=0x00,
	LCD_2LINE		=0x08,
	LCD_1LINE		=0x00,
	LCD_5x10DOTS		=0x04,
	LCD_5x8DOTS		=0x00
};
#endif
