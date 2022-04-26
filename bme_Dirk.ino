#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <ESP8266WebServer.h>
#include <Wire.h>

#define SEALEVELPRESSURE_HPA (1013.25)
#define voltageSensor A0
const float R1 = 30000.0f; // Der Widerstand R1 hat eine größe von 30 kOhm
const float R2 = 7500.0f; //  Der Widerstand R2 hat eine größe von 7,5 kOhm
const float MAX_VIN = 3.1f;

Adafruit_BME280 bme;

float temperature, humidity, pressure, altitude;
float vout,Voltage;
float data[10];
int i = 0;

/*Put your SSID & Password*/
const char* SSID = "WLAN-NWT";  // Enter SSID here
const char* password = "123456789";  //Enter Password here

ESP8266WebServer server(80);              
 
void setup() {
  Serial.begin(115200);
  delay(100);
  
  bme.begin(0x76);   

  Serial.println("Connecting to ");
  Serial.println(SSID);

  //connect to your local wi-fi network
  WiFi.begin(SSID, password);

  //check wi-fi is connected to wi-fi network
  while (WiFi.status() != WL_CONNECTED) {
  delay(1000);
  Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected..!");
  Serial.print("Got IP: ");  Serial.println(WiFi.localIP());

  server.on("/", handle_OnConnect);
  server.onNotFound(handle_NotFound);

  server.begin();
  Serial.println("HTTP server started");

}
void loop() {
  server.handleClient();
}

void handle_OnConnect() {
  
  temperature = bme.readTemperature();
  humidity = bme.readHumidity();
  pressure = bme.readPressure() / 100.0F+60; //DURCH VERGLEICH MIT lUFTDRUCKDATEN AUS DEM INTERNET +60hPa
  altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);
  vout = (analogRead(voltageSensor)* MAX_VIN) / 1024.0f;
  Voltage = vout / (R2/(R1+R2)); 
  data[i % 10]= temperature;
  
  server.send(200, "text/html", SendHTML2(temperature,humidity,pressure,Voltage, data)); 
  i++;
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}


/*
 * Diese Funktion verwendet einen andere Art, die HTML Tags zu übertragen. So ist es für die
 * SuS einfacher, zu schreiben. 
 * Es kann die ganze html Seite direkt in dem String geschrieben werden.
 * Sogenannte Raw Strings sind Strings, die speziell dazu entwickelt wurde um anderen Code in einem
 * String einzubetten. Gerade z.B. html Code. Damit ist es deutlich einfacher, die nötigen Anführungszeichen,
 * Escape-Zeichen, Klammern, etc. zu schreiben.
 * 
 */
String SendHTML2(float temperature,float humidity,float pressure,float altitude,float data[]){ 

String htmlCode = R"=====(
<!DOCTYPE html>
<html>
 <head>
 <meta name='viewport' content='width=device-width, initial-scale=1.0'/>
 <meta charset='utf-8'>
 <meta http-equiv='refresh' content='5'>
 <style>
   body { 
      background-color: teal; /*Hintergrundfarbe*/
      font-family: Arial, Helvetica, sans-serif;
      font-size: normal;
      height: 100%;
      margin: 0px;
      padding: 20px;
      }
      
  div#kopfbereich {
      background-color: #f3c600;  /*GElb-orange*/
      color:black;
      /*padding: 10px 10px 10px 10px;*/
      border: 1px solid red;
  }

  div#textbereich{
     margin-left: 10px;  /*Randabstand mind. in der Breite des navibereichs*/
     background-color: white;
     padding-left: 5px;
    }

  div#fussbereich{
    background-color: #f3c600;


    }
  address{
    text-align: center; /*zentrieren*/
    font-size: 80%;
    font-style: normal; /*nicht kursiv*/
    letter-spacing: 2px;
    line-height:1.5;

}
   
   
   h1 {text-align:center;}
   h2 {text-align:left; }
   p { text-align:left; }
 </style>
   <title>Beispiel zum Auto Update - alle 5 Sekunden</title>
   </head>

 <body>
   <div id="kopfbereich">
     <h1>Wetterstation</h1>
   </div> <!-- Ende Kopfbereich-->
   
  <div id="textbereich">
     <h2>Temperatur</h2> 
     <p>Gemessener Temperaturwert: %temperature °C</p>
     <h2>Luftfeuchte</h2>
     <p>Gemessene Luftfeuchte: %humidity %</p>
     <h2>Luftdruck</h2>
     <p>Gemessene Luftdruck: %pressure hPa</p>
     <h2>Spannung</h2>
     <p>Gemessene Spannung: %Voltage V </p>






<svg height="180" width="500">
  <polyline points=
  "0,%data[0] 20,%data[1] 40,%data[2] 60,%data[3] 80,%data[4]" 
  style="fill:white;stroke:red;stroke-width:4" />
  Sorry, your browser does not support inline SVG.
</svg>





     
     
  </div> <!-- Ende Textbereich-->

  <div id = "fussbereich">
  <address>
    Dirk Massinger, Kreisgymnasium Riedlingen </br>
    derekmassi@icloud.com
  </address>
  </div>  <!-- Ende Fußbereich-->
   
 </body>
</html>
)=====";

/*
 * Ersetzen der Variablen.
 * Der Urpsrüngliche String wird kopiert (wer will) und dann wird z.B. %temperature mit der übergebenen Variable temperature ersetzt.
 * Diese muss natürlich vorher in einen String umgewandelt werden.
 * Dann wird der tmpString zurückgegeben und oben an den Webserver versandt
 */
String tmpString = htmlCode;
tmpString.replace("%temperature",String(temperature));
tmpString.replace("%humidity",String(humidity));
tmpString.replace("%pressure",String(pressure));
tmpString.replace("%Voltage",String(Voltage));
tmpString.replace("%data[0]",String(data[0]));
tmpString.replace("%data[1]",String(data[1]));
tmpString.replace("%data[2]",String(data[2]));
tmpString.replace("%data[3]",String(data[3]));
tmpString.replace("%data[4]",String(data[4]));
Serial.print(tmpString);
return tmpString;
}
