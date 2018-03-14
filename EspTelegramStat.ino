
/**
* A bot to gather statistics on telegram chats at fablab and display those on rgb leds.                                                 
*                                                      
* written by Enkel Bici 
*
* Based on examples from Adafruit & UniversalTelegramBot library
*/

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <Adafruit_NeoPixel.h>
#include <EEPROM.h>

// Initialize Wifi connection to the router
char ssid[] = "xxxxxx";     // your network SSID (name)
char password[] = "yyyyyy"; // your network key

// Initialize Telegram BOT
#define BOTtoken "XXXXXXX:XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"  // your Bot Token (Get from Botfather)
String botname = "botname";

WiFiClientSecure client;
//bool debug = false;
//UniversalTelegramBot bot(BOTtoken, client, debug);
UniversalTelegramBot bot(BOTtoken, client);

int Bot_mtbs = 500; //mean time between scan messages
long Bot_lasttime;   //last time messages' scan has been done

#define PIN D4
//total number of leds
#define num_leds 56
//how many leds for each star
#define leds_star 8
Adafruit_NeoPixel strip = Adafruit_NeoPixel(num_leds, PIN, NEO_GRB + NEO_KHZ800);


typedef struct {
  String chatname;
  String chatid;
  uint16_t msgn;
  int firstled;
  int address;
}chat_counter;

//number of chats
#define num_chats 6
chat_counter chats[num_chats];
//total messages
uint16_t total=0;
int durata = 0;

void setup() {
  Serial.begin(115200);
  
  randomSeed(analogRead(A0));

  // Initialize chats
  chats[0] = (chat_counter) {"Restarters Torino","00000000",0,0,0};
  chats[1] = (chat_counter) {"AUG Torino","-11111111",0,chats[0].firstled + leds_star,chats[0].address + sizeof(uint16_t)};
  chats[2] = (chat_counter) {"FABLAB TORINO","-22222222",0,chats[1].firstled + leds_star,chats[1].address + sizeof(int)};
  chats[3] = (chat_counter) {"AUG - Telegram playground","-3333333",0,chats[2].firstled + leds_star,chats[2].address + sizeof(uint16_t)};
  chats[4] = (chat_counter) {"Turin 3D Printing Users Group","-4444444",0,chats[3].firstled + leds_star,chats[3].address + sizeof(uint16_t)}; //direct chat has no -
  chats[5] = (chat_counter) {"AHL VirtualRoom","-5555555",0,chats[4].firstled + leds_star,chats[4].address + sizeof(uint16_t)}; 

  EEPROM.begin(512);
  //run only the first time to clear the rom
  /*
  for (int i = 0; i < 512; i++){
    EEPROM.write(i, 0);
    EEPROM.commit();
  }
  */
  
  for(int i = 0; i < num_chats; i++){
    //read eeprom
    uint8_t high = 0;
    uint8_t low = 0;
    high = EEPROM.read(chats[i].address);
    low = EEPROM.read(chats[i].address + 1);
    //Serial.println("high: "+(String)high+"-low:"+(String)low);
    uint16_t tmp = (uint16_t)high;
    tmp = tmp << 8;
    tmp += (uint16_t)low;
    Serial.println(tmp);
    chats[i].msgn += tmp;

    Serial.println("\n"+chats[i].chatname+" : "+chats[i].chatid+"\naddress:"+chats[i].address+" - firstled:"+chats[i].firstled+" - msgnumber:"+chats[i].msgn);
    total+=chats[i].msgn;
  }

  // initialize rgb strip
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  // Set WiFi to station mode and disconnect from an AP if it was Previously
  // connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  // attempt to connect to Wifi network:
  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  //set to yellow
  starLight(0, num_leds, strip.Color(255,255, 255), 50);

  for(int i = 0; i < num_chats; i++){
    int start = chats[i].firstled;
    int end = start + leds_star;
    //strip.Color(100,100,100)

    //uint8_t color = (i*22)+100;
    //Serial.println(color);
    starLight(start, end, strip.Color(128,120, 0), 10);
  }
}

void loop() {

  if (millis() > Bot_lasttime + Bot_mtbs) {
    int lastNum[num_chats];

    for(int i = 0; i < num_chats; i++){
    lastNum[i] = chats[i].msgn;
    }

    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while(numNewMessages) {
      //Serial.println("got response");
      handleNewMessages(numNewMessages);
      //Serial.println(" inside while after handleNewMessages");

      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    //Serial.println(" after a while");
    Bot_lasttime = millis();

    
    for(int i = 0; i < num_chats; i++){
    int start = chats[i].firstled;
    int end = start + leds_star;

    if(lastNum[i] != chats[i].msgn){
      //la durata della rotazione è proporzionale al numero di messaggi
      durata = map(chats[i].msgn, 0, total, 1, 6);
      for(int j = 0; j < durata; j++){
        starLight(start, end, strip.Color(0,255,128), 1000/durata);
        starLight(start, end, strip.Color(0,100,50), 1000/durata);
      }
      starLight(start, end, strip.Color(128,120, 0), 10);

      //write to eeprom
      uint8_t high = (uint8_t) (chats[i].msgn >> 8);
      uint8_t low = (uint8_t) chats[i].msgn;

      //Serial.println("high: "+(String)high+"-low:"+(String)low);

      EEPROM.write(chats[i].address, high);
      EEPROM.write(chats[i].address + 1, low);
      EEPROM.commit();
    }
    uint8_t color = (i*22)+100;
    //starLight(start, end, strip.Color(128,120, 0), 10);
    }  
  }
  rainbow(20);

  /*
  // Some example procedures showing how to display to the pixels:
  colorWipe(strip.Color(255, 0, 0), 50); // Red
  colorWipe(strip.Color(0, 255, 0), 50); // Green
  colorWipe(strip.Color(0, 0, 255), 50); // Blue
  // Send a theater pixel chase in...
  theaterChase(strip.Color(127, 127, 127), 50); // White
  theaterChase(strip.Color(127, 0, 0), 50); // Red
  theaterChase(strip.Color(0, 0, 127), 50); // Blue
  
  rainbow(20);
  rainbowCycle(20);
  theaterChaseRainbow(50);
  */
}

void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i=0; i<numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;

    String from_name = bot.messages[i].from_name;
    if (from_name == "") from_name = "Guest";

    Serial.print("Chat id ");
    Serial.println(chat_id);
    //Serial.print("Text - ");
    //Serial.println(text);

    for(int i = 0; i < num_chats; i++){
      if(chats[i].chatid.equals(chat_id)){
          chats[i].msgn++;
          total++;          
          Serial.println(chats[i].msgn);
          Serial.print("Total ");
          Serial.println(total);
          break;
        }
    }

    if (text == "/status" || text == "/status@"+botname) {
      bot.sendChatAction(chat_id, "typing");

      String stats = "Here are the stats:\n";
      for(int i = 0; i < num_chats; i++){
        stats += chats[i].chatname+" - "+(String) chats[i].msgn +" messages\n";
      }
      stats += "Total - "+(String) total +" messages";
      //light up the last star a random color time it receives a status request
      starLight(num_leds-leds_star, num_leds, strip.Color(255,0, 0), 10);
      bot.sendMessage(chat_id, stats);
      starLight(num_leds-leds_star, num_leds, strip.Color(128,120, 0), 10);
    }

    if (text == "/start") {
      String welcome = "Hello " + from_name + " - This is the FablabTO Telegram Bot.\n";
      welcome += "This Bot gathers statistics on the chat usage of various communties inside FablabTO.\n\n";
      welcome += "/status : Read the gathered stats\n";
      bot.sendMessage(chat_id, welcome);
    }
  }
}

// Fill the dots one after the other with a color
void starLight(int first, int last, uint32_t c, uint8_t wait) {
  for(uint16_t i=first; i<last; i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, c);    //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

