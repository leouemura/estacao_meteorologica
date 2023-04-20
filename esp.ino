/*
 * There are three serial ports on the ESP known as U0UXD, U1UXD and U2UXD.
 * 
 * U0UXD is used to communicate with the ESP32 for programming and during reset/boot.
 * U1UXD is unused and can be used for your projects. Some boards use this port for SPI Flash access though
 * U2UXD is unused and can be used for your projects.
 * 
*/
// Iniciamos incluindo ao programa as seguintes bibliotecas:
#include <WiFi.h> // biblioteca utilizada para que a ESP32 se conecte ao WiFi local.
#include <WiFiClientSecure.h> // biblioteca utilizada para realização de conexão segura (TLS) entre a ESP32 (Publisher) e o Subscriber
//#include <WiFiClient.h> // biblioteca utilizada para realização de conexão não segura entre a ESP32 (Publisher) e o Subscriber
#include <PubSubClient.h> // biblioteca utilizada para que a ESP32 seja capaz de publicar e se inscrever em tópicos do MQTT
#include <ArduinoJson.h> // biblioteca que possibilita a utilização da linguagem JSON e de suas propriedades 

// Em seguida, definimos as constantes relativas ao WIFI local e ao MQTT:
const char* ssid = "tplinkestmet"; // insira aqui o SSID do seu WIFI
const char* password =  "gasimet2022"; // insira aqui a SENHA do seu WIFI
const char* mqttServer = "186.217.146.134"; // insira aqui o IP ou endereço URL do Broker MQTT
const int mqttPort = 1883; // utilize a porta 8883 para conexão segura (TLS); para conexão não segura utilize a porta 1883
const char* mqttUser = ""; // insira aqui seu USUÁRIO MQTT
const char* mqttPassword = ""; // insira aqui sua SENHA MQTT

unsigned long previousMillis = 0;
unsigned long interval = 30000;

//// A seguir inserimos cada linha do certificado criptografado (ca.crt) criado a partir da configuração TLS:
//const char* local_root_ca = \ // caso sua conexão seja não segura, comente esta linha
//
//"-----BEGIN CERTIFICATE-----\n" \ // caso sua conexão seja não segura, comente esta linha
//"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n" \
//"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n" \
//"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n" \
//"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n" \
//"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n" \
//"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n" \
//"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n" \
//"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n" \
//"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n" \
//"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n" \
//"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n" \
//"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n" \
//"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n" \
//"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n" \
//"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n" \
//"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n" \
//"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n" \
//"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n" \
//"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n" \
//"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n" \
//"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n" \
//"XXXXXXXXXXXXXXXXXXXXXXXX\n" \
//"-----END CERTIFICATE-----";

//WiFiClientSecure espClient; // declaração do objeto espClient
 WiFiClient espClient; // utilize WiFiClient,caso sua conexão seja não segura
PubSubClient client(espClient); // instancia o objeto cliente MQTT passando como parâmetro o objeto espClient

// Utilização dos pinos 16 e 17 como RX e TX
#define RXD2 16
#define TXD2 17

float PTemp;
float batt_volt;
float ML01_RAd;
float ML020_Rad;
float TempContato1;
float TempContato2;
float AirTemperature_act;
float RelHumidity_act;
float WindSpeed_act;
float WindDirection_act;
float RelAirPressure;
float WindSpeed_act2;
float HBr1;
float HBr2;
float RS1;
float RS2;
float RScomp1;
float RScomp2;
float WTB_Chuva;
float GlobalRadiation_act;

void setup() {
  // Note the format for setting a serial port is as follows: Serial2.begin(baud-rate, protocol, RX pin, TX pin);
  Serial.begin(115200);
  //Serial1.begin(9600, SERIAL_8N1, RXD2, TXD2);
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
  Serial.println("Serial Txd is on pin: "+String(TX));
  Serial.println("Serial Rxd is on pin: "+String(RX));

  WiFi.begin(ssid, password); // inicializa a conexão com o Wifi local
//  while (WiFi.status() != WL_CONNECTED) { // verificação do status da conexão. Caso não tenha conectado, são realizadas novas tentativas de conexão
//    delay(500);
//    Serial.println("Connecting to WiFi..");
//  }
  Serial.println("Connected to the WiFi network"); // mensagem indicando que a conexão com o Wifi local foi bem sucedida

//  espClient.setCACert(local_root_ca); // definição do certificado CA (é passado como parâmetro o certificado CA criptografado) ; *comente esta linha para conexão não segura
  client.setServer(mqttServer, mqttPort); // informa qual o Broker e a porta do MQTT que serão conectados 

  while (!client.connected()) { // Verificação de conexão com MQTT
    Serial.println("Connecting to MQTT...");
 
    if (client.connect("identificador_do_dispositivo", mqttUser, mqttPassword )) { // tentativa de conexão com o MQTT. 
       //São passados como parâmetros: identificação única de seu dispositivo (Ex: ESP32Client), USUÁRIO MQTT, SENHA MQTT
      Serial.println("connected"); // mensagem indicando que a conexão MQTT foi bem sucedida
    } else {
      Serial.println("failed with state "); // mensagem indicando que a conexão MQTT não foi bem sucedida
      Serial.println(client.state()); // é retornado um código indicando qual o problema na conexão
      // Possíveis valores para o client.state()
      //MQTT_CONNECTION_TIMEOUT     -4
      //MQTT_CONNECTION_LOST        -3
      //MQTT_CONNECT_FAILED         -2
      //MQTT_DISCONNECTED           -1
      //MQTT_CONNECTED               0
      //MQTT_CONNECT_BAD_PROTOCOL    1
      //MQTT_CONNECT_BAD_CLIENT_ID   2
      //MQTT_CONNECT_UNAVAILABLE     3
      //MQTT_CONNECT_BAD_CREDENTIALS 4
      //MQTT_CONNECT_UNAUTHORIZED    5
      delay(2000); // tempo de 2s até próxima tentativa de conexão
    }
  }
  
}

void loop() { //Choose Serial1 or Serial2 as required

  unsigned long currentMillis = millis();
  // if WiFi is down, try reconnecting every CHECK_WIFI_TIME seconds
  if ((WiFi.status() != WL_CONNECTED) && (currentMillis - previousMillis >=interval)) {
    Serial.print(millis());
    Serial.println("Reconnecting to WiFi...");
    WiFi.disconnect();
    WiFi.reconnect();
    previousMillis = currentMillis;
  }
  
  // make sure mqtt is connected  
  while (!client.connected()) { // Verificação de conexão com MQTT
    
    Serial.println("Connecting to MQTT...");
 
    if (client.connect("identificador_do_dispositivo", mqttUser, mqttPassword )) { // tentativa de conexão com o MQTT. 
       //São passados como parâmetros: identificação única de seu dispositivo (Ex: ESP32Client), USUÁRIO MQTT, SENHA MQTT
 
      Serial.println("connected"); // mensagem indicando que a conexão MQTT foi bem sucedida
 
    } else {
      unsigned long currentMillis = millis();
      // if WiFi is down, try reconnecting every CHECK_WIFI_TIME seconds
      if ((WiFi.status() != WL_CONNECTED) && (currentMillis - previousMillis >=interval)) {
        Serial.print(millis());
        Serial.println("Reconnecting to WiFi...");
        WiFi.disconnect();
        WiFi.reconnect();
        previousMillis = currentMillis;
      }
      Serial.println("failed with state "); // mensagem indicando que a conexão MQTT não foi bem sucedida
      Serial.println(client.state()); // é retornado um código indicando qual o problema na conexão
      // Possíveis valores para o client.state()
      //MQTT_CONNECTION_TIMEOUT     -4
      //MQTT_CONNECTION_LOST        -3
      //MQTT_CONNECT_FAILED         -2
      //MQTT_DISCONNECTED           -1
      //MQTT_CONNECTED               0
      //MQTT_CONNECT_BAD_PROTOCOL    1
      //MQTT_CONNECT_BAD_CLIENT_ID   2
      //MQTT_CONNECT_UNAVAILABLE     3
      //MQTT_CONNECT_BAD_CREDENTIALS 4
      //MQTT_CONNECT_UNAUTHORIZED    5
      
      delay(10000); // tempo de 10s até próxima tentativa de conexão
 
    }
  }

  // read new serial info  
  while (Serial2.available()) {
    String dado;
    String stringRead = Serial2.readString();
    int lenStringRead = stringRead.length() + 1;
    char fullString[lenStringRead];
    stringRead.toCharArray(fullString, lenStringRead);
    Serial.println("Novo Dado da Serial!");
    Serial.println(fullString);
    for(int i=0; i<lenStringRead; i++){
      //Serial.println(fullString[i]);
      if (fullString[i]== '*'){
        dado = ""; //reset data
      } else if(fullString[i]== '#'){
        char bufferDado[dado.length()+1];
        int lenDado = dado.length()+1;
        String variableNumber="";
        String variableData="";
        bool beforeTwoDots=true;
        dado.toCharArray(bufferDado, lenDado);
        for(int j=0; j<lenDado; j++){
          if(bufferDado[j]==':'){
            beforeTwoDots = false;
          } else {
            if (beforeTwoDots){
              variableNumber.concat(bufferDado[j]);
            } else{
              variableData.concat(bufferDado[j]);
            }
          }
        }
        
          if(variableNumber == "1"){
            PTemp=variableData.toFloat();
          } else if(variableNumber == "2"){
            batt_volt=variableData.toFloat();
          } else if(variableNumber == "3"){
            ML01_RAd=variableData.toFloat(); 
          } else if(variableNumber == "4"){
            ML020_Rad=variableData.toFloat();
          } else if(variableNumber == "5"){
            TempContato1=variableData.toFloat();
          } else if(variableNumber == "6"){
            TempContato2=variableData.toFloat();
          } else if(variableNumber == "7"){
            AirTemperature_act=variableData.toFloat();
          } else if(variableNumber == "8"){
            RelHumidity_act=variableData.toFloat();
          } else if(variableNumber == "9"){
            WindSpeed_act=variableData.toFloat();
          } else if(variableNumber == "10"){
            WindDirection_act=variableData.toFloat();
          } else if(variableNumber == "11"){
            RelAirPressure=variableData.toFloat();
          } else if(variableNumber == "12"){
            WindSpeed_act2=variableData.toFloat();
          } else if(variableNumber == "13"){
            HBr1=variableData.toFloat();
          } else if(variableNumber == "14"){
            HBr2=variableData.toFloat();
          } else if(variableNumber == "15"){
            RS1=variableData.toFloat();
          } else if(variableNumber == "16"){
            RS2=variableData.toFloat();
          } else if(variableNumber == "17"){
            RScomp1=variableData.toFloat();
          } else if(variableNumber == "18"){
            RScomp2=variableData.toFloat();
          } else if(variableNumber == "19"){
            WTB_Chuva=variableData.toFloat();
          } else if(variableNumber == "20"){
            GlobalRadiation_act=variableData.toFloat();
          } else {
            Serial.println("NO MATCHES FOUND!!! VERIFY YOUR CODE PLEASE!");
            Serial.println(variableNumber);
            Serial.println(variableData);
          }
          
      } else {
        dado.concat(fullString[i]);
      }
      }

      Serial.print("PTemp :>>");
      Serial.println(PTemp);
      Serial.print("batt_volt :>>");
      Serial.println(batt_volt);
      Serial.print("ML01_RAd :>>");
      Serial.println(ML01_RAd);
      Serial.print("ML020_Rad :>>");
      Serial.println(ML020_Rad);
      Serial.print("TempContato1 :>>");
      Serial.println(TempContato1);
      Serial.print("TempContato2 :>>");
      Serial.println(TempContato2);
      Serial.print("AirTemperature_act :>>");
      Serial.println(AirTemperature_act);
      Serial.print("RelHumidity_act :>>");
      Serial.println(RelHumidity_act);
      Serial.print("WindSpeed_act :>>");
      Serial.println(WindSpeed_act);
      Serial.print("WindDirection_act :>>");
      Serial.println(WindDirection_act);
      Serial.print("RelAirPressure :>>");
      Serial.println(RelAirPressure);
      Serial.print("WindSpeed_act2 :>>");
      Serial.println(WindSpeed_act2);
      Serial.print("HBr1 :>>");
      Serial.println(HBr1);
      Serial.print("HBr2 :>>");
      Serial.println(HBr2);
      Serial.print("RS1 :>>");
      Serial.println(RS1);
      Serial.print("RS2 :>>");
      Serial.println(RS2);
      Serial.print("RScomp1 :>>");
      Serial.println(RScomp1);
      Serial.print("RScomp2 :>>");
      Serial.println(RScomp2);
      Serial.print("WTB_Chuva :>>");
      Serial.println(WTB_Chuva);
      Serial.print("GlobalRadiation_act :>>");
      Serial.println(GlobalRadiation_act);
      
      String dataName[10]={"PTemp","batt_volt", "ML01_RAd", "ML020_Rad", "TempContato1", "TempContato2", "AirTemperature_act", "RelHumidity_act", "WindSpeed_act", "WindDirection_act"};
      String dataName2[10]={"RelAirPressure", "WindSpeed_act2", "HBr1", "HBr2", "RS1", "RS2", "RScomp1", "RScomp2", "WTB_Chuva", "GlobalRadiation_act"};
      float dataValue[10]={PTemp, batt_volt, ML01_RAd, ML020_Rad, TempContato1, TempContato2, AirTemperature_act, RelHumidity_act, WindSpeed_act, WindDirection_act};
      float dataValue2[10]={RelAirPressure, WindSpeed_act2, HBr1, HBr2, RS1, RS2, RScomp1, RScomp2, WTB_Chuva, GlobalRadiation_act};
      String unity[10]={"Celsius", "V", "ml01rad_unity", "ml020rad_unity", "Celsius", "Celsius", "Celsius", "relhumity_unity", "windspeed_unity", "winddirection_unity"};
      String unity2[10]={"relairpressure_unity","windspeedact2_unity", "hbr1_unity", "hbr2_unity", "rs1_unity", "rs2_unity", "rscomp1_unity", "rscomp2_unity", "wtbchuva_unity", "globalradiationact_unity"};
      publishMqtt(dataName, dataValue, unity);
      publishMqtt(dataName2, dataValue2, unity2);
    }
}

void publishMqtt(String dataName[20], float dataValue[20], String unity[20]) {
    StaticJsonDocument<400> doc; // declaração do objeto doc, que será utilizado para criar as mensagens JSON. É necessário especificar o tamanho do documento em bytes. 
    // Utilize este endereço para o cálculo de bytes, se necessário: https://arduinojson.org/v6/assistant/. Utilize os seguintes parâmetros para o cálculo:
    // Processor: ESP32
    // Mode: Serialize
    // Output type: String
    // Ex JSON: 
    //  { 
    //  "X": [30],
    //  "ID": "S1",
    //  "Aplicacao" : "Painel_Solar",
    //  "Local" : "GASI",
    //  "Tipo" :  "Sensor",
    //  "Variavel" :  "Temperatura",
    //  "Unidade" :  "Celsius",
    //  "Rede" :  "MQTT",
    //  "Professor" : "Paciencia"
    //  }
    // A seguir são adicionados os valores das variáveis de nosso payload. O mínimo requerido são 9 variáveis (ID, Valor, Aplicação, Local, Tipo, Variável, Unidade, Rede e Professor), como listado abaixo. 
    // É possível adicionar mais variáveis, se necessário.
    float X; // X corresponde à variável de retorno dos valores reais de seu device.
   // ***Insira aqui seu programa de obtenção de dados. Considere X como variável de retorno dos valores.***
   // Ex:
   //    X = random (0,100);

  for (int varLen = 0; varLen <= 10 ; varLen = varLen + 1){
    String mqttVariableName;
    float mqttVariableValue=0;
    
    doc["ID"] = "RoMiotto1"; // escreva aqui o ID do seu dispositivo. Ex: "S1_T" referente a um sensor de temperatura. //doc["ID"] = "S1_T";
    doc["Valor"] = dataValue[varLen]; // o valor contido em X é alocado para a variável "Valor"
    doc["Aplicacao"] = "Estacao_Meteorologica"; // escreva aqui o Número ou Nome da sua APLICAÇÃO. Ex: "1" ou "Painel_Solar" //doc["Aplicacao"] = "Painel_Solar";
    doc["Local"] = "Telhado-Predio"; // escreva aqui o Código do LOCAL. Ex: "GASI" //doc["Local"] = "GASI";
    doc["Tipo"] = "Sensor"; // escreva aqui o TIPO ou FONTE do seu device. Ex: "Sensor" //doc["Tipo"] = "Sensor";
    doc["Variavel"] = dataName[varLen]; // escreva aqui qual o tipo de sua VARIÁVEL. Ex: "Temperatura" //doc["Variavel"] = "Temperatura";
    doc["Unidade"] = unity[varLen]; // escreva aqui qual a unidade de medida utilizada. Ex: "Celsius" //doc["Unidade"] = "Celsius";
    doc["Rede"] = "MQTT"; // escreva aqui qual é a rede utilizada. Ex: "MQTT" //doc["Rede"] = "MQTT";
    doc["Professor"] = "Paciencia"; // escreva aqui o nome do professor. Ex: "Paciencia" //doc["Professor"] = "Paciencia";
  
  // **OBS: O nome do professor deve ser igual ao MEASUREMENT
  
    char JSONmessageDoc[800]; // é criada uma variável para o envio da mensagem JSON em formato STRING
    serializeJson(doc,JSONmessageDoc); // dados são compactados e armazenados na variável JSONmessageDoc
    Serial.println("Sending message to MQTT topic..");
    Serial.println(JSONmessageDoc); // impressão da mensagem enviada no Monitor Serial

    if (dataName[varLen] != 0){
      if (client.publish("smartcampus/estacao", JSONmessageDoc) == true) { // publicação da mensagem JSON no tópico MQTT
                                        //tópico MQTT                                    //mensagem JSON
        
        // O tópico deve ser escrito seguindo o seguinte padrão: "identificação única de seu dispositivo / número ou nome da sua aplicação / nome do professor (MEASUREMENT)". 
        // Ex: (client.publish("ESP32Client/Painel_Solar/Paciencia", JSONmessageDoc) 
        
        Serial.println("Success sending message"); // é printada uma mensagem informando que o payload foi publicado com sucesso no tópico
        delay(500);
      } else {
        Serial.println("Error sending message"); // é printada uma mensagem informando que houve erro ao publicar o payload no tópico
      }
    }
   
    client.loop(); // a função loop permite que as mensagens sejam publicadas regurlamente e a conexão MQTT seja mantida
    Serial.println("-------------");
  }
}
