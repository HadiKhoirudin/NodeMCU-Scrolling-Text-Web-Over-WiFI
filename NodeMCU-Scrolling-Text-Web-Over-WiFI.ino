// Hadi Khoirudin, S.Kom
// NodeMCU V3 LUI ESP8266E WI-FI version 1.0

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

// Declaration HW Config
#define HARDWARE_TYPE MD_MAX72XX::ICSTATION_HW
#define MAX_DEVICES 4

#define CLK_PIN   D8 // or SCK
#define CS_PIN    D7 // or SS
#define DATA_PIN  D6 // or MOSI


// Global message buffers shared by Wifi and Scrolling functions
#define CHAR_SPACING  1 // pixels between characters

// Global message buffers shared by Serial and Scrolling functions
#define BUF_SIZE  75
char dir = 'l';//change direction from left to right
char curMessage[BUF_SIZE];
char newMessage[BUF_SIZE];
bool newMessageAvailable = false;

uint16_t  scrollDelay = 50;  // in milliseconds
MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);
ESP8266WebServer server(80); //Initialize the server on Port 80

// Website Response
char WebResponse[] = "HTTP/1.1 200 OK\nContent-Type: text/html\n\n";
// Website GUI
char WebPage[] =
"<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width'><title>Pengaturan</title> <style>body{margin: auto;}.button {background-color: #285fbb; border: none; color: white; padding: 10px 10px;text-align: center; text-decoration: none; display: inline-block; font-size: 30px; margin: auto; cursor: pointer; } .content{ max-width: 500px; margin: auto; background: white; padding: 10px; } </style> </head> <body><center> <div class='content'> <marquee>   <h4>Node MCU V3 ESP8226E Dot Matrix Wireless    </h4></marquee><br><center><form action='/tulis' method='POST'><input type='button' class='button' name='b1' value='&nbsp;&nbsp;&nbsp;+&nbsp;&nbsp;&nbsp;' onclick='location.href=&#39;/percepat&#39;'><br><br><input type='button' class='button' name='b2' value='&nbsp;<< &nbsp;' onclick='location.href=&#39;/kiri&#39;'>&nbsp;&nbsp;&nbsp;&nbsp;<input type='button' class='button' name='b3' value='OFF' onclick='location.href=&#39;/off&#39;'>&nbsp;&nbsp;&nbsp;&nbsp;<input type='button' class='button' name='b4' value='&nbsp; >>&nbsp;' onclick='location.href=&#39;/kanan&#39;'><br><br> <input type='button' class='button' name='b5' value='&nbsp;&nbsp;&nbsp;-&nbsp;&nbsp;&nbsp;' onclick='location.href=&#39;/perlambat&#39;'><br><br><br><center><textarea name='i1' placeholder='Silahkan isi pesan.' rows='4'></textarea></center><center><button type='submit'>kirim</button></center><br></form></div></center></body></html>";

void scrollDataSink(uint8_t dev, MD_MAX72XX::transformType_t t, uint8_t col)
// Callback function for data that is being scrolled off the display
{
#if PRINT_CALLBACK
  Serial.print("\n cb ");
  Serial.print(dev);
  Serial.print(' ');
  Serial.print(t);
  Serial.print(' ');
  Serial.println(col);
#endif
}

uint8_t scrollDataSource(uint8_t dev, MD_MAX72XX::transformType_t t)
// Callback function for data that is required for scrolling into the display
{
  static char   *p = curMessage;
  static uint8_t  state = 0;
  static uint8_t  curLen, showLen;
  static uint8_t  cBuf[8];
  uint8_t colData;

  // finite state machine to control what we do on the callback
  switch(state)
  {
    case 0: // Load the next character from the font table
      showLen = mx.getChar(*p++, sizeof(cBuf)/sizeof(cBuf[0]), cBuf);
      curLen = 0;
      state++;

      // if we reached end of message, reset the message pointer
      if (*p == '\0')
      {
        p = curMessage;     // reset the pointer to start of message
        if (newMessageAvailable)  // there is a new message waiting
        {
          strcpy(curMessage, newMessage);  // copy it in
          newMessageAvailable = false;
        }
      }
      // !! deliberately fall through to next state to start displaying

    case 1: // display the next part of the character
      colData = cBuf[curLen++];
      if (curLen == showLen)
      {
        showLen = CHAR_SPACING;
        curLen = 0;
        state = 2;
      }
      break;

    case 2: // display inter-character spacing (blank column)
      colData = 0;
      if (curLen == showLen)
        state = 0;
      curLen++;
      break;                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              
    default:
      state = 0;
  }

  return(colData);
}

 void scrollText(char *d)
{
  static uint32_t prevTime = 0;
                                                                                                                                                                                                                                                                                                                                                                                                                                      
  // Is it time to scroll the text?
  if (millis()-prevTime >= scrollDelay)
  {
    if(*d == 'l'){
      mx.transform(MD_MAX72XX::TSL);  // scroll along - the callback will load all the data
    }
    else if(*d == 'r'){
      mx.transform(MD_MAX72XX::TSR);
    }
    prevTime = millis();      // starting point for next time
  }
}

void setup() {

  WiFi.mode(WIFI_AP); //Our ESP8266-12E is an AccessPoint 
  WiFi.softAP("NodeMCUV3", "123456789"); // Provide the (SSID, password) 
  //and password has to be longer than 8
  server.on("/", handleRoot);
  server.on("/percepat", handleAtas);
  server.on("/kiri", handleKiri);
  server.on("/kanan", handleKanan);
  server.on("/perlambat", handleBawah);
  server.on("/off", handleOff);
  server.on("/tulis", handleTulis);
  server.begin(); // Start the HTTP Server

  Serial.begin(115200);
  mx.begin();
  mx.setShiftDataInCallback(scrollDataSource);
  mx.setShiftDataOutCallback(scrollDataSink);
  strcpy(curMessage, "This is running text by Hadi Khoirudin, S.Kom.....          ");
  newMessage[0] = '\0';
}
void loop(){
  server.handleClient();
  scrollText(&dir);
}

void handleRoot(){
  server.send(200,"text/html",WebPage);
  strcpy(curMessage, "Selamat datang di pengaturan Dot Matrix...   ");
  
  }
void handleAtas(){
  scrollDelay--;
  server.send(200,"text/html",WebPage);
  
  }
void handleKiri(){
  char *direct;
  direct = &dir;
  *direct = 'l';
  server.send(200,"text/html",WebPage);
  strcpy(curMessage, "<<<<");
  
  }
void handleKanan(){
  char *direct;
  direct = &dir;
  *direct = 'r';
  server.send(200,"text/html",WebPage);
  strcpy(curMessage, "<<<<");
  
  }
  void handleBawah(){
  scrollDelay++;
  server.send(200,"text/html",WebPage);
  
  }
void handleOff(){
  char *direct;
  direct = &dir;
  *direct = 'l';
  server.send(200,"text/html",WebPage);
  strcpy(curMessage, " Please set-up messages...     ");
  WiFi.mode(WIFI_OFF);  
  }
void handleTulis(){
  char *direct;
  direct = &dir;
  *direct = 'l';

 String str1 = server.arg("i1");
 const char* tulisan = str1.c_str();
 
 Serial.print("Tulisannya :");
 Serial.println(str1);
 
 strcpy(curMessage, tulisan);

  server.send(200,"text/html",WebPage);
  
  }
