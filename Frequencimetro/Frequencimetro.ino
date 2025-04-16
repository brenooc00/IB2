#include "driver/pcnt.h"
#include <Ticker.h>

#define PCNT_UNIT PCNT_UNIT_0 //Unidade do contador
#define SIGNAL_PIN 4 //Pino D3, para sinal de entrada
#define CONTROL_PIN 7 //Pino de controle

Ticker timer_pulse;

volatile float freq; //Valor de frequência
int16_t pulsos; //Valor de pulsos no contador

void IRAM_ATTR countPulse() {
  pcnt_get_counter_value(PCNT_UNIT, &pulsos); //Conta a quantidade de pulsos do contador
  pcnt_counter_clear(PCNT_UNIT); //Zera o contador
}


void setup() {
  Serial.begin(115200);

  // Configuração da unidade PCNT
  pcnt_config_t pcnt_config;

  pcnt_config.pulse_gpio_num = SIGNAL_PIN; //Pino de entrada do sinal
  pcnt_config.ctrl_gpio_num = CONTROL_PIN; //Pino de controle

  pcnt_config.channel = PCNT_CHANNEL_1; //Canal 1
  pcnt_config.unit = PCNT_UNIT; //Unidade 0

  pcnt_config.pos_mode = PCNT_COUNT_INC; //Ativa o contador em borda de subida
  pcnt_config.neg_mode = PCNT_COUNT_DIS; //Ignora a borda de descida

  pcnt_config.counter_h_lim = 100000; //Limite superior do contador
  pcnt_config.counter_l_lim = 0; //Limite inferior do contador

  pcnt_config.lctrl_mode = PCNT_MODE_KEEP;   // Mantém contagem quando o pino de controle está LOW
  pcnt_config.hctrl_mode = PCNT_MODE_KEEP;    // Mantém contagem quando o pino de controle está HIGH
  
  pcnt_unit_config(&pcnt_config); // Aplica a configuração no módulo de contagem
  pcnt_counter_clear(PCNT_UNIT); //Limpa o contador
  pcnt_counter_resume(PCNT_UNIT); //Reinicia o contador

  //Configuração dos timer
  timer_pulse.attach(0.001, countPulse);  // Atualiza a cada 1 ms
}

void loop() {
  //Como o timer está a cada 1s, multiplica-se por 10000
  freq = pulsos * 1000;
  Serial.print("Frequência:  ");
  Serial.print(freq);
  Serial.println("  Hz");
  delay(100);

}
