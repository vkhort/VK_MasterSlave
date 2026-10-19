#pragma once
#include <Arduino.h>

// ====================================================================
// === ЧАСТЬ 1: ОБЩИЕ СТИЛИ И ШАПКА СЕТЕВОЙ СТРАНИЦЫ УПРАВЛЕНИЯ     ===
// ====================================================================
const char HTML_STYLE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>VK Center Hub</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body {
            font-family: 'Segoe UI', sans-serif;
            background: linear-gradient(135deg, #111827 0%, #1f2937 100%);
            min-height: 100vh;
            color: white;
            padding: 20px;
        }
        .container {
            max-width: 550px;
            margin: 0 auto;
            background: rgba(31, 41, 55, 0.95);
            border-radius: 20px;
            padding: 25px;
            box-shadow: 0 10px 30px rgba(0,0,0,0.5);
        }
        h1 { text-align: center; margin-bottom: 5px; color: #3b82f6; font-size: 2em; }
        .subtitle { text-align: center; color: #9ca3af; margin-bottom: 25px; font-size: 0.9em; }
        .section-title { color: #3b82f6; border-bottom: 1px solid #3b82f6; padding-bottom: 5px; margin: 25px 0 15px 0; font-size: 1.1em; font-weight: bold; }
        .form-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 12px; margin-bottom: 10px; }
        label { display: block; margin-bottom: 4px; color: #9ca3af; font-size: 0.85em; }
        input[type="text"] { 
            width: 100%; padding: 10px; border-radius: 8px; border: 1px solid #3b82f6; 
            background: #111827; color: white; font-size: 0.95em; outline: none; 
        }
        .btn { background: #3b82f6; color: white; border: none; padding: 12px 15px; border-radius: 8px; cursor: pointer; font-size: 1em; transition: all 0.2s; font-weight: bold; }
        .btn:hover { background: #2563eb; }
        .btn-wide { width: 100%; margin-top: 20px; grid-column: span 2; }
        .btn-secondary { background: #1f2937; border: 1px solid #3b82f6; }
        .btn-secondary:hover { background: #374151; }
        
        .remote-ui { display: flex; flex-direction: column; gap: 15px; margin: 15px 0; }
        .control-row { display: flex; justify-content: center; align-items: center; gap: 15px; }
        
        .btn-circle { 
            width: 60px; height: 60px; border-radius: 50%; border: 2px solid #3b82f6; 
            background: #111827; color: white; font-size: 1.6em; cursor: pointer; 
            display: flex; justify-content: center; align-items: center; transition: all 0.2s; 
        }
        .btn-circle:hover { background: #3b82f6; transform: scale(1.05); }
        .btn-circle.stop { border-color: #ef4444; font-size: 1em; font-weight: bold; color: #ef4444; }
        .btn-circle.stop:hover { background: #ef4444; color: white; }
        
        .status { margin-top: 15px; padding: 10px; border-radius: 8px; text-align: center; font-size: 0.9em; display: none; }
        .status.success { background: rgba(59, 130, 246, 0.2); color: #3b82f6; display: block; }
        .status.error { background: rgba(239, 68, 68, 0.2); color: #ef4444; display: block; }
        .status.loading { background: rgba(245, 158, 11, 0.2); color: #f59e0b; display: block; }
        .info { background: #111827; padding: 10px; border-radius: 8px; margin-top: 20px; font-size: 0.8em; text-align: center; color: #9ca3af; }
    </style>
</head>
<body>
    <div class="container">
        <h1>🚀 VK MASTER OTA HUB</h1>
)rawliteral";

// ====================================================================
// === ЧАСТЬ 2: ТЕЛО СТРАНИЦЫ С НАСТРОЙКАМИ И КНОПКАМИ УПРАВЛЕНИЯ   ===
// ====================================================================
const char HTML_BODY[] PROGMEM = R"rawliteral(
        <div class="subtitle">Центральный пульт распределенного управления</div>
        
        <!-- СЕКЦИЯ ДИСТАНЦИОННОГО ПУЛЬТА СВЯЗАНА С ТЕКУЩИМ ИСПОЛНИТЕЛЕМ (vk_stepper1.local) -->
        <div class="section-title">📡 Remote Control (Transmit via mDNS)</div>
        <div class="remote-ui">
            <label style="text-align: center; margin-bottom: -5px;">Stepper 1 Manual Control</label>
            <div class="control-row">
                <button type="button" class="btn-circle" onclick="sendRemoteCmd('stepper', 'ccw')">↺</button>
                <button type="button" class="btn-circle stop" onclick="sendRemoteCmd('stepper', 'stop')">STOP</button>
                <button type="button" class="btn-circle" onclick="sendRemoteCmd('stepper', 'cw')">↻</button>
            </div>
            
            <label style="text-align: center; margin-bottom: 2px; margin-top: 10px;">Servo 1 Manual Control</label>
            <div class="control-row">
                <button type="button" class="btn" onclick="sendRemoteCmd('servo', '0')">Angle 0°</button>
                <button type="button" class="btn btn-secondary" onclick="sendRemoteCmd('servo', '90')">Angle 90°</button>
                <button type="button" class="btn" onclick="sendRemoteCmd('servo', '180')">Angle 180°</button>
            </div>
        </div>

        <!-- ✅ СЕКЦИЯ НАСТРОЕК ХАБА (ОСТАЛИСЬ ТОЛЬКО SSID И PASS) -->
        <form id="configForm" onsubmit="return false;">
            <div class="section-title">⚙️ Hub Access Point Settings</div>
            <div class="form-grid">
                <div class="form-group">
                    <label>Hub WiFi SSID</label>
                    <input type="text" id="ap_ssid" value="%AP_SSID_VAL%">
                </div>
                <div class="form-group">
                    <label>Hub WiFi Password</label>
                    <input type="text" id="ap_pass" value="%AP_PASS_VAL%">
                </div>
                <button type="button" class="btn btn-wide" onclick="saveHubConfig()">💾 Save Settings & Reboot</button>
            </div>
        </form>
        
        <div id="status" class="status"></div>
        <div class="info">📡 Адресация исполнителей жестко зафиксирована на vk_stepper1.local .. vk_stepper4.local</div>
    </div>
)rawliteral";

// ====================================================================
// === ЧАСТЬ 3: СКРИПТЫ ВЗАИМОДЕЙСТВИЯ (AJAX / REST API КОМАНДЫ)     ===
// ====================================================================
const char HTML_SCRIPT[] PROGMEM = R"rawliteral(
    <script>
        function showStatus(msg, type) {
            const statusDiv = document.getElementById('status');
            statusDiv.textContent = msg; 
            statusDiv.className = 'status ' + type;
            setTimeout(() => { if (statusDiv.className === 'status ' + type) statusDiv.className = 'status'; }, 3000);
        }

        async function sendRemoteCmd(device, targetAction) {
            try {
                let url = `/api/forward?dev=${device}&action=${targetAction}`;
                const response = await fetch(url, { method: 'GET' });
                if (!response.ok) console.error('Forwarding error');
            } catch(e) { console.error('Communication error:', e); }
        }

        // ✅ ИСПРАВЛЕНО: Скрипт собирает только 2 параметра для отправки на Хаб
        async function saveHubConfig() {
            showStatus('Saving configuration...', 'loading');
            
            const formData = new URLSearchParams();
            formData.append('ap_ssid', document.getElementById('ap_ssid').value.trim()); 
            formData.append('ap_pass', document.getElementById('ap_pass').value);
            
            try {
                const response = await fetch('/save', { 
                    method: 'POST', 
                    headers: { 'Content-Type': 'application/x-www-form-urlencoded' }, 
                    body: formData 
                });
                if (response.ok) { 
                    showStatus('✅ Hub settings saved! Rebooting...', 'success'); 
                } else { 
                    showStatus('❌ Error saving configuration', 'error'); 
                }
            } catch(e) { showStatus('Connection error: ' + e.message, 'error'); }
        }
    </script>
</body>
</html>
)rawliteral";
