#include "driver/pcnt.h"
#include "esp_timer.h"

#define SIGNAL_PIN 4 //Pino de entrada do sinal
#define CONTROL_PIN 7 //Pino de controle
#define PCNT_UNIT PCNT_UNIT_0 //Unidade do contador

esp_timer_handle_t timer_handle;
int16_t pulse_count = 0;

void timer_callback(void* arg) {
  pcnt_get_counter_value(PCNT_UNIT, &pulse_count);
  pcnt_counter_clear(PCNT_UNIT);
}

void setup() {
  Serial.begin(115200);

  pcnt_config_t pcnt_config;

  pcnt_config.pulse_gpio_num = SIGNAL_PIN; //Pino de entrada do sinal
  pcnt_config.ctrl_gpio_num = CONTROL_PIN; //Pino de controle

  pcnt_config.channel = PCNT_CHANNEL_1; //Canal 1
  pcnt_config.unit = PCNT_UNIT; //Unidade 0

  pcnt_config.pos_mode = PCNT_COUNT_INC; //Ativa o contador em borda de subida
  pcnt_config.neg_mode = PCNT_COUNT_DIS; //Ignora a borda de descida

  pcnt_config.lctrl_mode = PCNT_MODE_KEEP; //Pino de controle não afeta a constafem
  pcnt_config.hctrl_mode = PCNT_MODE_KEEP; /Pino de controle não afeta a constafem
  pcnt_config.counter_h_lim = 100000; //Limite superior do contador
  pcnt_config.counter_l_lim = 0; //Limite inferior do contador
  
  pcnt_unit_config(&pcnt_config); //Aplica as configurações setadas

  pcnt_counter_pause(PCNT_UNIT); //Pausa o contador
  pcnt_counter_clear(PCNT_UNIT); //Limpa o contador
  pcnt_counter_resume(PCNT_UNIT); //Reinicia o contador

  //Definindo o timer de interrupção 
  const esp_timer_create_args_t timer_args = {
    .callback = &timer_callback, //Função a ser chamada quando o timer estourar
    .name = "freq_timer" //Nome do timer para debug
  };
  esp_timer_create(&timer_args, &timer_handle); //Cria o timer com base nas confiurações setadas
  esp_timer_start_periodic(timer_handle, 1000); //Ativa timer para cada 1ms
}
void loop() {
  unsigned long long int freq = pulse_count*1000;
  Serial.print("Frequência: ");
  Serial.print(freq);
  Serial.println(" Hz");
}