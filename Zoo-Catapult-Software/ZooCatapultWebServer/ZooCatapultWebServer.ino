#include <WiFi.h>
#include <WebServer.h>

const char* ap_ssid = "ZooCatapult-Remote";
const char* ap_password = "memphis123"; 

WebServer server(80);

void setup() {
  Serial.begin(9600); 
  WiFi.softAP(ap_ssid, ap_password);
  
  server.on("/", []() {
    String html = "<html><head><meta name='viewport' content='width=device-width, initial-scale=1.0'>";
    html += "<style>";
    // --- THEME VARIABLES ---
    html += ":root { --bg: #0f172a; --card: #1e293b; --text: #f8fafc; --sub: #94a3b8; --border: #334155; }";
    html += "[data-theme='light'] { --bg: #f1f5f9; --card: #ffffff; --text: #0f172a; --sub: #64748b; --border: #cbd5e1; }";

    html += "body { font-family: 'Segoe UI', sans-serif; text-align: center; background: var(--bg); color: var(--text); margin: 0; padding: 20px; transition: 0.3s; }";
    html += ".card { background: var(--card); border-radius: 20px; padding: 24px; margin: 15px auto; max-width: 450px; border: 1px solid var(--border); box-shadow: 0 4px 6px -1px rgba(0,0,0,0.1); }";
    
    // Theme Toggle Button Styling
    html += ".theme-btn { background: var(--border); color: var(--text); border: none; padding: 8px 16px; border-radius: 8px; cursor: pointer; margin-bottom: 10px; font-weight: 600; font-size: 12px; }";

    html += ".status-badge { display: inline-block; padding: 6px 16px; border-radius: 99px; font-size: 13px; font-weight: 600; background: var(--border); color: var(--sub); margin-bottom: 20px; text-transform: uppercase; }";
    
    html += ".mode-switch { background: var(--bg); padding: 10px; border-radius: 12px; margin-bottom: 20px; display: flex; justify-content: space-around; border: 1px solid var(--border); }";
    html += ".mode-btn { padding: 8px 20px; border-radius: 8px; cursor: pointer; font-size: 14px; font-weight: bold; border: none; background: transparent; color: var(--sub); }";
    html += ".mode-active { background: #38bdf8; color: white; }";

    html += ".slider-container { position: relative; width: 100%; height: 60px; background: var(--bg); border-radius: 30px; margin: 20px 0; overflow: hidden; border: 1px solid var(--border); }";
    html += ".slider-text { position: absolute; width: 100%; top: 50%; transform: translateY(-50%); font-weight: bold; color: var(--sub); pointer-events: none; z-index: 1; }";
    html += "#safety-slider { -webkit-appearance: none; width: 100%; height: 100%; background: transparent; cursor: pointer; outline: none; position: relative; z-index: 2; }";
    html += "#safety-slider::-webkit-slider-thumb { -webkit-appearance: none; width: 60px; height: 60px; background: #10b981; border-radius: 50%; border: 4px solid var(--card); }";

    html += "#log-window { height: 150px; background: #020617; border-radius: 12px; padding: 15px; text-align: left; overflow-y: auto; font-family: monospace; font-size: 12px; color: #34d399; border: 1px solid var(--border); }";
    html += ".stop { background: #ef4444; color: white; width: 100%; padding: 18px; margin: 10px 0; font-size: 16px; border: none; border-radius: 12px; font-weight: 700; cursor: pointer; }";
    html += ".reset { background: #64748b; color: white; display: none; width: 100%; padding: 18px; margin: 10px 0; font-size: 16px; border: none; border-radius: 12px; font-weight: 700; cursor: pointer; }";
    html += "</style></head><body>";

    // --- ADDED THEME BUTTON ---
    html += "<button class='theme-btn' onclick='toggleTheme()'>SWITCH THEME</button>";
    html += "<h1>ZOO CATAPULT <small>v3.4</small></h1>";
    
    html += "<div class='card'>";
    html += "<div id='status' class='status-badge'>SYSTEM READY</div>";
    
    html += "<div class='mode-switch'>";
    html += "<button id='btn-manual' class='mode-btn mode-active' onclick='setMode(\"manual\")'>MANUAL</button>";
    html += "<button id='btn-auto' class='mode-btn' onclick='setMode(\"auto\")'>AUTOMATIC</button>";
    html += "</div>";

    html += "<div id='auto-info' style='display:none; color:#38bdf8; margin-bottom:10px; font-weight:bold;'>NEXT LAUNCH IN: <span id='timer'>30</span>s</div>";

    html += "<div class='slider-container' id='slider-box'>";
    html += "<div class='slider-text' id='slider-label'>SLIDE TO LAUNCH >>></div>";
    html += "<input type='range' min='0' max='100' value='0' id='safety-slider' oninput='checkSlider(this)' onchange='resetSlider(this)'>";
    html += "</div>";

    html += "<button class='btn stop' onclick=\"cmd('/stop', 'E-STOP TRIGGERED', '#ef4444')\">EMERGENCY STOP</button>";
    html += "<button id='reset-btn' class='btn reset' onclick=\"cmd('/reset', 'SYSTEM RESET', '#334155')\">RESET & UNLOCK</button>";
    html += "</div>";

    html += "<div class='card' style='padding:15px;'><div id='log-window'></div></div>";

    html += "<script>";
    // --- ADDED THEME JAVASCRIPT ---
    html += "function toggleTheme() {";
    html += "  const root = document.documentElement;";
    html += "  if(root.getAttribute('data-theme') === 'light') root.setAttribute('data-theme', 'dark');";
    html += "  else root.setAttribute('data-theme', 'light');";
    html += "}";

    html += "let launchTimer; let autoInterval; let currentMode = 'manual'; let timeLeft = 30;";

    html += "function log(msg) {";
    html += "  const win = document.getElementById('log-window');";
    html += "  win.innerHTML = \"<div><span style='color:#64748b'>[\" + new Date().toLocaleTimeString() + \"]</span> \" + msg + \"</div>\" + win.innerHTML;";
    html += "}";

    html += "function setMode(mode) {";
    html += "  currentMode = mode;";
    html += "  document.getElementById('btn-manual').classList.toggle('mode-active', mode==='manual');";
    html += "  document.getElementById('btn-auto').classList.toggle('mode-active', mode==='auto');";
    html += "  document.getElementById('auto-info').style.display = (mode==='auto' ? 'block' : 'none');";
    html += "  document.getElementById('slider-box').style.display = (mode==='manual' ? 'block' : 'none');";
    html += "  if(mode === 'auto') startAutoTimer(); else clearInterval(autoInterval);";
    html += "  log('MODE CHANGED TO ' + mode.toUpperCase());";
    html += "}";

    html += "function startAutoTimer() {";
    html += "  clearInterval(autoInterval); timeLeft = 30;";
    html += "  autoInterval = setInterval(() => {";
    html += "    timeLeft--; document.getElementById('timer').innerText = timeLeft;";
    html += "    if(timeLeft <= 0) { clearInterval(autoInterval); executeLaunch(); }";
    html += "  }, 1000);";
    html += "}";

    html += "function cmd(path, msg, color) {";
    html += "  fetch(path); log(msg);";
    html += "  document.getElementById('status').innerText = msg;";
    html += "  document.getElementById('status').style.background = color;";
    html += "  if(path === '/stop') { clearInterval(launchTimer); clearInterval(autoInterval); lockUI(); }";
    html += "  if(path === '/reset') { unlockUI(); }";
    html += "}";

    html += "function lockUI() {";
    html += "  document.getElementById('safety-slider').disabled = true;";
    html += "  document.getElementById('slider-box').style.opacity = '0.3';";
    html += "  document.getElementById('reset-btn').style.display = 'block';";
    html += "}";

    html += "function unlockUI() {";
    html += "  document.getElementById('safety-slider').disabled = false;";
    html += "  document.getElementById('safety-slider').value = 0;";
    html += "  document.getElementById('slider-box').style.opacity = '1';";
    html += "  document.getElementById('reset-btn').style.display = 'none';";
    html += "  document.getElementById('status').innerText = 'SYSTEM READY';";
    html += "  document.getElementById('status').style.background = 'var(--border)';";
    html += "  if(currentMode === 'auto') startAutoTimer();";
    html += "}";

    html += "function executeLaunch() {";
    html += "  cmd('/go', 'LAUNCHING...', '#10b981');";
    html += "  setTimeout(() => {"; // Simulate logic return
    html += "    log('SUCCESS: LAUNCH COMPLETE');";
    html += "    if(currentMode === 'auto') startAutoTimer();";
    html += "    else { unlockUI(); }";
    html += "  }, 13000);"; // Total sequence length is ~13-15 seconds
    html += "}";

    html += "function checkSlider(el) { if(el.value == 100) { el.disabled = true; executeLaunch(); } }";
    html += "function resetSlider(el) { if(el.value < 100) el.value = 0; }";
    html += "</script></body></html>";

    server.send(200, "text/html", html);
  });

  server.on("/go", []() { Serial.print("G"); server.send(200, "text/plain", "OK"); });
  server.on("/stop", []() { Serial.print("S"); server.send(200, "text/plain", "OK"); });
  server.on("/reset", []() { Serial.print("R"); server.send(200, "text/plain", "OK"); });

  server.begin();
}

void loop() { server.handleClient(); }