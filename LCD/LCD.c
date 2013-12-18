#include "LCD_curses.h"
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include <stdint.h>
#include <stdarg.h>

#define iabs(a) ((a)<0?(-a):(a))

#define boundReset(l) { \
	(l)->row = ((l)->row + (l)->col / (l)->max_col) % (l)->max_row; \
	(l)->col %= (l)->max_col; \
	LCD_move((l), (l)->row, (l)->col); \
}

/* state configuration of RS and RW pins */
enum {
	RS_0 = 0, /* command mode */
	RS_1 = 2, /* data mode */
	RW_0 = 0, /* write mode */
	RW_1 = 1  /* read mode */
};

static const int16_t line_addr[] = {0x80, 0xc0, 0x94, 0xd4};

char *itoa(int32_t, uint32_t);

/* The internal low-level LCD operation */
static int LCD_send(const LCD_InitTypeDef *lcd, int ctrl, uint16_t data)
{
	uint16_t sum = 0;
	register int i;

	if (!lcd)
		return LCD_ERR;

	/* set data pins */
	for (i = 0; i < 8; ++i)
		sum |= (data >> i) & 0x1 ? lcd->DB_Pins[i] : 0;
	GPIO_SetBits(lcd->GPIO, sum);

	/* set RS and RW pins */
	if (ctrl & RS_1)
		GPIO_SetBits(lcd->GPIO, lcd->RS_Pin);
	else
		GPIO_ResetBits(lcd->GPIO, lcd->RS_Pin);
	if (ctrl & RW_1)
		GPIO_SetBits(lcd->GPIO, lcd->RW_Pin);
	else
		GPIO_ResetBits(lcd->GPIO, lcd->RW_Pin);

	/* send a pulse on E pin */
	GPIO_SetBits(lcd->GPIO, lcd->E_Pin);
	vTaskDelay(1);
	GPIO_ResetBits(lcd->GPIO, lcd->E_Pin);

	/* reset data pins */
	GPIO_ResetBits(lcd->GPIO, sum);

	return LCD_OK;
}

void LCD_Init(LCD_InitTypeDef *l)
{
	GPIO_ResetBits(l->GPIO, l->RS_Pin | l->RW_Pin | l->E_Pin);

	LCD_send(l, RS_0|RW_0, 0x38);
	vTaskDelay(5);

	LCD_send(l, RS_0|RW_0, 0x08);
	vTaskDelay(5);

	LCD_send(l, RS_0|RW_0, 0x01);
	vTaskDelay(5);

	LCD_send(l, RS_0|RW_0, 0x06);
	vTaskDelay(5);

	LCD_send(l, RS_0|RW_0, 0x0c);
	vTaskDelay(5);
}

int LCD_addch(LCD_InitTypeDef *lcd, uint16_t data)
{
	return LCD_send(lcd, RS_1|RW_0, data);
}

int LCD_addstr(LCD_InitTypeDef *lcdctl, const char *str)
{
	if (!lcdctl)
		return LCD_ERR;

	boundReset(lcdctl);
	vTaskDelay(10);

	for (; *str; str++) {
		switch (*str) {
			case '\n':
				lcdctl->row++;
				break;
			case '\r':
				lcdctl->col = 0;
				break;
			default:
				LCD_addch(lcdctl, *str);
				vTaskDelay(1);
				lcdctl->col++;
				break;
		}

		boundReset(lcdctl);
	}

	return LCD_OK;
}

int LCD_move(LCD_InitTypeDef *lcd, int row, int col)
{
	if (!lcd || row < 0 || col < 0 || row >= lcd->max_row - 1 || col >= lcd->max_col - 1)
		return LCD_ERR;
	return LCD_send(lcd, RS_0|RW_0, line_addr[row]+col);
}

int LCD_printf(LCD_InitTypeDef *lcdctl, const char *str, ...)
{
	int p = 0, vaint;
	va_list v1;

	va_start(v1, str);
	boundReset(lcdctl);
	while (str[p]) {
		if (str[p] == '%') {
			switch (str[p + 1]) {
				case 'd': /* integer in dec */
					vaint = va_arg(v1, int);
					LCD_addstr(lcdctl, itoa(vaint, 10));
					p++;
			}
		}
		else {
			LCD_addch(lcdctl, str[p]);
			lcdctl->col++;
		}
		boundReset(lcdctl);
		vTaskDelay(1);
		p++;
	}
	va_end(v1);
	return 0;
}

char *itoa(int32_t n, uint32_t base)
{
	static char buf[32] = {0};
	int neg = n < 0 ? 1 : 0;
	uint32_t num = iabs(n);

	if (num == 0) {
		buf[30] = '0';
		return &buf[30];
	}
	int i;
	for (i = 30; num != 1; --i, num/=base)
		buf[i] = "0123456789abcdef"[num % base];

	buf[i] = '-';

	return neg ? &buf[i]: &buf[i + 1];
}
