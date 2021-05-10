#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

#define MAX_AUTOS 10;

struct puente_semaforo
{
    int este;
    int oeste;
    int timer;
    int uso_timer;
    pthread_mutex_t mutex;
};

struct puente
{
    int en_uso;
    pthread_mutex_t puente_lock;
};

struct info_autos
{
    int *media_counter;
    double *media_creacion_automoviles;
    struct puente_semaforo *semaforos;
    struct puente *puente;
};

int iniciar_semaforo();
int main_runner(struct info_autos *info); //Esta funcion se encarga de crear y ejecutar threads que iniciaran la creacion de los semaforos y automoviles
void *create_semaforos(void *arg); //Esta funcion encapsula la creacion del semaforo ubicado al este y oeste del puente
void *cambiar_semaforo_este(void *arg);//Enciende el semaforo del oeste
void *cambiar_semaforo_oeste(void *arg);//Enciende el semaforo del oeste
void *create_automoviles(void *arg);//Funcion que crea los automoviles
void *automovil_este(void *arg);//Crea los automoviles por que entran por el este
void *automovil_oeste(void *arg);//Crea los automoviles que entran por el oeste
double ejecutar_integral(double *media, int *counter);//Funcion que ejecuta la integral exponencial encargada de decidir la velocidad con que se crean los automoviles;
double decidir_este_oeste(); //Funcion que decidira el sentido del automovil. Si es menor que 50 el automovil entra por el este, si es mayor entra por el oeste


int iniciar_semaforo()
{
    //Struct donde se guardan los datos de los semaforos
    struct puente_semaforo *semaforos = malloc(sizeof(struct puente_semaforo));

    //Struct donde se guardan los datos del puente
    struct puente *puente = malloc(sizeof(struct puente));

    pthread_mutex_init(&semaforos->mutex, NULL);
    pthread_mutex_init(&puente->puente_lock, NULL);

    //Inicializacion de variables del puente
    puente->en_uso = 0;

    //Inicializacion de variables de los semaforos
    semaforos->este = 1;
    semaforos->oeste = 0;
    semaforos->timer = 2;
    semaforos->uso_timer = 0;

    //Inicializacion de varibales de struct donde se encaptulan las structs puente y semaforos
    struct info_autos *info = malloc(sizeof(struct info_autos));
    info->puente = puente;
    info->semaforos = semaforos;
    info->media_creacion_automoviles = malloc(sizeof(double));
    *info->media_creacion_automoviles = 1;
    info->media_counter = malloc(sizeof(int));
    *info->media_counter = 0;

    //Llamada a la funcion principal que iniciara la simulacion de los semaforos
    main_runner(info);

    free(info);
    free(semaforos);
    free(puente);
}
int main_runner(struct info_autos *info){

    //Threads encargados de crear los semaforos automoviles
    /* pthread_t thread_semaforos_creator, thread_automoviles_creator;

    pthread_create(&thread_semaforos_creator, NULL, create_semaforos, (void *)info->semaforos);
    pthread_create(&thread_automoviles_creator, NULL, create_automoviles, (void *)info);

    pthread_join(thread_semaforos_creator, NULL);
    pthread_join(thread_automoviles_creator, NULL); */

    

    for (int i = 0; i < 10; i++){
        //info->media_creacion_automoviles = ejecutar_integral(info->media_creacion_automoviles);
        printf("%f ", ejecutar_integral(info->media_creacion_automoviles, info->media_counter));
    }

    pthread_exit(0);
}
void *create_semaforos(void *arg) 
{
    struct puente_semaforo *puente = (struct puente_semaforo *)arg;

    pthread_t thread_semaforo_este, thread_semaforo_oeste;
    
    pthread_create(&thread_semaforo_este, NULL, cambiar_semaforo_este, (void *)puente);
    pthread_create(&thread_semaforo_oeste, NULL, cambiar_semaforo_oeste, (void *)puente);
    
    pthread_join(thread_semaforo_este, NULL);
    pthread_join(thread_semaforo_oeste, NULL);

    pthread_exit(0);
}
void *cambiar_semaforo_este(void *arg)
{
    struct puente_semaforo *semaforitos = (struct puente_semaforo *)arg;
    while (1)
    {
        if (semaforitos->este == 0 && semaforitos->oeste == 1 && semaforitos->uso_timer == 0)
        {
            pthread_mutex_lock(&semaforitos->mutex);
            semaforitos->este = 1;
            semaforitos->oeste = 0;
            semaforitos->uso_timer = 1;
            printf("Semaforo 'este' encendido y 'oeste' apagado.\n");
            pthread_mutex_unlock(&semaforitos->mutex);

            sleep(semaforitos->timer);
            pthread_mutex_lock(&semaforitos->mutex);
            semaforitos->uso_timer = 0;
            pthread_mutex_unlock(&semaforitos->mutex);
        }
    }

    pthread_exit(0);
}
void *cambiar_semaforo_oeste(void *arg)
{
    struct puente_semaforo *semaforitos = (struct puente_semaforo *)arg;
    while (1)
    {
        if (semaforitos->este == 1 && semaforitos->oeste == 0 && semaforitos->uso_timer == 0)
        {
            pthread_mutex_lock(&semaforitos->mutex);
            semaforitos->este = 0;
            semaforitos->oeste = 1;
            semaforitos->uso_timer = 1;
            printf("Semaforo 'este' apagado y 'oeste' encendido.\n");
            pthread_mutex_unlock(&semaforitos->mutex);

            sleep(semaforitos->timer);
            pthread_mutex_lock(&semaforitos->mutex);
            semaforitos->uso_timer = 0;
            pthread_mutex_unlock(&semaforitos->mutex);
        }
    }

    pthread_exit(0);
}
void *create_automoviles(void *arg)
{

    struct info_autos *info = (struct info_autos *)arg;
    int total_autos = MAX_AUTOS;

    pthread_t automoviles_este[total_autos];
    pthread_t automoviles_oeste[total_autos];
    for (int i = 0; i < total_autos; i++)
    {
        pthread_create(&automoviles_este[i], NULL, automovil_este, (void *)info);
    }

    for (int i = 0; i < total_autos; i++)
    {
        pthread_create(&automoviles_oeste[i], NULL, automovil_oeste, (void *)info);
    }

    for (int i = 0; i < total_autos; i++)
    {
        pthread_join(automoviles_este[i], NULL);
    }

    for (int i = 0; i < total_autos; i++)
    {
        pthread_join(automoviles_oeste[i], NULL);
    }
    pthread_exit(0);
}
void *automovil_este(void *arg)
{
    struct info_autos *info = (struct info_autos *)arg;

    // printf("En uso: %d\n", semaforito->en_uso);
    // printf("Este: %d\n", semaforito->este);

    // if ((semaforito->en_uso == 0) && (semaforito->este == 1))
    // {
    //     pthread_mutex_lock(&semaforito->mutex);
    //     semaforito->en_uso = 1;
    //     printf("Auto pasando este->oeste\n");
    //     pthread_mutex_unlock(&semaforito->mutex);
    //     sleep(2);
    //     pthread_mutex_lock(&semaforito->mutex);
    //     printf("Auto paso\n");
    //     semaforito->en_uso = 0;
    //     pthread_mutex_unlock(&semaforito->mutex);
    // }

start:
    while (info->semaforos->oeste == 1)
    {
    }
    pthread_mutex_lock(&info->puente->puente_lock);
    if (info->semaforos->oeste == 1)
    {
        pthread_mutex_unlock(&info->puente->puente_lock);
        goto start;
    }
    printf("Auto pasando este->oeste\n");
    sleep(2);
    printf("Auto paso\n");
    pthread_mutex_unlock(&info->puente->puente_lock);
    pthread_exit(0);
}
void *automovil_oeste(void *arg)
{
    struct info_autos *info = (struct info_autos *)arg;

start:
    while (info->semaforos->este == 1)
    {
    }
    pthread_mutex_lock(&info->puente->puente_lock);
    if (info->semaforos->este == 1)
    {
        pthread_mutex_unlock(&info->puente->puente_lock);
        goto start;
    }
    printf("Auto pasando oeste->este\n");
    sleep(1);
    printf("Auto paso\n");
    pthread_mutex_unlock(&info->puente->puente_lock);
    pthread_exit(0);
}
double ejecutar_integral(double *media, int *counter){
    double range = (0.9999999999999999 - 0); 
    double div = RAND_MAX / range;
    double r = 0 + (rand() / div);
    double result = -*media*log(1-r);
    *counter++;
    //*media = mediu

    //return -media*log(1-r);
}











