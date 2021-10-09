#include "Arduino.h"
#include "farmIOT.h"
#include "FirebaseArduino.h"
#include "ESP8266WiFi.h"

void farmIOT::farmiotKonfig()
{
  const char *FIREBASE_HOST = "hidrophonikmonitoring.firebaseio.com";
  const char *FIREBASE_AUTH = "lVrMsQ7iKOpdn0sfJ7Dorv14B0Et9Pl4wiv4tTNv";
  Firebase.begin(FIREBASE_HOST,FIREBASE_AUTH);
}

void farmIOT::kirimData(String path,String nilai)
{
  Firebase.setString("Monitor/"+path,nilai);
  if(Firebase.failed()){
    Serial.println(Firebase.error());
    return;
  }
}
void farmIOT::sendReportPagi(String path,String jam,float nilai)
{
  Firebase.setFloat("Reporting/"+path+"/pagi/"+jam,nilai);
  if(Firebase.failed()){
    Serial.println(Firebase.error());
    return;
  }
}
void farmIOT::sendReportSiang(String path,String jam,float nilai)
{
  Firebase.setFloat("Reporting/"+path+"/siang/"+jam,nilai);
  if(Firebase.failed()){
    Serial.println(Firebase.error());
    return;
  }
}
void farmIOT::sendReportMalam(String path,String jam,float nilai)
{
  Firebase.setFloat("Reporting/"+path+"/malam/"+jam,nilai);
  if(Firebase.failed()){
    Serial.println(Firebase.error());
    return;
  }
}

String farmIOT::getData(String path,String variabel)
{
  String nilai = Firebase.getString("Monitor/"+path+"/"+variabel);
  return nilai;
}
String farmIOT::getStatusTombol(String path)
{
  String nilai = Firebase.getString("Monitor/"+path+"/power");
  return nilai;
}
