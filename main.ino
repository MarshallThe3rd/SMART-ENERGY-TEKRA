#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

// ================= KONFIGURASI PIN ESP32 =================
#define PIR_PIN     13  // Simulasi Radar LD2450
#define DHT_PIN     15  // Sensor Suhu
#define DHT_TYPE    DHT22
#define CO2_PIN     34  // Potensiometer 1 (Analog) - Simulasi SCD41
#define PWR_PIN     35  // Potensiometer 2 (Analog) - Simulasi PZEM
#define LDR_PIN     32  // LDR (Analog) - Simulasi TSL2591

#define RELAY_PIN   26  // Simulasi SSR Lampu
#define AC_LED_PIN  27  // Simulasi IR Blaster AC (PWM)
#define EXHAUST_PIN 14  // Simulasi Exhaust Fan (CO2)

// ================= OBJEK & VARIABEL =================
LiquidCrystal_I2C lcd(0x27, 16, 2);
DHT dht(DHT_PIN, DHT_TYPE);

// Variabel Timer Non-Blocking (RTOS Concept)
unsigned long previousMillis = 0;
const long interval = 1000; // Baca sensor setiap 1 detik

// Timer Simulasi Kelas Kosong (Dipercepat jadi 15 Detik untuk PoC)
unsigned long lastMotionTime = 0;
const unsigned long emptyTimeout = 15000; 
bool isOccupied = true;

void setup() {
  Serial.begin(115200);
  
  // Inisialisasi Pin
  pinMode(PIR_PIN, INPUT);
  // Pin Analog di ESP32 tidak perlu dideklarasikan pinMode, tapi aman dibiarkan
  
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(AC_LED_PIN, OUTPUT);
  pinMode(EXHAUST_PIN, OUTPUT);

  // Status Awal Aktuator (Menyala)
  digitalWrite(RELAY_PIN, HIGH);
  digitalWrite(AC_LED_PIN, LOW);
  digitalWrite(EXHAUST_PIN, LOW);

  // Inisialisasi LCD & DHT
  lcd.init();
  lcd.backlight();
  dht.begin();
  
  lcd.setCursor(0, 0);
  lcd.print("Smart Class Node");
  lcd.setCursor(0, 1);
  lcd.print("System Booting..");
  delay(2000); 
  lcd.clear();
}

void loop() {
  unsigned long currentMillis = millis();

  // 1. EVALUASI PRIORITAS 1: DETEKSI KEHADIRAN (PIR)
  int motionStatus = digitalRead(PIR_PIN);
  if (motionStatus == HIGH) {
    lastMotionTime = currentMillis; // Reset timer jika ada orang
    isOccupied = true;
  } else {
    // Jika tidak ada gerakan melebihi batas waktu
    if (currentMillis - lastMotionTime >= emptyTimeout) {
      isOccupied = false;
    }
  }

  // 2. AKUISISI DATA & EVALUASI (Tiap 1 Detik)
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // Membaca Sensor Lingkungan
    float temp = dht.readTemperature();
    
    // Konversi nilai Analog ADC ESP32 (0-4095) ke besaran simulasi riil
    int co2Raw = analogRead(CO2_PIN);
    int co2Level = map(co2Raw, 0, 4095, 400, 2000); // Simulasi 400 - 2000 ppm
    
    int ldrRaw = analogRead(LDR_PIN);
    int luxLevel = map(ldrRaw, 0, 4095, 0, 1000);   // Simulasi 0 - 1000 Lux

    int pwrRaw = analogRead(PWR_PIN);
    int pwrWatt = map(pwrRaw, 0, 4095, 0, 2000);    // Simulasi Beban 0 - 2000 Watt

    // ================= EDGE LOGIC (AKTUASI) =================
    if (isOccupied) {
      // --- KELAS ADA ORANG ---
      digitalWrite(RELAY_PIN, HIGH); // Lampu Kelas Menyala default
      
      // Logika Cahaya (Prioritas 3)
      if (luxLevel > 300) {
        digitalWrite(RELAY_PIN, LOW); // Matikan lampu jika siang terik
      }

      // Logika Termal (Prioritas 2)
      if (temp > 25.0) {
        analogWrite(AC_LED_PIN, 128); // Tembak IR AC On (LED Biru menyala setengah)
      } else {
        analogWrite(AC_LED_PIN, 0);   // AC Standby/Mati
      }

      // Logika Kualitas Udara (Prioritas 2)
      if (co2Level > 1000) {
        digitalWrite(EXHAUST_PIN, HIGH); // Nyalakan Exhaust Fan (LED Merah)
      } else {
        digitalWrite(EXHAUST_PIN, LOW);  // Matikan Exhaust Fan
      }
      
    } else {
      // --- KELAS KOSONG (Prioritas 1: Matikan Semua) ---
      digitalWrite(RELAY_PIN, LOW);    // Matikan Lampu (SSR)
      analogWrite(AC_LED_PIN, 0);      // Matikan AC
      digitalWrite(EXHAUST_PIN, LOW);  // Matikan Exhaust Fan
    }

    // ================= UPDATE DASHBOARD LOKAL (LCD) =================
    lcd.setCursor(0, 0);
    lcd.print("T:");
    lcd.print(temp, 1);
    lcd.print("C C:");
    lcd.print(co2Level);
    lcd.print("   "); // Padding agar angka tidak berbayang

    lcd.setCursor(0, 1);
    if (isOccupied) {
      lcd.print("STS:AKTIF W:");
    } else {
      lcd.print("STS:KOSNG W:");
    }
    lcd.print(pwrWatt);
    lcd.print("   ");
  }
}
