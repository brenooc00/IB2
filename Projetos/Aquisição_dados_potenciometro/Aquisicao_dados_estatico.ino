#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <ArduinoJson.h>

// Configurações do Firebase (Realtime Database)
#define FIREBASE_HOST "oximetro-6bd38-default-rtdb.firebaseio.com"  // Sem "https://"
#define FIREBASE_AUTH "X8mBCcJ2sxcTUogjBf6kMULUNcNTDM9uWGwNrt8P"

// Credenciais do Wi‑Fi
const char* ssid = "NET";
const char* password = "naosei123";

// Configurações do ADC e buffer
#define ADC_PIN A9             // Pino ADC
#define BATCH_SIZE 50          // Número de leituras antes do envio

float readings[BATCH_SIZE];    // Buffer para leituras
volatile int readingIndex = 0; // Índice do buffer
float freq = 10;               // Frequência inicial de amostragem (Hz)
volatile float value;

// Handles das tasks do FreeRTOS
TaskHandle_t Task1;  // Tarefa de leitura do ADC
TaskHandle_t Task2;  // Tarefa de envio dos dados

// Objetos do Firebase
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Tarefa para leitura do ADC (executada no Core 0)
void readADC(void *parameter) {
  while (1) {
    if (readingIndex < BATCH_SIZE) {
      value = analogRead(ADC_PIN) * (3.3 / 4096);
      readings[readingIndex++] = value;
      // Aguarda um tempo de acordo com a frequência (em milissegundos)
      vTaskDelay(pdMS_TO_TICKS(1000 / freq));
    }
  }
}

// Tarefa para criar um JSON com os dados e enviá-lo ao Firebase (executada no Core 1)
void sendToFirebase(void *parameter) {
  while (1) {
    if (readingIndex >= BATCH_SIZE) {
      // Cria um DynamicJsonDocument e obtém um array para adicionar os valores
      DynamicJsonDocument doc(1024);
      JsonArray array = doc.to<JsonArray>();
      for (int i = 0; i < BATCH_SIZE; i++) {
        array.add(readings[i]);
      }

      // Serializa o documento para uma string
      String jsonString;
      serializeJson(doc, jsonString);

      // Cria um FirebaseJson e define os dados JSON
      FirebaseJson fbJson;
      fbJson.setJsonData(jsonString);

      // Envia o JSON para o Firebase (nó "esp32/grafico_estatico")
      if (Firebase.RTDB.pushJSON(&fbdo, "/esp32/grafico_estatico", &fbJson)) {
        //Serial.println("Dados enviados com sucesso!");
      } else {
        //Serial.print("Erro ao enviar: ");
        //Serial.println(fbdo.errorReason());
      }

      // Atualiza a frequência a partir do Firebase (se houver alteração)
      if (Firebase.RTDB.getFloat(&fbdo, "/esp32/freq")) {
        freq = fbdo.floatData();
        //Serial.print("Nova frequência: ");
        //Serial.println(freq);
      } else {
        //Serial.print("Erro ao ler frequência: ");
        //Serial.println(fbdo.errorReason());
      }

      // Reinicia o buffer
      readingIndex = 0;
    }
    vTaskDelay(pdMS_TO_TICKS(500)); // Aguarda antes da próxima verificação
  }
}

void setup() {
  Serial.begin(115200);
  
  // Conecta-se ao Wi‑Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  // Configura o Firebase
  config.host = FIREBASE_HOST;
  config.signer.tokens.legacy_token = FIREBASE_AUTH;
  
  // Inicializa o Firebase com a configuração e autenticação
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  // Define a frequência inicial no Firebase
  Firebase.RTDB.setFloat(&fbdo, "/esp32/freq", 10)) {
    //Serial.println("Frequência inicial definida no Firebase.");
  } else {
    //Serial.print("Erro ao definir frequência: ");
    //Serial.println(fbdo.errorReason());
  }
  
  // Limpa dados anteriores
  Firebase.RTDB.deleteNode(&fbdo, "/esp32/grafico_estatico");

  // Cria as tasks nos núcleos separados
  xTaskCreatePinnedToCore(readADC, "ReadADC", 4096, NULL, 1, &Task1, 0);   // Core 0
  xTaskCreatePinnedToCore(sendToFirebase, "SendToFirebase", 8192, NULL, 1, &Task2, 1); // Core 1
}

void loop() {
  // Nada necessário aqui, pois as tasks cuidam do processamento
}
