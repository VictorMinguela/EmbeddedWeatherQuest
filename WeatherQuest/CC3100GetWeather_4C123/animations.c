//Functions display animations on LCD
//Used to create moving animations
//Victor Minguela
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "animations.h"
#include "ST7735.h"


void ST7735_PlotSun(int32_t shift,uint16_t color )
{
	ST7735_DrawLine(100+shift, 40, 108+shift, 40,color ); //right
	ST7735_DrawLine(60-shift, 40, 52-shift, 40,color ); //left
	ST7735_DrawLine(80, 20-shift, 80, 12-shift,color ); //up
	ST7735_DrawLine(80, 60+shift, 80, 68+shift,color ); //down
	ST7735_DrawLine(95+shift, 55+shift, 100+shift, 60+shift,color ); //bottom-right diagonal
	ST7735_DrawLine(95+shift, 25-shift, 100+shift, 20-shift,color ); //top-right diagonal
	ST7735_DrawLine(65-shift, 25-shift, 60-shift, 20-shift,color ); //top-left diagonal
	ST7735_DrawLine(65-shift, 55+shift, 60-shift, 60+shift,color); //bottom-left diagonal
}

void ST7735_PlotCloudy(int32_t shift,uint16_t color )
{
	ST7735_FillCircle(20+shift, 40, 20, color);
	ST7735_FillCircle(35+shift, 40, 20, color);	
	ST7735_FillCircle(65+shift, 35, 20, color);
	ST7735_FillCircle(45+shift, 25, 20, color);
	ST7735_FillCircle(42+shift, 55, 20, color);
}

void ST7735_PlotRain(int32_t shift,uint16_t color )
{	
	ST7735_FillCircle(40, 46+shift, 2, color);
	ST7735_FillCircle(20, 53+shift, 2, color);
	ST7735_FillCircle(55, 60+shift, 2, color);
}


