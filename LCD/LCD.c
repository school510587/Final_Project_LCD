#include "LCDConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include <stdint.h>
#include <stdarg.h>

#define iabs(a) ((a)<0?(-a):(a))
#define make_E_pulse(l) { \
	GPIO_SetBits((l)->GPIO, (l)->E_Pin); \
	vTaskDelay(1); \
	GPIO_ResetBits((l)->GPIO, (l)->E_Pin); \
}

#define RS_0(l) {GPIO_ResetBits((l)->GPIO, (l)->RS_Pin);}
#define RS_1(l) {GPIO_SetBits((l)->GPIO, (l)->RS_Pin);}

#define RW_0(l) {GPIO_ResetBits((l)->GPIO, (l)->RW_Pin);}
#define RW_1(l) {GPIO_SetBits((l)->GPIO, (l)->RW_Pin);}

#define boundReset(l) { \
	(l)->row=((l)->row+(l)->col/20)%4; \
	(l)->col=(l)->col%20; \
	LCD_MOVE((l)->LCD, (l)->row, (l)->col); \
}

static const int16_t line_addr[]={0x80, 0xc0, 0x94, 0xd4};

char *itoa(int32_t, uint32_t);
void lwrite(LCD_ControllerTypeDef *lcdctl, const char *str);
void LCD_CMD(LCD_InitTypeDef *, uint16_t );
void LCD_DATA(LCD_InitTypeDef *, uint16_t);
void LCD_MOVE(LCD_InitTypeDef *lcd, uint8_t row, uint8_t col);
int32_t LCD_printf(LCD_ControllerTypeDef *lcdctl, const char *str, ...);
void lSetDB(LCD_InitTypeDef *lcd, uint16_t number){
	uint16_t sum=0;
	register int i;
	for(i=0; i<8; ++i){
		sum|=(number>>i)&0x1? lcd->DB_Pins[i]: 0;
	}

	GPIO_SetBits(lcd->GPIO, sum);
}

void lResetDB(LCD_InitTypeDef *lcd, uint16_t number){
	uint16_t sum=0;
	register int i;
	for(i=0; i<8; ++i){
		sum|=(number>>i)&0x1? lcd->DB_Pins[i]: 0;
	}

	GPIO_ResetBits(lcd->GPIO, sum);
}

LCD_ControllerTypeDef new_LCD_Controller(LCD_InitTypeDef *l){
	LCD_ControllerTypeDef c={ .LCD=l, .col=0, .row=0, .lprintf=LCD_printf};
	return c;
}

void LCD_Init(LCD_InitTypeDef *l){
	LCD_CMD(l, 0x38);
	vTaskDelay(5);

	LCD_CMD(l, 0x08);
	vTaskDelay(5);

	LCD_CMD(l, 0x01);
	vTaskDelay(5);

	LCD_CMD(l, 0x06);
	vTaskDelay(5);

	LCD_CMD(l, 0x0c);
	vTaskDelay(5);

	//LCD_DATA(l, 'A');
}

void LCD_CMD(LCD_InitTypeDef *lcd, uint16_t cmd){
	lSetDB(lcd, cmd);
	RS_0(lcd);
	RW_0(lcd);

	/* pulse */
	make_E_pulse(lcd);

	lResetDB(lcd, cmd);
}

void LCD_DATA(LCD_InitTypeDef *lcd, uint16_t data){
	lSetDB(lcd, data);

	RS_1(lcd);
	RW_0(lcd);

	/* pulse */
	make_E_pulse(lcd);

	lResetDB(lcd, data);
}

void LCD_MOVE(LCD_InitTypeDef *lcd, uint8_t row, uint8_t col){
	LCD_CMD(lcd, line_addr[row]+col);
}

void lwrite(LCD_ControllerTypeDef *lcdctl, const char *str){
	//Jump to
	boundReset(lcdctl);
	vTaskDelay(10);

	for(;*str;str++){
		switch(*str){
			case '\n':
				lcdctl->row++;
				break;
			case '\r':
				lcdctl->col=0;
				break;
			default:
				LCD_DATA(lcdctl->LCD, *str);
				vTaskDelay(1);
				lcdctl->col++;
		}

		boundReset(lcdctl);
	}
}

int32_t LCD_printf(LCD_ControllerTypeDef *lcdctl, const char *str, ...){
	int p=0, vaint;
	va_list v1;
	va_start(v1, str);
	boundReset(lcdctl);
	while(str[p]){
		if(str[p]=='%'){
			switch(str[p+1]){
				case 'd': /* integer in dec */
					vaint=va_arg(v1, int);
					lwrite(lcdctl, itoa(vaint, 10));
					p++;
			}
		}else{
			LCD_DATA(lcdctl->LCD, str[p]);
			lcdctl->col++;
		}
		boundReset(lcdctl);
		vTaskDelay(1);
		p++;
	}
	va_end(v1);
	return 0;
}

char *itoa(int32_t n, uint32_t base){
	static char buf[32]={0};
	int neg=n<0?1:0;
	uint32_t num=iabs(n);
	if(num==0){
		buf[30]='0';
		return &buf[30];
	}
	int i;
	for(i=30;num!=1; --i, num/=base){
		buf[i]= "0123456789abcdef" [num % base];
	}

	buf[i]='-';

	return neg ? &buf[i]: &buf[i+1];
}
