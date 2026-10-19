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
    <title>VK Control Panel</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body {
            font-family: 'Segoe UI', sans-serif;
            background: linear-gradient(135deg, #1a1a2e 0%, #16213e 100%);
            min-height: 100vh;
            color: white;
            padding: 20px;
        }
        .container {
            max-width: 550px;
            margin: 0 auto;
            background: rgba(22, 33, 62, 0.95);
            border-radius: 20px;
            padding: 25px;
            box-shadow: 0 10px 30px rgba(0,0,0,0.3);
        }
        h1 { text-align: center; margin-bottom: 10px; color: #4CAF50; font-size: 2em; }
        .subtitle { text-align: center; color: #888; margin-bottom: 25px; font-size: 0.9em; }
        .section-title { color: #4CAF50; border-bottom: 1px solid #4CAF50; padding-bottom: 5px; margin: 25px 0 15px 0; font-size: 1.1em; font-weight: bold; }
        .form-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 10px; margin-bottom: 10px; }
        .form-group { margin-bottom: 12px; }
        .form-group.full { grid-column: span 2; }
        label { display: block; margin-bottom: 4px; color: #aaa; font-size: 0.85em; }
        
        input[type="text"], input[type="password"], input[type="number"] { 
            width: 100%; padding: 10px; border-radius: 8px; border: 1px solid #4CAF50; 
            background: #0f3460; color: white; font-size: 0.95em; outline: none; 
        }
        
        .password-wrapper { 
            display: flex; 
            position: relative; 
            width: 100%; 
            align-items: center;
        }
        .password-wrapper input { 
            padding-right: 45px; 
        }
        .toggle-password { 
            position: absolute; 
            right: 10px; 
            background: none; 
            border: none; 
            color: #4CAF50; 
            cursor: pointer; 
            font-size: 1.2em; 
            outline: none;
            padding: 5px;
        }
        
        .btn { background: #4CAF50; color: white; border: none; padding: 12px 15px; border-radius: 8px; cursor: pointer; font-size: 1em; transition: all 0.2s; font-weight: bold; }
        .btn:hover { background: #45a049; }
        .btn-wide { width: 100%; margin-top: 20px; }
        .btn-secondary { background: #0f3460; border: 1px solid #4CAF50; }
        .btn-secondary:hover { background: #1a4a80; }
        
        .stepper-ui { display: flex; justify-content: center; align-items: center; gap: 20px; margin: 15px 0; }
        .btn-circle { 
            width: 60px; height: 60px; border-radius: 50%; border: 2px solid #4CAF50; 
            background: #0f3460; color: white; font-size: 1.6em; cursor: pointer; 
            display: flex; justify-content: center; align-items: center; transition: all 0.2s; 
        }
        .btn-circle:hover { background: #4CAF50; transform: scale(1.05); }
        .btn-circle.stop { border-color: #e94560; font-size: 1em; font-weight: bold; color: #e94560; }
        .btn-circle.stop:hover { background: #e94560; color: white; }
        
        .servo-ui { display: flex; gap: 10px; margin: 15px 0; }
        
        .status { margin-top: 15px; padding: 10px; border-radius: 8px; text-align: center; font-size: 0.9em; display: none; }
        .status.success { background: rgba(76, 175, 80, 0.2); color: #4CAF50; display: block; }
        .status.error { background: rgba(244, 67, 54, 0.2); color: #e94560; display: block; }
        .status.loading { background: rgba(255, 152, 0, 0.2); color: #ff9800; display: block; }
        .info { background: #0f3460; padding: 10px; border-radius: 8px; margin-top: 20px; font-size: 0.8em; text-align: center; color: #aaa; }
    </style>
</head>
<body>
    <div class="container">
        <h1>⚙️ VK CONTROL PANEL</h1>
)rawliteral";

// ====================================================================
// === ЧАСТЬ 2: ТЕЛО СТРАНИЦЫ С НАСТРОЙКАМИ И КНОПКАМИ УПРАВЛЕНИЯ   ===
// ====================================================================
const char HTML_BODY[] PROGMEM = R"rawliteral(
        <div class="subtitle">Integrated Hardware Control System</div>
        
        <form id="configForm" onsubmit="return false;">
            <div class="section-title">📡 Network Settings</div>
            <div class="form-grid">
                <div class="form-group">
                    <label>Device Name (espname)</label>
                    <input type="text" id="espname" value="%ESPNAME_VAL%">
                </div>
                <div class="form-group">
                    <label>Wi-Fi SSID</label>
                    <input type="text" id="ssid" value="%SSID_VAL%">
                </div>
                <div class="form-group full">
                    <label>Wi-Fi Password (PASS)</label>
                    <div class="password-wrapper">
                        <input type="password" id="password" value="%PASS_VAL%" autocomplete="current-password">
                        <button type="button" class="toggle-password" onclick="togglePassword(this)">👁️</button>
                    </div>
                </div>
            </div>

            <div class="section-title">💡 Stepper Configuration</div>
            <div class="form-grid">
                <div class="form-group">
                    <label>Step Pin</label>
                    <input type="number" id="step_pin" value="%STEP_PIN_VAL%">
                </div>
                <div class="form-group">
                    <label>Dir Pin</label>
                    <input type="number" id="dir_pin" value="%DIR_PIN_VAL%">
                </div>
                <div class="form-group">
                    <label>Speed (steps/s)</label>
                    <input type="number" id="step_speed" value="%STEP_SPEED_VAL%">
                </div>
                <div class="form-group">
                    <label>Acceleration (steps/s²)</label>
                    <input type="number" id="step_accel" value="%STEP_ACCEL_VAL%">
                </div>
                <div class="form-group">
                    <label>Steps Per Rev</label>
                    <input type="number" id="step_rev" value="%STEP_REV_VAL%">
                </div>
                <div class="form-group">
                    <label>Step Delay (us)</label>
                    <input type="number" id="step_delay" value="%STEP_DELAY_VAL%">
                </div>
            </div>

            <div class="section-title">🔄 Stepper Manual Control</div>
            <div class="stepper-ui">
                <button type="button" class="btn-circle" onclick="runStepper('ccw')">↺</button>
                <button type="button" class="btn-circle stop" onclick="runStepper('stop')">STOP</button>
                <button type="button" class="btn-circle" onclick="runStepper('cw')">↻</button>
            </div>

            <!-- ✅ ИСПРАВЛЕНО: Новая трехколоночная сетка для Servo параметров на одной строке -->
            <div class="section-title">📐 Servo Configuration</div>
            <div class="form-grid" style="grid-template-columns: repeat(3, 1fr);">
                <div class="form-group" style="grid-column: span 3;">
                    <label>Servo Pin</label>
                    <input type="number" id="servo_pin" value="%SERVO_PIN_VAL%">
                </div>
                <div class="form-group">
                    <label>Min Pulse (us)</label>
                    <input type="number" id="servo_min" value="%SERVO_MIN_VAL%">
                </div>
                <div class="form-group">
                    <label>Max Pulse (us)</label>
                    <input type="number" id="servo_max" value="%SERVO_MAX_VAL%">
                </div>
                <div class="form-group">
                    <label>Step Delay (ms)</label>
                    <input type="number" id="servo_delay" value="%SERVO_DELAY_VAL%">
                </div>
            </div>

            <div class="section-title">🦾 Servo Manual Control</div>
            <div class="servo-ui">
                <button type="button" class="btn" onclick="moveServo('0')">Angle 0°</button>
                <button type="button" class="btn btn-secondary" onclick="moveServo('90')">Angle 90°</button>
                <button type="button" class="btn" onclick="moveServo('180')">Angle 180°</button>
            </div>

            <button type="button" class="btn btn-wide" onclick="saveAllConfig()">💾 Save Settings & Reboot</button>
        </form>
        
        <div id="status" class="status"></div>
        <div class="info">⚙️ Hardware limits apply after reboot. Pulse buttons work immediately.</div>
    </div>
)rawliteral";

// ====================================================================
// === ЧАСТЬ 3: СКРИПТЫ ВЗАИМОДЕЙСТВИЯ (AJAX / REST API КОМАНДЫ)     ===
// ====================================================================
const char HTML_SCRIPT[] PROGMEM = R"rawliteral(
    <script>
        function togglePassword(btnElement) {
            const pwd = document.getElementById('password');
            if (pwd.type === 'password') {
                pwd.type = 'text';
                btnElement.textContent = '🙈';
            } else {
                pwd.type = 'password';
                btnElement.textContent = '👁️';
            }
        }

        function showStatus(msg, type) {
            const statusDiv = document.getElementById('status');
            statusDiv.textContent = msg; 
            statusDiv.className = 'status ' + type;
        }

        async function runStepper(action) {
            try {
                let url = '/api/stepper?cmd=' + action;
                const response = await fetch(url, { method: 'GET' });
                if (!response.ok) console.error('Stepper error');
            } catch(e) { console.error('Conn error:', e); }
        }

        async function moveServo(angle) {
            try {
                let url = '/api/servo?angle=' + angle;
                const response = await fetch(url, { method: 'GET' });
                if (!response.ok) console.error('Servo error');
            } catch(e) { console.error('Conn error:', e); }
        }

        async function saveAllConfig() {
            showStatus('Saving configuration...', 'loading');
            
            const formData = new URLSearchParams();
            formData.append('espname', document.getElementById('espname').value.trim()); 
            formData.append('ssid', document.getElementById('ssid').value.trim()); 
            formData.append('password', document.getElementById('password').value);
            
            formData.append('step_pin', document.getElementById('step_pin').value);
            formData.append('dir_pin', document.getElementById('dir_pin').value);
            formData.append('step_speed', document.getElementById('step_speed').value);
            formData.append('step_accel', document.getElementById('step_accel').value);
            formData.append('step_rev', document.getElementById('step_rev').value);
            formData.append('step_delay', document.getElementById('step_delay').value);
            
            formData.append('servo_pin', document.getElementById('servo_pin').value);
            formData.append('servo_min', document.getElementById('servo_min').value);
            formData.append('servo_max', document.getElementById('servo_max').value);
            formData.append('servo_delay', document.getElementById('servo_delay').value);

            try {
                const response = await fetch('/save', { 
                    method: 'POST', 
                    headers: { 'Content-Type': 'application/x-www-form-urlencoded' }, 
                    body: formData 
                });
                if (response.ok) { 
                    showStatus('✅ Saved successfully! Rebooting...', 'success'); 
                } else { 
                    showStatus('❌ Error saving data', 'error'); 
                }
            } catch(e) { showStatus('Conn error: ' + e.message, 'error'); }
        }
    </script>
</body>
</html>
)rawliteral";
