#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>

void LCD_Command(uint8_t command)
{
	PORTD=command;
	
	PORTB|=(1<<0); //E
	_delay_us(20);
	PORTB&=~(1<<0); //E
	
	_delay_ms(2);
}

void LCD_Char(uint8_t data)
{
	PORTB|=(1<<1); //RS
	_delay_us(20);
	LCD_Command(data);
	PORTB&=~(1<<1); //RS
	
	_delay_ms(2);
}

void LCD_Init()
{
	LCD_Command(0b00111000); //8bit, 2line, 5x8
	LCD_Command(0b00001100); //on, no cursor, no blink
	LCD_Command(0b00000110); //increment, no shift
}

void LCD_Clear()
{
	LCD_Command(1);
}

void LCD_Home()
{
	LCD_Command(2);
}

void LCD_Print(char* x)
{
	while(*x)
	{
		LCD_Char(*x++);
	}
}

void LCD_Cursor(uint8_t x, uint8_t y)
{
	LCD_Command(0x80|(x+y*64));
}

//Screen Shift

void LCD_Custom(uint8_t add, uint8_t* data)
{
	LCD_Command(0x40|add);
	
	while(*data)
	{
		LCD_Char(*data++);
	}
	
	LCD_Cursor(0,0);
}

void Print_Meter(uint8_t val)
{
	LCD_Cursor(0,1);
	
	for(uint8_t i=0; i<16; ++i)
	{
		LCD_Char(0);
	}
	
	LCD_Cursor(0,1);
	
	val*=80.0/100.0;
	
	while(1)
	{
		if(val>5)
			LCD_Char(5);
		else
		{
			LCD_Char(val);
			break;
		}
		
		val-=5;
	}
}

uint16_t get_ADC()
{
	ADCSRA|=(1<<6); //Start Convertion
	while(ADCSRA&(1<<6)); //Wait
	return ADC;
}


int main(void)
{
	DDRB|=(1<<0)+(1<<1)+(1<<2);
	DDRD|=0xFF;
	
	LCD_Init();
	
	LCD_Clear();
	
	uint8_t bar[]={0b00000000,0b00010000,0b00011000,0b00011100,0b00011110,0b00011111};
		
	LCD_Command(0x40);
	
	for(uint8_t i=0; i<6; ++i)
	{
		for(uint8_t ii=0; ii<8; ++ii)
		{
			LCD_Char(bar[i]);
		}
	}
	
	
	ADMUX|=(1<<6); //REFS0 5V Reference, 0 Channel Set
	ADCSRA|=(1<<7)+(1<<2)+(1<<1)+(1<<0); //ADC On, 128 Prescaler
	
	
	TCCR1A|=(1<<0)+(1<<5); //8bit PWM, Compare Output
	TCCR1B|=(1<<2)+(1<<0); //1024 Prescaler
	

    while (1) 
    {
		LCD_Clear();
		LCD_Home();
		
		uint16_t val=get_ADC();
		
		uint8_t pwm=val*(256.0/1024.0);
		float voltage=val*(5.0/1024.0);
		uint8_t bar=val*(100.0/1024.0);
		
		OCR1B=pwm;
		
		char str[16];
		sprintf(str,"%d %.2f %d %d",val,voltage,bar,pwm);
		
		LCD_Print(str);
		Print_Meter(bar);
		
		//_delay_ms(100);
    }
}