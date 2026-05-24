# SMART-ENERGY-TEKRA
Purwarupa IoT Smart Energy &amp; Environment Monitoring berbasis Edge Computing untuk efisiensi fasilitas Universitas Brawijaya

# Smart Energy & Environment Monitoring (SITL Simulation)
Repository ini berisi *source code* simulasi *Software-in-the-Loop* (SITL) untuk sistem IoT pemantauan energi dan lingkungan kelas. 

## Deskripsi Arsitektur Simulasi
Desain *hardware* asli dari purwarupa ini menggunakan arsitektur *dual-processor* (STM32F411 sebagai Edge Logic dan ESP8266 sebagai Network Coprocessor) serta sensor kelas industri (Radar LD2450, SENSIRION SCD41 NDIR). 

Mengingat limitasi platform simulasi virtual web, kami melakukan penyesuaian (*mapping*) komponen untuk keperluan uji coba (*Proof of Concept*) logika *Edge Computing* menggunakan platform Wokwi.
 
## Pemetaan Sensor (Wokwi Map)
* **Otak Pemrosesan:** Digabung menggunakan 1 buah ESP32.
* **Radar LD2450:** Disubstitusi dengan PIR Sensor.
* **SENSIRION SCD41:** Disubstitusi dengan DHT22 (Suhu/Lembap) + Slide Potentiometer (Simulasi fluktuasi gas CO2).
* **TSL2591:** Disubstitusi dengan LDR (Photoresistor).
* **PZEM-004T:** Disubstitusi dengan Slide Potentiometer (Simulasi fluktuasi beban listrik AC).

## Alur Kerja State Machine
Kode ini memuat logika utama efisiensi:
1. Membaca sensor lingkungan dan gerak secara *asynchronous*.
2. Memverifikasi toleransi waktu batas kelas kosong (15 menit pada kondisi asli, diatur 15 detik pada simulasi untuk mempercepat pembuktian).
3. Melakukan eksekusi pemutusan daya lokal (Relay Lampu & Simulasi Sinyal AC).
4. Mengirim *payload* metrik ke MQTT Broker dalam format JSON yang sangat ringan.

## Cara Menjalankan Simulasi
1. Buka Wokwi (wokwi.com).
2. Buat proyek ESP32 baru.
3. Rangkai komponen virtual sesuai pemetaan pin pada `main.ino`.
4. *Paste* kode dari `main.ino` ke editor.
5. Tambahkan library `PubSubClient`, `DHT sensor library`, dan `ArduinoJson` pada *Library Manager* Wokwi.
6. Jalankan simulasi dan pantau pengiriman data JSON pada Serial Monitor.
