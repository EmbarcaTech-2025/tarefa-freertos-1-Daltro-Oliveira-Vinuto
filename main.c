/*
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include "pico/stdlib.h"

#include "hardware/i2c.h"

void led_task() {
  const uint LED_PIN = 11;
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);
  while (true) {
    gpio_put(LED_PIN, 1);
    vTaskDelay(100);
    gpio_put(LED_PIN, 0);
    vTaskDelay(100);
  }
}

int main() {
  stdio_init_all();

  xTaskCreate(led_task, "LED_Task", 256, NULL, 1, NULL);
  vTaskStartScheduler();

  while(1){};
}
*/

// main.c
#include <stdio.h>
#include "pico/stdlib.h"

// FreeRTOS includes
#include "FreeRTOS.h"
#include "task.h"

// Definição dos pinos GPIO para o LED RGB (ATUALIZADO)
#define LED_R_PIN 13
#define LED_G_PIN 11
#define LED_B_PIN 12

// Definição do pino GPIO para o Buzzer (ATUALIZADO)
#define BUZZER_PIN 21

// Definição dos pinos GPIO para os botões (ATUALIZADO)
#define BUTTON_A_PIN 5
#define BUTTON_B_PIN 6

// Variáveis globais para os handles das tarefas
TaskHandle_t xLedTaskHandle = NULL;
TaskHandle_t xBuzzerTaskHandle = NULL;

// Enumeração para os estados do LED RGB
typedef enum {
    COLOR_RED,
    COLOR_GREEN,
    COLOR_BLUE
} rgb_color_t;

// Função para configurar os GPIOs
void gpio_init_custom() {
    // Configura os pinos do LED RGB como saída
    gpio_init(LED_R_PIN);
    gpio_set_dir(LED_R_PIN, GPIO_OUT);
    gpio_init(LED_G_PIN);
    gpio_set_dir(LED_G_PIN, GPIO_OUT);
    gpio_init(LED_B_PIN);
    gpio_set_dir(LED_B_PIN, GPIO_OUT);

    // Configura o pino do Buzzer como saída
    gpio_init(BUZZER_PIN);
    gpio_set_dir(BUZZER_PIN, GPIO_OUT);

    // Configura os pinos dos botões como entrada com pull-up
    gpio_init(BUTTON_A_PIN);
    gpio_set_dir(BUTTON_A_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_A_PIN); // Habilita pull-up interno
    
    gpio_init(BUTTON_B_PIN);
    gpio_set_dir(BUTTON_B_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_B_PIN); // Habilita pull-up interno
}

// Tarefa do LED RGB
void vLedTask(void *pvParameters) {
    rgb_color_t current_color = COLOR_RED;

    for (;;) {
        switch (current_color) {
            case COLOR_RED:
                // printf("LED: Vermelho\n"); // Para debug no terminal
                gpio_put(LED_R_PIN, 1);
                gpio_put(LED_G_PIN, 0);
                gpio_put(LED_B_PIN, 0);
                current_color = COLOR_GREEN;
                break;
            case COLOR_GREEN:
                // printf("LED: Verde\n"); // Para debug no terminal
                gpio_put(LED_R_PIN, 0);
                gpio_put(LED_G_PIN, 1);
                gpio_put(LED_B_PIN, 0);
                current_color = COLOR_BLUE;
                break;
            case COLOR_BLUE:
                // printf("LED: Azul\n"); // Para debug no terminal
                gpio_put(LED_R_PIN, 0);
                gpio_put(LED_G_PIN, 0);
                gpio_put(LED_B_PIN, 1);
                current_color = COLOR_RED; // Volta para vermelho
                break;
        }
        vTaskDelay(pdMS_TO_TICKS(500)); // Espera 500ms
    }
}

// Tarefa do Buzzer
void vBuzzerTask(void *pvParameters) {
    for (;;) {
        // printf("Buzzer: Beep!\n"); // Para debug no terminal
        gpio_put(BUZZER_PIN, 1); // Liga o buzzer
        vTaskDelay(pdMS_TO_TICKS(100)); // Beep curto (100ms)
        gpio_put(BUZZER_PIN, 0); // Desliga o buzzer
        vTaskDelay(pdMS_TO_TICKS(900)); // Espera para o próximo beep (total de 1 segundo)
    }
}

// Tarefa dos Botões
void vButtonTask(void *pvParameters) {
    bool led_suspended = false;
    bool buzzer_suspended = false;

    for (;;) {
        // Leitura do Botão A (suspende/retoma LED)
        if (!gpio_get(BUTTON_A_PIN)) { // Botão pressionado (pull-up, então LOW quando pressionado)
            if (!led_suspended) {
                printf("Botao A pressionado: Suspendendo tarefa do LED\n");
                vTaskSuspend(xLedTaskHandle);
                gpio_put(LED_R_PIN, 0); // Desliga o LED quando suspenso
                gpio_put(LED_G_PIN, 0);
                gpio_put(LED_B_PIN, 0);
                led_suspended = true;
                vTaskDelay(pdMS_TO_TICKS(200)); // Pequeno delay para debounce
            }
        } else { // Botão solto
            if (led_suspended) {
                printf("Botao A solto: Retomando tarefa do LED\n");
                vTaskResume(xLedTaskHandle);
                led_suspended = false;
                vTaskDelay(pdMS_TO_TICKS(200)); // Pequeno delay para debounce
            }
        }

        // Leitura do Botão B (suspende/retoma Buzzer)
        if (!gpio_get(BUTTON_B_PIN)) { // Botão pressionado
            if (!buzzer_suspended) {
                printf("Botao B pressionado: Suspendendo tarefa do Buzzer\n");
                vTaskSuspend(xBuzzerTaskHandle);
                gpio_put(BUZZER_PIN, 0); // Desliga o buzzer quando suspenso
                buzzer_suspended = true;
                vTaskDelay(pdMS_TO_TICKS(200)); // Pequeno delay para debounce
            }
        } else { // Botão solto
            if (buzzer_suspended) {
                printf("Botao B solto: Retomando tarefa do Buzzer\n");
                vTaskResume(xBuzzerTaskHandle);
                buzzer_suspended = false;
                vTaskDelay(pdMS_TO_TICKS(200)); // Pequeno delay para debounce
            }
        }

        vTaskDelay(pdMS_TO_TICKS(100)); // Polling a cada 100ms
    }
}

int main() {
    // Inicializa a stdio para comunicação serial 
    stdio_init_all();

    // Inicializa os GPIOs 
    gpio_init_custom();

    // Cria a tarefa do LED RGB
    xTaskCreate(vLedTask,
                "LED_Task",
                configMINIMAL_STACK_SIZE, // Tamanho mínimo da pilha
                NULL,
                tskIDLE_PRIORITY + 1,     // Prioridade (pouco acima do IDLE)
                &xLedTaskHandle);

    // Cria a tarefa do Buzzer
    xTaskCreate(vBuzzerTask,
                "Buzzer_Task",
                configMINIMAL_STACK_SIZE,
                NULL,
                tskIDLE_PRIORITY + 1,
                &xBuzzerTaskHandle);

    // Cria a tarefa dos Botões
    xTaskCreate(vButtonTask,
                "Button_Task",
                configMINIMAL_STACK_SIZE,
                NULL,
                tskIDLE_PRIORITY + 2,     // Prioridade um pouco maior para reagir aos botões
                NULL);                    // Não precisamos do handle para a tarefa de botões neste exemplo

    // Inicia o escalonador do FreeRTOS
    vTaskStartScheduler();

    // Este ponto nunca deve ser alcançado se o escalonador iniciar corretamente
    for (;;) {
        // Loop infinito caso o escalonador retorne (o que não deve acontecer em sistemas embarcados)
    }
    return 0;
}
