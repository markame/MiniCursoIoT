#include <WiFi.h>
#include <PubSubClient.h>
#include <DHTesp.h>

const int DHT_PIN = 15; 
DHTesp dht;
const char* ssid = "Wokwi-GUEST";  // Nome da rede Wi-Fi do Wokiwi
const char* password = "";  // Sem senha para que o Wokiwi entenda que ira usar seu simulador
const char* mqtt_server = "test.mosquitto.org";  // Broker MQTT alternativo
const int mqtt_port = 1883;  // Porta padrão MQTT sem SSL

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
float temp = 0;
float hum = 0;

// Função para conectar ao Wi-Fi
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

// Função callback que é chamada quando uma nova mensagem chega no tópico subscrito
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

// Função de reconexão ao MQTT
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32Client-12345";  // ID fixo para teste

    // Verificando o estado da conexão MQTT
    int mqttState = client.connect(clientId.c_str());
    if (mqttState == MQTT_CONNECTION_TIMEOUT) {
      Serial.println("Connection timeout");
    } else if (mqttState == MQTT_CONNECTION_LOST) {
      Serial.println("Connection lost");
    } else if (mqttState == MQTT_CONNECT_FAILED) {
      Serial.println("Connection failed");
    }

    if (client.connect(clientId.c_str())) {
      Serial.println("Connected");
      client.publish("/Markame/Publish", "Welcome");
      client.subscribe("/Markame/Subscribe");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 10 seconds");
      delay(10000);  // Aumentando o intervalo de reconexão para 10 segundos
    }
  }
}

void setup() {
  pinMode(2, OUTPUT);    
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);  // Usando a porta 1883 (padrão para MQTT)
  client.setCallback(callback);
  dht.setup(DHT_PIN, DHTesp::DHT22);  // Configuração do sensor DHT22
}

void loop() {
  if (!client.connected()) {
    reconnect();  // Reconectar se desconectar
  }
  client.loop();  // Manter o loop do MQTT ativo

  unsigned long now = millis();
  if (now - lastMsg > 2000) { 
    lastMsg = now;
    TempAndHumidity data = dht.getTempAndHumidity();

    String temp = String(data.temperature, 2);
    client.publish("/Markame/temp", temp.c_str());  // Publica a temperatura no tópico, troque Markame pelo seu Nome ou nickname de seu jogo favorito :-)
    String hum = String(data.humidity, 1);
    client.publish("/Markame/hum", hum.c_str());  // Publica a umidade no tópico, troque Markame pelo seu Nome ou nickname de seu jogo favorito :-)

    Serial.print("Temperature: ");
    Serial.println(temp);
    Serial.print("Humidity: ");
    Serial.println(hum);
  }
}
