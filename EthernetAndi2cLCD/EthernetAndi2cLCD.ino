#include <Wire.h>
// https://bitbucket.org/fmalpartida/new-liquidcrystal/downloads
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <Ethernet.h>

/*-----( Declare Constants )-----*/
/*-----( Declare objects )-----*/
// set the LCD address to 0x27 for a 20 chars 4 line display
// Set the pins on the I2C chip used for LCD connections:
//                    addr, en,rw,rs,d4,d5,d6,d7,bl,blpol
LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

IPAddress ip(10, 0, 0, 9); //fallback

EthernetClient client;

char server[] = "api.thingspeak.com"; //thingspeak API

unsigned long lastConnectionTime = 0;             // last time you connected to the server, in milliseconds
const unsigned long postingInterval = 1L * 1000L; // delay between updates, in milliseconds
// the "L" is needed to use long type numbers

void setup()   /*----( SETUP: RUNS ONCE )----*/
{
  Serial.begin(9600);  // Used to type in characters

  lcd.begin(20, 4);        // initialize the lcd for 20 chars 4 lines, turn on backlight

  // ------- Quick 3 blinks of backlight  -------------
  for (int i = 0; i < 3; i++)
  {
    lcd.backlight();
    delay(100);
    lcd.noBacklight();
    delay(50);
  }
  lcd.backlight();



  lcd.setCursor(3, 0); // 0 indexed: character, line
  lcd.print("Hello, world!");

  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip);
  }
  Serial.println("got IP!");
  lcd.setCursor(0, 3);
  lcd.print(Ethernet.localIP());
  Serial.println(Ethernet.localIP());

  if (client.connect(server, 80)) {
    Serial.println("connected");
    Serial.println("GET /update?api_key=RVLD035W0B2H5AGY&field1=0 HTTP/1.0");
    client.println("GET /update?api_key=RVLD035W0B2H5AGY&field1=0 HTTP/1.0");
    client.println();
    lcd.setCursor(0, 0);
    lcd.print("Sent!");
  } else {
    Serial.println("connection failed");
  }
}

void httpRequest() {
  // close any connection before send a new request.
  // This will free the socket on the WiFi shield
  client.stop();

  // if there's a successful connection:
  if (client.connect(server, 80)) {
    Serial.println("connecting...");
    // send the HTTP GET request:
    String mil = String(millis(), DEC);
    lcd.setCursor(0, 2);
    lcd.print(millis());
    Serial.println(mil);
    client.println("GET /update?api_key=RVLD035W0B2H5AGY&field1=" + mil + " HTTP/1.0");
    client.println("Host: www.arduino.cc");
    client.println("User-Agent: arduino-ethernet");
    client.println("Connection: close");
    client.println();


    // note the time that the connection was made:
    lastConnectionTime = millis();
  } else {
    // if you couldn't make a connection:
    Serial.println("connection failed");
  }
}

void loop()   /*----( LOOP: RUNS CONSTANTLY )----*/
{
  while (client.available()) {
    char c = client.read();
    Serial.print(c);
  }

  if (millis() - lastConnectionTime > postingInterval) {
    httpRequest();
  }

  switch (Ethernet.maintain())
  {
    case 1:
      //renewed fail
      Serial.println("Error: renewed fail");
      break;

    case 2:
      //renewed success
      Serial.println("Renewed success");

      //print your local IP address:
      Serial.println(Ethernet.localIP());
      break;

    case 3:
      //rebind fail
      Serial.println("Error: rebind fail");
      break;

    case 4:
      //rebind success
      Serial.println("Rebind success");

      //print your local IP address:
      Serial.println(Ethernet.localIP());
      break;

    default:
      //nothing happened
      break;

  }
}

