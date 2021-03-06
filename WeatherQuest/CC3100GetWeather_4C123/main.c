// main.c
// Program retrieves weather information of user input
// Displays information on computer terminal and LCD
// LCD will display weather animation based on current weather data
// Victor Minguela

/* 
 * Contains code from UT.6.02x Embedded Systems - Shape the World
 * Jonathan Valvano and Ramesh Yerraballi
 * October 27, 2015
 * Hardware requirements 
     TM4C123 LaunchPad, optional Nokia5110
     CC3100 wifi booster and 
     an internet access point with OPEN, WPA, or WEP security
 
 * derived from TI's getweather example
 * Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/
 *
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
*/

/*
 * Application Name     -   Get weather
 * Application Overview -   This is a sample application demonstrating how to
                            connect to openweathermap.org server and request for
              weather details of a city.
 * Application Details  -   http://processors.wiki.ti.com/index.php/CC31xx_SLS_Get_Weather_Application
 *                          doc\examples\sls_get_weather.pdf
 */
 /* CC3100 booster pack connections (unused pins can be used by user application)
Pin  Signal        Direction      Pin   Signal     Direction
P1.1  3.3 VCC         IN          P2.1  Gnd   GND      IN
P1.2  PB5 UNUSED      NA          P2.2  PB2   IRQ      OUT
P1.3  PB0 UART1_TX    OUT         P2.3  PE0   SSI2_CS  IN
P1.4  PB1 UART1_RX    IN          P2.4  PF0   UNUSED   NA
P1.5  PE4 nHIB        IN          P2.5  Reset nRESET   IN
P1.6  PE5 UNUSED      NA          P2.6  PB7  SSI2_MOSI IN
P1.7  PB4 SSI2_CLK    IN          P2.7  PB6  SSI2_MISO OUT
P1.8  PA5 UNUSED      NA          P2.8  PA4   UNUSED   NA
P1.9  PA6 UNUSED      NA          P2.9  PA3   UNUSED   NA
P1.10 PA7 UNUSED      NA          P2.10 PA2   UNUSED   NA

Pin  Signal        Direction      Pin   Signal      Direction
P3.1  +5  +5 V       IN           P4.1  PF2 UNUSED      OUT
P3.2  Gnd GND        IN           P4.2  PF3 UNUSED      OUT
P3.3  PD0 UNUSED     NA           P4.3  PB3 UNUSED      NA
P3.4  PD1 UNUSED     NA           P4.4  PC4 UART1_CTS   IN
P3.5  PD2 UNUSED     NA           P4.5  PC5 UART1_RTS   OUT
P3.6  PD3 UNUSED     NA           P4.6  PC6 UNUSED      NA
P3.7  PE1 UNUSED     NA           P4.7  PC7 NWP_LOG_TX  OUT
P3.8  PE2 UNUSED     NA           P4.8  PD6 WLAN_LOG_TX OUT
P3.9  PE3 UNUSED     NA           P4.9  PD7 UNUSED      IN (see R74)
P3.10 PF1 UNUSED     NA           P4.10 PF4 UNUSED      OUT(see R75)

UART0 (PA1, PA0) sends data to the PC via the USB debug cable, 115200 baud rate
Port A, SSI0 (PA2, PA3, PA5, PA6, PA7) sends data to Nokia5110 LCD

*/
#include "ST7735.h"
#include "animations.h"
#include "..\cc3100\simplelink\include\simplelink.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "utils/cmdline.h"
#include "application_commands.h"
#include "LED.h"
//#include "Nokia5110.h"
#include <string.h>


// To Do: replace the following three lines with your access point information
#define SSID_NAME  "SpectrumSetup-00" /* Access point name to connect to */
#define SEC_TYPE   SL_SEC_TYPE_WPA
#define PASSKEY    "wittyrobin037"  /* Password in case of secure AP */ 

#define BAUD_RATE   115200
void UART_Init(void){
  SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
  GPIOPinConfigure(GPIO_PA0_U0RX);
  GPIOPinConfigure(GPIO_PA1_U0TX);
  GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
  UARTStdioConfig(0,BAUD_RATE,50000000);
}
#define MAXLEN 100
char City[MAXLEN];
char Temp_min[MAXLEN];
char Temp_max[MAXLEN];
char Weather[MAXLEN];
char Sky[MAXLEN];
char Humidity[MAXLEN];

#define MAX_RECV_BUFF_SIZE  1024
#define MAX_SEND_BUFF_SIZE  512
#define MAX_HOSTNAME_SIZE   40
#define MAX_PASSKEY_SIZE    32
#define MAX_SSID_SIZE       32


#define SUCCESS             0

#define CONNECTION_STATUS_BIT   0
#define IP_AQUIRED_STATUS_BIT   1

/* Application specific status/error codes */
typedef enum{
    DEVICE_NOT_IN_STATION_MODE = -0x7D0,/* Choosing this number to avoid overlap w/ host-driver's error codes */

    STATUS_CODE_MAX = -0xBB8
}e_AppStatusCodes;


/* Status bits - These are used to set/reset the corresponding bits in 'g_Status' */
typedef enum{
    STATUS_BIT_CONNECTION =  0, /* If this bit is:
                                 *      1 in 'g_Status', the device is connected to the AP
                                 *      0 in 'g_Status', the device is not connected to the AP
                                 */

    STATUS_BIT_IP_AQUIRED,       /* If this bit is:
                                 *      1 in 'g_Status', the device has acquired an IP
                                 *      0 in 'g_Status', the device has not acquired an IP
                                 */

}e_StatusBits;


#define SET_STATUS_BIT(status_variable, bit)    status_variable |= (1<<(bit))
#define CLR_STATUS_BIT(status_variable, bit)    status_variable &= ~(1<<(bit))
#define GET_STATUS_BIT(status_variable, bit)    (0 != (status_variable & (1<<(bit))))
#define IS_CONNECTED(status_variable)           GET_STATUS_BIT(status_variable, \
                                                               STATUS_BIT_CONNECTION)
#define IS_IP_AQUIRED(status_variable)          GET_STATUS_BIT(status_variable, \
                                                               STATUS_BIT_IP_AQUIRED)

typedef struct{
    UINT8 SSID[MAX_SSID_SIZE];
    INT32 encryption;
    UINT8 password[MAX_PASSKEY_SIZE];
}UserInfo;

/*
 * GLOBAL VARIABLES -- Start
 */

char REQUEST[MAX_SEND_BUFF_SIZE];
char Recvbuff[MAX_RECV_BUFF_SIZE];
char SendBuff[MAX_SEND_BUFF_SIZE];
char HostName[MAX_HOSTNAME_SIZE];
unsigned long DestinationIP;
int SockID;


typedef enum{
    CONNECTED = 0x01,
    IP_AQUIRED = 0x02,
    IP_LEASED = 0x04,
    PING_DONE = 0x08

}e_Status;
UINT32  g_Status = 0;
/*
 * GLOBAL VARIABLES -- End
 */

void Serial_LCD_Print(void);
void Draw_Weather(void);
void Print_Menu(void);
void User_Selection(unsigned char option);
void Retrieve_Info(void);
 /*
 * STATIC FUNCTION DEFINITIONS  -- Start
 */

static int32_t configureSimpleLinkToDefaultState(char *);


/*
 * STATIC FUNCTION DEFINITIONS -- End
 */


void Crash(uint32_t time){
  while(1){
    for(int i=time;i;i--){};
    LED_RedToggle();
  }
}


int main(void){
	
	int32_t retVal;  SlSecParams_t secParams;
  char *pConfig = NULL; INT32 ASize = 0; SlSockAddrIn_t  Addr;
	unsigned char option;
  initClk();        // PLL 50 MHz
	ST7735_InitR(INITR_REDTAB);
  UART_Init();      // Send data to PC, 115200 bps
	SysTick_Init();
  //LED_Init();       // initialize LaunchPad I/O 
	Print_Menu();
	option = UARTgetc();
	UARTprintf("\n");
	User_Selection(option);
	
  retVal = configureSimpleLinkToDefaultState(pConfig); // set policies
  if(retVal < 0)Crash(4000000);
  retVal = sl_Start(0, pConfig, 0);
  if((retVal < 0) || (ROLE_STA != retVal) ) Crash(8000000);
  secParams.Key = PASSKEY;
  secParams.KeyLen = strlen(PASSKEY);
  secParams.Type = SEC_TYPE; // OPEN, WPA, or WEP
  sl_WlanConnect(SSID_NAME, strlen(SSID_NAME), 0, &secParams, 0);
  while((0 == (g_Status&CONNECTED)) || (0 == (g_Status&IP_AQUIRED))){
    _SlNonOsMainLoopTask();
  }
  UARTprintf("Connected\n");
    strcpy(HostName,"api.openweathermap.org");
    retVal = sl_NetAppDnsGetHostByName(HostName,
             strlen(HostName),&DestinationIP, SL_AF_INET);
    if(retVal == 0){
      Addr.sin_family = SL_AF_INET;
      Addr.sin_port = sl_Htons(80);
      Addr.sin_addr.s_addr = sl_Htonl(DestinationIP);// IP to big endian 
      ASize = sizeof(SlSockAddrIn_t);
      SockID = sl_Socket(SL_AF_INET,SL_SOCK_STREAM, 0);
      if( SockID >= 0 ){
        retVal = sl_Connect(SockID, ( SlSockAddr_t *)&Addr, ASize);
      }
      if((SockID >= 0)&&(retVal >= 0)){
        strcpy(SendBuff,REQUEST); 
        sl_Send(SockID, SendBuff, strlen(SendBuff), 0);// Send the HTTP GET 
        sl_Recv(SockID, Recvbuff, MAX_RECV_BUFF_SIZE, 0);// Receive response 
				
				Recvbuff[strlen(Recvbuff)] = '\0';		
				Retrieve_Info();
				
        sl_Close(SockID);
       // LED_GreenOn();
				if(strcmp(Sky, "Rain")){
					ST7735_FillScreen(ST7735_CYAN);
				} else{
					ST7735_FillScreen(ST7735_MAGENTA);
				}
        Serial_LCD_Print();
				Draw_Weather();
      }
    }

	
}

//Prints information to UART serial terminal and LCD
void Serial_LCD_Print(void){
	UARTprintf("\r\n\r\n");
	strcat(Temp_min, " Fahrenheit");
	strcat(Temp_max, " Fahrenheit");
  UARTprintf(City); UARTprintf("\r\n");
  UARTprintf("Temperature minimum: "); UARTprintf(Temp_min); UARTprintf("\r\n");
	UARTprintf("Temperature maximum: ");UARTprintf(Temp_max); UARTprintf("\r\n");
	UARTprintf("Humidity: ");UARTprintf(Humidity); UARTprintf("\r\n");
	UARTprintf("Weather: ");UARTprintf(Weather); UARTprintf("\r\n");
	UARTprintf(Sky); UARTprintf("\r\n");
	
	//DrawString functions are used to display info on LCD
	ST7735_DrawString(1,9, "Temperature minimum: ", ST7735_BLACK);
	ST7735_DrawString(1,10, Temp_min, ST7735_BLACK);
	ST7735_DrawString(1,11, "Temperature maximum: ", ST7735_BLACK);
	ST7735_DrawString(1,12, Temp_max, ST7735_BLACK);
	ST7735_DrawString(1,13, "Humidity: ", ST7735_BLACK);
	ST7735_DrawString(10,13, Humidity, ST7735_BLACK);
	ST7735_DrawString(1,14, "Weather:", ST7735_BLACK);
	ST7735_DrawString(1,15, Weather, ST7735_BLACK);
	
}
//Displays animation to LCD
void Draw_Weather(void){
	int32_t p;
	p=0;
	//Create moving clear sky animation
	if(strcmp(Sky, "Clear") == 0){
		ST7735_FillCircle(80, 40, 20, ST7735_YELLOW);
		ST7735_PlotSun(0, ST7735_YELLOW);
		while(1){
			ST7735_PlotSun( p, ST7735_YELLOW);
			SysTick_Wait10ms(8);
			ST7735_PlotSun( p, ST7735_CYAN);
			SysTick_Wait10ms(1);
			p = p + 2;
			if(p > 8){
				p = 0;	
			}
			
		}
	}
	//Moving rain animation
	else if(strcmp(Sky, "Rain") == 0){
		ST7735_FillCircle(20, 25, 12, ST7735_BLACK);//left
		ST7735_FillCircle(35, 25, 12, ST7735_BLACK);//middle
		ST7735_FillCircle(45, 16, 12, ST7735_BLACK);//right
		ST7735_FillCircle(30, 13, 12, ST7735_BLACK);//up
		ST7735_FillCircle(42, 30, 12, ST7735_BLACK);//dwn
		ST7735_PlotRain(0, ST7735_BLUE);
		while(1){
			ST7735_PlotRain( p, ST7735_BLUE);
			SysTick_Wait10ms(10);
			ST7735_PlotRain( p, ST7735_MAGENTA);
			SysTick_Wait10ms(1); 
			p = p + 2;
			if(p > 26){
				p = 0;	
			}
		}
  }
	
	//Moving cloud animation
	else {
		ST7735_PlotCloudy(0, ST7735_WHITE);
		while(1){
			ST7735_PlotCloudy( p, ST7735_WHITE);
			SysTick_Wait10ms(10);
			ST7735_PlotCloudy( p, ST7735_CYAN);
			SysTick_Wait(100000); 
			p = p + 2;
			if(p > 32){
				p = 0;	
			}
		}
	}
	
		
}
//Displays query to user in the terminal
void Print_Menu(void){
	UARTprintf("Welcome to my Embedded Weather Quest\n");
	UARTprintf("Please choose your query criteria:\n");
	UARTprintf("1. City Name\n");
	UARTprintf("2. City ID\n");
	UARTprintf("3. Geographic Coordinates\n");
	UARTprintf("4. Zip Code\n");
}
//Stores data into REQUEST buffer
//REQUEST data is used to retrieve weather information from weather server
//Data requested is dependent on user input
//REQUEST must be in one of the following forms:
//https://api.openweathermap.org/data/2.5/weather?lat={lat}&lon={lon}&appid={API key}
//https://api.openweathermap.org/data/2.5/weather?q={city name}&appid={API key}
//https://api.openweathermap.org/data/2.5/weather?id={city id}&appid={API key}
//https://api.openweathermap.org/data/2.5/weather?zip={zip code},{country code}&appid={API key}
void User_Selection(unsigned char option){
	if(option == '1'){
		char city [255];
		UARTprintf("Enter City: ");
		UARTgets(city,254);
		sprintf(REQUEST, "GET /data/2.5/weather?q=%s&APPID=d402827f2b2f6580e89c2776ad480b0e&units=imperial HTTP/1.1\r\nUser-Agent: Keil\r\nHost:api.openweathermap.org\r\nAccept: */*\r\n\r\n", city);
		
	}else if(option == '2'){
		char cityID [255];
		UARTprintf("Enter City ID: ");
		UARTgets(cityID,254);
		sprintf(REQUEST, "GET /data/2.5/weather?id=%s&APPID=d402827f2b2f6580e89c2776ad480b0e&units=imperial HTTP/1.1\r\nUser-Agent: Keil\r\nHost:api.openweathermap.org\r\nAccept: */*\r\n\r\n", cityID);
		
	}else if(option == '3'){
		char lon [255];
		char lat [255];
		UARTprintf("Enter longitude: ");
		UARTgets(lon,254);
		UARTprintf("Enter latitude: ");
		UARTgets(lat,254);
		sprintf(REQUEST, "GET /data/2.5/weather?lat=%s&lon=%s&APPID=d402827f2b2f6580e89c2776ad480b0e&units=imperial HTTP/1.1\r\nUser-Agent: Keil\r\nHost:api.openweathermap.org\r\nAccept: */*\r\n\r\n", lat, lon);
		
	}else if(option == '4'){
		char zip [255];
		char country [255];
		UARTprintf("Enter zip code: ");
		UARTgets(zip,254);
		UARTprintf("Enter country: ");
		UARTgets(country,254);
		sprintf(REQUEST, "GET /data/2.5/weather?zip=%s,%s&APPID=d402827f2b2f6580e89c2776ad480b0e&units=imperial HTTP/1.1\r\nUser-Agent: Keil\r\nHost:api.openweathermap.org\r\nAccept: */*\r\n\r\n", zip, country);
	} else{
		UARTprintf("invalid\n");
	}
}
//Retrieves certain weather information from server by indexing the recieved string data
//Each weather data is stored to individual buffers
void Retrieve_Info(void){
	char *pt = NULL;
	uint32_t i; 
	
	 pt = strstr(Recvbuff, "\"main\"");
				i = 0;
				if( NULL != pt ){                                            
					pt = pt + 8; // skip over "main":"                         
					while((i<MAXLEN)&&(*pt)&&(*pt!='\"')){
						Sky[i] = *pt; // copy into City string
  					pt++; i++;
					}
				}
				Sky[i] = 0; 
				
				
	 pt = strstr(Recvbuff, "\"name\"");
				i = 0;
				if( NULL != pt ){                                            
					pt = pt + 8; // skip over "name":"                         
					while((i<MAXLEN)&&(*pt)&&(*pt!='\"')){
						City[i] = *pt; // copy into City string
  					pt++; i++;
					}
				}
				City[i] = 0; 
				pt = strstr(Recvbuff, "\"temp_min\"");
				i = 0;
				if( NULL != pt ){                                            
					pt = pt + 11; // skip over "temp_min":                         
					while((i<MAXLEN)&&(*pt)&&(*pt!=',')){
						Temp_min[i] = *pt; // copy into City string
  					pt++; i++;
					}
				}
				Temp_min[i] = 0; 
				pt = strstr(Recvbuff, "\"temp_max\"");
				i = 0;
				if( NULL != pt ){                                            
					pt = pt + 11; // skip over "temp_max":                         
					while((i<MAXLEN)&&(*pt)&&(*pt!=',')){
						Temp_max[i] = *pt; // copy into City string
  					pt++; i++;
					}
				}
				Temp_max[i] = 0; 
				pt = strstr(Recvbuff, "\"description\"");
				i = 0;
				if( NULL != pt ){
					pt = pt + 15; // skip over "description":"   
					while((i<MAXLEN)&&(*pt)&&(*pt!='\"')){
						Weather[i] = *pt; // copy into string 
						pt++; i++;
					}
				}
				Weather[i] = 0;
				
				pt = strstr(Recvbuff, "\"humidity\"");
				i = 0;
				if( NULL != pt ){
					pt = pt + 11; // skip over "\humidity\""   
					while((i<MAXLEN)&&(*pt)&&(*pt!='}')&&(*pt!=',')){
						Humidity[i] = *pt; // copy into string 
						pt++; i++;
					}
				}
				Humidity[i] = 0;
}
/*!
    \brief This function puts the device in its default state. It:
           - Set the mode to STATION
           - Configures connection policy to Auto and AutoSmartConfig
           - Deletes all the stored profiles
           - Enables DHCP
           - Disables Scan policy
           - Sets Tx power to maximum
           - Sets power policy to normal
           - Unregister mDNS services

    \param[in]      none

    \return         On success, zero is returned. On error, negative is returned
*/
static int32_t configureSimpleLinkToDefaultState(char *pConfig){
  SlVersionFull   ver = {0};
  UINT8           val = 1;
  UINT8           configOpt = 0;
  UINT8           configLen = 0;
  UINT8           power = 0;

  INT32           retVal = -1;
  INT32           mode = -1;

  mode = sl_Start(0, pConfig, 0);


    /* If the device is not in station-mode, try putting it in station-mode */
  if (ROLE_STA != mode){
    if (ROLE_AP == mode){
            /* If the device is in AP mode, we need to wait for this event before doing anything */
      while(!IS_IP_AQUIRED(g_Status));
    }

        /* Switch to STA role and restart */
    retVal = sl_WlanSetMode(ROLE_STA);

    retVal = sl_Stop(0xFF);

    retVal = sl_Start(0, pConfig, 0);

        /* Check if the device is in station again */
    if (ROLE_STA != retVal){
            /* We don't want to proceed if the device is not coming up in station-mode */
      return DEVICE_NOT_IN_STATION_MODE;
    }
  }
    /* Get the device's version-information */
  configOpt = SL_DEVICE_GENERAL_VERSION;
  configLen = sizeof(ver);
  retVal = sl_DevGet(SL_DEVICE_GENERAL_CONFIGURATION, &configOpt, &configLen, (unsigned char *)(&ver));

    /* Set connection policy to Auto + SmartConfig (Device's default connection policy) */
  retVal = sl_WlanPolicySet(SL_POLICY_CONNECTION, SL_CONNECTION_POLICY(1, 0, 0, 0, 1), NULL, 0);

    /* Remove all profiles */
  retVal = sl_WlanProfileDel(0xFF);

    /*
     * Device in station-mode. Disconnect previous connection if any
     * The function returns 0 if 'Disconnected done', negative number if already disconnected
     * Wait for 'disconnection' event if 0 is returned, Ignore other return-codes
     */
  retVal = sl_WlanDisconnect();
  if(0 == retVal){
        /* Wait */
     while(IS_CONNECTED(g_Status));
  }

    /* Enable DHCP client*/
  retVal = sl_NetCfgSet(SL_IPV4_STA_P2P_CL_DHCP_ENABLE,1,1,&val);

    /* Disable scan */
  configOpt = SL_SCAN_POLICY(0);
  retVal = sl_WlanPolicySet(SL_POLICY_SCAN , configOpt, NULL, 0);

    /* Set Tx power level for station mode
       Number between 0-15, as dB offset from max power - 0 will set maximum power */
  power = 0;
  retVal = sl_WlanSet(SL_WLAN_CFG_GENERAL_PARAM_ID, WLAN_GENERAL_PARAM_OPT_STA_TX_POWER, 1, (unsigned char *)&power);

    /* Set PM policy to normal */
  retVal = sl_WlanPolicySet(SL_POLICY_PM , SL_NORMAL_POLICY, NULL, 0);

    /* TBD - Unregister mDNS services */
  retVal = sl_NetAppMDNSUnRegisterService(0, 0);


  retVal = sl_Stop(0xFF);


  g_Status = 0;
  memset(&Recvbuff,0,MAX_RECV_BUFF_SIZE);
  memset(&SendBuff,0,MAX_SEND_BUFF_SIZE);
  memset(&HostName,0,MAX_HOSTNAME_SIZE);
  DestinationIP = 0;;
  SockID = 0;


  return retVal; /* Success */
}




/*
 * * ASYNCHRONOUS EVENT HANDLERS -- Start
 */

/*!
    \brief This function handles WLAN events

    \param[in]      pWlanEvent is the event passed to the handler

    \return         None

    \note

    \warning
*/
void SimpleLinkWlanEventHandler(SlWlanEvent_t *pWlanEvent){
  switch(pWlanEvent->Event){
    case SL_WLAN_CONNECT_EVENT:
    {
      SET_STATUS_BIT(g_Status, STATUS_BIT_CONNECTION);

            /*
             * Information about the connected AP (like name, MAC etc) will be
             * available in 'sl_protocol_wlanConnectAsyncResponse_t' - Applications
             * can use it if required
             *
             * sl_protocol_wlanConnectAsyncResponse_t *pEventData = NULL;
             * pEventData = &pWlanEvent->EventData.STAandP2PModeWlanConnected;
             *
             */
    }
    break;

    case SL_WLAN_DISCONNECT_EVENT:
    {
      sl_protocol_wlanConnectAsyncResponse_t*  pEventData = NULL;

      CLR_STATUS_BIT(g_Status, STATUS_BIT_CONNECTION);
      CLR_STATUS_BIT(g_Status, STATUS_BIT_IP_AQUIRED);

      pEventData = &pWlanEvent->EventData.STAandP2PModeDisconnected;

            /* If the user has initiated 'Disconnect' request, 'reason_code' is SL_USER_INITIATED_DISCONNECTION */
      if(SL_USER_INITIATED_DISCONNECTION == pEventData->reason_code){
        UARTprintf(" Device disconnected from the AP on application's request \r\n");
      }
      else{
        UARTprintf(" Device disconnected from the AP on an ERROR..!! \r\n");
      }
    }
    break;

    default:
    {
      UARTprintf(" [WLAN EVENT] Unexpected event \r\n");
    }
    break;
  }
}

/*!
    \brief This function handles events for IP address acquisition via DHCP
           indication

    \param[in]      pNetAppEvent is the event passed to the handler

    \return         None

    \note

    \warning
*/
void SimpleLinkNetAppEventHandler(SlNetAppEvent_t *pNetAppEvent){
  switch(pNetAppEvent->Event)
  {
    case SL_NETAPP_IPV4_ACQUIRED:
    {

      SET_STATUS_BIT(g_Status, STATUS_BIT_IP_AQUIRED);
        /*
             * Information about the connected AP's ip, gateway, DNS etc
             * will be available in 'SlIpV4AcquiredAsync_t' - Applications
             * can use it if required
             *
             * SlIpV4AcquiredAsync_t *pEventData = NULL;
             * pEventData = &pNetAppEvent->EventData.ipAcquiredV4;
             * <gateway_ip> = pEventData->gateway;
             *
             */

    }
    break;

    default:
    {
            UARTprintf(" [NETAPP EVENT] Unexpected event \r\n");
    }
    break;
  }
}

/*!
    \brief This function handles callback for the HTTP server events

    \param[in]      pServerEvent - Contains the relevant event information
    \param[in]      pServerResponse - Should be filled by the user with the
                    relevant response information

    \return         None

    \note

    \warning
*/
void SimpleLinkHttpServerCallback(SlHttpServerEvent_t *pHttpEvent,
                                  SlHttpServerResponse_t *pHttpResponse){
    /*
     * This application doesn't work with HTTP server - Hence these
     * events are not handled here
     */
  UARTprintf(" [HTTP EVENT] Unexpected event \r\n");
}

/*!
    \brief This function handles general error events indication

    \param[in]      pDevEvent is the event passed to the handler

    \return         None
*/
void SimpleLinkGeneralEventHandler(SlDeviceEvent_t *pDevEvent){
    /*
     * Most of the general errors are not FATAL are are to be handled
     * appropriately by the application
     */
  UARTprintf(" [GENERAL EVENT] \r\n");
}

/*!
    \brief This function handles socket events indication

    \param[in]      pSock is the event passed to the handler

    \return         None
*/
void SimpleLinkSockEventHandler(SlSockEvent_t *pSock){
  switch( pSock->Event )
  {
    case SL_NETAPP_SOCKET_TX_FAILED:
    {
            /*
            * TX Failed
            *
            * Information about the socket descriptor and status will be
            * available in 'SlSockEventData_t' - Applications can use it if
            * required
            *
            * SlSockEventData_t *pEventData = NULL;
            * pEventData = & pSock->EventData;
            */
      switch( pSock->EventData.status )
      {
        case SL_ECLOSE:
          UARTprintf(" [SOCK EVENT] Close socket operation failed to transmit all queued packets\r\n");
          break;


        default:
          UARTprintf(" [SOCK EVENT] Unexpected event \r\n");
          break;
      }
    }
    break;

    default:
      UARTprintf(" [SOCK EVENT] Unexpected event \r\n");
    break;
  }
}
/*
 * * ASYNCHRONOUS EVENT HANDLERS -- End
 */


