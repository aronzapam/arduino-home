#include <SPI.h>
#include <Ethernet.h>
#include <DHT.h>
#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>
//Ponemos la dirección MAC e IP que queremos que use nuestro Arduino para conectarse al Router
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 1, 10);
String readString;
//Definimos que el puerto HTTP sera el 81
EthernetServer server2(80);
//Variables de control
int rele = 7;
int rele2 = 8;
int rele3 = 9;
// definicion dht11
#define DHTPIN 4
#define DHTTYPE DHT11
// Inicializamos el sensor DHT11
DHT dht(DHTPIN, DHTTYPE);
int getFingerprintIDez();
SoftwareSerial mySerial(2, 3); // pines de comunicacion
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

// declaracion de consumo
float Sensibilidad = 0.100;
;
void setup()
{
  //Iniciar la conexión de red y serie
  Ethernet.begin(mac, ip);
  server2.begin();
  Serial.begin(9600);
  pinMode(rele, OUTPUT);
  pinMode(rele2, OUTPUT);
  dht.begin();


  Serial.println("prueba de dedo");
  pinMode(rele3, OUTPUT); //declaro el pin que usare para control
  // ajusta velocidad del sensor
  finger.begin(57600);
  if (finger.verifyPassword()) {
    Serial.println("sensor de huella encontrado!");
  } else {
    Serial.println("sensor no encontrado:(");
    while (1);
  }
  Serial.println("esperando huella");
}


void loop()
{

  getFingerprintIDez();
  delay(250);
  // Esperamos que venga algún cliente web al puerto 81
  EthernetClient cliente = server2.available();
  if (cliente) {
    boolean currentLineIsBlank = true;
    while (cliente.connected()) {
      if (cliente.available()) {
        char c = cliente.read();
        if (readString.length() < 100) {
          //Almacenar caracteres en un string
          readString += c;
        }
        //Si ha terminado la petición del cliente
        if (c == 'n') {
          if (readString.indexOf("?e1") > 0)
          {
            digitalWrite(rele, HIGH);
          }
          else if (readString.indexOf("?a1") > 0)
          {
            digitalWrite(rele, LOW);
          }

          if (readString.indexOf("?e2") > 0)
          {
            digitalWrite(rele2, HIGH);
          }
          else if (readString.indexOf("?a2") > 0)
          {
            digitalWrite(rele2, LOW);
          }


          int h = dht.readHumidity();
          // Leemos la temperatura en grados centígrados (por defecto)
          int t = dht.readTemperature();
          // obtener consumo
          float voltajeSensor = analogRead(A0) * (5.0 / 1023.0); //lectura del sensor
          float I = (voltajeSensor - 2.5) / Sensibilidad; //Ecuación  para obtener la corriente
          Serial.print("Corriente: ");
          Serial.println(I, 3);
          delay(200);


          //Enviamos la cabecera web
          cliente.println("HTTP/1.1 200 OK");
          cliente.println("Refresh: 5");
          cliente.println("Content-Type: text/html");
          cliente.println();
          //Mostrqamos la web de control
          cliente.println("<center> <h2>Temperatura C°</h2> ");
          cliente.println(t);
          cliente.println("<center> <h2>Humedad %</h2> ");
          cliente.println(h);
          cliente.println("<center> <h2>Consumo Watt*segundos</h2> ");
          cliente.println(I, 3);
          cliente.println(" </center>");
          cliente.println("<br />");
          break;
        }
      }
    }
    // Darle un respiro al navegador web para recibir los datos
    delay(1);
    //Limpiar String para proxima etición HTTP
    readString = "";
    //Cerrar conexión
    cliente.stop();
  }



}

uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println("No finger detected");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK success!

  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK converted!
  p = finger.fingerFastSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Found a print match!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Did not find a match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  // found a match!
  Serial.print(" ID #"); Serial.print(finger.fingerID);
  Serial.print("Numero confidencial "); Serial.println(finger.confidence);
}

// retorna -1 si if falla
int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -1;
  /////////comparo si es el numero asignado
  if (finger.fingerID == 1)
  {
    Serial.println(" bienvenido aron ");
    digitalWrite(rele3, HIGH);
    delay (1000);
    digitalWrite(rele3, LOW);

  }

  if (finger.fingerID == 2)
  {
    Serial.println(" bienvenida pedro ");
  }
  //////////////////////

  // found a match!
  Serial.print("ID #");
  Serial.print(finger.fingerID);
  Serial.print("Numero Confidencial");
  Serial.println(finger.confidence);

  return finger.fingerID;

}
