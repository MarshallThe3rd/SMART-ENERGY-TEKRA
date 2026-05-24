# SMART-ENERGY-TEKRA
Sistem IoT Pemantauan Energi dan Lingkungan untuk Menghemat Listrik di Universitas Brawijaya.

## Catatan Penting
Repositori ini berisi kode simulasi menggunakan web Wokwi. Ada sedikit perbedaan komponen antara alat asli yang ditulis di proposal dengan versi simulasi ini.

Pada proposal, kami menggunakan dua otak pemroses (STM32F411 dan ESP8266) serta sensor industri asli seperti Radar LD2450. Karena keterbatasan di web simulator, simulasi ini disederhanakan menggunakan satu buah ESP32 dan sensor standar (PIR, DHT22, LDR, dan Potensiometer). 

Penyederhanaan ini hanya untuk membuktikan bahwa logika kode otomatisasi ruangan dan pengiriman data ke server berjalan dengan baik.

## Pemetaan Komponen Simulasi (Wokwi)
* Otak Sistem: ESP32
* Sensor Gerak (Radar LD2450): Diganti dengan PIR Sensor
* Sensor Suhu & CO2 (SCD41): Diganti dengan DHT22 (Suhu/Lembap) dan Potensiometer (Simulasi nilai CO2)
* Sensor Cahaya (TSL2591): Diganti dengan LDR
* Sensor Listrik Daya (PZEM-004T): Diganti dengan Potensiometer (Simulasi nilai Watt AC)

## Alur Kerja Sistem
Kode ini memuat logika utama untuk efisiensi fasilitas:
1. Membaca data sensor gerak dan lingkungan secara bersamaan.
2. Mengecek apakah kelas benar-benar kosong. (Waktu tunggu aslinya 15 menit, tetapi di simulasi ini dipercepat menjadi 15 detik agar mudah diuji).
3. Mematikan relay lampu dan memberi sinyal mematikan AC secara otomatis jika ruangan kosong.
4. Mengirim data kondisi ruangan ke server (MQTT Broker) dalam bentuk teks JSON.

## Format Data (JSON):

```json
{
  "ruangan_kosong": true,
  "suhu": 24.5,
  "co2": 800,
  "lux": 150,
  "daya_watt": 0,
  "lampu_on": false,
  "ac_on": false
}
