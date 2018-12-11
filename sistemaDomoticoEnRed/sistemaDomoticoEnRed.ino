/*
 * INSTITUTO TECNOLÓGICO DE LEÓN
 * SISTEMA DOMÓTICO EN RED (WIFI)
 * ELABORADO POR:
  * JORGE ENRIQUE AGUADO GUANÍ
  * NATALIA MÉNDEZ MARTÍNEZ
*/

#include <ESP8266WiFi.h>
#include <DHT11.h>

/* Cambie el ssid y contraseña por su red WiFi*/
const char* ssid = "INFINITUM3021_2.4";
const char* password = "9nNxwB07ba";

/* Puerto 80 HTTP */
WiFiServer server(80);

/* Salidas digitales GPIO*/
int foco = 13;     
int ledRojo = 14;  
int buzzer = 12;   
const int in1Pin = 5;   //Entrada 2 del puente-H
const int in2Pin = 16;  // Entrada 7 del puente-H
const int sensorPin = 2;

/* Variables sensor de temperatura */
int pin = 4;  //GPIO4
DHT11 dht11(pin);

/* Variables para el control de la casa */
boolean focoPrendidoPorFaltaDeLuz = false;
boolean alarmaPorIntruso = false;
boolean puertaAbierta = false;


void setup() {
  Serial.begin(115200);
  delay(10);
  
  pinMode(foco, OUTPUT);
  pinMode(ledRojo, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(in1Pin, OUTPUT); 
  pinMode(in2Pin, OUTPUT);
  pinMode(sensorPin , INPUT);
  
  noTone(buzzer);
  digitalWrite(foco,LOW);
  digitalWrite(ledRojo,LOW);

/* Conecta a la red WiFi */
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

/* Inicia el servidor */
  server.begin();
  Serial.println("Server started");

/* Imprime la IP del servidor */
  Serial.print("Use this URL to connect: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");

}

void loop() {  
/* ---Luminosidad (sensor 1)--- */
  // Lee el valor de la salida analógica del dispositivo
  int sensorValue = analogRead(A0);
  
  // Convierte el valor (0 - 1023) a luminosidad en voltaje (0 - 5V):
  float luminosidad = sensorValue * (5.0 / 1023.0) *200;
  
  // Imprime el valor que se lee de la fotoresistencia
  Serial.print("Valor luz: ");
  Serial.println(luminosidad);                         

  // Si la luminosidad es baja, enciende el foco de la casa (led amarillo)
  if(luminosidad < 10) {
    focoPrendidoPorFaltaDeLuz = true;
    digitalWrite(foco, HIGH);
  }

  delay(200);

/* ---Sensor Infrarrojo (sensor 2)--- */
  int sensorInfrarrojo = 0;
  sensorInfrarrojo = digitalRead(sensorPin);  //lectura digital de pin del infrarrojo

  Serial.println(sensorInfrarrojo);

  // Si detecta algo, su señal es LOW (0) y enciende la alarma
  if (sensorInfrarrojo == LOW) {
      digitalWrite(ledRojo, HIGH);
      tone(buzzer,1000);
      alarmaPorIntruso = true;
  } else {
    
  }

/* ---Temperatura (sensor 3)--- */
  int err;
  float temp, hum;
  if((err = dht11.read(hum, temp)) == 0) { // Si devuelve 0 es que ha leído bien
    //Imprime el valor de la temperatura y humedad leídos
    Serial.print("Temperatura: ");
    Serial.print(temp);
    Serial.print(" Humedad: ");
    Serial.print(hum);
    Serial.println();
   } else {
    Serial.println();
    Serial.print("Error Num :");
    Serial.print(err);
    Serial.println();
   }
  
  delay(1000);

/* ---Uso del actuador---*/
  //Si la temperatura es excesiva, se abre la ventanilla para permitir el paso a la ventilación
  if(temp > 23 && puertaAbierta == false) {
    // Abrir puerta    
    digitalWrite(in1Pin,LOW); 
    digitalWrite(in2Pin,HIGH);
    delay(400);
    // Parar
    digitalWrite(in1Pin,LOW);
    digitalWrite(in2Pin,LOW);
    puertaAbierta = true;
  }
  
  delay(100);

/* ---Cliente---*/
  WiFiClient client = server.available();
  if(!client) {
    return;
  }

  // Espera hasta que el cliente envía algo
  Serial.println("new client");
  while(!client.available()); {
    delay(1);
  }

  // Lee la primera línea de la solicitud
  String request = client.readStringUntil('\r');
  Serial.println(request);
  client.flush();
  

/* ---Coincidencias de la solicitud  del cliente--- */
  int value = LOW;
  int valueRed = LOW;
  int valueMotor = LOW;
  
  if(request.indexOf("/LED=ON") != -1) {
    digitalWrite(foco, HIGH);
    value = HIGH;
  }
  if(request.indexOf("/LED=OFF") != -1) {
    digitalWrite(foco, LOW);
    value = LOW;
    focoPrendidoPorFaltaDeLuz = false;
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
  if(request.indexOf("/REFRESH=ON") != -1) {
    // Se usa para actualizar la página del servidor
  }
  if(request.indexOf("/DOOR=OPEN") != -1) {
    // Abrir puerta
    if(valueMotor == LOW) {      
      digitalWrite(in1Pin,LOW); 
      digitalWrite(in2Pin,HIGH);
      delay(400);
      // Parar
      digitalWrite(in1Pin,LOW);
      digitalWrite(in2Pin,LOW);
      valueMotor = HIGH;
      puertaAbierta = true;
    }
    
  }
  if(request.indexOf("/DOOR=CLOSE") != -1) {
    // Cerrar puerta
    digitalWrite(in1Pin,HIGH);
    digitalWrite(in2Pin,LOW);
    delay(400);
    // Parar
    digitalWrite(in1Pin,LOW);
    digitalWrite(in2Pin,LOW);
    valueMotor = LOW;
    puertaAbierta = false;    
  }

/* --- Sección: página web--- */
  // El servidor web responde de acuerdo a la solicitud del cliente

  //Regresa la respuesta
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("");
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");

  client.println("<div style=\"padding: 0px 0px 0px 20px;\">");
  client.println("<h1>Control del sistema</h1><br>");
  client.println("<a href=\"/REFRESH=ON\"\"><button>Actualizar </button></a><br /> <br />");
  
  client.print("El foco esta: ");

  if(value == HIGH || focoPrendidoPorFaltaDeLuz == true) {
    client.print("Encendido");
  }else {
    client.print("Apagado");    
  }
  client.println("<br><br>");
  client.println("<a href=\"/LED=ON\"\"><button>Prender foco </button></a>");
  client.println("<a href=\"/LED=OFF\"\"><button>Apagar foco </button></a><br /> <br />");

  client.print("La alarma esta: ");

  if(valueRed == HIGH || alarmaPorIntruso == true) {
    client.print("Encendida");
  }else {
    client.print("Apagada");    
  }

  if(alarmaPorIntruso == true) {
    client.println("<br><span style=\"color:red\">INTRUSO!</span>");
  }
  
  client.println("<br><br>");
  client.println("<a href=\"/LEDROJO=ON\"\"><button>Prender alarma </button></a>");
  client.println("<a href=\"/LEDROJO=OFF\"\"><button>Apagar alarma </button></a><br /> <br />");

  client.print("La puerta esta: ");

  if(valueMotor == HIGH || puertaAbierta == true) {
      client.print("Abierta");   
  }else {
    client.print("Cerrada");    
  }
  client.println("<br><br>");
  client.println("<a href=\"/DOOR=OPEN\"\"><button>Abrir puerta </button></a>");
  client.println("<a href=\"/DOOR=CLOSE\"\"><button>Cerrar puerta </button></a><br />");
  client.println("</div>");
  client.println("</html>");
  
  delay(1);
  Serial.println("Client disconnected");
  Serial.println("");

}
