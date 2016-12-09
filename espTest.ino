#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>

#define SSID "R1 HD "
#define PSWD "12345678"
#define URI "http://api.humandroid.us/marwell-api/add"


HTTPClient http;
void setup()
{
  Serial.begin(115200);
  WiFi.begin(SSID,PSWD);

  Serial.print("Waiting for Connection..");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.println("Connected..!");

  http.begin(URI);
  http.addHeader("Content-Type", "application/json");
  Serial.println("Sending POST Request");
  String json = "{\"sensorRaw\":500, \"liters\":2.54}";
  int httpCode = http.POST(json);
  Serial.print("Post Request Sent: ");
  Serial.println(httpCode);
  String payload = http.getString();
  Serial.println(payload);
  http.end();
  
}

void loop()
{
  
}
