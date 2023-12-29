#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>

const char* ssid     = "SSID Wi-Fi";
const char* password = "password Wi-Fi";
char mqtt[]="адрес сервера MQTT";

int nowifi;
byte input[7]={14,21,32,33,25,26,27};
byte wig[2]={17,16};
byte rele[4]={22,23,19,18};
boolean mq=true;
int gatepoz;
byte statusg=255;
byte obn,releon,avto,lamps,intr,sec,block;
byte levelgate,gateon;
unsigned long wiegand=0;
unsigned long secund,timer,tiwi,reconn,pulse,timestat;
byte cou=26;
byte wi=0;

WiFiClient espClient;

PubSubClient mqclient(espClient);

 void IRAM_ATTR nul() {
  bitClear(wiegand,cou);
  cou--;
   //wiegand
   wi=1;
}
void IRAM_ATTR wan() {
   //wiegand
   bitSet(wiegand,cou);
   cou--;
   wi=1;
}

void receivedCallback(char* topic, byte* payload, unsigned int length) {
  String tt,top;
  int str;
  //unsigned long tr;
  Serial.print("Message received: ");
  Serial.println(topic);
 
  Serial.print("payload: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    tt+=(char)payload[i];
  }
  top=String(topic);
  str=tt.toInt();



  
            if (top.indexOf("open")>-1){
              if (gatepoz!=str){
                //подать сигнал на открытие ворот
                reley();
                levelgate=str;
                intr=1;
              }
            }
            if (top.indexOf("lamp")>-1){
           if(str==1){
            digitalWrite(rele[1],LOW);
            digitalWrite(rele[0],LOW);
            lamps=1;
            mqt(3,0,1);
           }else{
            digitalWrite(rele[1],HIGH);
            digitalWrite(rele[0],HIGH);
            mqt(3,0,0);
            lamps=0;
           }
            }
             if (top.indexOf("block")>-1){
              block=str;
			  mqt(5,0,block);
             }
       
}

void setup() {
  Serial.begin(9600);
  for (int ee=0; ee<7;ee++){
    pinMode(input[ee],INPUT_PULLUP);
  }
  for (int ee=0; ee<4;ee++){
    pinMode(rele[ee],OUTPUT);
    digitalWrite(rele[ee],HIGH);
  }
 
  pinMode(wig[0],INPUT_PULLUP);
pinMode(wig[1],INPUT_PULLUP);
  //attachInterrupt(wig[0], nul, RISING);
  //attachInterrupt(wig[1], wan, RISING);
  
  int i=0;
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      Serial.print(WiFi.status());
        delay(500);
        i++;
        if (i>60){ 
          nowifi=1;
          break;
        }
       
        //Serial.print(".");
    }
    if (nowifi==0){
      
    
     Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
      mqclient.setServer(mqtt, 1883);
     
 
  mqclient.setCallback(receivedCallback);
  mqt(99,0,1);
    }

}

void loop() {
  if (millis()-reconn>15000){
           if (WiFi.status() != WL_CONNECTED)  {
      Serial.println("reconnect:");
    WiFi.disconnect();
    WiFi.reconnect();
      mqt(99,0,1);
        }
        reconn=millis();
  }
  if (wi==1){
   tiwi=millis();
   wi=2; 
  }
  if ((wi==2)&&(millis()-tiwi>200)){
    mqt(98,0,wiegand);
    wi=0;
    wiegand=0;
  }
  if (millis()-secund>500){
    if (releon>0) {
      releoff();
    }
    sec++;
    if (sec>60){
      
    
    timestat++;
    mqt(4,0,timestat);
    sec=0;
    if (timestat>120)timestat=0;
    }
 
   mqclient.loop();
   for (int count=0; count<7; count++){
if (bitRead(statusg,count)!=digitalRead(input[count])){
  bitWrite(statusg,count,digitalRead(input[count])); // состояние пина
  bitWrite(obn,count,1); // признак обновление данного пина.
}
   }
   if (obn>0){
  if (bitRead(obn,0)==1){
    Serial.println("gateon");
    if (bitRead(statusg,0)==0){
      //нажата кнопка ворот
      Serial.print("open gate key ");
      Serial.println (bitRead(statusg,5));
      reley();
      //intr=1;
      //levelgate=100;
     if (bitRead(statusg,5)==0){
      
      //reley();
     
      gateon=2;
      
      levelgate=100;
     }
    }
  }
    if (bitRead(obn,1)==1){
      Serial.println("dindon");
    if (bitRead(statusg,1)==0){
      //нажата кнопка звонка
      mqt(1,0,1);
    }
  }
                   if (bitRead(obn,6)==1){
                  Serial.print("Closes ");
                  Serial.println(bitRead(statusg,6));
    if (bitRead(statusg,6)==0){
      gateon=1;
      
    }
    }
              if (bitRead(obn,5)==1){
                Serial.print("opener");
                Serial.println(bitRead(statusg,5));
    if (bitRead(statusg,5)==0){
      gateon=1;
      Serial.print("201p ");
      Serial.println(gateon);
     
    }
    }
      if (bitRead(obn,2)==1){
        Serial.println("openfull");
    if (bitRead(statusg,2)==1){
      //ворота полностью открыты
      gatepoz=100;
      gateon=2;
 
      mqt(2,0,gatepoz);
      intr=0;
    }
  }
        if (bitRead(obn,3)==1){
           Serial.println("closefull");
    if (bitRead(statusg,3)==1){
      //ворота полностью закрыты
      gatepoz=0;
      gateon=2;

      mqt(2,0,gatepoz);
      intr=0;
    }
  }
  

          if (bitRead(obn,4)==1){
            Serial.print("shit ");
            Serial.println(bitRead(statusg,4));
    if (bitRead(statusg,4)==0){
      //препятствие
    
      if ((gateon==1)&&(gatepoz>10)){
      
      reley();
       gateon=2;
   
      avto=1;
      Serial.println("avtostart");
      }
          if ((gatepoz==100)){
      
      
      avto=1;
      Serial.println("avtostart on stop");
      }
    }
    if ((bitRead(statusg,4)==1)&&(avto==1)&&(block<1)){
      timer=millis();
      avto=2;
      Serial.println("avtostart2");
    }
          }


      
    
  

obn=0;
   }

     
          if (gateon==1){
          Serial.println("starst ");
    if (bitRead(statusg,5)==0){
      //ворота открываются
      gatepoz=gatepoz+4;

      if (gatepoz>99) gatepoz=93;
      if ((levelgate<gatepoz)&&(intr==1)){
        //остановить ворота
        if(levelgate<90){
          reley();
        }
 
        gateon=2;

      }
      
      avto=1;
      mqt(2,0,gatepoz);
      
    }
  

          
    if (bitRead(statusg,6)==0){
      //ворота закрываются
      gatepoz=gatepoz-4;
      if (gatepoz<3) gatepoz=5;
      mqt(2,0,gatepoz);
      
    }
  }
   
   secund=millis();
   if ((avto==2)&&(millis()-timer>15000)){
    avto=0;
    reley();
   }
  }
  if ((gateon==1)||(avto==2)){
    if (millis()-pulse>2000){
      digitalWrite(rele[0],!digitalRead(rele[0]));
      pulse=millis();
    }
  }
  if (gateon==2){
    gateon=0;

    if (lamps==1){
     digitalWrite(rele[0],LOW); 
    }else{
      digitalWrite(rele[0],HIGH);
    }
  }
}


void mqt(byte reg, unsigned int nom, unsigned long state){

  if (mq){
     recon();
  String top,pay;
  top="home/gate/";
  if( reg==1){
  top+="call/";
  }
  if( reg==2){
  top+="pozition/";
  }
  if( reg==3){
  top+="lamp/";
  }
   if( reg==4){
  top+="timestat/";
  }
   if( reg==5){
  top+="block/";
  }
  if( reg==98){
  top+="wiegand/";
  }
  if( reg==99){
  top+="load/";
  }
  pay=String(state,DEC);
  char topik[30];
  //Serial.print("send");
  //Serial.println(pay);
  char payl[40];
  top.toCharArray(topik,30);
  pay.toCharArray(payl,40);


  mqclient.publish(topik,payl);
 
  
  }
 
 }
 void recon(){
  String conn;
  
  if (mq){
    
  if (!mqclient.connected()) {

 mqclient.connected();
if (mqclient.connect("gaterobot")) {
 
      mqclient.subscribe("home/gatein/#");

    }
  }
    

  }
 }

 void releoff(){
  
    digitalWrite(rele[3],HIGH);
    releon=0;
  
 }

void reley(){
  digitalWrite(rele[3],LOW);
  releon=1;
  Serial.println("reley");
}
