#ifndef H_CONTROLE
#define H_CONTROLE

#define QNTD_MAXIMA_PESOS 1500
extern TaskHandle_t esteira1_handle;
extern TaskHandle_t esteira2_handle;
extern TaskHandle_t esteira3_handle;
extern SemaphoreHandle_t semEsteira1;
extern SemaphoreHandle_t semEsteira2;
extern SemaphoreHandle_t semEsteira3;
extern SemaphoreHandle_t semDisplay;
extern QueueHandle_t filaDasEsteiras;
extern uint64_t tempoDisplay;

void task_controle(void * pvp);

struct 
{
    uint16_t peso;
    uint8_t esteira;

} typedef dadoEsteira;

#endif