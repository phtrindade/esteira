#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "controle.h"
QueueHandle_t filaDasEsteiras;
uint64_t temposComputadorLivre[QNTD_MAXIMA_PESOS] = {0};
int pesosEsteiras[QNTD_MAXIMA_PESOS] = {0};
int pesoTotal = 0;  //criando variavel para armazenar o peso total dos produtos
uint64_t tempoDisplay = 0; //criando variavel para armazenar o tempo que o diplay leva para atualizar
uint64_t WCET = 0;  //pior caso maior tempo de pesagem total das 1500 pessagens
float taxaDeAtualizacao = 0; // cirando variavel para o tempo da taxa de atualização 

void somaPesoTotal()  // criando função para soma do pesso total
{

    vTaskSuspend(esteira1_handle);   //suspendendo  tarefa esteira 1
    vTaskSuspend(esteira2_handle);  //suspendendo esteira 2
    vTaskSuspend(esteira3_handle);  //suspendendo esteira 3
    uint64_t tempoAnterior = esp_timer_get_time();  // tempo em microsegundos de esp ligado
    for (int i = 0; i < QNTD_MAXIMA_PESOS; i++)
        pesoTotal += pesosEsteiras[i]; // soma pesos
    uint64_t tempoDepois = esp_timer_get_time();  // guarda o tempo depois de terminar a contagem do peso total dos produtos
    uint64_t tempo = (tempoDepois - tempoAnterior) / 100;  //transforma de micro pra milli
    WCET = (tempo > WCET) ? tempo : WCET;  // só recebe maior tempo
    int somaTempos = 0; 
    for (int i = 0; i < QNTD_MAXIMA_PESOS; i++) //tempo livre sem receber dados dos sensores da esteira (média)
        somaTempos += temposComputadorLivre[i];

    taxaDeAtualizacao = ((float)somaTempos / QNTD_MAXIMA_PESOS) / 100;  

    vTaskResume(esteira1_handle);  //retorna  processo da esteira 1
    vTaskResume(esteira2_handle);  
    vTaskResume(esteira3_handle);  
}

void task_controle(void *pvt)  //iniciando o controle geral
{

    filaDasEsteiras = xQueueCreate(20, sizeof(dadoEsteira));  //criando  fila para armazenar mensagens de outras tarefas
    if (filaDasEsteiras == NULL)
    {
        printf("Fila nao criada");
        esp_restart();
    }

    dadoEsteira produtoReceive;
    uint64_t taxaAnterior = 0;  //criação da varivel que pega os tempos antes de começa o processo
    uint64_t taxaSeguinte = 0;  //criação da varivel que pega os tempos depois de começa o processo
    int contador = 0;  //contador para manipular vetor

    for (;;)
    {

        taxaAnterior = esp_timer_get_time();  // tempo sem recebimento de dados do sensor (inicial)
        xQueueReceive(filaDasEsteiras, &produtoReceive, portMAX_DELAY);
        taxaSeguinte = esp_timer_get_time(); //// tempo sem recebimento de dados do sensor (final)
        temposComputadorLivre[contador] = (taxaSeguinte - taxaAnterior);  //calcula tempo livre 
        pesosEsteiras[contador] = produtoReceive.peso; // add vertor de 1500 de pesos
        contador++;
        if (contador >= QNTD_MAXIMA_PESOS) // imprime o relatório de tempos a cada 1500 produtos recebidos
        {
            somaPesoTotal();  //chamndo funcão para somar os pesos
            contador = 0;  // zerando o contador de produtos para mostrar o relatório depois de cada 1500 produtos guardados
            printf("Peso total: %d mg\n", pesoTotal);
            printf("Taxa de Atualização: %f ms\n", taxaDeAtualizacao);
            printf("WCET: %lld ms\n", WCET);
            xSemaphoreTake(semDisplay, portMAX_DELAY); 
            printf("Tempo do display: %lld ms\n", tempoDisplay);
            xSemaphoreGive(semDisplay);
        }
    }
}