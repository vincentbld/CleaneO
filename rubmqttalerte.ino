#include <ESP8266WiFi.h>  // ou <WiFi.h> pour ESP32
#include <PubSubClient.h>
#include <Adafruit_NeoPixel.h>

#define LED_PIN     14     // Broche de données du ruban LED
#define NUM_LEDS    16     // Nombre de LEDs dans le ruban
#define BRIGHTNESS  64     // Luminosité du ruban LED

const char* ssid = "Ideas-Lab Invités";
const char* wifi_password = "autonomie";
const char* mqtt_server = " 192.168.68.121";
const char* mqtt_user = "jeedom";
const char* mqtt_password = "aaV2ARx8dDrV639SwwABeTnO3MIfVV3iOFVNmon5ZTkZbFVMpMECIeivUDTyCRwx";
const char* mqtt_topic_control = "led/control";
const char* mqtt_topic_status = "led/status";  // Sujet pour les messages de statut et heartbeat
const char* mqtt_topic_alert = "led/alert";    // Sujet pour les messages d'alerte

WiFiClient espClient;
PubSubClient client(espClient);

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

int currentDay = 0;  // Compteur de jour
bool alertState = false;  // État de l'alerte
bool blinkState = false;  // État de clignotement
unsigned long lastHeartbeat = 0;  // Pour le heartbeat
const unsigned long heartbeatInterval = 60000;  // Intervalle de heartbeat en millisecondes (1 minute)
unsigned long lastBlink = 0;  // Pour le clignotement de la LED en alerte
const unsigned long blinkInterval = 500;  // Intervalle de clignotement en millisecondes

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, wifi_password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  
  strip.begin();
  strip.setBrightness(BRIGHTNESS);
  updateLeds();  // Met à jour les LEDs selon le jour actuel
}

void callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.print("Message received: ");
  Serial.println(message);

  if (message.equals("reset")) {
    currentDay = 0;
    alertState = false;  // Désactiver l'alerte
    client.publish(mqtt_topic_alert, "normal");  // Envoyer un message "normal"
    updateLeds();
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP8266Client", mqtt_user, mqtt_password)) {  // Ajouter les identifiants MQTT ici
      Serial.println("connected");
      client.subscribe(mqtt_topic_control);
      client.publish(mqtt_topic_status, "Device connected");  // Envoyer un message à la connexion
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void updateLeds() {
  if (alertState) {
    // Clignotement rouge en alerte
    unsigned long currentMillis = millis();
    if (currentMillis - lastBlink >= blinkInterval) {
      lastBlink = currentMillis;
      blinkState = !blinkState;  // Inverser l'état de clignotement
      for (int i = 0; i < NUM_LEDS; i++) {
        if (blinkState) {
          strip.setPixelColor(i, strip.Color(255, 0, 0));  // Allumer en rouge
        } else {
          strip.setPixelColor(i, strip.Color(0, 0, 0));  // Éteindre la LED
        }
      }
      strip.show();
    }
  } else {
    // Mettre à jour les LEDs selon le jour actuel
    for (int i = 0; i < NUM_LEDS; i++) {
      if (i < currentDay * 2 + 1) {  // Chaque jour correspond à deux LEDs
        if (i < 6) {
          strip.setPixelColor(i, strip.Color(0, 255, 0));  // Vert
        } else if (i < 11) {
          strip.setPixelColor(i, strip.Color(255, 165, 0));  // Orange
        } else {
          strip.setPixelColor(i, strip.Color(255, 0, 0));  // Rouge
        }
      } else {
        strip.setPixelColor(i, strip.Color(0, 0, 0));  // Éteindre la LED
      }
    }
    strip.show();
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if (!alertState) {
    // Simulation d'un jour de progression (incrémenter currentDay tous les 24 heures)
    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate >= 86400000) { // 86400000 ms = 24 heures
      lastUpdate = millis();
      currentDay++;
      if (currentDay > 6) {  // Si le cycle de 7 jours est terminé
        alertState = true;  // Activer l'alerte
        client.publish(mqtt_topic_alert, "alerte");  // Envoyer une alerte
      }
      updateLeds();
    }
  } else {
    updateLeds();  // Mettre à jour les LEDs pour le clignotement en alerte
  }

  // Envoyer un heartbeat périodiquement
  if (millis() - lastHeartbeat >= heartbeatInterval) {
    lastHeartbeat = millis();
    client.publish(mqtt_topic_status, "Device is online");
  }
}
