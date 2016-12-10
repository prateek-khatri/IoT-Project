#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>

#define SSID "R1 HD "
#define PSWD "12345678"
#define URI "http://api.humandroid.us/milk-api/add"
#define CONTENT_TYPE "application/json"
#define MAX_NO_MILK_VAL 100
#define UPPER_OFFSET  0
#define LOWER_OFFSET  1
#define BOTTLE_PICK_TIMER 20
#define MAX_CAPACITY 3.78

enum STAGES
{
  _X100, _X70, _X50, _X20, _X0, _XX
};
enum STATES
{
  NOINIT, MILK_FINISHED,MILK_REQUIRED,MILK_NOT_REQ,NO_BOTTLE
};
                 //_X100     _X70      _X50      _X20      _X0     _XX 
int lookup[] = {800, 595, 475, 555, 535, 515, 440, 370, 225, 155, 145, 0};
float capacities[] = {3.78,2.78,1.78,0.78, 0.0};

HTTPClient http;
int state;

void initHttp()
{
  http.begin(URI);
  http.addHeader("Content-Type", CONTENT_TYPE);
}

bool connectWiFi()
{
  WiFi.begin(SSID,PSWD);
  Serial.println();Serial.println();
  Serial.println("Waiting for Connection..");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(250);
  }

  Serial.print("Connected to AP: ");Serial.println(SSID);
  return true;
}

String createJson(int sensorRaw, float liters)
{
  String json = "{\"sensorRaw\":" + String(sensorRaw) + ", \"liters\":" + String(liters, 2) + ", \"capacity\":" + String(MAX_CAPACITY) +"}";
  return json;
}

bool sendPostRequest(String json)
{
  Serial.println("Sending :");
  Serial.println(json);
  int httpCode = http.POST(json);
  if (httpCode != 200)
  {
    Serial.println("HTTP ERROR");
    return false;
  }
  String payload = http.getString();
  Serial.println("Response :");Serial.println(payload);
  return true;
}

float findCapacity(int sensorValue)
{
   float literValue = 0.0;
   if (sensorValue >= 530 )
   {
        literValue = capacities[_X100];
        state = MILK_NOT_REQ;
   }
   else if (sensorValue >= 490)
   {
        literValue = capacities[_X70];
        state = MILK_NOT_REQ;
   }
   else if (sensorValue >= 430 )
   {
        literValue = capacities[_X50];
        state = MILK_NOT_REQ;
   }
   else if (sensorValue >= 280)
   {
        literValue = capacities[_X20];
        state = MILK_REQUIRED;
   }
   else if (sensorValue >= 155)
   {
        literValue = capacities[_X0];
        state = MILK_FINISHED;
   }
   else if (sensorValue >= 0)
   {
        // Bottle Was Picked up;
        return -1;
   }
   Serial.print("Capacity = ");Serial.println(literValue);
   return literValue;
   
}

void setup()
{
    Serial.begin(115200);
    connectWiFi();
    initHttp();
    
    state = MILK_FINISHED;
    int sensorValue = analogRead(A0);

    Serial.println("System Initialized..No Items Present");
    Serial.print("Waiting...");
    while (sensorValue < 100)
    {
      //Bottle Not Present
      Serial.print(".");
      delay(500);
      sensorValue = analogRead(A0);
    }
    Serial.println();
}

void loop()
{
    // Bottle Present
    int sensorValue = analogRead(A0);

    float bottleCapacity = findCapacity(sensorValue);
    if (bottleCapacity < 0 && state != MILK_FINISHED)
    {
        // Bottle Was Picked Up
        Serial.print("Bottle Picked...waiting ");Serial.print(BOTTLE_PICK_TIMER);Serial.println(" Seconds");
        delay(BOTTLE_PICK_TIMER * 1000);

        sensorValue = analogRead(A0);
        bottleCapacity = findCapacity(sensorValue);
        if (bottleCapacity < 1)
        {
           state = MILK_FINISHED;
           String json = createJson(sensorValue, bottleCapacity);
           sendPostRequest(json);
        }
    }
    else if (bottleCapacity > -1)
    {
        Serial.println("Sending Values...");
        String json = createJson(sensorValue, bottleCapacity);
        sendPostRequest(json);
    }

   delay(20000);
  
}
