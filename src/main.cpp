#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>

#include <ModbusMaster232.h>

/*
 * SSID TeleCentro-82ba
 * PASS U2N2ZMLR2NQZ
*/

const char* ssid = "TeleCentro-82ba";
const char* password = "U2N2ZMLR2NQZ";

ESP8266WebServer server(80);


// Instantiate ModbusMaster object as slave ID 1
ModbusMaster232 node(1);

int response = 0;

// Define one address for reading
#define address 1
// Define the number of bits to read
#define bitQty 1


/**********dynamic contents ***********/
const char get_toggle_digitalOut[]={"/digital_outputs/toggle"};
const char get_status_dig_out[]={"/digital_outputs"};
const char get_status_dig_in[]={"/digital_inputs"};
const char get_status_analog_output[]= "/analog_outputs";
const char get_update_analog_output[]= "/analog_outputs/update";
const char get_status_analog_inputs[]= "/analog_inputs";
const char get_status_registers[]= "/registers";

float temperatureC;
unsigned int pwmDutyValue=0;
int status = WL_IDLE_STATUS;
void readTemperatureSensor(void);




char toggleOutput(String pinName)
{   
  if (pinName.equals("dout1"))
  {
       digitalWrite(D4, !digitalRead(D4));
       return digitalRead(D4);
  }
  else if(pinName.equals("dout2"))
  {
       digitalWrite(D5, !digitalRead(D5));
       return digitalRead(D5);
  }
  else if(pinName.equals("dout3"))
  {
       digitalWrite(D6, !digitalRead(D6));
       return digitalRead(D6);
  }
  else if(pinName.equals("dout4"))
  {
       digitalWrite(D7, !digitalRead(D7));
       return digitalRead(D7);
  }
   else return 2;
}

String getContentType(String filename){
  if(server.hasArg("download")) return "application/octet-stream";
  else if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".xml")) return "text/xml";
  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  else if(filename.endsWith(".zip")) return "application/x-zip";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

bool handleFileRead(String path){
 String htmlType="text/html";
  if(path.endsWith("/")) path += "index.html";
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if(SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)){
    if(SPIFFS.exists(pathWithGz))
      path += ".gz";
    File file = SPIFFS.open(path, "r");
    if(htmlType!=contentType)
    {
      server.sendHeader("Expires", "Mon, 1 Jan 2222 10:10:10 GMT");
    }
    size_t sent = server.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}

//get_status_dig_out
void handleDigitalOutStatusJson() {
    char someBuffer[200];
    sprintf(someBuffer,"{\"digital_outputs\":{\"dout1\":%c,\"dout2\":%c,\"dout3\":%c,\"dout4\":%c}}",!digitalRead(D4)+48,
                  !digitalRead(D5)+48,!digitalRead(D6)+48,!digitalRead(D7)+48);

    server.send ( 200, "application/json",someBuffer);
}
//get_toggle_out
void handleDigitalOutToggle(){    
    String someBuffer="";
    char stateOfPin=toggleOutput(server.arg(0));
    someBuffer+=String(stateOfPin,DEC);
    server.send ( 200, "text/plain",someBuffer);
}
void handleDigitalInStatusJson(){
            char someBuffer[200];
            sprintf(someBuffer,"{\"digital_inputs\":{\"din1\":%c,\"din2\":%c,\"din3\":%c}}",digitalRead(D0)+48,
            digitalRead(D1)+48,digitalRead(D2)+48);
            server.send ( 200, "application/json",someBuffer);
}
//get_status_an_out
void handleAnalogOutStatus(){
            String someBuffer=  String(12*(float)pwmDutyValue/1023.0, 2);              
            server.send ( 200, "text/plain",someBuffer);            
}

void handleSetAnalogOut(){            
            pwmDutyValue=(server.arg(0).toFloat())/12.0*1023.0;
            analogWrite(D8,pwmDutyValue);            
            String someBuffer=  String(12*(float)pwmDutyValue/1023.0, 2);              
            server.send ( 200, "text/plain",someBuffer);            
}
void handleAnalogInStatus(){       
  readTemperatureSensor();     
   server.send (200,"text/plain",String(temperatureC));  
}

void handleResgistersStatus(){       
  //readTemperatureSensor();   
    int reg1 = random(1, 999 );  
    int reg2 = random(1, 999 );  
    char someBuffer[200];
    sprintf(someBuffer,"{\"registers\":{\"reg1\":%d,\"reg2\":%d}}",reg1,reg2);
    server.send ( 200, "application/json",someBuffer);
 
}

void handleNotFound() {
    String message = "File Not Found\n\n";
    server.send ( 404, "text/plain", message );    
}

void readTemperatureSensor(void)
{
int reading = analogRead(A0); 
 // converting that reading to voltage, for 3.3v arduino use 3.3
 float voltage = reading * 3.3;
 voltage /= 1023.0;
 temperatureC = (voltage - 0.5) * 100 ;  //converting from 10 mv per degree wit 500 mV offset
                                              //to degrees ((voltage - 500mV) times 100)
 //Serial.print(temperatureC); Serial.println(" degrees C");
}


void setup ( void ) {    
    pinMode(D0, INPUT);
    pinMode(D1, INPUT);
    pinMode(D2, INPUT);
    
    pinMode(A0, INPUT);//ADC

    pinMode(D4, OUTPUT);
    pinMode(D5, OUTPUT);
    pinMode(D6, OUTPUT);
    pinMode(D7, OUTPUT);
    digitalWrite(D4,1);
    digitalWrite(D5,1);
    digitalWrite(D6,1);
    digitalWrite(D7,1);

    pinMode(D8, OUTPUT);

 //   Serial.begin ( 115200 );  
   
    SPIFFS.begin();

    WiFi.begin(ssid, password);
  //  Serial.println("");
    
    while (WiFi.status() != WL_CONNECTED) {
    delay(500);

    }
  //  Serial.print("Local IP address:");  
   // Serial.println(WiFi.localIP());

  // Initialize Modbus communication baud rate
    node.begin(19200);
    delay(100);

    server.on (get_status_dig_out,handleDigitalOutStatusJson);
    server.on (get_toggle_digitalOut,handleDigitalOutToggle);
    server.on (get_status_dig_in,handleDigitalInStatusJson);
    server.on (get_status_analog_output,handleAnalogOutStatus);
    server.on (get_update_analog_output,handleSetAnalogOut);
    server.on (get_status_analog_inputs,handleAnalogInStatus);
     server.on (get_status_registers,handleResgistersStatus);

    server.onNotFound([](){
    if(!handleFileRead(server.uri()))
      server.send(404, "text/plain", "FileNotFound");
  });
    server.begin();
}

void loop ( void ) {
    server.handleClient();
    
}
