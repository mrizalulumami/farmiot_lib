#include <farmIOT.h>

//deklarasikan library yang akan digunakan
#include <ESP8266WiFi.h> //karena alat menggunakan koneksi wifi/hotspot maka include library ini
#include <WiFiClientSecure.h>
#include <FirebaseArduino.h> //karena alat menggunakan database firebase maka gunakan library ini
#include <EEPROM.h> //eeprom gunanya untuk menyimpan kondisi terakhir dari nilai sebuah variabel
#include <SoftwareSerial.h> //untuk penggunaan komunikasi serial
#include <DS3231.h>//karena alat menggunakan RTC untuk waktu realtime maka include library ini

farmIOT farm;
//ini berfungsi untuk mengambil data dari komunikasi serial monitor dari arduino nano ke esp8266
SoftwareSerial dataNutrisi(12, 14);

RTClib rtc; //deklarasi rtc

//kinfigurasi firebase
//#define FIREBASE_HOST "hidrophonikmonitoring.firebaseio.com" //deklarasikan variabel untuk menyimpan host dari database yg digunakan aplikasi
//#define FIREBASE_AUTH "lVrMsQ7iKOpdn0sfJ7Dorv14B0Et9Pl4wiv4tTNv" //deklarasikan variabel untuk menyimpan auth dari database yg digunakan



#ifndef STASSID
#define STASSID "share_virus"        // deklarasikan nama hotspot yang digunakan
#define STAPSK  "12345678"        // deklarasikan password hotspot yang digunakan
#endif

const char* ssid = STASSID;
const char* password = STAPSK;


#define SensorpH A0                 //Sensor pH pada pin A0

float teganganPh2 = 1.53;   //Nilai kalibrasi
float teganganPh1 = 1.35;    //Nilai kalibrasi
float Po;

#include <OneWire.h> //library untuk komunikasi 1 kabel
#include <DallasTemperature.h>      //Library sensor suhu air
#define ONE_WIRE_BUS 0              //Deklarasi pin komunikasi one wire
#define sensor 0                    //Sensor pada pin D3 untuk sensor suhu air
OneWire oneWire(ONE_WIRE_BUS);      //Kenalkan pin komunikasi one wire
DallasTemperature Suhu(&oneWire);   //Masukkan pada library

float tAir;               //Variabel penampung nilai suhu

int h = 1;

#define relay 16   //saklar mesin air
int relay2 = 12;  //relay untuk otomatis on off sensor PH
int relay3 = 14; //relay untuk otomatis on off sensor Nutrisi

void setup() {
  Serial.begin(115200);//Inisialisasi baudrate esp8266
  dataNutrisi.begin(9600);
  Suhu.begin();           //Mulai sensor suhu
  //deklarasi pin output
  pinMode(relay, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(relay3, OUTPUT);
  //deklarasi pin input
  pinMode(SensorpH, INPUT);

  EEPROM.begin(512);
  Wire.begin();

  //koneksi ke wifi
  Serial.println();
  Serial.print("connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  //koneksi ke firebase
  //Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  farm.farmiotKonfig();

  digitalWrite(relay, HIGH);
  digitalWrite(relay2, HIGH);
  digitalWrite(relay3, HIGH);
}

void loop() {
  DateTime now = rtc.now();
  //Serial.println(now.minute());
  getPower();
  //mengirimkan data secara bergantian selama 5 menit
  if (now.minute() >= 1 && now.minute() <= 5 || now.minute() >= 11 && now.minute() <= 15 || now.minute() >= 21 && now.minute() <= 25 || now.minute() >= 31
      && now.minute() <= 35 || now.minute() >= 41 && now.minute() <= 45 || now.minute() >= 51 && now.minute() <= 55) {
    kirimData(now.hour());
  } else {
    kirimDataNutrisi(now.hour());
  }

}

void getPower() {
  delay(5000);
  //  String nilainya = String(Firebase.getString("Monitor/-MgY1QcaTs0x5atzkYEw/power"));
  String nilainya = farm.getStatusTombol("-MgY1QcaTs0x5atzkYEw");
  if (nilainya == "1") {
    digitalWrite(relay, LOW);
    Serial.println("Mesin menyala");
  } else if (nilainya == "0") {
    digitalWrite(relay, HIGH);
    Serial.println("mesin mati");
  }
}
void kirimData(int jam) {
  delay(5000);

  //menentukan hari sesuai 24 jam setiap hari sampai hari ke7
  if (jam == 1) {
    h = h + 1;
  }
  if (jam >= 7) {
    h = 1;
  }
  digitalWrite(relay2, HIGH);
  digitalWrite(relay3, LOW);
  int sensorValue = analogRead(SensorpH);         //Baca Sensor
  float voltage1 = sensorValue * (5.0 / 1023.0);   //Konversi menjadi nilai tegangan
  //Rumus mencari pH
  Po = 6.86 + ((teganganPh2 - voltage1) / ((teganganPh1 - teganganPh2) / 3));;
  float PH = (6.4516 * voltage1) - 5.7742;

  Suhu.requestTemperatures();       //Baca suhu
  tAir =  Suhu.getTempCByIndex(0);  //Masukkan pada variabel

  Serial.print("PH = ");
  Serial.println(voltage1);

  String Stringsuhu = String(tAir);
  String Stringph = String(Po);
  // handle error
  if (!Firebase.failed()) {

    //Firebase.setString("Monitor/-MTdXxNfIKyLtlUf9QPJ/valuenya", Stringph);
    farm.kirimData("-MTdXxNfIKyLtlUf9QPJ/valuenya", Stringph);
    //    Firebase.setString("Monitor/-MgY0ebMR61zTncMF9XB/valuenya", Stringsuhu);
    farm.kirimData("-MgY0ebMR61zTncMF9XB/valuenya", Stringsuhu);
    if (jam > 0 && jam <= 11) {
      //Firebase.setFloat("Reporting/-MTdXxNfIKyLtlUf9QPJ/pagi/h" + String(h), Po);
      farm.sendReportPagi("-MTdXxNfIKyLtlUf9QPJ",String(h),Po);
      //Firebase.setFloat("Reporting/-MgY0ebMR61zTncMF9XB/pagi/h" + String(h), tAir);
      farm.sendReportPagi("-MgY0ebMR61zTncMF9XB",String(h),tAir);
    } else if (jam > 11 && jam <= 14) {
      //Firebase.setFloat("Reporting/-MTdXxNfIKyLtlUf9QPJ/siang/h" + String(h), Po);
      farm.sendReportSiang("-MTdXxNfIKyLtlUf9QPJ",String(h),Po);
      //Firebase.setFloat("Reporting/-MgY0ebMR61zTncMF9XB/siang/h" + String(h), tAir);
      farm.sendReportSiang("-MgY0ebMR61zTncMF9XB",String(h),tAir);
    } else if (jam > 14 && jam <= 23) {
      //Firebase.setFloat("Reporting/-MTdXxNfIKyLtlUf9QPJ/malam/h" + String(h), Po);
      farm.sendReportMalam("-MTdXxNfIKyLtlUf9QPJ",String(h),Po);
      //Firebase.setFloat("Reporting/-MgY0ebMR61zTncMF9XB/malam/h" + String(h), tAir);
      farm.sendReportMalam("-MgY0ebMR61zTncMF9XB",String(h),tAir);
    }
    Serial.print("Berhasil Mengirim");
    //Serial.println(Firebase.error());
    return;
  } else {
    Serial.print("Gagal Mengirim");
    Serial.println(Firebase.error());
    return;
  }
}
void kirimDataNutrisi(int jam) {
  if (jam == 1) {
    h = h + 1;
  }
  if (jam >= 7) {
    h = 1;
  }
  delay(5000);
  digitalWrite(relay2, LOW);
  digitalWrite(relay3, HIGH);
  int sensorValue = analogRead(SensorpH);         //Baca Sensor
  float nutrisi = 0.3417 * sensorValue + 281.08;

  Serial.print("Nutrisi = ");
  Serial.print(nutrisi);
  Serial.println("PPM");

  String StringNutrisi = String(nutrisi);
  // handle error
  if (!Firebase.failed()) {

    //Firebase.setString("Monitor/-MinEbszCBlFfeFUS39V/valuenya", StringNutrisi);
    farm.kirimData("-MinEbszCBlFfeFUS39V/valuenya", StringNutrisi);
    if (jam > 0 && jam <= 11) {
      Firebase.setFloat("Reporting/-MinEbszCBlFfeFUS39V/pagi/h" + String(h), nutrisi);
    } else if (jam > 11 && jam <= 14) {
      Firebase.setFloat("Reporting/-MinEbszCBlFfeFUS39V/siang/h" + String(h), nutrisi);
    } else if (jam > 14 && jam <= 23) {
      Firebase.setFloat("Reporting/-MinEbszCBlFfeFUS39V/malam/h" + String(h), nutrisi);
    }
    Serial.print("Berhasil Mengirim");
    // Serial.println(Firebase.error());
    return;
  } else {
    Serial.print("Gagal Mengirim");
    Serial.println(Firebase.error());
    return;
  }
}
