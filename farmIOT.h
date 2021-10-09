#ifndef farmIOT_h
#define farmIOT_h

#include "Arduino.h"
#include "FirebaseArduino.h"
#include "ESP8266WiFi.h"

class farmIOT
{
  public:
  void farmiotKonfig();
  void kirimData(String path,String nilai);
  String getData(String path, String variabel);
  String getStatusTombol(String path);
  void sendReportPagi(String path,String jam,float nilai);
  void sendReportSiang(String path,String jam,float nilai);
  void sendReportMalam(String path,String jam,float nilai);
};
#endif
