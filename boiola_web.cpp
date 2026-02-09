#include "boiola_web.h"

static AsyncWebServer server(80);
static DNSServer dnsServer;
static bool serverRunning = false;
static char currentFilename[64] = "";

// recibe el nombre del archivo qque el gps está usando
void web_set_filename(const char* filename) {
  if (filename) {
    strncpy(currentFilename, filename, sizeof(currentFilename) - 1);
    currentFilename[sizeof(currentFilename) - 1] = '\0';
    // currentFilename ahora tiene el nombre del archivo pasado.
  }
}

// está corriendo el server?
bool web_is_running() {
  return serverRunning;
}

// configs que reciomienda la ia por ahora no hay nada
bool web_init() {
return true;
}


bool web_start() {
  if (serverRunning) return false;

  //  WiFi AP
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASS, AP_CHANNEL, AP_HIDDEN, AP_MAX_CONNECTIONS);
  
  unsigned long startTime = millis();
  while (WiFi.softAPIP() == IPAddress(0, 0, 0, 0)) {
    if (millis() - startTime > 5000) { 
      return false;  // Failed to start
    }
    delay(10);
  } 
  IPAddress IP = WiFi.softAPIP(); // obtiene AP IP 
  
  dnsServer.start(DNS_PORT, "*", IP); // DNS server captive portal

  
  if (!MDNS.begin("boiola-tool")) { // activa boiola-tool.local
  }
  
  web_setup_routes();
  
  server.begin();
  
  serverRunning = true;
  return true;
}


void web_setup_routes() {
  
  // Root page - shows list of GPX files
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    String html = web_generate_file_list();
    request->send(200, "text/html", html);
  });
  
  // Download individual file
  server.on("/download", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("file")) {
      String filename = request->getParam("file")->value();
      
      // Security: prevent directory traversal attacks
      if (filename.indexOf("..") >= 0 || filename.indexOf("/") >= 0) {
        request->send(403, "text/plain", "Forbidden");
        return;
      }
      
      // Check if it's the current active file
      String fullPath = "/gpx/" + filename;
      if (strcmp(fullPath.c_str(), currentFilename) == 0) {
        request->send(423, "text/plain", "File currently in use");
        return;
      }
      
      // Send the file
      request->send(SD, fullPath, "application/gpx+xml", true);
    } else {
      request->send(400, "text/plain", "Missing file parameter");
    }
  });
  
  // Catch-all route for captive portal
  // Redirects everything else to root
  server.onNotFound([](AsyncWebServerRequest *request) {
    request->redirect("/");
  });
}


String web_generate_file_list() {
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<title>BOIOLA-TOOL</title>";
  html += "</head><body>";
  html += "<h1>BOIOLA-TOOL</h1>";
  html += "<h5>Archivos:</h5>";
  
  File root = SD.open("/gpx");
  if (!root || !root.isDirectory()) {
    html += "<p>No GPX files found or /gpx directory missing</p>";
  } else {
    html += "<ul style=\"font-size: 20px;\">";
    
    bool hasFiles = false;
    File file = root.openNextFile();
    
    while (file) {
      if (!file.isDirectory() && String(file.name()).endsWith(".gpx")) {
        hasFiles = true;
        String fileName = String(file.name());
        String fullPath = String("/gpx/") + fileName;
        bool isActive = (strcmp(fullPath.c_str(), currentFilename) == 0);
        
        html += "<li style=\"padding: 10px 0;\">";
        
        if (isActive) {
          html += "[Currently logging] " + fileName;
        } else {
          html += "<a href='/download?file=" + fileName + "'>" + fileName + "</a>";
        }
        
        html += " (" + String(file.size()) + " bytes)";
        html += "</li>";
      }
      file = root.openNextFile();
    }
    
    if (!hasFiles) {
      html += "<p>No GPX files found</p>";
    }
    
    html += "</ul>";
  }
  
  html += "</body></html>";
  return html;
}

void web_stop() {
  if (!serverRunning) return;
  server.end();
  dnsServer.stop();
  MDNS.end();
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_OFF);
  serverRunning = false;
}
