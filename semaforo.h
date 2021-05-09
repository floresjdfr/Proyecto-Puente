#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

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
    struct puente_semaforo *semaforos;
    struct puente *puente;
};

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
    sleep(2);
    printf("Auto paso\n");
    pthread_mutex_unlock(&info->puente->puente_lock);
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
}

void *create_semaforos(void *arg)
{
    struct puente_semaforo *puente = (struct puente_semaforo *)arg;
    pthread_t thread_encender, thread_apagar;
    pthread_create(&thread_encender, NULL, cambiar_semaforo_este, (void *)puente);
    pthread_create(&thread_apagar, NULL, cambiar_semaforo_oeste, (void *)puente);
    pthread_join(thread_encender, NULL);
    pthread_join(thread_apagar, NULL);
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

int main_runner(struct info_autos *info)
{

    //Threads encargados de crear los semaforos automoviles
    pthread_t thread_semaforos_creator, thread_automoviles_creator;

    pthread_create(&thread_semaforos_creator, NULL, create_semaforos, (void *)info->semaforos);
    pthread_create(&thread_automoviles_creator, NULL, create_automoviles, (void *)info);

    pthread_join(thread_semaforos_creator, NULL);
    pthread_join(thread_automoviles_creator, NULL);

    // pthread_create(&automovil, NULL, automovil_este, (void *)puente);
    // pthread_create(&automovil2, NULL, automovil_oeste, (void *)puente);

    // pthread_join(automovil, NULL);

    // pthread_join(automovil2, NULL);

    // pthread_t automovil_thread;
}

int iniciar_semaforo()
{
    //Estructura donde se guardan los datos de los semaforos
    struct puente_semaforo *semaforos = malloc(sizeof(struct puente_semaforo));

    //Struct donde se guardan los datos del puente
    struct puente *puente = malloc(sizeof(struct puente));

    pthread_mutex_init(&semaforos->mutex, NULL);
    pthread_mutex_init(&puente->puente_lock, NULL);

    puente->en_uso = 0;

    semaforos->este = 1;
    semaforos->oeste = 0;
    semaforos->timer = 5;
    semaforos->uso_timer = 0;

    struct info_autos *info = malloc(sizeof(struct info_autos));
    info->puente = puente;
    info->semaforos = semaforos;

    main_runner(info);

    free(info);
    free(semaforos);
    free(puente);
}