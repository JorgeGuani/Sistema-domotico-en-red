#include <ESP8266WiFi.h>

const char* ssid = "INFINITUM3021_2.4";
const char* password = "9nNxwB07ba";
//const char* ssid = "INFINITUM733AD1";
//const char* password = "09A510608C";

int foco = 13;     // GPIO13
int ledRojo = 14;  // GPIO14
int buzzer = 12;   // GPIO12

const int sensorPin = 2;

WiFiServer server(80);

//Variables para el sensor de luminosidad
const long A = 1000;      //Resistencia en oscuridad en KΩ
const int B = 15;         //Resistencia a la luz (10 Lux) en KΩ
const int Rc = 10;        //Resistencia calibracion en KΩ
const int PINLUZ = A0;    //Entrada analógica 0
int valorAnalogicoLuz = 0;
int valorLuminosidad = 0;

boolean focoPrendidoPorFaltaDeLuz = false;
boolean alarmaPorIntruso = false;

void setup() {
  Serial.begin(115200);
  delay(10);

  pinMode(foco, OUTPUT);
  pinMode(ledRojo, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(sensorPin , INPUT);
  
  noTone(buzzer);
  digitalWrite(foco,LOW);
  digitalWrite(ledRojo,LOW);

  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print('.');
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.print("Use this URL to connect: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");

}

void loop() {  
  // read the input on analog pin 0:
  int sensorValue = analogRead(A0);
  
  // Convert the analog reading (which goes from 0 - 1023) to a luminosidad (0 - 5V):
  float luminosidad = sensorValue * (5.0 / 1023.0) *200;
  
  // print out the value you read:
  Serial.print("Valor luz: ");
  Serial.println(luminosidad);                         
  
  if(luminosidad < 10) {
    //tone(foco,1000);
    digitalWrite(foco, HIGH);
  }

  delay(200);
  
  // Sensor infrarrojo
  int sensorInfrarrojo = 0;
  sensorInfrarrojo = digitalRead(sensorPin);  //lectura digital de pin del infrarrojo

  Serial.println(sensorInfrarrojo);
 
  if (sensorInfrarrojo == LOW) {
      //Serial.println("Detectado obstaculo");
      digitalWrite(ledRojo, HIGH);
      tone(buzzer,1000);
      alarmaPorIntruso = true;
  } else {
    
  }

  delay(200);
  
  WiFiClient client = server.available();
  if(!client) {
    return;
  }

  // Wait until the client sends some data
  Serial.println("new client");
  while(!client.available()); {
    delay(1);
  }

  // Read the first line of the request
  String request = client.readStringUntil('\r');
  Serial.println(request);
  client.flush();
  
  


  // Match the request

  int value = LOW;
  int valueRed = LOW;
  
  if(request.indexOf("/LED=ON") != -1) {
    digitalWrite(foco, HIGH);
    //tone(foco,1000);
    value = HIGH;
  }
  if(request.indexOf("/LED=OFF") != -1) {
    digitalWrite(foco, LOW);
    //noTone(foco);
    value = LOW;
  }
  if(request.indexOf("/LEDROJO=ON") != -1) {
    digitalWrite(ledRojo, HIGH);
    tone(buzzer, 1000);
    valueRed = HIGH;
  }
  if(request.indexOf("/LEDROJO=OFF") != -1) {
    digitalWrite(ledRojo, LOW);
    noTone(buzzer);
    valueRed = LOW;
    alarmaPorIntruso = false;
  }
  
  
/*--- Sección página web--- */

  // Set foco according to the request
  // digitalWrite(foco, value);

  //Return the response
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("");
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");

  client.println("<div style=padding: 0px 0px 0px 20px;>");
  client.println("<h1>Control del sistema</h1><br>");
  
  client.print("El foco esta: ");

  if(value == HIGH) {
    client.print("Encendido");
  }else {
    client.print("Apagado");    
  }
  client.println("<br><br>");
  client.println("<a href=\"/LED=ON\"\"><button>Prender foco </button></a>");
  client.println("<a href=\"/LED=OFF\"\"><button>Apagar foco </button></a><br /> <br />");

  client.print("La alarma esta: ");

  if(valueRed == HIGH) {
    client.print("Encendida");
  }else {
    client.print("Apagada");    
  }
  client.println("<br><br>");
  client.println("<a href=\"/LEDROJO=ON\"\"><button>Prender alarma </button></a>");
  client.println("<a href=\"/LEDROJO=OFF\"\"><button>Apagar alarma </button></a><br /> <br />");

  /*client.print("La puerta esta: ");

  if(value == HIGH) {
    client.print("Abierta");
  }else {
    client.print("Cerrada");    
  }
  client.println("<br><br>");
  client.println("<a href=\"/LED=ON\"\"><button>Abrir puerta </button></a>");
  client.println("<a href=\"/LED=OFF\"\"><button>Cerrar puerta </button></a><br />");*/
  client.println("</div>");
  client.println("</html>");
  
  delay(1);
  Serial.println("Client disconnected");
  Serial.println("");

}
