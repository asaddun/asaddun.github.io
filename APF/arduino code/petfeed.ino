#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Servo.h>
#include <Ticker.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define servoPin D5
#define trigPin D6
#define echoPin D7

WiFiManager wifiManager;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
ESP8266WebServer server(80);
Servo servo;
Ticker ticker;
LiquidCrystal_I2C lcd(0x27, 16, 2);

int toggleInterval, toggleSchedule, toggleSensor, toggleManual;
int intervalSelected;
unsigned long previousMillis, currentMillis, numberInterval;
String hourSelected1, minuteSelected1, hourSelected2, minuteSelected2;
String time1, time2;
String ipAddress;
long duration;
float distance;
int distanceReal;
bool feedDone = false;

const char index_html[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>Automatic Pet Feeder</title>
    <link rel="stylesheet" type="text/css" href="/style.css">
    <link rel="icon" type="image/png" href="https://i.imgur.com/qyy4vnh.png">
  </head>
  <body>
    <div id="mySidebar" class="sidebar">
        <a class="closebtn" onclick="closeNav()">&#215;</a>
        <div class="title"><a href="/">Automatic Pet Feeder</a></div>
        <br>
        <a href="/interval">Interval</a>
        <a href="/time">Schedule</a>
        <a href="/sensor">Sensor</a>
        <a href="/manual">Manual</a>
        <a href="/reset">Reset</a>
    </div>
    <header>
      <div class="toggle-sidebar" onclick="openNav()">&#9776;</div>
      <div class="title-section">Automatic Pet Feeder</div>
    </header>
    <div class="main">
      <p>Automatic Pet Feeder. IoT device that can Automatically feed your pet.</p>
      <p>Automatic Pet Feeder has 4 options to operate:</p>
      <ol>
        <li>Interval Method: Feed your pet in predetermined time intervals.</li>
        <li>Schedule Method: Feed your pet at specific times that you set.</li>
        <li>Sensor Method: Feed your pet whenever sensor is triggered (requires pet training).</li>
        <li>Manual Method: You can manually feed your pet by pressing a button.</li>
      </ol>
    </div>
    <script src="script.js"></script>
  </body>
</html>
)=====";

const char interval_html[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>Automatic Pet Feeder</title>
    <link rel="stylesheet" type="text/css" href="/style.css">
    <link rel="icon" type="image/png" href="https://i.imgur.com/qyy4vnh.png">
  </head>
  <body>
    <div id="mySidebar" class="sidebar">
        <a class="closebtn" onclick="closeNav()">&#215;</a>
        <div class="title"><a href="/">Automatic Pet Feeder</a></div>
        <br>
        <a href="/interval">Interval</a>
        <a href="/time">Schedule</a>
        <a href="/sensor">Sensor</a>
        <a href="/manual">Manual</a>
        <a href="/reset">Reset</a>
    </div>
    <header>
      <div class="toggle-sidebar" onclick="openNav()">&#9776;</div>
      <div class="title-section">Interval Method</div>
    </header>
      <div class="main">
        <form action="/submitinterval">
        <div class="menu">Interval</div>
        <div class="toggle">
          <input type="checkbox" id="toggleBtn" onclick="toggleButton()">
          <label for="toggleBtn"></label>
        </div>
        <input type="hidden" id="toggleValue" name="toggleInterval">
        <br>
        <div class="menu">Interval time</div>
        <div class="menuInterval">
          every
          <input type="number" id="numberInterval" min="0" max="59" name="numberInterval">
          <select id="intervalSelect" onchange="selectInterval()">
            <option value="0"></option>
            <option value="1">sec(s)</option>
            <option value="2">min(s)</option>
            <option value="3">hour(s)</option>
          </select>
        </div>
      <input type="hidden" id="intervalSelected" name="intervalSelected">
      <script src="script.js"></script>
    </div>
    <div class="bottomnav">
      <button class="save-button">Save</button>
    </div>
  </form>
  </body>
</html>
)=====";

const char schedule_html[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>Automatic Pet Feeder</title>
    <link rel="stylesheet" type="text/css" href="/style.css">
    <link rel="icon" type="image/png" href="https://i.imgur.com/qyy4vnh.png">
  </head>
  <body>
    <div id="mySidebar" class="sidebar">
        <a class="closebtn" onclick="closeNav()">&#215;</a>
        <div class="title"><a href="/">Automatic Pet Feeder</a></div>
        <br>
        <a href="/interval">Interval</a>
        <a href="/time">Schedule</a>
        <a href="/sensor">Sensor</a>
        <a href="/manual">Manual</a>
        <a href="/reset">Reset</a>
    </div>
    <header>
      <div class="toggle-sidebar" onclick="openNav()">&#9776;</div>
      <div class="title-section">Schedule Method</div>
    </header>
    <div class="main">
        <form action="/submitschedule">
        <div class="menu">Schedule</div>
        <div class="toggle">
        <input type="checkbox" id="toggleBtn" onclick="toggleButton()">
        <label for="toggleBtn"></label>
        </div>
        <input type="hidden" id="toggleValue" name="toggleSchedule">
        <br>
        <div class="menu">Schedule 1</div>
        <div class="menuTime">
          <select id="hourSelect1" onchange="selectTime1()">
            <option value="0"></option>
            <script>
              for (let i = 0; i <= 23; i++) {
                document.write(`<option value="${i.toString().padStart(2, '0')}">${i.toString().padStart(2, '0')}</option>`);
              }
            </script>
          </select>
          Hour
          <select id="minuteSelect1" onchange="selectTime1()">
            <option value="0"></option>
            <script>
              for (let i = 0; i <= 59; i++) {
                document.write(`<option value="${i.toString().padStart(2, '0')}">${i.toString().padStart(2, '0')}</option>`);
              }
            </script>
          </select>
          Minute
        </div>
        <input type="hidden" id="hourSelected1" name="hourSelected1">
        <input type="hidden" id="minuteSelected1" name="minuteSelected1">
        <br>
        <div class="menu">Schedule 2</div>
        <div class="menuTime">
          <select id="hourSelect2" onchange="selectTime2()">
            <option value="0"></option>
            <script>
              for (let i = 0; i <= 23; i++) {
                document.write(`<option value="${i.toString().padStart(2, '0')}">${i.toString().padStart(2, '0')}</option>`);
              }
            </script>
          </select>
          Hour
          <select id="minuteSelect2" onchange="selectTime2()">
            <option value="0"></option>
            <script>
              for (let i = 0; i <= 59; i++) {
                document.write(`<option value="${i.toString().padStart(2, '0')}">${i.toString().padStart(2, '0')}</option>`);
              }
            </script>
          </select>
          Minute
        </div>
        <input type="hidden" id="hourSelected2" name="hourSelected2">
        <input type="hidden" id="minuteSelected2" name="minuteSelected2">
        <script src="script.js"></script>
    </div>
    <div class="bottomnav">
      <button class="save-button">Save</button>
    </div>
  </form>
  </body>
</html>
)=====";

const char sensor_html[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>Automatic Pet Feeder</title>
    <link rel="stylesheet" type="text/css" href="/style.css">
    <link rel="icon" type="image/png" href="https://i.imgur.com/qyy4vnh.png">
  </head>
  <body>
    <div id="mySidebar" class="sidebar">
        <a class="closebtn" onclick="closeNav()">&#215;</a>
        <div class="title"><a href="/">Automatic Pet Feeder</a></div>
        <br>
        <a href="/interval">Interval</a>
        <a href="/time">Schedule</a>
        <a href="/sensor">Sensor</a>
        <a href="/manual">Manual</a>
        <a href="/reset">Reset</a>
    </div>
    <header>
      <div class="toggle-sidebar" onclick="openNav()">&#9776;</div>
      <div class="title-section">Sensor Method</div>
    </header>
    <div class="main">
        <form action="/submitsensor">
        <div class="menu">Ultrasonic Sensor</div>
        <div class="toggle">
        <input type="checkbox" id="toggleBtn" onclick="toggleButton()">
        <label for="toggleBtn"></label>
        </div>
        <input type="hidden" id="toggleValue" name="toggleSensor">
        <script src="script.js"></script>
    </div>
    <div class="bottomnav">
      <button class="save-button" type="submit">Save</button>
    </div>
  </form>
  </body>
</html>
)=====";

const char submit_html[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>Automatic Pet Feeder</title>
    <link rel="stylesheet" type="text/css" href="/style.css">
    <link rel="icon" type="image/png" href="https://i.imgur.com/qyy4vnh.png">
    <script>
      function goBack() {
        window.location.href = "/";
      }
      setTimeout(goBack, 5000);
    </script>
  </head>
  <body>
    <div id="mySidebar" class="sidebar">
        <a class="closebtn" onclick="closeNav()">&#215;</a>
        <div class="title"><a href="/">Automatic Pet Feeder</a></div>
        <br>
        <a href="/interval">Interval</a>
        <a href="/time">Schedule</a>
        <a href="/sensor">Sensor</a>
        <a href="/manual">Manual</a>
        <a href="/reset">Reset</a>
    </div>
    <header>
      <div class="toggle-sidebar" onclick="openNav()">&#9776;</div>
      <div class="title-section">Save Setting</div>
    </header>
  <div class="main">
    <p>Setting has been saved.</p>
    <p>Redirecting to home page...</p>
    <script src="script.js"></script>
  </div>
  </body>
</html>
)=====";

const char manual_html[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>Automatic Pet Feeder</title>
    <link rel="stylesheet" type="text/css" href="/style.css">
    <link rel="icon" type="image/png" href="https://i.imgur.com/qyy4vnh.png">
  </head>
  <body>
    <div id="mySidebar" class="sidebar">
        <a class="closebtn" onclick="closeNav()">&#215;</a>
        <div class="title"><a href="/">Automatic Pet Feeder</a></div>
        <br>
        <a href="/interval">Interval</a>
        <a href="/time">Schedule</a>
        <a href="/sensor">Sensor</a>
        <a href="/manual">Manual</a>
        <a href="/reset">Reset</a>
    </div>
    <header>
      <div class="toggle-sidebar" onclick="openNav()">&#9776;</div>
      <div class="title-section">Manual Method</div>
    </header>
    <div class="main">
      <p>You can manually feed your pet by pressing the button.</p>
      <form action="/submitmanual">
        <input type="hidden" name="toggleManual" value="1">
        <button class="manual-button" type="submit">Feed</button>
      </form>
      <script src="script.js"></script>
    </div>
  </body>
</html>
)=====";

const char submitmanual_html[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>Automatic Pet Feeder</title>
    <link rel="stylesheet" type="text/css" href="/style.css">
    <link rel="icon" type="image/png" href="https://i.imgur.com/qyy4vnh.png">
    <script>
      function goBack() {
        window.location.href = "/";
      }
      setTimeout(goBack, 5000);
    </script>
  </head>
  <body>
    <div id="mySidebar" class="sidebar">
        <a class="closebtn" onclick="closeNav()">&#215;</a>
        <div class="title"><a href="/">Automatic Pet Feeder</a></div>
        <br>
        <a href="/interval">Interval</a>
        <a href="/time">Schedule</a>
        <a href="/sensor">Sensor</a>
        <a href="/manual">Manual</a>
        <a href="/reset">Reset</a>
    </div>
    <header>
      <div class="toggle-sidebar" onclick="openNav()">&#9776;</div>
      <div class="title-section">Manual Method</div>
    </header>
  <div class="main">
    <p>Device has successfully fed.</p>
    <p>Redirecting to home page...</p>
    <script src="script.js"></script>
  </div>
  </body>
</html>
)=====";

const char reset_html[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>Automatic Pet Feeder</title>
    <link rel="stylesheet" type="text/css" href="/style.css">
    <link rel="icon" type="image/png" href="https://i.imgur.com/qyy4vnh.png">
  </head>
  <body>
    <div id="mySidebar" class="sidebar">
        <a class="closebtn" onclick="closeNav()">&#215;</a>
        <div class="title"><a href="/">Automatic Pet Feeder</a></div>
        <br>
        <a href="/interval">Interval</a>
        <a href="/time">Schedule</a>
        <a href="/sensor">Sensor</a>
        <a href="/manual">Manual</a>
        <a href="/reset">Reset</a>
    </div>
    <header>
      <div class="toggle-sidebar" onclick="openNav()">&#9776;</div>
      <div class="title-section">Reset Device</div>
    </header>
    <div class="main">
      <p>Reset saved WiFi and device setting?</p>
      <button class="reset-button" onclick="confirmReset()">Reset</button>
      <script src="script.js"></script>
    </div>
  </body>
</html>
)=====";

const char doreset_html[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>Automatic Pet Feeder</title>
    <link rel="stylesheet" type="text/css" href="/style.css">
    <link rel="icon" type="image/png" href="https://i.imgur.com/qyy4vnh.png">
    <script>
      function goBack() {
        window.location.href = "/";
      }
      setTimeout(goBack, 5000);
    </script>
  </head>
  <body>
    <div id="mySidebar" class="sidebar">
        <a class="closebtn" onclick="closeNav()">&#215;</a>
        <div class="title"><a href="/">Automatic Pet Feeder</a></div>
        <br>
        <a href="/interval">Interval</a>
        <a href="/time">Schedule</a>
        <a href="/sensor">Sensor</a>
        <a href="/manual">Manual</a>
        <a href="/reset">Reset</a>
    </div>
    <header>
      <div class="toggle-sidebar" onclick="openNav()">&#9776;</div>
      <div class="title-section">Reset Device</div>
    </header>
  <div class="main">
    <p>Device is resetting...</p>
    <p>Redirecting to home page...</p>
    <script src="script.js"></script>
  </div>
  </body>
</html>
)=====";

const char style_css[] PROGMEM = R"=====(
header {
    display: flex;
    justify-content: space-between;
    align-items: center;
    padding: 10px 40px 10px 20px;
    background-color: #003f5c;
}
.toggle-sidebar {
    cursor: pointer;
    margin-right: 10px;
    font-size: 24px;
    color: white;
}
.toggle-sidebar:hover {
    color: #277092;
}
.title-section {
    font-size: 24px;
    font-weight: bold;
    color: white;
}
/* side bar */
.sidebar {
    height: 100%;
    width: 0;
    position: fixed;
    z-index: 1;
    top: 0;
    left: 0;
    background-color: #465881;
    overflow-x: hidden;
    transition: 0.5s;
    padding-top: 60px;
}
.sidebar a {
    padding: 8px 8px 8px 32px;
    text-decoration: none;
    font-size: 25px;
    color: #ffffff ;
    display: block;
    transition: 0.3s;
}
.sidebar a:hover {
    color: #8690a6;
}
.sidebar .closebtn {
    position: absolute;
    cursor: pointer;
    top: 0;
    right: 25px;
    font-size: 36px;
    margin-left: 50px;
}
body {
    background-color: #f2f2f2;
}
.main {
    transition: margin-left .5s;
    margin-left: 50px;
    padding: 16px;
}
h1 {
    text-align: center;
}
p, ol {
    font-size: 18px;
    line-height: 1.5;
    color: #333333;
}
.menu {
    display: inline-block;
    margin-bottom: 20px;
    font-size: 18px;
    color: #333333;
}
.menu:first-of-type {
    margin-top: 25px;
}
.menuInterval {
    display: inline-block;
    margin-left: 160px;
}
.menuTime {
    display: inline-block;
    margin-left: 172px;
    color: #333333;
}
.bottomnav {
    position: fixed;
    bottom: 0;
    width: 100%;
    height: 60px;
    background-color: #ffffff;
    display: flex;
    justify-content: center;
    align-items: center;
    padding-right: 20px;
}
.save-button {
    padding: 10px 20px;
    font-size: 16px;
    border: none;
    background-color: #4caf50;
    color: #ffffff;
    border-radius: 5px;
    cursor: pointer;
}
.save-button:hover {
    background-color: #419e46;
}
.manual-button {
    display: block;
    margin: 0 auto;
    margin-top: 100px;
    padding: 10px 20px;
    font-size: 16px;
    border: none;
    background-color: #4caf50;
    color: #fff;
    border-radius: 5px;
    cursor: pointer;
}
.manual-button:hover {
    background-color: #419e46;
}
.reset-button {
    display: block;
    margin: 0 auto;
    margin-top: 50px;
    padding: 10px 20px;
    font-size: 16px;
    border: none;
    background-color: #bc4e4e;
    color: #fff;
    border-radius: 5px;
    cursor: pointer;
}
.reset-button:hover {
    background-color: #b71a1a;
}
/* toggle button */
.toggle {
    position: absolute;
    top: 95px;
    left: 327px;
    display: inline-block;
    width: 50px;
    height: 24px;
    margin-bottom: 10px;
}
.toggle input[type="checkbox"] {
    display: none;
}
.toggle label {
    position: relative;
    display: inline-block;
    width: 50px;
    height: 24px;
    background-color: #ccc;
    border-radius: 20px;
    cursor: pointer;
}
.toggle label:after {
    content: "";
    position: absolute;
    top: 2px;
    left: 2px;
    width: 20px;
    height: 20px;
    background-color: #fff;
    border-radius: 50%;
    transition: transform 0.3s ease-in-out;
}
.toggle input[type="checkbox"]:checked + label {
    background-color: #69C77B;
}
.toggle input[type="checkbox"]:checked + label:after {
    transform: translateX(26px);
}
/* Responsive for mobile browser */
@media only screen and (max-device-width: 720px) {
    header {
        padding: 10px 20px 10px 20px;
    }
    .title-section {
        font-size: 18px;
    }
    .sidebar {
        padding-top: 15px;
    }
    .sidebar a {
        font-size: 18px;
    }
    .main{
        margin-left: 10px;
        padding: 0px;
    }
    p, ol {
        font-size: 15px;
    }
    .toggle{
        top: 83px;
        left: 175px;
    }
    .menuInterval{
        margin-left: 43px;
    }
    .menuTime{
        margin-left: 62px;
    }
}
)=====";

const char script_js[] PROGMEM = R"=====(
  function openNav() {
    document.getElementById("mySidebar").style.width = "300px";
}
  
function closeNav() {
    document.getElementById("mySidebar").style.width = "0";
}

function toggleButton() {
    var toggleBtn = document.getElementById("toggleBtn");
    var toggleValue = document.getElementById("toggleValue");
    if (toggleBtn.checked) {
      toggleValue.value = "1";
    } else {
      toggleValue.value = "0";
    }
}

var button = document.getElementById("toggleBtn");
var value = document.getElementById("toggleValue");
if (button.checked) {
} else {
    value.value = "0";
}

function selectInterval() {
    var interval = document.getElementById("intervalSelect");
    var numberInterval = document.getElementById("numberInterval");
    var selectedInterval = interval.value;
    if (selectedInterval == "3" && numberInterval.value >= 24) {
      numberInterval.value = 23;
    } else {}

    var nameInterval = document.getElementById("intervalSelected");
    nameInterval.value = selectedInterval;
}

function selectTime1() {
    var hour1 = document.getElementById("hourSelect1");
    var minute1 = document.getElementById("minuteSelect1");
    var selectedHour1 = hour1.options[hour1.selectedIndex].text;
    var selectedMinute1 = minute1.options[minute1.selectedIndex].text;

    var nameHour1 = document.getElementById("hourSelected1");
    var nameMinute1 = document.getElementById("minuteSelected1");
    nameHour1.value = selectedHour1;
    nameMinute1.value = selectedMinute1;;
}

function selectTime2() {
    var hour2 = document.getElementById("hourSelect2");
    var minute2 = document.getElementById("minuteSelect2");
    var selectedHour2 = hour2.options[hour2.selectedIndex].text;
    var selectedMinute2 = minute2.options[minute2.selectedIndex].text;

    var nameHour2 = document.getElementById("hourSelected2");
    var nameMinute2 = document.getElementById("minuteSelected2");
    nameHour2.value = selectedHour2;
    nameMinute2.value = selectedMinute2;
}
function confirmReset() {
    if (confirm("Are you sure you want to reset?")) {
        window.location.href = "/doreset";
    } else {
    }
}
)=====";

void handleCSS() {
  server.sendHeader("Content-Type", "text/css");
  server.send_P(200, "text/css", style_css, sizeof(style_css));
}

void handleJS() {
  server.send_P(200, "text/javascript", script_js);
}

void handleRoot() {
  server.send_P(200, "text/html", index_html);
}

void intervalPage() {
  server.send_P(200, "text/html", interval_html);
}

void schedulePage() {
  server.send_P(200, "text/html", schedule_html);
}

void sensorPage() {
  server.send_P(200, "text/html", sensor_html);
}

void manualPage() {
  server.send_P(200, "text/html", manual_html);
}

void handleReset(){
  server.send_P(200, "text/html", reset_html);
}

void resetting(){
  delay(100);
  wifiManager.resetSettings();
  delay(100);
  ESP.restart();  
}

void doReset(){
  server.send_P(200, "text/html", doreset_html);
  lcdsaved(8);
  ticker.attach(7, resetting);
}

void submitinterval() {
  server.send_P(200, "text/html", submit_html);
  toggleInterval = server.arg("toggleInterval").toInt();
  numberInterval = server.arg("numberInterval").toInt();
  intervalSelected = server.arg("intervalSelected").toInt();

  if (intervalSelected == 1){
    numberInterval = numberInterval * 1000;
  } else if (intervalSelected == 2){
    numberInterval = numberInterval * 1000 * 60;
  } else if (intervalSelected == 3){
    numberInterval = numberInterval * 1000 * 60 * 60;
  }

  if (toggleInterval == 1 && toggleSchedule == 1){
    toggleSchedule = 0;
  }

  if(toggleInterval == 1){
    lcdsaved(1);
  } else {
    lcdsaved(2);
  }

  // currentMillis = millis();

  Serial.print("toggleSchedule: ");
  Serial.println(toggleSchedule);
  Serial.print("toggleInterval: ");
  Serial.println(toggleInterval);
}

void submitschedule() {
  server.send_P(200, "text/html", submit_html);
  toggleSchedule = server.arg("toggleSchedule").toInt();
  hourSelected1 = server.arg("hourSelected1");
  minuteSelected1 = server.arg("minuteSelected1");
  hourSelected2 = server.arg("hourSelected2");
  minuteSelected2 = server.arg("minuteSelected2");

  time1 = hourSelected1 + ":" + minuteSelected1 + ":00";
  time2 = hourSelected2 + ":" + minuteSelected2 + ":00";

  if (toggleSchedule == 1 && toggleInterval == 1){
    toggleInterval = 0;
  }

  if(toggleSchedule == 1){
    lcdsaved(3);
  } else {
    lcdsaved(4);
  }
  
  Serial.print("toggleInterval: ");
  Serial.println(toggleInterval);
  Serial.print("toggleSchedule: ");
  Serial.println(toggleSchedule);
}

void submitsensor() {
  server.send_P(200, "text/html", submit_html);
  toggleSensor = server.arg("toggleSensor").toInt();

  if(toggleSensor == 1){
    lcdsaved(5);
  } else {
    lcdsaved(6);
  }
  
  Serial.print("toggleSensor: ");
  Serial.println(toggleSensor);
}

void submitmanual() {
  server.send_P(200, "text/html", submitmanual_html);
  toggleManual = server.arg("toggleManual").toInt();
  
  if(toggleManual == 1){
    lcdsaved(7);
  }
  
  // Serial.println(toggleManual);
}

void setup() {
  Serial.begin(115200);
  delay(500);

  lcd.init();                      
  lcd.backlight();
  lcd.setCursor(0,0);
  String msg = "Connect to \"APF\"";
  lcd.print(msg);
  lcd.setCursor(0,1);
  lcd.print("Open 192.168.4.1");

  wifiManager.autoConnect("APF");

  ipAddress = WiFi.localIP().toString();

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  servo.attach(servoPin);

  server.on("/style.css", HTTP_GET, handleCSS);
  server.on("/script.js", HTTP_GET, handleJS);
  server.on("/", handleRoot);
  server.on("/interval", intervalPage);
  server.on("/time", schedulePage);
  server.on("/sensor", sensorPage);
  server.on("/manual", manualPage);
  server.on("/reset", handleReset);
  server.on("/doreset", doReset);
  server.on("/submitinterval", submitinterval);
  server.on("/submitschedule", submitschedule);
  server.on("/submitsensor", submitsensor);
  server.on("/submitmanual", submitmanual);

  server.begin();

  // NTP Setup
  timeClient.begin();
  timeClient.setTimeOffset(25200); // GMT +7 (60(sec) * 60(min) * 7(hour))

  servo.write(180);
  lcdloop();
}

void lcdsaved(int action){
  // list action:
  // 1 = interval on
  // 2 = interval off
  // 3 = time on
  // 4 = time off
  // 5 = sensor on
  // 6 = sensor off
  // 7 = manual
  // 8 = reset
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Setting Saved");
  lcd.setCursor(0, 1);
  
  if (action == 1){
    lcd.print("Interval: ON");
    delay(2000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Interval for:");
    lcd.setCursor(0, 1);
    String lcdInterval;
    if (intervalSelected == 1){
      lcdInterval = String(numberInterval / 1000);
      lcdInterval += " Second";
    } else if (intervalSelected == 2){
      lcdInterval = String(numberInterval / 1000 / 60);
      lcdInterval += " Minute";
    } else if (intervalSelected == 3){
      lcdInterval = String(numberInterval / 1000 / 60 / 60);
      lcdInterval += " Hour";
    }
    lcd.print(lcdInterval);
  } if (action == 2){
    lcd.print("Interval: OFF");
  }
  
  if (action == 3){
    lcd.print("Schedule: ON");
    delay(2000);
    lcd.clear();
    lcd.setCursor(0, 0);
    String lcdTime1 = "Time 1: ";
    lcdTime1 += time1;
    lcd.print(lcdTime1);
    lcd.setCursor(0, 1);
    String lcdTime2 = "Time 2: ";
    lcdTime2 += time2;
    lcd.print(lcdTime2);
  } if (action == 4){
    lcd.print("Schedule: OFF");
  }

  if (action == 5){
    lcd.print("Sensor: ON");
  } if (action == 6){
    lcd.print("Sensor: OFF");
  }

  if (action == 7){
    lcd.print("Perform Feed");
  }

  if (action == 8){
    lcd.print("Resetting Device");
  }

  delay(4000);
  lcdloop();
}

void lcdloop() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Setting to:");
  lcd.setCursor(0,1);
  lcd.print(ipAddress);
}

void petfeed(){
  // feed using motor servo
  servo.write(180);
  delay(300);
  servo.write(0);
  delay(300);
  servo.write(180);
}

void loop() {
  server.handleClient();
  timeClient.update();

  if (!WiFi.isConnected()) {
    Serial.print("WiFi disconnected, reconnecting..");
    digitalWrite(LED_BUILTIN, HIGH);
    // connect to saved WiFi credentials
    WiFi.begin();
    // wait for connection
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.print(".");
    }
    Serial.println("");
    digitalWrite(LED_BUILTIN, LOW);
    ipAddress = WiFi.localIP().toString();
    Serial.println("Connected to WiFi");
  }

  if (toggleSensor == 1){
    // ultrasonic sensor code
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    
    duration = pulseIn(echoPin, HIGH);
    distance = duration / 29.1 / 2;
    distanceReal = distance;
    
    if (distanceReal < 11){
      if (feedDone == false){
        petfeed();
        Serial.println("Sensor");
        feedDone = true;
      }
    } else {
      feedDone = false;
    }
    Serial.println(distanceReal);
    delay(300);
  }

  if (toggleInterval == 1){
    if (currentMillis - previousMillis >= numberInterval) {
      previousMillis = currentMillis;
      petfeed();
      Serial.println("Interval");
    }
    currentMillis = millis();
    Serial.println(currentMillis / 1000);
    delay(1000);
  }

  if (toggleSchedule == 1){
    // get NTP
    time_t epochTime = timeClient.getEpochTime();
    int hour = timeClient.getHours();
    int minute = timeClient.getMinutes();
    int second = timeClient.getSeconds();
    String hourString;
    String minuteString;
    String secondString;
    if (hour < 10) {
      hourString = "0" + String(hour);
    } else {
      hourString = String(hour);
    }
    if (minute < 10) {
      minuteString = "0" + String(minute);
    } else {
      minuteString = String(minute);
    }
    if (second < 10) {
      secondString = "0" + String(second);
    } else {
      secondString = String(second);
    }
    String timeNTP = hourString + ":" + minuteString + ":" + secondString;

    Serial.print("NTP time: ");
    Serial.println(timeNTP);
    Serial.print("inputted time1: ");
    Serial.println(time1);
    // Serial.println(time2);
    if (timeNTP == time1 || timeNTP == time2){
      petfeed();
      Serial.println("Time");
    }
  }

  if (toggleManual == 1){
    toggleManual = 0;
    petfeed();
    Serial.println("Manual");
  }
  // delay(1000);

}