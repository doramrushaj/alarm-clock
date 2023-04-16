#include "WiFi.h" // We need this library to connect to WiFi 
#include "ESPAsyncWebServer.h" // We need this library to make a webpage (e.g. Web-App)


// Defining the names of variables that we will use later
int val = 0;

String Current_Time; // This string will hold our current time
String Alarm_Set;  // This string will hold the time we set for the alarm to go off
String Sch;
int buzzer = 18;  // Buzzer is connected to pin 18 of the ESP32 Board
const int buttonPin = 12;     // Push button is connected to pin 12 of the ESP32 Board
int t=0;
int p=0;



// Replace with your network credentials
const char* ssid = "YOURWIFI";
const char* password = "YOURPASS";

// Setting up real-time clock
const char* NTP_SERVER = "ch.pool.ntp.org";
const char* TZ_INFO    = "CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00";  // enter your time zone (https://remotemonitoringsystems.ca/time-zone-abbreviations.php)
tm timeinfo;
time_t now;
long unsigned lastNTPtime;
unsigned long lastEntryTime;

// My input from web-app to the arduino
const char* PARAM_INPUT_1 = "input1";
const char* PARAM_INPUT_11 = "input11";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Here we made a function that seperates the string so we dont read space or :
String getValue(String data, char separator, int index)
{
    int maxIndex = data.length() - 1;
    int j = 0;
    String chunkVal = "";

    for (int i = 0; i <= maxIndex && j <= index; i++)
    {
        chunkVal.concat(data[i]);

        if (data[i] == separator)
        {
            j++;

            if (j > index)
            {
                chunkVal.trim();
                return chunkVal;
            }

            chunkVal = "";
        }
        else if ((i == maxIndex) && (j < index)) {
            chunkVal = "";
            return chunkVal;
        }
    }   
}

// Here we made a function get real time clock :
bool getNTPtime(int sec) {

  {
    uint32_t start = millis();
    do {
      time(&now);
      localtime_r(&now, &timeinfo);
      delay(10);
      
    } while (((millis() - start) <= (1000 * sec)) && (timeinfo.tm_year < (2016 - 1900)));
    if (timeinfo.tm_year <= (2016 - 1900)) return false;  // the NTP call was not successful

    char time_output[30];
    strftime(time_output, 30, "%a  %d-%m-%y %T", localtime(&now));
  }
  return true;
}

// This function print real time clock on serial monitor (used for testing)
void showTime(tm localTime) {
//  Serial.println('\n');
//  Serial.print(localTime.tm_hour + 6);
//  Serial.print(':');
//  Serial.print(localTime.tm_min);
//  Serial.print(':');
//  Serial.println(localTime.tm_sec);
}

// Get the Hour
String Hour(tm localTime) {
   int t=localTime.tm_hour -6;
  if (isnan(t)) {    
    Serial.println("Failed to read from DHT sensor!");
    return "--";
  }
  else {
  //  Serial.println(t);
    if (t<0){
      t=24+t;
    }
    return String(t);
  }
}

// Get the Minutes
String Minutes(tm localTime) {
  int t=localTime.tm_min;
  if (isnan(t)) {    
    Serial.println("Failed to read from DHT sensor!");
    return "--";
  }
  else {
   // Serial.println(t);
    return String(t);
  }
}

// Get the Seconds
String Seconds(tm localTime) {
  int t=localTime.tm_sec;
  if (isnan(t)) {    
    Serial.println("Failed to read from DHT sensor!");
    return "--";
  }
  else {
  //  Serial.println(t);
    return String(t);
  }
}

// ******************************************************************** HTML CSS CODE STARTS HERE *************************************************************************************** //
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <style>
    html {
     font-family: Arial;
     display: inline-block;
     margin: 0px auto;
    }
    h2 { font-size: 3.0rem; }
    p { font-size: 3.0rem; }
    .units { font-size: 1.2rem; }
    .dht-labels{
      font-size: 1.5rem;
      vertical-align:middle;
      padding-bottom: 15px;
    }

    input[type=text], select {
  
  display: inline-block;
  border: 1px solid #ccc;
  border-radius: 4px;
  box-sizing: border-box;
}

input[type=submit] {
   cursor: pointer;
}

  </style>
</head>
<body>
  <br/> <br/> <br/> <br/> <br/>
  <h2>CURRENT TIMER</h2>
  <p style=" " >
    <i class=" " style=""></i> 
    <span id="HOURS">%HOURS%</span> :
        <span id="MINUTES">%MINUTES%</span> :
            <span id="SECONDS">%int(SECONDS)%</span>
  </p>
  <p>
  Set the Alarm
  </p>
  
  <form action="/get">
   <label > </label>
   <input type="time" id="inputText" name="input1" size="400" onchange="timers(this.value)"  />
    <input type="submit" value="Submit">
  </form>
  
 
  
</body>
<script>

// ****In BODY instead of %HOURS% put a value getting from the function **** //
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
//  Serial.println(xhttp);
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("HOURS").innerHTML = this.responseText;
      }
  };
  xhttp.open("GET", "/HOURS", true);
  xhttp.send();
}, 1000 ) ;

// ****In BODY instead of %MINUTES% put a value getting from the function **** //
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("MINUTES").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/MINUTES", true);
  xhttp.send();
}, 500 ) ;

// ****In BODY instead of %SECONDS% put a value getting from the function **** //
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("SECONDS").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/SECONDS", true);
  xhttp.send();
}, 10 ) ;

</script>
</html>)rawliteral";
// ******************************************************************** HTML CSS CODE ENDS HERE *************************************************************************************** //

// Provoding the Hour or Min or Sec time
String processor(const String& var){
  if(var == "HOURS"){
    return Hour(timeinfo);
  }
  else if(var == "MINUTES"){
    return Minutes(timeinfo);
  }
  else if(var == "SECONDS"){
    return Seconds(timeinfo);
  }
  return String();
}

void setup(){
  Serial.begin(115200); 
  pinMode(buttonPin, INPUT); // Push button is now input
  ledcSetup(0,1E5,12); // Setup for buzzer on ESP32
  ledcAttachPin(buzzer,0); // Setup for buzzer on ESP32
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP()); // Print the wifi IP ("Website link") on serial monitor

  // Time Configuration
  configTime(0, 0, NTP_SERVER);
  setenv("TZ", TZ_INFO, 1);
  if (getNTPtime(10)) {  // wait up to 10sec to sync
  } else {
  //  Serial.println("Time not set");
    ESP.restart();
  }
  showTime(timeinfo);
  lastNTPtime = time(&now);
  lastEntryTime = millis();
  

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/HOURS", HTTP_GET, [](AsyncWebServerRequest *request){
  //  Serial.println((Hour(timeinfo)).toInt());
    request->send_P(200, "text/plain", (Hour(timeinfo)).c_str());
  });
  server.on("/MINUTES", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", Minutes(timeinfo).c_str());
  });
  server.on("/SECONDS", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", Seconds(timeinfo).c_str());
  });

// ******************************************************* HERE WE GET THE INPUT FROM THE WEBSITE (ALARM SET) ********************************************************************** //
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html, processor);
    String inputMessage;
    String inputParam;
    if (request->hasParam(PARAM_INPUT_1)) {
      inputMessage = request->getParam(PARAM_INPUT_1)->value(); // HERE WE ARE GETTING THE ALARM SET FROM THE WEBSITE
      inputParam = PARAM_INPUT_1;
    }
    else {
      inputMessage = "No message sent";
      inputParam = "none";
    }
     Serial.println(inputMessage); // print the alarm on serial port
     
     Current_Time=Hour(timeinfo) +':'+Minutes(timeinfo) ;
     Alarm_Set=inputMessage; // SAVE THE SET ALARM IN A STRING CALLED "ALARM_SET"
     t=0;
     p=0;
      });

// ************************************************************************************************************************************************************************** //

  // Start server
  server.begin();
}
 
 
void loop(){
  getNTPtime(10);
  showTime(timeinfo);

// From "ALARM_SET" variable get the hour, and minute
  String Hour = getValue(Alarm_Set, ':', 0); 
  Hour = getValue(Hour, ':', 0);
  String Minz=getValue(Alarm_Set, ':', 1);

// ******************************************************* HERE IS THE CONDITION WHEN THE CURRENT TIME MATCHES ALAM SET ********************************************************************** //
        int h=digitalRead(buttonPin);     // Check to see if the button is pushed

  if(getValue(Alarm_Set, ':', 0).toInt()!=0){ // If ALARM_SET variabel is not empty
    if(getValue(Alarm_Set, ':', 0).toInt()==getValue(Current_Time, ':', 0).toInt()){ // If Hour is same
      if(getValue(Alarm_Set, ':', 1).toInt()==Minutes(timeinfo).toInt()){ // If Minute is same
        while (t==0){ // If this is first time this is happening
          val = hallRead();

          if (abs(val)> 10){
             t=1;
          }
          
          
           delay(2000);

          Serial.println(t);
          ledcWriteTone(0,800); 
          uint8_t octave = 1;
          delay(2000);
         ledcWriteNote(0,NOTE_C,octave);   // Make the buzzer sound
         if (t==1){
          ledcWriteTone(0,80000); // Turn off the buzzer
         }

//                  if (h == 1) {  // If the button is pushed
//                         t=1;
//                          Serial.println("BUTTON PRESSED"); // Print that button is pushed
//                        ledcWriteTone(0,80000); // Turn off the buzzer
//                        delay(20000); // Wait for a while
//                        }
//        Serial.println(" ALARM IS ACTIVATED"); // Print on serial port
//        Serial.print(h);
//        ledcWriteTone(0,800); 
//        uint8_t octave = 1;
//                delay(2000);
//
//        ledcWriteNote(0,NOTE_C,octave);   // Make the buzzer sound
//        int h=digitalRead(buttonPin);     // Check to see if the button is pushed


                }
      }
    }
  }
// ************************************************************************************************************************************************************************** //


  delay(200);
}
