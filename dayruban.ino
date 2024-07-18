#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// Configuration Wi-Fi
const char* ssid = "Ideas-Lab Invités";
const char* password = "autonomie";

// Configuration des LEDs
#define PIN            14
#define NUMPIXELS      21
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

// Configuration du NTP
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 3600, 60000); // UTC+1

// Définir les couleurs pour chaque jour
uint32_t colors[7] = {
  pixels.Color(102, 255, 102), // Lundi - Vert clair
  pixels.Color(102, 178, 255), // Mardi - Bleu clair
  pixels.Color(255, 102, 178), // Mercredi - Rose
  pixels.Color(255, 255, 102), // Jeudi - Jaune
  pixels.Color(51, 153, 255),  // Vendredi - Bleu
  pixels.Color(255, 153, 51),  // Samedi - Orange
  pixels.Color(178, 102, 255)  // Dimanche - Mauve
};

void setup() {
  Serial.begin(115200);
  delay(10);

  // Connexion au Wi-Fi
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi connected");
  
  // Initialisation des LEDs
  pixels.begin();
  pixels.show(); // Initialise toutes les LEDs à 'off'

  // Initialisation du client NTP
  timeClient.begin();
}

void loop() {
  timeClient.update();
  
  // Obtenir le jour de la semaine (0 = dimanche, 1 = lundi, ..., 6 = samedi)
  int dayOfWeek = (timeClient.getDay() + 6) % 7; // pour aligner avec le ruban LED (commencer à 0 pour lundi)
  
  // Effacer toutes les LEDs
  pixels.clear();
  
  // Allumer 3 LEDs selon le jour de la semaine
  for (int i = 0; i < 3; i++) {
    int ledIndex = (dayOfWeek * 3 + i) % NUMPIXELS;
    pixels.setPixelColor(ledIndex, colors[dayOfWeek]);
  }
  
  // Afficher les changements
  pixels.show();
  
  // Attendre avant de rafraîchir (ajuster si nécessaire)
  delay(1000);
}
