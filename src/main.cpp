//INCLUDES

#include <Arduino.h>
#include <EEPROM.h>
#include <WiFi.h>
#include <WebServer.h>


#include "SSD1306Wire.h" // legacy include: `#include "SSD1306.h"`
#include "SH1106Wire.h" 


// Include custom images
#include <images.h>
#include <font.h>

// Include the UI lib
#include "OLEDDisplayUi.h"


// Initialize the OLED display using Wire library
//SSD1306Wire display(0x3c, 21, 22);
SH1106Wire display(0x3c, 21, 22); 

#include <TimeLib.h>

//Wifi

// Set web server port number to 80
WebServer servidor(80);


//DEFINES

#define VERSAO "0.2.3"

#define pino_leitura_piloto 34 //34 por causa do wifi
#define pino_pwm 18
#define pino_LED 2
#define pino_rele 19
#define canalpwm 1
#define bits_pwm 10

#define MAX_PWM_DIODO 1100
#define MIN_PWM_A 3600
#define MIN_PWM_B 3100
#define MIN_PWM_C 1400

#define wifi_ciclos 20
#define wifi_menor 10

//Correcção de bug no wifi no ficheiro WiFiGeneric.cpp
/*
bool WiFiGenericClass::mode(wifi_mode_t m)
{
    wifi_mode_t cm = getMode();
    if(cm == m) {
        return true;
    }
    if(!cm && m){
        if(!espWiFiStart(_persistent)){
            return false;
        }
    } else if(cm && !m){
        return espWiFiStop();
    }
    sleep(1);  // Delay between espWiFiStart() and esp_wifi_set_mode() to correct bug
 
    esp_err_t err;
    err = esp_wifi_set_mode(m);
    if(err){
        log_e("Could not set mode! %d", err);
        return false;
    }
    return true;
}
*/





//Variaveis globais

volatile int conta_interrupt, conta_fim;

//hw_timer_t *timerx1 = NULL;
//portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

//hw_timer_t * timerx2 = NULL;

long timer1, timer2, timer3, timer4, timer5, timer6, timer7, timer8 = 0;
long timersaidapwm = 0;
int leitura_piloto = 0;
int leitura_piloto_max = 4096;
int leitura_piloto_min = 0;
int leitura_piloto_max2 = 4096;
int leitura_piloto_min2 = 0;
int leitura_piloto_max3 = 4096;
int leitura_piloto_min3 = 0;

int posicao = 0;
int leituras[100];
int leiturasmax[100];
int leiturasmin[100];
int i;
long temporaria;
long cycle = 0;
long auxcycle;

bool pisca = false;
bool piscar = false;
int amperes = 6;
int amperes_temp = 6;
int estado = 0;
int estado_anterior = 0;
float percent_pwm = 10.0;
char linha1[10];
char linha2[10];
char linha3[10];
float aux = 0.0;
float auxpwm = 0.0;

char buffer[10];
int tempocarga = 0;
int janelamax = 0;
int janelamin = 0;
int touch0;
int toque = 0;
int toque_anterior = 0;
int menu = 0;
bool bloquear = false;
bool escolher_amperes = false;
bool testar_piloto = false;
bool screensaver = false;
bool memorizar = false;
unsigned int contadisplay = 0;
int teste_piloto = 0;
float pwm_anterior = -1.0;
bool inibir_carga = false;
bool errodiodo_paracarga = true;
bool escolher_diodo = false;
bool luzes_ss = true;
bool escolher_luzes = false;
bool remote = true;
bool escolher_remote = false;
bool teste_pwm_inibir_errodiodo = false;
bool erro_diodo = false;
int wifi_count = 0;
bool ciclo_estadoB = false;
bool jafezcicloB = false;
bool condA = false;
bool condB = false;
bool condC = false;
bool condD = false;
bool condE = false;

const char* ssid     = "EVSEServer";
const char* password = "zzzzzzzz";

/* Put IP Address details */
IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

// Variable to store the HTTP request
String header;

// Auxiliar variables to store the current output state
String output26State = "off";
String output27State = "off";




// INICIO


//***********************FUNCOES******************************************

/*
void IRAM_ATTR onTimer()
{
  portENTER_CRITICAL_ISR(&timerMux);
  if ((conta_interrupt == 0) & (conta_fim != 0))
    digitalWrite(pino_pwm, true);
  if (conta_interrupt == conta_fim)
    digitalWrite(pino_pwm, false);
  conta_interrupt = conta_interrupt + 1;
  if (conta_interrupt > 99)
    conta_interrupt = 0;
  portEXIT_CRITICAL_ISR(&timerMux);
}
*/


void saida_pwm(float valor_em_percentagem)
{

  int aux33;
  float factor33;
  //float factor44;
  //int aux44;

  // 8 bits  2.55
  // 9 bits  5.11
  //10 bits 10.23
  //11 bits 20.47
  //12 bits 40.95 
  //13 bits 81.91
  //14 bits 163.83
  //15 bits 327.67
  factor33 = ( pow (2.0,  double(bits_pwm))/*-1*/) / 100.0;
  aux33 = int(valor_em_percentagem * factor33);


  //factor44 = valor_em_percentagem * 1.0;
  if (valor_em_percentagem > 99.9) {
    
  //  factor44 = 100.0;
    factor33 = 100.0;
  }
  //aux44 = int(factor44) + 1;
  //if (valor_em_percentagem < 9.9)
  //  aux44 = 0;

 

  //se houver mudanças
  if (valor_em_percentagem != pwm_anterior)
  {
  
    Serial.print("factor33:");
    Serial.println(factor33);
    Serial.print("aux33:");
    Serial.println(aux33);
    //Serial.print("ledcwrite:");
    //Serial.println(int(factor33 * 100.0));

    //portENTER_CRITICAL(&timerMux);
    //conta_fim = aux44;
    //portEXIT_CRITICAL(&timerMux);

    ledcWrite(canalpwm, aux33);
    //ledcWrite(canalpwm, int(factor33 * 100.0));
    //ledcDetachPin(pino_pwm);
    //delayMicroseconds(500);
    //ledcAttachPin(pino_pwm, canalpwm);
   
  }

   pwm_anterior = valor_em_percentagem;
}

struct timer
{                   //This structure defines a new variable type that consists on:
  boolean in;       //IN: a digital input, that starts the timing after it is at ON state;
  unsigned long pt; //PT: after PT-time, output Q shall trigger to ON state; (Preset Time)
  unsigned long et; //ET: this is the amount of time after IN has gone to ON; (Elapsed Time)
  boolean q;        //Q: the timer output, that would be ON after ET=PT.
};
unsigned long lastTime; //it must be long to receive the output of millis()

#define MAXT 50 //how many timers I have in the program (or more)
timer t[MAXT];  //Create x timers

void timers_manager()
{ //routine to manage all the timers
  byte i;
  // timers
  for (i = 0; i < MAXT; i++)
  {
    if (!t[i].in)
    {                    //if input is off
      t[i].q = false;    //put output off
      t[i].et = t[i].pt; //preload the "elapsed time" with "preset time". Nota that the count is towards zero.
    }
    t[i].q = (t[i].in && (t[i].et == 0)); //output is high when "et" arrives to zero (and "in" is 1)
  }

  if (millis() >= lastTime)
  {                            //lastTime is put to xxx ms forward compared to the present time. When the present time (millis()) reach it
    lastTime = millis() + 1;   //we move it forward again and
    for (i = 0; i < MAXT; i++) // for each timer
      if (t[i].in && t[i].et > 0)
        t[i].et--; //decrement the counting value ElapsedTime if the in is acrive and if et>0.
  }
}

//**********************************************WEbserver**************


String SendHTML(int indice,int valor){
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>EVSE WebServer</title>\n";
  ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr +="body{margin-top: 5px;} h1 {color: #444444;margin: 10px auto 10px;} h3 {color: #444444;margin-bottom: 10px;}\n";
  ptr +=".button {display: block;width: 80px;background-color: #3498db;border: none;color: white;padding: 10px 10px;text-decoration: none;font-size: 16px;margin: auto auto auto;cursor: pointer;border-radius: 4px;}\n";
  ptr +=".button-on {background-color: #3498db;}\n";
  ptr +=".button-on:active {background-color: #2980b9;}\n";
  ptr +=".button-off {background-color: #34495e;}\n";
  ptr +=".button-off:active {background-color: #2c3e50;}\n";
  ptr +="p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  ptr +="table, th, td {\n  border: 1px solid black;\n  border-collapse: collapse;\n";
  ptr +="</style>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<p><font size=\"5\">EVSE Web Server</font></p>";
  ptr +="<font size=\"4\">Corrente</font>";
  ptr +="<table style=\"width:100%\">\n  <tr>\n    <th valign=\"middle\" colspan=\"2\"><font size=\"6\"><b>";
  if (amperes==0) ptr +="OFF ";
  ptr +=String(amperes);
  ptr +="A</b></font></th> \n    <th>";
  ptr +="<a class=\"button button-off\" href=\"/refresh\">Ler</a> </th>\n  </tr>\n";
  ptr +="<tr>\n<td><a class=\"button button-on\" href=\"/maisC\">Mais</a></td>\n";
  ptr +="<td><a class=\"button button-on\" href=\"/menosC\">Menos</a></td>\n";
  ptr +="<td><a class=\"button button-off\" href=\"/memorC\">Gravar</a></td></tr>\n";
  ptr +="</table>\n";
  
  ptr +="<br>\n";
  ptr +="<font size=\"4\">Controlo remoto</font>";
  ptr +="<table style=\"width:100%\">\n  <tr>\n    <th valign=\"middle\" colspan=\"2\"><font size=\"6\"><b>";
  if (remote==false) ptr +="Desligado";
  if (remote==true) ptr +="Ligado";
  ptr +="</b></font></th> \n    <th>";
  ptr +="<a class=\"button button-off\" href=\"/refresh\">Estado</a> </th>\n  </tr>\n";
  ptr +="<tr>\n<td><a class=\"button button-on\" href=\"/ligarremote\">Ligar</a></td>\n";
  ptr +="<td><a class=\"button button-on\" href=\"/desligarremote\">Desligar</a></td>\n";
  ptr +="<td><a class=\"button button-off\" href=\"/memorremote\">Gravar</a></td></tr>\n";
  ptr +="</table>\n";


  //ptr +=String("<h3>Version ")+String(VERSAO)+String(" Beta</h3>\n");
  //ptr +=String("<p><Corrente: ")+String(amperes)+String("A</p>");
  //ptr +="<h3>Corrente: ";
  //if (amperes==0) ptr +="OFF ";
  //ptr +=String(amperes);
  //ptr +="A</h3>\n";
  //ptr +="<a class=\"button button-on\" href=\"/maisC\">Mais</a>\n";
  //ptr +="<a class=\"button button-on\" href=\"/menosC\">Menos</a>\n";
  //ptr +="<a class=\"button button-off\" href=\"/refresh\">Actualizar</a>\n";
 

  ptr +=String("<p>\nVersion ")+String(VERSAO)+String(" Beta</p>");
  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}


void handle_OnConnect() {
  Serial.println("Handle OnConnect");
  
  servidor.send(200, "text/html", SendHTML(1,1)); 
}

void handle_mais_corrente() {
  Serial.println("Handle Mais corrente");

  amperes = amperes + 1;
  if (amperes > 32) amperes = 0;
  if (amperes == 1) amperes = 6;

  servidor.send(200, "text/html", SendHTML(2,1)); 
}

void handle_menos_corrente() {
  Serial.println("Handel Menos corrente");

  amperes = amperes - 1;
  if (amperes == -1) amperes = 32;
  if (amperes < 6) amperes = 0;
  

  servidor.send(200, "text/html", SendHTML(2,2)); 
}


void handle_ligar_remote() {
  Serial.println("Handle ligar remote");

  remote = true; 
  
  servidor.send(200, "text/html", SendHTML(4,1)); 
}

void handle_desligar_remote() {
  Serial.println("Handle desligar remote");

  remote = false; 
  
  servidor.send(200, "text/html", SendHTML(4,2)); 
}




void handle_corrente0() {
  if (remote) amperes = 0;
  servidor.send(200, "text/html", SendHTML(3,0)); 
  wifi_count = wifi_ciclos;
}
void handle_corrente6() {
  if (remote) amperes = 6;
  servidor.send(200, "text/html", SendHTML(3,0)); 
  wifi_count = wifi_ciclos;
}
void handle_corrente7() {
  if (remote) amperes = 7;
  servidor.send(200, "text/html", SendHTML(3,0)); 
  wifi_count = wifi_ciclos;
}
void handle_corrente8() {
  if (remote) amperes = 8;
  servidor.send(200, "text/html", SendHTML(3,0)); 
  wifi_count = wifi_ciclos;
}
void handle_corrente9() {
  if (remote) amperes = 9;
  servidor.send(200, "text/html", SendHTML(3,0)); 
  wifi_count = wifi_ciclos;
}
void handle_corrente10() {
  if (remote) amperes = 10;
  servidor.send(200, "text/html", SendHTML(3,0)); 
  wifi_count = wifi_ciclos;
}
void handle_corrente11() {
  if (remote) amperes = 11;
  servidor.send(200, "text/html", SendHTML(3,0)); 
  wifi_count = wifi_ciclos;
}
void handle_corrente12() {
  if (remote) amperes = 12;
  servidor.send(200, "text/html", SendHTML(3,0)); 
  wifi_count = wifi_ciclos;
}
void handle_corrente13() {
  if (remote) amperes = 13;
  servidor.send(200, "text/html", SendHTML(3,0)); 
  wifi_count = wifi_ciclos;
}
void handle_corrente14() {
  if (remote) amperes = 14;
  servidor.send(200, "text/html", SendHTML(3,0)); 
  wifi_count = wifi_ciclos;
}
void handle_corrente15() {
  if (remote) amperes = 15;
  servidor.send(200, "text/html", SendHTML(3,0)); 
  wifi_count = wifi_ciclos;
}
void handle_corrente16() {
  if (remote) amperes = 16;
  servidor.send(200, "text/html", SendHTML(3,0)); 
  wifi_count = wifi_ciclos;
}
void handle_corrente17() {
  if (remote) amperes = 17;
  servidor.send(200, "text/html", SendHTML(3,0)); 
  wifi_count = wifi_ciclos;
}
void handle_corrente18() {
  if (remote) amperes = 18;
  servidor.send(200, "text/html", SendHTML(3,0)); 
  wifi_count = wifi_ciclos;
}
void handle_corrente19() {
  if (remote) amperes = 19;
  servidor.send(200, "text/html", SendHTML(3,0)); 
  wifi_count = wifi_ciclos;
}
void handle_corrente20() {
  if (remote) amperes = 20;
  servidor.send(200, "text/html", SendHTML(3,0)); 
  wifi_count = wifi_ciclos;
}
void handle_corrente21() {
  if (remote) amperes = 21;
  servidor.send(200, "text/html", SendHTML(3,0)); 
  wifi_count = wifi_ciclos;
}
void handle_corrente22() {
  if (remote) amperes = 22;
  servidor.send(200, "text/html", SendHTML(3,0)); 
  wifi_count = wifi_ciclos;
}
void handle_corrente23() {
  if (remote) amperes = 23;
  servidor.send(200, "text/html", SendHTML(3,0)); 
  wifi_count = wifi_ciclos;
}
void handle_corrente24() {
  if (remote) amperes = 24;
  servidor.send(200, "text/html", SendHTML(3,0)); 
  wifi_count = wifi_ciclos;
}
void handle_corrente25() {
  if (remote) amperes = 25;
  servidor.send(200, "text/html", SendHTML(3,0)); 
  wifi_count = wifi_ciclos;
}
void handle_corrente26() {
  if (remote) amperes = 26;
  servidor.send(200, "text/html", SendHTML(3,0)); 
  wifi_count = wifi_ciclos;
}
void handle_corrente27() {
  if (remote) amperes = 27;
  servidor.send(200, "text/html", SendHTML(3,0)); 
  wifi_count = wifi_ciclos;
}
void handle_corrente28() {
  if (remote) amperes = 28;
  servidor.send(200, "text/html", SendHTML(3,0)); 
  wifi_count = wifi_ciclos;
}
void handle_corrente29() {
  if (remote) amperes = 29;
  servidor.send(200, "text/html", SendHTML(3,0)); 
  wifi_count = wifi_ciclos;
}
void handle_corrente30() {
  if (remote) amperes = 30;
  servidor.send(200, "text/html", SendHTML(3,0)); 
  wifi_count = wifi_ciclos;
}
void handle_corrente31() {
  if (remote) amperes = 31;
  servidor.send(200, "text/html", SendHTML(3,0)); 
  wifi_count = wifi_ciclos;
}
void handle_corrente32() {
  if (remote) amperes = 32;
  servidor.send(200, "text/html", SendHTML(3,0)); 
  wifi_count = wifi_ciclos;
}


void handle_memo_corrente() {
  Serial.println("Handle Memo corrente");
  
  EEPROM.write(0, amperes);
  EEPROM.commit();
  
  servidor.send(200, "text/html", SendHTML(2,3)); 
  
}


void handle_memo_remote() {
  Serial.println("Handle Memo remote");
  

  if (remote)
        EEPROM.write(3, 1);
               else
                  EEPROM.write(3, 0);

                      EEPROM.commit();

  
  servidor.send(200, "text/html", SendHTML(4,3)); 
  
}


void handle_refresh() {
  Serial.println("Handle Refresh");
  servidor.send(200, "text/html", SendHTML(3,1)); 
  wifi_count = wifi_ciclos;
}


void handle_NotFound(){
  servidor.send(404, "text/plain", "Not found");
  wifi_count = wifi_ciclos;
}




//***********************FIM FUNCOES******************************************

void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Serial.println("Init");

  pinMode(18, OUTPUT);
  //portENTER_CRITICAL(&timerMux);
  //conta_interrupt = 0;
  //conta_fim = 10;
  //portEXIT_CRITICAL(&timerMux);

  //timerx1 = timerBegin(0, 80, true);
  //timerAttachInterrupt(timerx1, &onTimer, true);
  //timerAlarmWrite(timerx1, 10, true);
  //timerAlarmEnable(timerx1);

  //timerx2 = timerBegin(0, 80, true);
  //timerAttachInterrupt(timerx2, &onTimerDesliga, true);
  //timerAlarmWrite(timerx2, 1000000, true);
  //timerAlarmEnable(timerx2);

  // Initialising the UI will init the display too.
  display.init();

  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);

  display.clear();

  pinMode(pino_LED, OUTPUT);
  digitalWrite(pino_LED, true);

  pinMode(pino_rele, OUTPUT);
  digitalWrite(pino_rele, false);

  // configure PWM functionalitites
  ledcSetup(canalpwm, 1000, bits_pwm);

  // attach the channel to the GPIO to be controlled
  ledcAttachPin(pino_pwm, canalpwm);

  pinMode(pino_leitura_piloto, INPUT_PULLDOWN);
  digitalWrite(pino_leitura_piloto, false);

  display.setFont(Monospaced_plain_20); //fonte w12xh24
  display.setTextAlignment(TEXT_ALIGN_LEFT);

  display.drawString(4, 0, String(" Welcome! "));
  display.drawString(4, 44, String("EVSE ")+String(VERSAO));
  display.display();

  t[1].pt = 50;    //preset time
  t[2].pt = 100;   //preset time   100
  t[3].pt = 500;   //erro diodo
  t[4].pt = 100;   //preset time
  t[5].pt = 50;    //touch rápido
  t[6].pt = 1000;  //touch confirmação
  t[7].pt = 60000; //screensaver
  t[8].pt = 1000;  //tempo duplo clique
  t[9].pt = 30000; //timeout menus
  t[10].pt = 2500; //timer relé
  t[11].pt = 50;   //timer desligar relé
  t[12].pt = 250;  //timer erro geral
  t[13].pt = 100;  //timer entrada pwm diodo
  t[14].pt = 100;  //timer entrada pwm diodo
  t[15].pt = 1000; //timer arranque contadisplay
  t[16].pt = 1500; //timer para detectar falha curta de wifi
  t[17].pt = 5000; //timer para detectar falha longa de wifi
  t[18].pt = 2000; //ciclo estado B -12V
  t[19].pt = 2000; //estado 11
  t[20].pt = 5000; //falha no estado 11, regressa ao 0
  t[21].pt = 2000; //estado b1
  t[22].pt = 2000; //estado b2
  t[23].pt = 500; //tempo mínimo no estado 21

  EEPROM.begin(10);

  amperes = EEPROM.read(0);
  if (EEPROM.read(1) == 0)
    errodiodo_paracarga = false;
  else
    errodiodo_paracarga = true;
  
  if (EEPROM.read(2) == 0)
    luzes_ss = false;
  else
    luzes_ss = true;

   if (EEPROM.read(3) == 0)
    remote = false;
  else
    remote = true; 


  if (amperes > 32)
    amperes = 6;

// Connect to Wi-Fi network with SSID and password
  Serial.print("Setting AP (Access Point)…");
  // Remove the password parameter, if you want the AP (Access Point) to be open
  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  delay(100);



  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  
  servidor.on("/", handle_OnConnect);
  servidor.on("/maisC", handle_mais_corrente);
  servidor.on("/menosC", handle_menos_corrente);
  servidor.on("/refresh", handle_refresh);
  servidor.on("/memorC", handle_memo_corrente);
  servidor.on("/0", handle_corrente0);
  servidor.on("/6", handle_corrente6);
  servidor.on("/7", handle_corrente7);
  servidor.on("/8", handle_corrente8);
  servidor.on("/9", handle_corrente9);
  servidor.on("/10", handle_corrente10);
  servidor.on("/11", handle_corrente11);
  servidor.on("/12", handle_corrente12);
  servidor.on("/13", handle_corrente13);
  servidor.on("/14", handle_corrente14);
  servidor.on("/15", handle_corrente15);
  servidor.on("/16", handle_corrente16);
  servidor.on("/17", handle_corrente17);
  servidor.on("/18", handle_corrente18);
  servidor.on("/19", handle_corrente19);
  servidor.on("/20", handle_corrente20);
  servidor.on("/21", handle_corrente21);
  servidor.on("/22", handle_corrente22);
  servidor.on("/23", handle_corrente23);
  servidor.on("/24", handle_corrente24);
  servidor.on("/25", handle_corrente25);
  servidor.on("/26", handle_corrente26);
  servidor.on("/27", handle_corrente27);
  servidor.on("/28", handle_corrente28);
  servidor.on("/29", handle_corrente29);
  servidor.on("/30", handle_corrente30);
  servidor.on("/31", handle_corrente31);
  servidor.on("/32", handle_corrente32);
  servidor.on("/ligarremote", handle_ligar_remote);
  servidor.on("/desligarremote", handle_desligar_remote);
  servidor.on("/memorremote", handle_memo_remote);



  servidor.onNotFound(handle_NotFound);
  
  servidor.begin();
  Serial.println("HTTP server started");


  //server.begin();


  delay(1000);

  Serial.println("Finished Setup");
}

void loop()
{
  // clear the display
  //display.clear();
  // draw the current demo method
  //demos[demoMode]();
  auxcycle = micros();
  timers_manager();

  //ciclo leituras

  if (millis() - timer3 > 1)
  {

    posicao = posicao + 1;
    if (posicao > 10)
      posicao = 0;

    leituras[posicao] = analogRead(pino_leitura_piloto);

    temporaria = 0;
    for (i = 0; i <= 9; i++)
    {
      temporaria = temporaria + leituras[i];
    }
    leitura_piloto = temporaria / 10;

    leiturasmax[posicao] = leitura_piloto_max2;
    temporaria = 0;
    for (i = 0; i <= 9; i++)
    {
      temporaria = temporaria + leiturasmax[i];
    }
    leitura_piloto_max3 = temporaria / 10;

    leiturasmin[posicao] = leitura_piloto_min2;
    temporaria = 0;
    for (i = 0; i <= 9; i++)
    {
      temporaria = temporaria + leiturasmin[i];
    }
    leitura_piloto_min3 = temporaria / 10;

    timer3 = millis();
  }

  //maximos e minimos piloto, fazer em todos os ciclos

  temporaria = analogRead(pino_leitura_piloto);

  if (temporaria < leitura_piloto_min)
    leitura_piloto_min = temporaria;
  if (temporaria > leitura_piloto_max)
    leitura_piloto_max = temporaria;

  //ciclo reset max e min, decisões EVSE
  if (millis() - timer4 > 100)
  {
    //actualizar máximos e mínimos
    leitura_piloto_max2 = leitura_piloto_max;
    leitura_piloto_min2 = leitura_piloto_min;
    leitura_piloto_max = leitura_piloto;
    leitura_piloto_min = leitura_piloto;


    //Condição A sem nada ligado sem erro
    if (leitura_piloto_max3 > MIN_PWM_A)    
    {
      condA = true;
    }
    else
    {
      condA = false;
      
    }
    
    //Condição B - VE ligado
    if ((leitura_piloto_max3 > MIN_PWM_B) & (leitura_piloto_max3 < MIN_PWM_A))  
    {
      condB = true;
    }
    else
    {
      condB = false;
      
    }

    //Condição C - VE pede carga
    if ((leitura_piloto_max3 > MIN_PWM_C) & (leitura_piloto_max3 < MIN_PWM_B) )  
    {
      condC = true;
    }
    else
    {
      condC = false;
      
    }

    //Condição D - Erro diodo

    if ((leitura_piloto_min3 > MAX_PWM_DIODO) & (auxpwm !=100.0))
    {
      t[3].in = true;
    }
    else
    {
      t[3].in = false;
    }

    if (t[3].q == true)
    {
      condD = true;
    }
    else
    {
      condD = false;
    }
    
    //Condição E - Erro geral
    if ((leitura_piloto_max3 < MIN_PWM_C) & (auxpwm>0.0))
    {
      t[12].in = true;
    }
    else
    {
      t[12].in = false;
    }

    if (t[12].q == true)
    {
      condE = true;
    }
    else
    {
      condE  = false;
    }



    //Estados
    // A = 0            Pronto
    // B = 11,12 e 13   VE Ligado 
    // C = 21           VE pede carga
    // D = 31           erro diodo
    // E = 41           erro geral
    

    if ( ((estado==21) || (estado==13)) &&  (condA) )
    {
    estado = 0;

    }

    if (estado==0)
    {
      if (condB) estado = 11;
      if (condC) estado = 21;
      //if ((condD) & errodiodo_paracarga) estado = 31;
      if (condE)  estado = 41;
      auxpwm = 100.0;
      digitalWrite(pino_rele, false);
      sprintf(linha1, "  Pronto  ");
      t[21].in=false;
      t[22].in=false;
    }

    if (estado == 21)
    {
      t[23].in = true;
      if (t[23].q)
      {
        if (condA) estado = 0;
        if (condB) estado = 13;
        if ((condD) & errodiodo_paracarga) estado = 31;
        if (condE)  estado = 41;
      }
      auxpwm = float(amperes) / 0.6;
      digitalWrite(pino_rele, true);
      sprintf(linha1, "A carregar");
    }
    else
      t[23].in = false;

    if (estado==11)
    {
      auxpwm = 0.0;
      digitalWrite(pino_rele, false);
      sprintf(linha1, "  Espere  ");
      t[21].in = true;
      
      if (t[21].q) estado = 12;
    }

    if (estado==12)
    {
      auxpwm = 100.0;
      digitalWrite(pino_rele, false);
      sprintf(linha1, "  Espere  ");
      t[22].in = true;
      if (t[22].q) estado = 13;
    }

    if (estado==13)
    {
      if (condA) estado = 0;
      if (condC) estado = 21;
      if ((condD) & errodiodo_paracarga) estado = 31;
      if (condE)  estado = 41;
      sprintf(linha1, "VE Ligado ");
      auxpwm = float(amperes) / 0.6;
      digitalWrite(pino_rele, false);
    }

    if (estado==31)  //Erro diodo
    {
      if (condA & !condB & !condC) estado = 0;
      sprintf(linha1, "Erro diodo");
      auxpwm = 100.0;
      digitalWrite(pino_rele, false);
    }

    if (estado==41)  //Erro geral, só é possível voltar ao zero se condição desaparecer
    {
      if (condA & !condE) estado = 0;
      sprintf(linha1, "Erro geral");
      auxpwm = 100.0;
      digitalWrite(pino_rele, false);
    }


      //barras estado
      janelamin = int((float(leitura_piloto_min3) - 700.0) / 7.0);
      if (estado == 0)
        janelamin = 100;
      if (janelamin > 100)
        janelamin = 100;
      if (janelamin < 0)
        janelamin = 0;

      if (estado == 0)
        janelamax = int((float(leitura_piloto_max3) - 4000.0) / 2.0);
      if (estado == 1)
        janelamax = int((float(leitura_piloto_max3) - 3050.0) / 5.5);
      if (estado == 2)
        janelamax = int((float(leitura_piloto_max3) - 2700.0) / 3.5);

      if (janelamax > 100)
        janelamax = 100;
      if (janelamax < 0)
        janelamax = 0;

      timer4 = millis();
    }

    // Sem carro ligado, Estado A, deverá manter o piloto a 12V positivos
    // para 12V positivos entre 4000 e 4096
    // para 12V negativos abaixo de 1400

    // Estado B, carro ligado, insere 2.7kOhm, EVSE deverá oscilar a 1000Hz
    // Os 12V positivos deverão baixar para 9V, à volta de 3300
    // Os 12V negativos deverão continuar a -12V (-9V à volta de 1300)

    // Estado C, carro ligado a pedir carga, insere 882Ohm, EVSE deverá oscilar a 1000Hz
    // Os 12V positivos deverão baixar para 6V, à volta de 2900
    // Os 12V negativos deverão continuar a -12V (-6V à volta de ) 1600

    //Decisão estado
    //O mínimo tem de estar abaixo de 1100, se não estiver ERRO (excepto no estado A)
    //O estado depende do máximo
    // > 3600 -> Estado A
    // entre 3000 e 3600 -> Estado B
    // entre 1100 e 3000 -> Estado C
    // máximo abaixo de 1100 dá erro



    if (inibir_carga)
      auxpwm = 100.0;
    if (amperes == 0)
      auxpwm = 100.0;

    //se testes piloto

    if (teste_piloto == 1)
      auxpwm = 10.0 / 0.6; //PWM 10A
    if (teste_piloto == 2)
      auxpwm = 100.0; //+12V
    if (teste_piloto == 3)
      auxpwm = 0.0; //-12V


    
    saida_pwm(auxpwm);

        
    if ((teste_piloto != 0) | (inibir_carga) | (amperes == 0))
    {
      teste_pwm_inibir_errodiodo = true;
    }
    else
    {
      teste_pwm_inibir_errodiodo = false;
    }
    
    

    //Verificar falha de comunicação em modo remote ********************************

    if (remote) {
      //se não receber nada durante 2s reduz corrente para 6A se actualmente for maior -t16
      if ((wifi_count==0) & (amperes >6)) t[16].in=true; else t[16].in=false;
      if (t[16].q) amperes = 6;


      //se não receber nada durante 5s desliga - t17
      if (wifi_count==0) t[17].in=true; else t[17].in=false;
      if (t[17].q) amperes = 0;
    }


    //ciclo de update do display
    if (millis() - timer2 > 20)
    {
      if (contadisplay==0) t[15].in=true; else t[15].in=false;
      if (contadisplay >=1) contadisplay = contadisplay + 1;
      if (t[15].q) contadisplay = 1;
      
      display.clear();
      display.setFont(Monospaced_plain_20); //fonte w12xh24
      display.setTextAlignment(TEXT_ALIGN_LEFT);

      wifi_count = wifi_count -1;
      if (wifi_count < 0) wifi_count = 0;

      if (!screensaver)
      {
        display.drawString(4, 0, String(linha1));

        
        if (digitalRead(pino_rele))
          display.drawXbm(63, 23, fechado2_width, fechado2_height, fechado2_bits);
        else
          display.drawXbm(63, 23, aberto2_width, aberto2_height, aberto2_bits);
        

        //Simbolo carro
        if (condB || condC)
          display.drawXbm(30, 23, carro_width, carro_height, carro_bits);
        
        // Simbolo de erro
        if (((estado == 31) | (estado==41)) & piscar)
          display.drawXbm(9, 23, perigo_width, perigo_height, perigo_bits);

        

        
        if (wifi_count > wifi_menor)
          display.drawXbm(90, 28, wifi_width, wifi_height, wifi_bits);
        
    

        if (remote)
          display.drawXbm(110, 24, remote_width, remote_height, remote_bits);
        

        //sprintf(buffer,"%ds",tempocarga);
        //display.drawString(4, 22, String(buffer));
        //display.drawString(4, 22, String(leitura_piloto));

        //                      x   y    w   h  %
        //display.drawProgressBar(0, 44, 34, 8, janelamin);
        //display.drawProgressBar(0, 53, 34, 8, janelamax);

        //display.drawString(64, 22, String(leitura_piloto_max3));
        //display.drawString(4, 22, String(leitura_piloto_min3));
        //display.drawString(64, 44, String(estado));

        //display.drawString(4, 0, String(auxpwm));
        
      }
      else //caso esteja com screen saver ligado
      {
        //se luzes ss ligadas vai piscar ecran
        //caso não esteja a carregar

        //calculo de desenho

        display.setFont(Monospaced_plain_32); //fonte w19xh38
        display.setTextAlignment(TEXT_ALIGN_LEFT);

        if ((estado != 2) & (luzes_ss))
        {
          if (((millis() % 3000) >= 0) & ((millis() % 3000) <= 25))
          {
            display.fillRect(0, 0, 128, 64);
          }

          if (((millis() % 3000) >= 25) & ((millis() % 3000) <= 75))
          {
            sprintf(buffer, "%2dA", amperes);
            display.drawString(64, 12, String(buffer));
            if ((estado != 0) & (estado != 3)) display.drawXbm(10, 23, carro_width, carro_height, carro_bits);
            
          }
        }

        if ((estado == 2) & (luzes_ss))
        {
          if (((millis() % 1000) >= 0) & ((millis() % 1000) <= 25))
          {
            display.fillRect(0, 0, 128, 64);
          }

          if (((millis() % 1000) >= 25) & ((millis() % 1000) <= 75))
          {
            sprintf(buffer, "%2dA", amperes);
            display.drawString(64, 12, String(buffer));
            display.drawXbm(0, 23, carro_width, carro_height, carro_bits);
            display.drawXbm(33, 23, fechado2_width, fechado2_height, fechado2_bits);
          }
        }
      }


      touch0 = touchRead(4);
      //display.drawString(64, 22, String(touch0));
      if (touch0 < 20)
      {
        t[5].in = true;
        t[6].in = true;
      }
      else
      {
        t[5].in = false;
        t[6].in = false;
        toque = 0;
        bloquear = false;
      }
      //se toque rápido
      if ((t[5].q) & !(t[6].q))
      {
        digitalWrite(pino_LED, false);
        display.drawXbm(0, 24, mao_width, mao_height, mao_bits);
        contadisplay = 0;
        //display.drawString(48, 22, String("#"));
        toque = 1;
      }
      else
      {
        digitalWrite(pino_LED, true);
      }
      //se toque confirmacao
      if (t[6].q)
      {
        digitalWrite(pino_LED, piscar);
        if (piscar)
          display.drawXbm(0, 24, mao_width, mao_height, mao_bits); //display.drawString(48, 22, String("#"));
        toque = 2;
      }
      else
      {
      }

      //Duplo clique,

      //se vier novo clique dentro do timer
      if ((toque == 0) & (toque_anterior == 1) & (menu == 0) & (!bloquear) & (t[8].in == true) & (t[8].et > 0))
      {
        Serial.println("Duplo Click no menu principal");
        menu = 1;
        memorizar = false;
        amperes_temp = amperes;
        escolher_amperes = true;
        toque_anterior = -1;
      }
      //arranca timer
      if ((toque == 0) & (toque_anterior == 1) & (menu == 0) & (!bloquear))
      {
        t[8].in = true;
      }
      //reset ao timer
      if (t[8].q)
        t[8].in = false;

      if ((toque == 0) & (menu == 0) & (!screensaver))
      {
        if (amperes != 0)
        {
          sprintf(buffer, "%02dA", amperes);
          display.drawString(84, 44, String(buffer));
        }
        else
        {
          sprintf(buffer, "OFF 00A");
          display.drawString(36, 44, String(buffer));
        }
      }

      //display.setTextAlignment(TEXT_ALIGN_RIGHT);
      //display.drawString(124, 44, String(cycle));

      //Menus
      if ((toque == 1) & (menu == 0) & (!bloquear))
      {
        display.drawString(0, 44, String("Ir p/ Menu"));
      }

      if ((menu == 1) & ((toque == 0) || (toque == 1)) & (!bloquear) & (!escolher_amperes))
      {

        display.drawString((0 + (-1 * (contadisplay % 144))), 44, String("Memorizar Corrente"));
        if ((contadisplay % 144) == 0) contadisplay = 0;
      }

      if ((menu == 1) & (escolher_amperes) & (!bloquear))
      {
        if (piscar)
        {

          if (amperes_temp != 0)
          {
            sprintf(buffer, "%02dA", amperes_temp);
            display.drawString(84, 44, String(buffer));
          }
          else
          {
            sprintf(buffer, "OFF 00A");
            display.drawString(36, 44, String(buffer));
          }
        }
      }

      if ((menu == 2) & ((toque == 0) || (toque == 1)) & (!bloquear))
      {
        display.drawString(0, 44, String("Sair"));
      }

      if ((menu == 3) & ((toque == 0) || (toque == 1)) & (!bloquear) & (!testar_piloto))
      {
        display.drawString(0, 44, String("Testar CP"));
      }

      if ((menu == 4) & ((toque == 0) || (toque == 1)) & (!bloquear) & (!escolher_diodo))
      {
        //display.drawString(0, 44, String("DiodeCheck"));
        display.drawString((0 + (-1 * (contadisplay % 144))), 44, String("Verificação Díodo"));
      }

      if ((menu == 5) & ((toque == 0) || (toque == 1)) & (!bloquear) & (!escolher_luzes))
      {
        //display.drawString(0, 44, String("DiodeCheck"));
        display.drawString((0 + (-1 * (contadisplay % 144))), 44, String("Luzes Screensaver"));
      }

      if ((menu == 6) & ((toque == 0) || (toque == 1)) & (!bloquear) & (!escolher_remote))
      {
        //display.drawString(0, 44, String("DiodeCheck"));
        display.drawString((0 + (-1 * (contadisplay % 144))), 44, String("Controlo Remoto"));
      }

      if ((menu == 3) & (testar_piloto) & (!bloquear))
      {
        if (piscar)
        {
          if (teste_piloto == 0)
            sprintf(buffer, " Normal");
          if (teste_piloto == 1)
            sprintf(buffer, "PWM 10A");
          if (teste_piloto == 2)
            sprintf(buffer, "   +12V");
          if (teste_piloto == 3)
            sprintf(buffer, "   -12V");
          display.drawString(36, 44, String(buffer));
        }
      }

      if ((menu == 4) & (escolher_diodo) & (!bloquear))
      {
        if (piscar)
        {
          if (errodiodo_paracarga == true)
            sprintf(buffer, " Normal");
          if (errodiodo_paracarga == false)
            sprintf(buffer, "Inibido");

          display.drawString(36, 44, String(buffer));
        }
      }

      if ((menu == 5) & (escolher_luzes) & (!bloquear))
      {
        if (piscar)
        {
          if (luzes_ss == true)
            sprintf(buffer, "   Ligadas");
          if (luzes_ss == false)
            sprintf(buffer, "Desligadas");

          display.drawString(4, 44, String(buffer));
        }
      }

      if ((menu == 6) & (escolher_remote) & (!bloquear))
      {
        if (piscar)
        {
          if (remote == true)
            sprintf(buffer, "    Ligado");
          if (remote == false)
            sprintf(buffer, " Desligado");

          display.drawString(4, 44, String(buffer));
        }
      }

      if (bloquear)
      {
        display.drawString(0, 44, String("Confirmado"));
        contadisplay = -1;
      }

      //Entrar no menu, toque confirmacao
      if ((toque == 2) & (toque_anterior == 1) & (menu == 0) & (!bloquear))
      {
        menu = 1;
        toque = 0;
        toque_anterior = 0;
        bloquear = true;
      }

      //
      //Dentro dos menus

      if (menu == 1)
      {
        //toque rapido
        if ((toque == 0) & (toque_anterior == 1) & (!bloquear) & (!escolher_amperes))
        {
          menu = 2;
          toque_anterior = -1;
          Serial.println("Menu 1, toque rapido menu seguinte");
        }

        if ((toque == 0) & (toque_anterior == 1) & (!bloquear) & (escolher_amperes))
        {
          amperes_temp = amperes_temp + 1;
          if (amperes_temp > 32)
            amperes_temp = 0;
          if (amperes_temp == 1)
            amperes_temp = 6;
          toque_anterior = -1;
          Serial.println("Menu 1, toque rapido escolher amperes");
        }

        //toque confirmacao
        if ((toque == 2) & (toque_anterior == 1) & (!bloquear) & (!escolher_amperes))
        {
          toque = 0;
          toque_anterior = 0;
          escolher_amperes = true;
          amperes_temp = amperes;
          memorizar = true;
          bloquear = true;
          Serial.println("Menu 1, entrada na escolha corrente");
        }

        //toque confirmacao
        if ((toque == 2) & (toque_anterior == 1) & (!bloquear) & (escolher_amperes))
        {
          toque = 0;
          toque_anterior = 0;
          amperes = amperes_temp;
          escolher_amperes = false;

          if (!memorizar)
            menu = 0;

          if (memorizar)
          {
            EEPROM.write(0, amperes);
            EEPROM.commit();
            memorizar = false;
          }
          bloquear = true;
          Serial.println("Menu 1, saida da escolha de corrente");
        }
      }

      if (menu == 2)
      {
        //toque rapido
        if ((toque == 0) & (toque_anterior == 1) & (!bloquear))
        {
          menu = 3;
          toque_anterior = -1;
          Serial.println("Menu 2, toque rapido menu seguinte");
        }

        //toque confirmacao
        if ((toque == 2) & (toque_anterior == 1) & (!bloquear))
        {
          menu = 0;
          toque = 0;
          toque_anterior = 0;
          bloquear = true;
        }
      }

      if (menu == 3)
      {
        //toque rapido
        if ((toque == 0) & (toque_anterior == 1) & (!bloquear) & (!testar_piloto))
        {
          menu = 4;
          toque_anterior = -1;
          Serial.println("Menu 3, toque rapido menu seguinte");
        }

        if ((toque == 0) & (toque_anterior == 1) & (!bloquear) & (testar_piloto))
        {
          teste_piloto = teste_piloto + 1;
          if (teste_piloto > 3)
            teste_piloto = 0;
          toque_anterior = -1;
          Serial.println("Menu 3, toque rapido escolher teste");
        }

        //toque confirmacao
        if ((toque == 2) & (toque_anterior == 1) & (!bloquear) & (!testar_piloto))
        {
          toque = 0;
          toque_anterior = 0;
          testar_piloto = true;
          bloquear = true;
          Serial.println("Menu 3, entrada na escolha testar piloto");
        }

        //toque confirmacao
        if ((toque == 2) & (toque_anterior == 1) & (!bloquear) & (testar_piloto))
        {
          toque = 0;
          toque_anterior = 0;
          testar_piloto = false;
          teste_piloto = 0;

          bloquear = true;
          Serial.println("Menu 3, saida da escolha de testar piloto");
        }
      }

      if (menu == 4)
      {
        //toque rapido
        if ((toque == 0) & (toque_anterior == 1) & (!bloquear) & (!escolher_diodo))
        {
          menu = 5;
          toque_anterior = -1;
          Serial.println("Menu 4, toque rapido menu seguinte");
        }

        if ((toque == 0) & (toque_anterior == 1) & (!bloquear) & (escolher_diodo))
        {
          errodiodo_paracarga = !errodiodo_paracarga;
          toque_anterior = -1;
          Serial.println("Menu 4, toque rapido escolher opcao");
        }

        //toque confirmacao
        if ((toque == 2) & (toque_anterior == 1) & (!bloquear) & (!escolher_diodo))
        {
          toque = 0;
          toque_anterior = 0;
          escolher_diodo = true;
          bloquear = true;
          Serial.println("Menu 4, entrada na escolha diodo");
        }

        //toque confirmacao
        if ((toque == 2) & (toque_anterior == 1) & (!bloquear) & (escolher_diodo))
        {
          toque = 0;
          toque_anterior = 0;

          if (errodiodo_paracarga)
            EEPROM.write(1, 1);
          else
            EEPROM.write(1, 0);

          EEPROM.commit();

          escolher_diodo = false;

          bloquear = true;
          Serial.println("Menu 4, saida da escolha de diodo");
        }
      }

      if (menu == 5)
      {
        //toque rapido
        if ((toque == 0) & (toque_anterior == 1) & (!bloquear) & (!escolher_luzes))
        {
          menu = 6;
          toque_anterior = -1;
          Serial.println("Menu 5, toque rapido menu seguinte");
        }

        if ((toque == 0) & (toque_anterior == 1) & (!bloquear) & (escolher_luzes))
        {
          luzes_ss = !luzes_ss;
          toque_anterior = -1;
          Serial.println("Menu 5, toque rapido escolher opcao");
        }

        //toque confirmacao
        if ((toque == 2) & (toque_anterior == 1) & (!bloquear) & (!escolher_luzes))
        {
          toque = 0;
          toque_anterior = 0;
          escolher_luzes = true;
          bloquear = true;
          Serial.println("Menu 5, entrada na escolha luzes");
        }

        //toque confirmacao
        if ((toque == 2) & (toque_anterior == 1) & (!bloquear) & (escolher_luzes))
        {
          toque = 0;
          toque_anterior = 0;

          if (luzes_ss)
            EEPROM.write(2, 1);
          else
            EEPROM.write(2, 0);

          EEPROM.commit();

          escolher_luzes = false;

          bloquear = true;
          Serial.println("Menu 5, saida da escolha de luzes");
        }
      }



      if (menu ==6)
      {
        //toque rapido
        if ((toque == 0) & (toque_anterior == 1) & (!bloquear) & (!escolher_remote))
        {
          menu = 1;
          toque_anterior = -1;
          Serial.println("Menu 6, toque rapido menu seguinte");
        }

        if ((toque == 0) & (toque_anterior == 1) & (!bloquear) & (escolher_remote))
        {
          remote = !remote;
          toque_anterior = -1;
          Serial.println("Menu 6, toque rapido escolher opcao");
        }

        //toque confirmacao
        if ((toque == 2) & (toque_anterior == 1) & (!bloquear) & (!escolher_remote))
        {
          toque = 0;
          toque_anterior = 0;
          escolher_remote = true;
          bloquear = true;
          Serial.println("Menu 6, entrada na escolha remote");
        }

        //toque confirmacao
        if ((toque == 2) & (toque_anterior == 1) & (!bloquear) & (escolher_remote))
        {
          toque = 0;
          toque_anterior = 0;

          if (remote)
            EEPROM.write(3, 1);
          else
            EEPROM.write(3, 0);

          EEPROM.commit();

          escolher_remote = false;

          bloquear = true;
          Serial.println("Menu 6, saida da escolha de remote");
        }
      }

      //screen saver
      if ((toque == 0) & (estado == estado_anterior))
        t[7].in = true;
      else
        t[7].in = false;

      if (t[7].q == true)
      {
        screensaver = true;
        //Serial.println ("Screensaver on");
      }
      else
        screensaver = false;

      //timeout
      if (toque == 0)
        t[9].in = true;
      else
        t[9].in = false;

      if (t[9].q == true)
      {
        menu = 0;
        bloquear = false;
        escolher_amperes = false;
        teste_piloto = 0;
        testar_piloto = false;
      }

      //guardar toque
      toque_anterior = toque;
      estado_anterior = estado;

      // write the buffer to the display
      display.display();

      timer2 = millis();
    }

    //ciclo segundos

    if (millis() - timer1 > 1000)
    {
      if (!pisca)
        pisca = true;
      else
        pisca = false;

      if (t[4].q)
        tempocarga = tempocarga + 1;

      timer1 = millis();
    }

    //ciclo pisca 250ms

    if (millis() - timer5 > 250)
    {
      if (!piscar)
        piscar = true;
      else
        piscar = false;

      timer5 = millis();
    }

    cycle = micros() - auxcycle;


//************************* WI FI *******************************

  servidor.handleClient();


  }
