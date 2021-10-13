#include "System/system.h"
#include "System/delay.h"
#include "System/clock.h"

#include "oledDriver/oledC.h"
#include "oledDriver/oledC_colors.h"
#include "oledDriver/oledC_shapes.h"

#define PORT_S1 PORTAbits.RA11
#define PORT_S2 PORTAbits.RA12
#define LED_1 LATAbits.LATA8
#define LED_2 LATAbits.LATA9

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


unsigned char clockCords[60][2] = {{47,7},{51,7},{55,8},{59,9},{63,10},{67,12},{71,15},{74,17},{77,20},{79,23},{82,27},{84,31},{85,35},{86,39},{87,43},
                                    {87,47},{87,51},{86,55},{85,59},{84,63},{82,67},{79,71},{77,74},{74,77},{71,79},{67,82},{63,84},{59,85},{55,86},{51,87},
                                    {47,87},{43,87},{39,86},{35,85},{31,84},{27,82},{23,79},{20,77},{17,74},{15,71},{12,67},{10,63},{9,59},{8,55},{7,51},
                                    {7,47},{7,43},{8,39},{9,35},{10,31},{12,27},{15,23},{17,20},{20,17},{23,15}, {27,12},{31,10},{35,9},{39,8},{43,7} };

unsigned daysPerMonth[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

unsigned char midPoint[2] = {47,47};

Controller controller;
DateTime setClock, prevTime, currentTime;

void digitsToStr(int value, char *str)
{
	str[0] = (value / 10) + 48;
	str[1] = (value % 10) + 48;
	str[2] = '\0';
}
void getDateStr(DateTime *time, char *str)
{
	char date_string[6];
	char buff[3];
    
	digitsToStr(time->day, buff);
	strcpy(date_string, buff);
	strcat(date_string, "/");
	digitsToStr(time->month, buff);
	strcat(date_string, buff);

	strcpy(str, date_string);
}
void getTimeStr(DateTime *time, char *str)
{
	char time_string[9];

	char buff[3];

	int temp_hrs = time->hours;
    
	if (controller.ampmMode == 0 && temp_hrs >= 12) 
      temp_hrs -= 12;

	digitsToStr(temp_hrs, buff);
	strcpy(time_string, buff);
	strcat(time_string, ":");

	digitsToStr(time->minutes, buff);
	strcat(time_string, buff);
	strcat(time_string, ":");

	digitsToStr(time->seconds, buff);
	strcat(time_string, buff);

	strcpy(str, time_string);
}

void showAMPM(void)
{
	cleanAMPM();

	if (currentTime.hours > 12)
	{
        oledC_DrawString(1, 88, 1, 1, "PM", OLEDC_COLOR_GREEN);
	}
	else
	{
        oledC_DrawString(1, 88, 1, 1, "AM", OLEDC_COLOR_GREEN);
	}
}
void showDate(void)
{
	cleanDate();

	char date[6];
    
	getDateStr(&currentTime, date);

	oledC_DrawString(65, 88, 1, 1, date, OLEDC_COLOR_GREEN);
}
void showAnalogHands(void)
{
	oledC_DrawCircle(midPoint[0], midPoint[1], 4, OLEDC_COLOR_GOLD);

	unsigned char x,y;
    
    x = midPoint[0] + (clockCords[currentTime.hours *5 % 60][0] - midPoint[0]) / 2;
	y = midPoint[1] + (clockCords[currentTime.hours *5 % 60][1] - midPoint[1]) / 2;
    
	oledC_DrawLine(midPoint[0], midPoint[1],  x, y, 3, OLEDC_COLOR_GOLD);
    
	x = clockCords[currentTime.seconds][0] - (clockCords[currentTime.seconds][0] - midPoint[0]) / 5;
	y = clockCords[currentTime.seconds][1] - (clockCords[currentTime.seconds][1] - midPoint[1]) / 5;
    
	oledC_DrawLine(midPoint[0], midPoint[1], x, y, 1, OLEDC_COLOR_GOLD);
    
	x = clockCords[currentTime.minutes][0] - (clockCords[currentTime.minutes][0] - midPoint[0]) / 5;
	y = clockCords[currentTime.minutes][1] - (clockCords[currentTime.minutes][1] - midPoint[1]) / 5;
    
	oledC_DrawLine(midPoint[0], midPoint[1],  x, y, 2, OLEDC_COLOR_GOLD);
    
	if (controller.AMPM_Visible)
	{
		showAMPM();
		controller.AMPM_Visible = false;
	}
	if (controller.DATE_Visible)
	{
		showDate();
		controller.DATE_Visible = false;
	}
}
void showAnalog(void)
{
	oledC_clearScreen();
	uint8_t i;

	unsigned char newPoint[2];
	unsigned char width = 2;
    
	for (i = 0; i < 60; i++)
	{
		if (i % 5 == 0)
		{
			newPoint[0] = midPoint[0] + 9 *(clockCords[i][0] - midPoint[0]) / 10;
			newPoint[1] = midPoint[1] + 9 *(clockCords[i][1] - midPoint[1]) / 10;
			oledC_DrawLine(newPoint[0], newPoint[1], clockCords[i][0], clockCords[i][1], width, OLEDC_COLOR_GOLD);
		}
	}

	controller.AMPM_Visible = true;
	controller.DATE_Visible = true;
	showAnalogHands();


	prevTime = currentTime;
}
void showDigitalLCD(void)
{
	char time[9];
	getTimeStr(&currentTime, time);

	oledC_DrawString(1, 37, 2, 2, time, OLEDC_COLOR_GOLD);

	if (controller.ampmMode == 0 && controller.AMPM_Visible)
	{
		showAMPM();
		controller.AMPM_Visible = false;
	}

	if (controller.DATE_Visible)
	{
		showDate();
		controller.DATE_Visible = false;
	}
}
void showDigital(void)
{
	oledC_clearScreen();
	showDigitalLCD();

    controller.DATE_Visible = true;

	if (controller.ampmMode == 0) controller.AMPM_Visible = true;

    prevTime = currentTime;
}
void showMenu(void)
{
	oledC_clearScreen();

	controller.menuOption = 0;
	controller.numberMOptions = 5;

	oledC_DrawString(12, 1, 1, 1, "Set Mode", OLEDC_COLOR_GOLD);
	oledC_DrawString(12, 12, 1, 1, "Set Time", OLEDC_COLOR_GOLD);
	oledC_DrawString(12, 23, 1, 1, "Set Date", OLEDC_COLOR_GOLD);
	oledC_DrawString(12, 34, 1, 1, "12H/24H Int.", OLEDC_COLOR_GOLD);
	oledC_DrawString(12, 44, 1, 1, "Alarm", OLEDC_COLOR_GOLD);
	showMenuStatus();

	if (controller.ampmMode == 0)
        controller.AMPM_Visible = true;

}
void showMenuStatus(void)
{
	char time[9];
	getTimeStr(&currentTime, time);

	oledC_DrawString(48, 88, 1, 1, time, OLEDC_COLOR_GREEN);
	oledC_DrawString(3, 1 + (controller.menuOption *11), 1, 1, "*", OLEDC_COLOR_GREEN);

	if (controller.ampmMode == 0 && controller.AMPM_Visible)
	{
		showAMPM();
		controller.AMPM_Visible = false;
	}
}
void showSetModeMenu(void)
{
	oledC_clearScreen();

	controller.menuOption = 0;
	controller.numberMOptions = 2;

	oledC_DrawString(12, 1, 1, 1, "Analog", OLEDC_COLOR_GOLD);
	oledC_DrawString(12, 12, 1, 1, "Digital", OLEDC_COLOR_GOLD);
	showMenuStatus();

	if (controller.ampmMode == 0) controller.AMPM_Visible = true;
}
void showSetTimeMenu(void)
{
	oledC_clearScreen();

	controller.menuOption = 0;
	controller.numberMOptions = 5;

    setClock.day = 8;
    setClock.month = 10;
    setClock.seconds = 16;
	setClock.minutes = 5;
    setClock.hours = 9;
	setClock = currentTime;
    
	char time[3];
	char option[16];

	digitsToStr(setClock.hours, time);
	strcpy(option, "Hours: ");
	option[7] = time[0];
	option[8] = time[1];
	option[9] = '\0';
	oledC_DrawString(12, 1, 1, 1, option, OLEDC_COLOR_GOLD);

	digitsToStr(setClock.minutes, time);
	strcpy(option, "Minutes: ");
	option[9] = time[0];
	option[10] = time[1];
	option[11] = '\0';
	oledC_DrawString(12, 12, 1, 1, option, OLEDC_COLOR_GOLD);

	digitsToStr(setClock.seconds, time);
	strcpy(option, "Seconds: ");
	option[9] = time[0];
	option[10] = time[1];
	option[11] = '\0';
	oledC_DrawString(12, 23, 1, 1, option, OLEDC_COLOR_GOLD);

	oledC_DrawString(12, 34, 1, 1, "Confirm", OLEDC_COLOR_GOLD);
	oledC_DrawString(12, 45, 1, 1, "Back", OLEDC_COLOR_GOLD);

	showMenuStatus();

	if (controller.ampmMode == 0) controller.AMPM_Visible = true;

}
void showSetTimeCases(void)
{
	char time[3];

	switch (controller.menuOption)
	{
		case 0:
			setClock.hours = (setClock.hours + 1) % 24;
			oledC_DrawRectangle(54, 1, 64, 9, OLEDC_COLOR_BLACK);
			digitsToStr(setClock.hours, time);
			oledC_DrawString(54, 1, 1, 1, time, OLEDC_COLOR_GOLD);
			break;
		case 1:
			setClock.minutes = (setClock.minutes + 1) % 60;
			oledC_DrawRectangle(66, 12, 78, 20, OLEDC_COLOR_BLACK);
			digitsToStr(setClock.minutes, time);
			oledC_DrawString(66, 12, 1, 1, time, OLEDC_COLOR_GOLD);
			break;
		case 2:
			setClock.seconds = 0;
			oledC_DrawRectangle(66, 23, 78, 9, OLEDC_COLOR_BLACK);
			digitsToStr(setClock.seconds, time);
			oledC_DrawString(66, 23, 1, 1, time, OLEDC_COLOR_GOLD);
			break;
		case 3:
			currentTime = setClock;
			controller.currMenu = 0;
			showMenu();
			break;
		case 4:
			controller.currMenu = 0;
			showMenu();
			break;
		default:
			break;
	}
}
void showHourMenu(void)
{
	oledC_clearScreen();

	controller.menuOption = 0;
	controller.numberMOptions = 2;

	oledC_DrawString(12, 1, 1, 1, "12H Interval", OLEDC_COLOR_GOLD);
	oledC_DrawString(12, 12, 1, 1, "24H Interval", OLEDC_COLOR_GOLD);
	showMenuStatus();

	if (controller.ampmMode == 0)
        controller.AMPM_Visible = true;
}
void showSetDateMenu(void) {

    oledC_clearScreen();
    
    controller.menuOption = 0;
    controller.numberMOptions = 4;
    
    setClock.day = 8;
    setClock.month = 10;
    setClock.seconds = 16;
	setClock.minutes = 5;
    setClock.hours = 9;
    setClock = currentTime;
    char date[3];
    char option[16];
    
    digitsToStr(setClock.month, date);
    strcpy(option, "Month: ");
    option[7] = date[0];
    option[8] = date[1];
    option[9] = '\0';
    oledC_DrawString(12, 1, 1, 1, option, OLEDC_COLOR_GOLD); 
    
    digitsToStr(setClock.day, date);
    strcpy(option, "Day: ");
    option[5] = date[0];
    option[6] = date[1];
    option[7] = '\0';
    oledC_DrawString(12, 12, 1, 1, option, OLEDC_COLOR_GOLD);
    
    oledC_DrawString(12, 23, 1, 1, "Confirm", OLEDC_COLOR_GOLD);
    oledC_DrawString(12, 34, 1, 1, "Back", OLEDC_COLOR_GOLD);
    
    showMenuStatus();  
    
    if(controller.ampmMode == 0)
    {
        controller.AMPM_Visible = true;
    }
}
void showSetDateCases(void) {
    
    char date[3];
    
     switch(controller.menuOption) {
         
        case 0:
            setClock.month = (setClock.month + 1) % 13;
            oledC_DrawRectangle(54, 1, 64, 9, OLEDC_COLOR_BLACK);
            digitsToStr(setClock.month, date);
            oledC_DrawString(54, 1, 1, 1, date, OLEDC_COLOR_GOLD);
            break;
        case 1:
            setClock.day = (setClock.day + 1) % 32;
            oledC_DrawRectangle(43, 12, 78, 20, OLEDC_COLOR_BLACK);
            digitsToStr(setClock.day, date);
            oledC_DrawString(43, 12, 1, 1, date, OLEDC_COLOR_GOLD);
            break;
        case 2:
            currentTime = setClock;
            controller.currMenu = 0;
            showMenu();
            break;
        case 3:
            controller.currMenu = 0;
            showMenu();
            break;
        default:
            break;
    }
}

void cleanAMPM(void)
{
	oledC_DrawRectangle(1, 88, 15, 95, OLEDC_COLOR_BLACK);
}
void cleanDate(void)
{
	oledC_DrawRectangle(68, 88, 95, 95, OLEDC_COLOR_BLACK);
}
void cleanAnalog(void)
{
	unsigned char vals[2];
	vals[0] = clockCords[prevTime.seconds][0] - (clockCords[prevTime.seconds][0] - midPoint[0]) / 5;
	vals[1] = clockCords[prevTime.seconds][1] - (clockCords[prevTime.seconds][1] - midPoint[1]) / 5;
	oledC_DrawLine(midPoint[0], midPoint[1], vals[0], vals[1], 1, OLEDC_COLOR_BLACK);
	if (prevTime.minutes != currentTime.minutes)
	{
		vals[0] = clockCords[prevTime.minutes][0] - (clockCords[prevTime.minutes][0] - midPoint[0]) / 5;
		vals[1] = clockCords[prevTime.minutes][1] - (clockCords[prevTime.minutes][1] - midPoint[1]) / 5;
		oledC_DrawLine(midPoint[0], midPoint[1], vals[0], vals[1], 2, OLEDC_COLOR_BLACK);
	}

	if (prevTime.hours != currentTime.hours)
	{
		vals[0] = midPoint[0] + (clockCords[prevTime.hours *5 % 60][0] - midPoint[0]) / 2;
		vals[1] = midPoint[1] + (clockCords[prevTime.hours *5 % 60][1] - midPoint[1]) / 2;
		oledC_DrawLine(midPoint[0], midPoint[1], vals[0], vals[1], 4, OLEDC_COLOR_BLACK);
	}

	prevTime = currentTime;
}
void cleanDigital(void)
{
	int x1, y1, x2, y2;
    
	y1 = 37; y2 = 53;

	if (currentTime.seconds % 10 == 0)
	{
		x1 = 67; x2 = 95;
		oledC_DrawRectangle(x1, y1, x2, y2, OLEDC_COLOR_BLACK);
	}
	else
	{
		x1 = 78; x2 = 95;
		oledC_DrawRectangle(x1, y1, x2, y2, OLEDC_COLOR_BLACK);
	}

	if (prevTime.minutes != currentTime.minutes)
	{
		if (currentTime.minutes % 10 == 0)
		{
			x1 = 34; x2 = 56;
			oledC_DrawRectangle(x1, y1, x2, y2, OLEDC_COLOR_BLACK);
		}
		else
		{
			x1 = 45; x2 = 56;
			oledC_DrawRectangle(x1, y1, x2, y2, OLEDC_COLOR_BLACK);
		}
	}

	if (prevTime.hours != currentTime.hours)
	{
		if (currentTime.hours % 10 == 0)
		{
			x1 = 1; x2 = 23;
			oledC_DrawRectangle(x1, y1, x2, y2, OLEDC_COLOR_BLACK);
		}
		else
		{
			x1 = 12; x2 = 23;
			oledC_DrawRectangle(x1, y1, x2, y2, OLEDC_COLOR_BLACK);
		}
	}

	prevTime = currentTime;
}
void cleanMenu(void)
{
	int x1, y1, x2, y2;
	y1 = 88;
	y2 = 95;

	if (currentTime.seconds % 10 == 0)
	{
		x1 = 84;
		x2 = 95;
		oledC_DrawRectangle(x1, y1, x2, y2, OLEDC_COLOR_BLACK);
	}
	else
	{
		x1 = 90;
		x2 = 95;
		oledC_DrawRectangle(x1, y1, x2, y2, OLEDC_COLOR_BLACK);
	}

	if (prevTime.minutes != currentTime.minutes)
	{
		if (currentTime.minutes % 10 == 0)
		{
			x1 = 66;
			x2 = 78;
			oledC_DrawRectangle(x1, y1, x2, y2, OLEDC_COLOR_BLACK);
		}
		else
		{
			x1 = 72;
			x2 = 78;
			oledC_DrawRectangle(x1, y1, x2, y2, OLEDC_COLOR_BLACK);
		}
	}

	if (prevTime.hours != currentTime.hours)
	{
		if (currentTime.hours % 10 == 0)
		{
			x1 = 48;
			x2 = 60;
			oledC_DrawRectangle(x1, y1, x2, y2, OLEDC_COLOR_BLACK);
		}
		else
		{
			x1 = 54;
			x2 = 60;
			oledC_DrawRectangle(x1, y1, x2, y2, OLEDC_COLOR_BLACK);
		}
	}

	prevTime = currentTime;

	if (controller.prevMenuOption != controller.menuOption)
	{
		oledC_DrawRectangle(3, 1 + (controller.prevMenuOption *11), 9, 9 + (controller.prevMenuOption *11), OLEDC_COLOR_BLACK);
		controller.prevMenuOption = controller.menuOption;
	}
}

void setOption(void)
{
   if (controller.currentMode == M)
            if (controller.prevMenuOption == controller.menuOption)
				controller.menuOption = (controller.menuOption + 1) % controller.numberMOptions;
}
void setTime(DateTime *time)
{
	time->seconds++;

	if (time->seconds == 60)
	{
		time->seconds = 0;
		time->minutes += 1;
        
        
		if (time->minutes == 60)
		{
			time->minutes = 0;
			time->hours += 1;
            
            
			if (time->hours == 24)
			{
                time->day += 1;
				time->hours = 0;
				
				controller.DATE_Visible = true;
                controller.AMPM_Visible = true;

				if (time->day > daysPerMonth[time->month - 1])
				{
					
					time->month += 1;
                    time->day = 1;
                    
					if (time->month > 12)
						time->month = 1;
				}
			}
			else if (time->hours == 12)
			{
				controller.AMPM_Visible = true;
			}
		}
	}
}
void setMenu(void)
{
	if (controller.currMenu == 0)
	{
		switch (controller.menuOption)
		{
			case 0:
				showSetModeMenu();
				controller.currMenu = 1;
				break;
			case 1:
				showSetTimeMenu();
				controller.currMenu = 2;
				break;
            case 2:
                showSetDateMenu();
				controller.currMenu = 3;
				break;
			case 3:
				showHourMenu();
				controller.currMenu = 4;
				break;
		}
	}
	else if (controller.currMenu == 1)
	{
		switch (controller.menuOption)
		{
			case 0:
				showAnalog();
				controller.currentMode = A;
				controller.lastMode = A;
				showAnalog();
				break;
			case 1:
				controller.currentMode = D;
				controller.lastMode = D;
				showDigital();
				break;
		}

		controller.currMenu = 0;
	}
	else if (controller.currMenu == 2)
	{
		showSetTimeCases();
	}
    else if(controller.currMenu == 3) {    
        showSetDateCases();
    }
	else if (controller.currMenu == 4)
	{
		switch (controller.menuOption)
		{
			case 0:
				controller.ampmMode = 0;
				controller.AMPM_Visible = true;
				break;
			case 1:
				controller.ampmMode = 1;
				controller.AMPM_Visible = false;
				break;
		}

		controller.currMenu = 0;
		controller.currentMode = controller.lastMode;
        
		if (controller.lastMode == A)
            showAnalog();
		else showDigital();
        
	} else if (controller.currMenu == 5)
    {
        	
    }
}

void run(void)
{
    int cnt_runner = 0;
    SYSTEM_Initialize();
    
    //initialize TRISA
	TRISAbits.TRISA8 = 0;
    TRISAbits.TRISA9 = 0;

    // initialize BACKGROUND
    oledC_setBackground(OLEDC_COLOR_BLACK);
	oledC_clearScreen();
        
    // initialize DATE TIME
	currentTime.day = 8;
    currentTime.month = 10;
    currentTime.seconds = 16;
	currentTime.minutes = 5;
    currentTime.hours = 18;
    prevTime = currentTime;
    
    // initialize CONTROLLER
    controller.currentMode = A;
	controller.lastMode = A;
	controller.ampmMode = off;
	controller.alarm = false;
	controller.AMPM_Visible = false;
	controller.DATE_Visible = true;
	controller.menuOption = 0;
	controller.prevMenuOption = 0;
	controller.numberMOptions = 5;
	controller.s2Pressed = false;
	controller.currMenu = 0;

	showAnalog();
    
	// Timer initialize
	T1CONbits.TON = 1; /*Start Timer */
	T1CONbits.TCKPS = 0b11; /*Select 1:256 Pre scaler */
	PR1 = 16595; /*Count limit */
	IFS0bits.T1IF = 0; /*Flag interrupt */
	IEC0bits.T1IE = 1; /*Enable interrupt */

    // LOOP
	for (;;)
	{
        cnt_runner++;
        
		if (PORT_S1 == 0)
		{
			LED_1 = 1; // ON

			setOption();

			DELAY_milliseconds(100);
            
			LED_1 = 0; // OFF
		}

		if (PORT_S2 == 0)
		{
            controller.s2Pressed = true;

			LED_2 = 1; // ON
            
			DELAY_milliseconds(100);
            
			LED_2 = 0; // OFF
		}

	}
}

void __attribute__((__interrupt__)) _T1Interrupt(void)
{
	if (PORT_S1 == 0)
	{
		controller.s1Cnt += 1;
	}
	else controller.s1Cnt = 0;

	if (controller.s1Cnt >= 2)
	{
		controller.s1Cnt = 0;

		if (controller.currentMode != M)
		{
			controller.currentMode = M;
			showMenu();
		}
		else
		{
			controller.currentMode = controller.lastMode;
            
           	controller.menuOption = 0;

			controller.currMenu = 0;
            
          	controller.numberMOptions = 5;
            
			if (controller.lastMode == A) 
            {
                showAnalog();
            }
			else {
                showDigital();
            }
		}
	}

	switch (controller.currentMode)
	{
		case M:
			if (controller.s2Pressed)
			{
				setMenu();
				controller.s2Pressed = false;
			}
			if (controller.currentMode == M)
			{
				cleanMenu();
				showMenuStatus();
			}
			break;
		case A:
			cleanAnalog();
			showAnalogHands();
			break;
		case D:
			cleanDigital();
			showDigitalLCD();
			break;

	}
	setTime(&currentTime);
	IFS0bits.T1IF = 0;	//flag interrupt 
}

int main(void)
{
    run();
	return 1;
}