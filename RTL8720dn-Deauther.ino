#include "vector"
#include "map"
#include "wifi_conf.h"
#include "wifi_cust_tx.h"
#include "wifi_util.h"
#include "wifi_structures.h"
#include "debug.h"
#include "WiFi.h"
#include "WiFiServer.h"
#include "WiFiClient.h"

/********************************
 * LED Indicators
 * - Red: System active
 * - Green: Server communication
 * - Blue: Deauth frames sent
 *******************************/
#define FRAMES_PER_DEAUTH 5

// LED Pins
#define LED_SYSTEM LED_R
#define LED_SERVER LED_G
#define LED_ATTACK LED_B

/********************************
 * WiFi Network Scan Result Structure
 ********************************/
typedef struct {
    String ssid;
    String bssid_str;
    uint8_t bssid[6];
    short rssi;
    uint8_t channel;
} WiFiScanResult;

/********************************
 * Global Variables
 ********************************/
const char *AP_SSID = "RTL8720dn-Deauther";
const char *AP_PASS = "0123456789";
int current_channel = 1;
uint8_t deauth_bssid[6];
uint16_t deauth_reason = 2;

WiFiServer webServer(80);
std::vector<WiFiScanResult> scanResults;
std::vector<int> deauthTargets;

/********************************
 * WiFi Scan Result Handler
 ********************************/
rtw_result_t handleScanResult(rtw_scan_handler_result_t *scan_result) {
    if (scan_result->scan_complete == 0) {
        rtw_scan_result_t *record = &scan_result->ap_details;
        record->SSID.val[record->SSID.len] = 0;

        WiFiScanResult result;
        result.ssid = String((const char *)record->SSID.val);
        result.channel = record->channel;
        result.rssi = record->signal_strength;
        memcpy(&result.bssid, &record->BSSID, 6);
        char bssid_str[] = "XX:XX:XX:XX:XX:XX";
        snprintf(bssid_str, sizeof(bssid_str), "%02X:%02X:%02X:%02X:%02X:%02X",
                 result.bssid[0], result.bssid[1], result.bssid[2],
                 result.bssid[3], result.bssid[4], result.bssid[5]);
        result.bssid_str = bssid_str;

        scanResults.push_back(result);
    }
    return RTW_SUCCESS;
}

/********************************
 * WiFi Network Scanner
 ********************************/
bool scanWiFiNetworks() {
    DEBUG_SER_PRINT("Scanning WiFi networks (5s)...\n");
    scanResults.clear();

    if (wifi_scan_networks(handleScanResult, NULL) == RTW_SUCCESS) {
        delay(5000); // Wait for scan to complete
        DEBUG_SER_PRINT("Scan complete!\n");
        return true;
    } else {
        DEBUG_SER_PRINT("Scan failed!\n");
        return false;
    }
}

/********************************
 * HTTP Response Helpers
 ********************************/
String buildResponseHeader(int code, String contentType) {
    return "HTTP/1.1 " + String(code) + " OK\n" +
           "Content-Type: " + contentType + "\n" +
           "Connection: close\n\n";
}

String buildRedirectResponse(String url) {
    return "HTTP/1.1 307 Temporary Redirect\n" +
           "Location: " + url + "\n";
}

/********************************
 * Request Parsing Functions
 ********************************/
String extractPathFromRequest(const String &request) {
    int start = request.indexOf(' ') + 1;
    int end = request.indexOf(' ', start);
    return request.substring(start, end);
}

std::vector<std::pair<String, String>> parsePOSTData(String &request) {
    std::vector<std::pair<String, String>> params;

    int bodyStart = request.indexOf("\r\n\r\n");
    if (bodyStart == -1) return params; // No body found
    String postData = request.substring(bodyStart + 4);

    int start = 0;
    int end = postData.indexOf('&');
    while (end != -1) {
        String keyValue = postData.substring(start, end);
        int delimiter = keyValue.indexOf('=');
        if (delimiter != -1) {
            params.push_back({keyValue.substring(0, delimiter), keyValue.substring(delimiter + 1)});
        }
        start = end + 1;
        end = postData.indexOf('&', start);
    }

    // Last parameter
    String lastPair = postData.substring(start);
    int delimiter = lastPair.indexOf('=');
    if (delimiter != -1) {
        params.push_back({lastPair.substring(0, delimiter), lastPair.substring(delimiter + 1)});
    }

    return params;
}

/********************************
 * Web Server Handlers
 ********************************/
void handleRoot(WiFiClient &client) {
    String response = buildResponseHeader(200, "text/html") +
                      "<!DOCTYPE html><html><head><title>RTL8720dn-Deauther</title></head>";
    response += "<body><h1>WiFi Deauther</h1>";
    response += "<p>Use this tool to scan and deauth WiFi networks.</p>";
    response += "</body></html>";
    client.write(response.c_str());
}

void handleRescan(WiFiClient &client) {
    client.write(buildRedirectResponse("/").c_str());
    scanWiFiNetworks();
}

void handleDeauth(WiFiClient &client, String &request) {
    auto postData = parsePOSTData(request);
    for (auto &param : postData) {
        if (param.first == "network") {
            deauthTargets.push_back(param.second.toInt());
        } else if (param.first == "reason") {
            deauth_reason = param.second.toInt();
        }
    }
}

/********************************
 * Main Functions
 ********************************/
void setup() {
    pinMode(LED_SYSTEM, OUTPUT);
    pinMode(LED_SERVER, OUTPUT);
    pinMode(LED_ATTACK, OUTPUT);

    DEBUG_SER_INIT();
    WiFi.apbegin(AP_SSID, AP_PASS, (char *)String(current_channel).c_str());
    scanWiFiNetworks();

    webServer.begin();
    digitalWrite(LED_SYSTEM, HIGH);
}

void loop() {
    WiFiClient client = webServer.available();
    if (client.connected()) {
        digitalWrite(LED_SERVER, HIGH);
        String request = "";
        while (client.available()) request += (char)client.read();
        String path = extractPathFromRequest(request);

        if (path == "/") handleRoot(client);
        else if (path == "/rescan") handleRescan(client);
        else if (path == "/deauth") handleDeauth(client, request);
        else client.write(buildResponseHeader(404, "text/plain") + "Not Found!");

        client.stop();
        digitalWrite(LED_SERVER, LOW);
    }
}
