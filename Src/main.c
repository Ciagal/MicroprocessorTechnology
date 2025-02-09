#include "lcd1602.h"
#include "i2c.h"
#include "leds.h"
#include "klaw.h"
#include "sht35.h"
#include "frdm_bsp.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


// modes: 0 - displaying and measurement, 1 - changing limits
uint8_t mode = 0;

// flags for buttons
volatile uint8_t S1_press = 0;
volatile uint8_t S2_press = 0;
volatile uint8_t S3_press = 0;
volatile uint8_t S4_press = 0;

// globals for measurments
volatile float current_temperature = 0.0;
volatile float current_humidity = 0.0;

// LCD buffer
char display[24] = {0};

// alarms
uint8_t temp_alarm = 0;
uint8_t hum_alarm  = 0;
uint8_t both_alarm = 0;

// limits
float temp_min = 15.0;
float temp_max = 23.0;
float hum_min  = 40.0;
float hum_max  = 60.0;

#define BLINK_PERIOD 50


uint16_t blink_sec_timer = BLINK_PERIOD;  
uint8_t blink_sec_ready = 0;


void PORTA_IRQHandler(void) 
{
    uint32_t buf;
    buf = PORTA->ISFR & (S1_MASK | S2_MASK | S3_MASK | S4_MASK);
    switch (buf) 
    {
        case S1_MASK:
            DELAY(100);
            if (!(PTA->PDIR & S1_MASK)) 
            {
                DELAY(100);
                if (!(PTA->PDIR & S1_MASK)) 
                {
                    S1_press = 1;
                }
            }
            break;
        case S2_MASK:
            DELAY(100);
            if (!(PTA->PDIR & S2_MASK)) 
            {
                DELAY(100);
                if (!(PTA->PDIR & S2_MASK) && mode) 
                {
                    S2_press = 1;
                }
            }
            break;
        case S3_MASK:
            DELAY(100);
            if (!(PTA->PDIR & S3_MASK)) 
            {
                DELAY(100);
                if (!(PTA->PDIR & S3_MASK) && mode) 
                {
                    S3_press = 1;
                }
            }
            break;
        case S4_MASK:
            DELAY(100);
            if (!(PTA->PDIR & S4_MASK)) 
            {
                DELAY(100);
                if (!(PTA->PDIR & S4_MASK) && mode) 
                {
                    S4_press = 1;
                }
            }
            break;
        default:
            break;
    }
    PORTA->ISFR |= S1_MASK | S2_MASK | S3_MASK | S4_MASK;  // deleting flags
    NVIC_ClearPendingIRQ(PORTA_IRQn);
}


extern volatile uint8_t sht35_ready;
extern volatile uint8_t sht35_timer;

void SysTick_Handler(void)
{
    
    if (blink_sec_timer > 0)
    {
        blink_sec_timer--;
        if (blink_sec_timer == 0)
        {
            blink_sec_ready = 1;  // BLINK_PERIOD*10ms = 500ms
            blink_sec_timer = BLINK_PERIOD;  // restart blink timer
        }
    }
    
    // timer for sht35
    if (sht35_timer > 0) 
    {
        sht35_timer--;
        if (sht35_timer == 0) 
        {
            sht35_ready = 1;  
        }
    }
}



void Check_Alarms(float temperature, float humidity)
{
    both_alarm = 0;
    temp_alarm = 0;
    hum_alarm = 0;
    
    if ((temperature > temp_max || temperature < temp_min) &&
        (humidity    > hum_max || humidity    < hum_min))
    {
        both_alarm = 1;
    }
    else
    {
        if (temperature > temp_max || temperature < temp_min)
            temp_alarm = 1;
        if (humidity > hum_max || humidity < hum_min)
            hum_alarm = 1;
    }
		if (!(temp_alarm || hum_alarm || both_alarm))
    {
        PTB->PSOR = LED_R_MASK;
        PTB->PSOR = LED_B_MASK;
    }
}


void Alarms_Task(void)
{
  
    static uint8_t both_alarm_counter = 0;  
    static uint8_t temp_blink_state = 0;    
    static uint8_t hum_blink_state  = 0;     

    //5-time sequence for both alarms
		if (both_alarm)
    {
        if (both_alarm_counter < 10)
        {
            temp_blink_state = !temp_blink_state;
            if (temp_blink_state)
            {
                PTB->PCOR = LED_R_MASK;
								PTB->PSOR = LED_B_MASK;
                LCD1602_SetCursor(0, 0);
                sprintf(display, "Temp: %5.1f \xDF""C   ", current_temperature);
                LCD1602_Print(display);
								LCD1602_SetCursor(0, 1);
                sprintf(display, "Hum:  %5.1f %%", current_humidity);
                LCD1602_Print(display);
								
            }
            else
            {
                PTB->PSOR = LED_R_MASK;
								PTB->PSOR = LED_B_MASK;
                LCD1602_SetCursor(0, 0);
                LCD1602_Print("                ");
								LCD1602_SetCursor(0, 1);
                sprintf(display, "Hum:  %5.1f %%   ", current_humidity);
                LCD1602_Print(display);
            }
        }
				else if (both_alarm_counter == 10)
				{
					PTB->PSOR = LED_R_MASK;
					PTB->PSOR = LED_B_MASK;
					LCD1602_SetCursor(0, 0);
					LCD1602_Print("                ");
          
					
					hum_blink_state = 0; 
					
				} 
        else if (both_alarm_counter < 20)
        {
            hum_blink_state = !hum_blink_state;
            if (hum_blink_state)
            {
                PTB->PSOR = LED_R_MASK;
								PTB->PCOR = LED_B_MASK;
								LCD1602_SetCursor(0, 0);
                sprintf(display, "Temp: %5.1f \xDF""C   ", current_temperature);
                LCD1602_Print(display);
                LCD1602_SetCursor(0, 1);
                sprintf(display, "Hum:  %5.1f %%   ", current_humidity);
                LCD1602_Print(display);
            }
            else
            {
                PTB->PSOR = LED_R_MASK;
								PTB->PSOR = LED_B_MASK;
								LCD1602_SetCursor(0, 0);
                sprintf(display, "Temp: %5.1f \xDF""C   ", current_temperature);
                LCD1602_Print(display);
                LCD1602_SetCursor(0, 1);
                LCD1602_Print("                ");
            }
        }
				if (both_alarm_counter == 20)
        {
          PTB->PSOR = LED_R_MASK;
					PTB->PSOR = LED_B_MASK;
					LCD1602_SetCursor(0, 1);
          LCD1602_Print("                "); 
        }
        if (both_alarm_counter > 20)
        {
            PTB->PSOR = LED_R_MASK;
						PTB->PSOR = LED_B_MASK;
						LCD1602_SetCursor(0, 0);
            sprintf(display, "Temp: %5.1f \xDF""C   ", current_temperature);
            LCD1602_Print(display);
						LCD1602_SetCursor(0, 1);
            sprintf(display, "Hum:  %5.1f %%   ", current_humidity);
            LCD1602_Print(display);
						both_alarm_counter = 0; 

						temp_blink_state = 0; 
        }
				both_alarm_counter++;
    }
    // only temp_alarm
    else if (temp_alarm)
    {
        temp_blink_state = !temp_blink_state;
        if (temp_blink_state)
        {
            PTB->PCOR = LED_R_MASK;
						PTB->PSOR = LED_B_MASK;
            LCD1602_SetCursor(0, 0);
            sprintf(display, "Temp: %5.1f \xDF""C   ", current_temperature);
            LCD1602_Print(display);
						LCD1602_SetCursor(0, 1);
            sprintf(display, "Hum:  %5.1f %%   ", current_humidity);
            LCD1602_Print(display);
        }
        else
        {
            PTB->PSOR = LED_R_MASK;
						PTB->PSOR = LED_B_MASK;
            LCD1602_SetCursor(0, 0);
            LCD1602_Print("                ");
						LCD1602_SetCursor(0, 1);
            sprintf(display, "Hum:  %5.1f %%   ", current_humidity);
            LCD1602_Print(display);
        }
    }
    // only hum_alarm
    else if (hum_alarm)
    {
        hum_blink_state = !hum_blink_state;
        if (hum_blink_state)
        {
            PTB->PSOR = LED_R_MASK;
						PTB->PCOR = LED_B_MASK;
            LCD1602_SetCursor(0, 0);
            sprintf(display, "Temp: %5.1f \xDF""C   ", current_temperature);
            LCD1602_Print(display);
						LCD1602_SetCursor(0, 1);
            sprintf(display, "Hum:  %5.1f %%   ", current_humidity);
            LCD1602_Print(display);
        }
        else
        {
            PTB->PSOR = LED_R_MASK;
						PTB->PSOR = LED_B_MASK;
            LCD1602_SetCursor(0, 0);
            sprintf(display, "Temp: %5.1f \xDF""C   ", current_temperature);
            LCD1602_Print(display);
						LCD1602_SetCursor(0, 1);
            LCD1602_Print("                ");
        }
    }
    else
    {
        PTB->PSOR = LED_R_MASK;
        PTB->PSOR = LED_B_MASK;
    }
}


int main(void) 
{
    uint8_t selected_setting = 1;  // 1 = Temp Min, 2 = Temp Max, 3 = Hum Min, 4 = Hum Max

    Klaw_Init();
    Klaw_S1_4_Int();
    LED_Init();
    LCD1602_Init();
    LCD1602_Backlight(TRUE);
    LCD1602_ClearAll();
    SHT35_Init();
    SysTick_Config(SystemCoreClock / 100);  // SysTick 10ms
    SHT35_StartMeasurement();

    while(1)
    {
        if (S1_press)
        {
            mode = !mode;
            LCD1602_ClearAll();
            
						both_alarm = 0;					// reset for all alarms when you in limit setting mode
						temp_alarm = 0;
						hum_alarm = 0;
						PTB->PSOR = LED_R_MASK;
						PTB->PSOR = LED_B_MASK;
						
						S1_press = 0;
        }
        
        if (!mode)
        {
            
            if (sht35_ready)
            {
                if (SHT35_Read((float *)&current_temperature, (float *)&current_humidity)) 
                {
                    
                }
                SHT35_StartMeasurement();
            }
            
           
            Check_Alarms(current_temperature, current_humidity);
            
            
            if (!(temp_alarm || hum_alarm || both_alarm))
            {
                LCD1602_SetCursor(0, 0);
                sprintf(display, "Temp: %5.1f \xDF""C   ", current_temperature);
                LCD1602_Print(display);
                LCD1602_SetCursor(0, 1);
                sprintf(display, "Hum:  %5.1f %%   ", current_humidity);
                LCD1602_Print(display);
            }
            else
            {
               
                if (blink_sec_ready)
                {
                    Alarms_Task();
                    blink_sec_ready = 0;  
                }
            }
        }
        else  // limit setting mode
        {
            if (S4_press)
            {
                selected_setting = (selected_setting % 4) + 1;
                LCD1602_ClearAll();
                S4_press = 0;
            }
            
            LCD1602_SetCursor(0, 0);
            switch (selected_setting) 
            {
                case 1: LCD1602_Print("Set Temp Min:"); break;
                case 2: LCD1602_Print("Set Temp Max:"); break;
                case 3: LCD1602_Print("Set Hum Min:");  break;
                case 4: LCD1602_Print("Set Hum Max:");  break;
            }
            
            if (S3_press) 
            {
                if (selected_setting == 1 && temp_min + 0.5 < temp_max) temp_min += 0.5;
                else if (selected_setting == 2 && temp_max + 0.5 >= temp_min && temp_max < 125) temp_max += 0.5;
                else if (selected_setting == 3 && hum_min + 1.0 < hum_max) hum_min += 1.0;
                else if (selected_setting == 4 && hum_max + 1.0 >= hum_min && hum_max < 100) hum_max += 1.0;
                S3_press = 0;
            }
            
            if (S2_press) 
            {
                if (selected_setting == 1 && temp_min - 0.5 < temp_max && temp_min > -40) temp_min -= 0.5;
                else if (selected_setting == 2 && temp_max - 0.5 > temp_min) temp_max -= 0.5;
                else if (selected_setting == 3 && hum_min - 1.0 < hum_max && hum_min > 0) hum_min -= 1.0;
                else if (selected_setting == 4 && hum_max - 1.0 > hum_min) hum_max -= 1.0;
                S2_press = 0;
            }
            
            LCD1602_SetCursor(0, 1);
            switch (selected_setting) 
            {
                case 1: sprintf(display, "Temp: %5.1f \xDF""C   ", temp_min); break;
                case 2: sprintf(display, "Temp: %5.1f \xDF""C   ", temp_max); break;
                case 3: sprintf(display, "Hum: %5.1f   %%   ", hum_min); break;
                case 4: sprintf(display, "Hum: %5.1f   %%   ", hum_max); break;
            }
            LCD1602_Print(display);
        }
    }
}
