#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <Servo.h>


#define WIFI_SSID "SSID"
#define WIFI_PASS "PASSWORD"


#define MQTT_SERV "io.adafruit.com"
#define MQTT_PORT 1883
#define MQTT_NAME "buswaroop94822"
#define MQTT_PASS "311fde6006f245c19584cbdbf9792b2e"
#define ROBO_REPLY_MSG "I am connected"


int forward1 = D0;
int reverse1 = D1;
int left1    = D7;
int right1   = D8;
int servoPin = D3;
int laserPin = D4;

char* servoAngleChar;
String servoAngleString;
int servoAngleInt;

Servo myservo;
WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, MQTT_SERV, MQTT_PORT, MQTT_NAME, MQTT_PASS);


Adafruit_MQTT_Subscribe d_forward = Adafruit_MQTT_Subscribe(&mqtt, MQTT_NAME "/f/forward");
Adafruit_MQTT_Subscribe d_reverse = Adafruit_MQTT_Subscribe(&mqtt, MQTT_NAME "/f/reverse");
Adafruit_MQTT_Subscribe d_left = Adafruit_MQTT_Subscribe(&mqtt, MQTT_NAME "/f/left");
Adafruit_MQTT_Subscribe d_right = Adafruit_MQTT_Subscribe(&mqtt, MQTT_NAME "/f/right");
Adafruit_MQTT_Subscribe servo_feed = Adafruit_MQTT_Subscribe(&mqtt, MQTT_NAME "/f/servoslider");
Adafruit_MQTT_Subscribe laser = Adafruit_MQTT_Subscribe(&mqtt, MQTT_NAME "/f/laser");

Adafruit_MQTT_Publish robotreplymessage = Adafruit_MQTT_Publish(&mqtt, MQTT_NAME "/f/message");

void setup()
{
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  //WiFi.begin("GSP iPhone","Gs9844990667");
  //WiFi.begin("NodeMCU12","NodeMCU12");
  

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(">");
    delay(50);
  }

  //subscribe for the necessary feeds
  mqtt.subscribe(&d_forward);
  mqtt.subscribe(&d_reverse);
  mqtt.subscribe(&d_left);
  mqtt.subscribe(&d_right);
  mqtt.subscribe(&servo_feed);
  mqtt.subscribe(&laser);
  
  myservo.attach(servoPin);
  robotreplymessage.publish(ROBO_REPLY_MSG);
  

  //pinMode(led, OUTPUT);
  pinMode(forward1,OUTPUT);
  pinMode(reverse1,OUTPUT);
  pinMode(left1,OUTPUT);
  pinMode(right1,OUTPUT);
  pinMode(servoPin,OUTPUT);
  pinMode(laserPin,OUTPUT);
  
  
}

void loop()
{
  //Connect/Reconnect to MQTT
  MQTT_connect();

  //Read from our subscription queue until we run out, or
  //wait up to 5 seconds for subscription to update
  Adafruit_MQTT_Subscribe * subscription;
  while ((subscription = mqtt.readSubscription(5000)))
  {
    //If we're in here, a subscription updated...
    //robotreplymessage.publish(ROBO_REPLY_MSG);
    if (subscription == &d_forward)
    {
      
      //scan whether forward button is pressed
      if (strcmp((char*) d_forward.lastread, "0"))
      {
        digitalWrite(forward1, HIGH);
        digitalWrite(reverse1, LOW);
        digitalWrite(left1, HIGH);
        digitalWrite(right1, LOW);        
        robotreplymessage.publish("Moving Forward");
      }
      else// if (strcmp((char*) d_forward.lastread, "1"))
      {
        digitalWrite(forward1, LOW);
        digitalWrite(reverse1, LOW);
        digitalWrite(left1, LOW);
        digitalWrite(right1, LOW);   
        //robotreplymessage.publish("Idle");
      }
    }
    if (subscription == &d_reverse)
    {
      
      //scan whether reverse button is pressed
      if (strcmp((char*) d_reverse.lastread, "0"))
      {
        digitalWrite(forward1, LOW);
        digitalWrite(reverse1, HIGH);
        digitalWrite(left1, LOW);
        digitalWrite(right1, HIGH); 
        robotreplymessage.publish("Moving Reverse");
      }
      else //if (strcmp((char*) d_reverse.lastread, "0"))
      {
          digitalWrite(forward1, LOW);
          digitalWrite(reverse1, LOW);
          digitalWrite(left1, LOW);
          digitalWrite(right1, LOW);  
          //robotreplymessage.publish("Idle");
      }
    }
    if (subscription == &d_left)
    {
      
      //scan whether reverse button is pressed
      if (strcmp((char*) d_left.lastread, "0"))
      {
        digitalWrite(forward1, LOW);
        digitalWrite(reverse1, HIGH);
        digitalWrite(left1, HIGH);
        digitalWrite(right1, LOW);
        robotreplymessage.publish("Took Left"); 
      }
      else //if (strcmp((char*) d_left.lastread, "0"))
      {
        
          digitalWrite(forward1, LOW);
          digitalWrite(reverse1, LOW);
          digitalWrite(left1, LOW);
          digitalWrite(right1, LOW);
          //robotreplymessage.publish("Idle");  
      }
    }
    if (subscription == &d_right)
    {
      
      //scan whether reverse button is pressed
      if (strcmp((char*) d_right.lastread, "0"))
      {
        digitalWrite(forward1, HIGH);
        digitalWrite(reverse1, LOW);
        digitalWrite(left1, LOW);
        digitalWrite(right1, HIGH); 
        robotreplymessage.publish("Took Right");
      }
      else //if (strcmp((char*) d_right.lastread, "0"))
      {
        
          digitalWrite(forward1, LOW);
          digitalWrite(reverse1, LOW);
          digitalWrite(left1, LOW);
          digitalWrite(right1, LOW); 
          //robotreplymessage.publish("Idle"); 
      }
    }

    if (subscription == &laser)
    {
      if (strcmp((char*) laser.lastread, "ON"))
      {
        digitalWrite(laserPin, HIGH);
        robotreplymessage.publish("Laser Turned On");
      }
      else
      {        
          digitalWrite(laserPin, LOW);
          robotreplymessage.publish("Laser Turned Off");
      }
    }
    
    if (subscription == &servo_feed)
    {
      servoAngleChar   = (char*)servo_feed.lastread;
      servoAngleString = (String) servoAngleChar;
      servoAngleInt = servoAngleString.toInt();
      
      if(servoAngleInt > 180)
      {
        myservo.write(0);
        robotreplymessage.publish("Angle cannot be more than 180");
      }
      else
      {
        myservo.write(servoAngleInt);
        robotreplymessage.publish("Changed laser direction");  
      }      
    }
    
  }
}


void MQTT_connect()
{

  //  // Stop if already connected
  if (mqtt.connected() && mqtt.ping())
  {
    //    mqtt.disconnect();
    return;
  }

  int8_t ret;

  mqtt.disconnect();

  Serial.print("Connecting to MQTT... ");
  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) // connect will return 0 for connected
  {
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000);  // wait 5 seconds
    retries--;
    if (retries == 0)
    {
      ESP.reset();
    }
  }
  Serial.println("MQTT Connected!");
}
