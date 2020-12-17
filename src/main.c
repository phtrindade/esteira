#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "esp_task_wdt.h"
#include "controle.h"
#include "driver/gpio.h"

#define PIN_BOOT 0

void somaPesoTotal();  //processo para somar o peso total e também pegar o tempo que isso leva

int contadorEsteira1 = 0;  //iniciando contador da esteira 1
int contadorEsteira2 = 0;  //iniciando contador da esteira 2
int contadorEsteira3 = 0;  //iniciando contador da esteira 3

TaskHandle_t esteira1_handle;
TaskHandle_t esteira2_handle;
TaskHandle_t esteira3_handle;
SemaphoreHandle_t semEsteira1;
SemaphoreHandle_t semEsteira2;
SemaphoreHandle_t semEsteira3;
SemaphoreHandle_t semDisplay;

void task_esteira1(void *pvp) //criando função do processo da esteira 1
{
    semEsteira1 = xSemaphoreCreateMutex();
    dadoEsteira produto;
    produto.esteira = 1;  
    produto.peso = 5000; //peso do produto em mg evitando float 

    for (;;)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);  
        xQueueSend(filaDasEsteiras, &produto, portMAX_DELAY);  
        xSemaphoreTake(semEsteira1, portMAX_DELAY);  
        contadorEsteira1++;  
        xSemaphoreGive(semEsteira1);
    }
}
void task_esteira2(void *pvp)  //criando função do processo da esteira 2
{
    semEsteira2 = xSemaphoreCreateMutex(); // para controle da secção critica  com mutex

    dadoEsteira produto;
    produto.esteira = 2;  
    produto.peso = 2000;  //peso do produto em mg

    for (;;)
    {
        vTaskDelay(500 / portTICK_PERIOD_MS);  //tempo em ms que leva para o produto da esteira 2
        xQueueSend(filaDasEsteiras, &produto, portMAX_DELAY);  // envia msm do produto para a tarefa controle
        xSemaphoreTake(semEsteira2, portMAX_DELAY); // solicita o acesso a secção critica
        contadorEsteira2++;  //contando quantidade de produtos esteira 2
        xSemaphoreGive(semEsteira2); // libera acesso da secção critica ( variavem contador esteira)
    }
}
void task_esteira3(void *pvp)  //criando função do processo da esteira 3
{
    semEsteira3 = xSemaphoreCreateMutex();

    dadoEsteira produto;
    produto.esteira = 3;  
    produto.peso = 500;  

    for (;;)
    {
        vTaskDelay(100 / portTICK_PERIOD_MS);  
        xQueueSend(filaDasEsteiras, &produto, portMAX_DELAY);  
        xSemaphoreTake(semEsteira3, portMAX_DELAY);
        contadorEsteira3++;  
        xSemaphoreGive(semEsteira3);
    }
}

void task_display(void *pvp)
{
    semDisplay = xSemaphoreCreateMutex();
    uint64_t tempoAnterior = 0, tempoDepois = 0;
    for (;;)
    {
        vTaskDelay(2000 / portTICK_PERIOD_MS);  // ATUALIZA A CADA  2 segundos para iniciar o processo do display
        tempoAnterior = esp_timer_get_time();  //pegando tempo antes de começar a mostra dados no display
        xSemaphoreTake(semEsteira1, portMAX_DELAY); // solicita o acesso a secção critica 
        printf("Display Esteira 1: %d\n", contadorEsteira1); // atua na secção critica
        xSemaphoreGive(semEsteira1); // libera a secção para demais tarefas

        xSemaphoreTake(semEsteira2, portMAX_DELAY);
        printf("Display Esteira 2: %d\n", contadorEsteira2);
        xSemaphoreGive(semEsteira2);

        xSemaphoreTake(semEsteira3, portMAX_DELAY);
        printf("Display Esteira 3: %d\n", contadorEsteira3);
        xSemaphoreGive(semEsteira3);
        tempoDepois = esp_timer_get_time();  //pegando tempo depois de termina de mostrar os dados no display
        xSemaphoreTake(semDisplay, portMAX_DELAY);
        tempoDisplay = (tempoDepois - tempoAnterior) / 100;  //calculando tempo que levou para atualizar o display
        xSemaphoreGive(semDisplay);
    }
}

void app_main()
{
    xTaskCreate(task_controle, "Task_Controle", 8192, NULL, 1, NULL);  //Iniciando proceço do controle
    xTaskCreate(task_esteira1, "Task_Esteira1", 8192, NULL, 1, &esteira1_handle);  //Iniciando proceço da esteira 1
    xTaskCreate(task_esteira2, "Task_Esteira2", 8192, NULL, 1, &esteira2_handle);  //Iniciando proceço da esteira 2
    xTaskCreate(task_esteira3, "Task_Esteira3", 8192, NULL, 1, &esteira3_handle);  //Iniciando proceço da esteira 3
    xTaskCreate(task_display, "Task_Display", 8192, NULL, 1, NULL);  //Iniciando proceço do display

    for (;;)
    {
        if (!gpio_get_level(PIN_BOOT)){
            vTaskDelete(esteira1_handle);
            vTaskDelete(esteira2_handle);
            vTaskDelete(esteira3_handle);
        }
        esp_task_wdt_reset();
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
    // nunca vai chegar aqui
}
