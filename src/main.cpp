/*
https://randomnerdtutorials.com/esp32-async-web-server-espasyncwebserver-library/
https://randomnerdtutorials.com/esp32-dht11-dht22-temperature-humidity-web-server-arduino-ide/
*/

#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoOTA.h>

#define Knob 7

const char* ssid     = "ESP32-Access-Point";
const char* password = "123456789";

IPAddress local_IP(192,168,4,22);
IPAddress gateway(192,168,4,9);
IPAddress subnet(255,255,255,0);

AsyncWebServer server(80);

const char* PARAM_INPUT_1 = "output";
const char* PARAM_INPUT_2 = "state";

int a = 0;

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>ESP Web Server</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" href="data:,">
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    h2 {font-size: 3.0rem;}
    p {font-size: 2.0rem;}
    body {max-width: 600px; margin:0px auto; padding-bottom: 25px;}
    .switch {position: relative; display: inline-block; width: 120px; height: 68px} 
    .switch input {display: none}
    .slider {position: absolute; top: 0; left: 0; right: 0; bottom: 0; background-color: #ccc; border-radius: 6px}
    .slider:before {position: absolute; content: ""; height: 52px; width: 52px; left: 8px; bottom: 8px; background-color: #fff; -webkit-transition: .4s; transition: .4s; border-radius: 3px}
    input:checked+.slider {background-color: #b30000}
    input:checked+.slider:before {-webkit-transform: translateX(52px); -ms-transform: translateX(52px); transform: translateX(52px)}
  </style>
</head>
<body>
  <h2>ESP Web Server</h2>
  %BUTTONPLACEHOLDER%
<script>function toggleCheckbox(element) {
  var xhr = new XMLHttpRequest();
  if(element.checked){ xhr.open("GET", "/update?output="+element.id+"&state=1", true); }
  else { xhr.open("GET", "/update?output="+element.id+"&state=0", true); }
  xhr.send();
}
</script>
<p>Limit switch: <span id="konz">%KONZ%</span></p>
</body>
<script>
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("konz").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/konz", true);
  xhttp.send();
}, 1000 ) ;
</script>
</html>
)rawliteral";

String Konz() {
  if (digitalRead(Knob) == HIGH) return "Not pressed";
  else return "Pressed";
}

String outputState(int output) {
  if(digitalRead(output)){
    return "checked";
  }
  else {
    return "";
  }
}

String processor(const String& var){
  if (var == "BUTTONPLACEHOLDER") {
    String buttons = "";
    buttons += "<h4>Button1</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"4\" " + outputState(4) + "><span class=\"slider\"></span></label>";
    buttons += "<h4>Button2</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"2\" " + outputState(2) + "><span class=\"slider\"></span></label>";
    return buttons;
  }
  if (var == "KONZ") {
    return Konz();
  }
  return String();
}

void setup() {
  Serial.begin(115200);
  pinMode(Knob, INPUT_PULLUP);

  WiFi.softAPConfig(local_IP, gateway, subnet);
  WiFi.softAP(ssid, password);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  server.on("/konz", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", Konz().c_str());
  });

  server.on("/update", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage1;
    String inputMessage2;
    if (request->hasParam(PARAM_INPUT_1) && request->hasParam(PARAM_INPUT_2)) {
      inputMessage1 = request->getParam(PARAM_INPUT_1)->value();
      inputMessage2 = request->getParam(PARAM_INPUT_2)->value();
      if (inputMessage1.toInt() == 4) {
        if (inputMessage2.toInt() == 0) {
          a = 1;
        }
        if (inputMessage2.toInt() == 1) {
          a = 2;
        }
      }
      if (inputMessage1.toInt() == 2) {
        if (inputMessage2.toInt() == 0) {
          a = 3;
        }
        if (inputMessage2.toInt() == 1) {
          a = 4;
        }
      }
    }
    else {
      inputMessage1 = "No message sent";
      inputMessage2 = "No message sent";
    }
    request->send(200, "text/plain", "OK");
  });

  server.begin();
  ArduinoOTA.begin();
}

void loop() {
  ArduinoOTA.handle();
}