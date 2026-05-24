#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <ArduinoJson.h>

// Konfigurasi Wi-Fi & MQTT (Wokwi default)
const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* mqtt_server = "broker.hivemq.com"; // Broker publik untuk testing

WiFiClient espClient;
PubSubClient client(espClient);

// Definisi Pin Sensor
#define PIR_PIN 27
#define DHT_PIN 4
#define DHT_TYPE DHT22
#define LDR_PIN 34
#define CO2_SIM_PIN 35
#define WATT_SIM_PIN 32

// Definisi Pin Aktuator
#define RELAY_LAMPU 25
#define IR_AC_SIM 26
#define RELAY_EXHAUST 33

DHT dht(DHT_PIN, DHT_TYPE);

// Variabel Waktu & Logika
unsigned long waktuTerakhirAdaOrang = 0;
// Simulasi diset 15 detik (15000 ms) agar juri tidak menunggu lama
const unsigned long BATAS_WAKTU_KOSONG = 15000; 

bool ruanganKosong = true;

void setup_wifi() {
  delay(10);
  Serial.println("Menghubungkan ke Wi-Fi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi terhubung.");
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Menghubungkan ke MQTT...");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("Terhubung ke MQTT Broker");
    } else {
      Serial.print("Gagal, status=");
      Serial.print(client.state());
      Serial.println(" coba lagi dalam 5 detik");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);

  dht.begin();
  
  pinMode(PIR_PIN, INPUT);
  pinMode(RELAY_LAMPU, OUTPUT);
  pinMode(IR_AC_SIM, OUTPUT);
  pinMode(RELAY_EXHAUST, OUTPUT);

  digitalWrite(RELAY_LAMPU, LOW);
  digitalWrite(IR_AC_SIM, LOW);
  digitalWrite(RELAY_EXHAUST, LOW);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // 1. Baca Sensor
  int gerak = digitalRead(PIR_PIN);
  float suhu = dht.readTemperature();
  int cahayaRaw = analogRead(LDR_PIN); 
  
  // Mapping analog ke nilai estimasi (lux, ppm, watt)
  int lux = map(cahayaRaw, 0, 4095, 0, 1000); 
  int co2 = map(analogRead(CO2_SIM_PIN), 0, 4095, 400, 2000);
  int daya = map(analogRead(WATT_SIM_PIN), 0, 4095, 0, 6000);

  // 2. Evaluasi Kehadiran
  if (gerak == HIGH) {
    waktuTerakhirAdaOrang = millis();
    ruanganKosong = false;
  } else {
    if (millis() - waktuTerakhirAdaOrang > BATAS_WAKTU_KOSONG) {
      ruanganKosong = true;
    }
  }

  // 3. Logika Aktuasi (Edge Computing)
  if (ruanganKosong) {
    digitalWrite(RELAY_LAMPU, LOW);
    digitalWrite(IR_AC_SIM, LOW);
    digitalWrite(RELAY_EXHAUST, LOW);
  } else {
    if (lux < 300) {
      digitalWrite(RELAY_LAMPU, HIGH);
    } else {
      digitalWrite(RELAY_LAMPU, LOW);
    }

    if (suhu > 25.0) {
      digitalWrite(IR_AC_SIM, HIGH); 
    } else {
      digitalWrite(IR_AC_SIM, LOW);
    }

    if (co2 > 1000) {
      digitalWrite(RELAY_EXHAUST, HIGH);
    } else {
      digitalWrite(RELAY_EXHAUST, LOW);
    }
  }

  // 4. Buat Payload JSON
  StaticJsonDocument<200> doc;
  doc["ruangan_kosong"] = ruanganKosong;
  doc["suhu"] = suhu;
  doc["co2"] = co2;
  doc["lux"] = lux;
  doc["daya_watt"] = daya;
  doc["lampu_on"] = !ruanganKosong && (lux < 300);
  doc["ac_on"] = !ruanganKosong && (suhu > 25.0);

  char jsonBuffer[256];
  serializeJson(doc, jsonBuffer);

  // 5. Transmisi Data tiap 3 detik
  static unsigned long lastMsg = 0;
  if (millis() - lastMsg > 3000) {
    lastMsg = millis();
    client.publish("filkom/gedungF/g2_1/telemetry", jsonBuffer);
    Serial.print("Data dikirim: ");
    Serial.println(jsonBuffer);
  }
}
