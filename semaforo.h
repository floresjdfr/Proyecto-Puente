#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

#define MAX_AUTOS 10;
char MEDIA_ESTE[] = "MEDIA_ESTE";
char MEDIA_OESTE[] = "MEDIA_OESTE";

struct puente_semaforo
{
    int tiempo_verde_este;
    int tiempo_verde_oeste;
    int este;
    int oeste;
    int uso_timer;
    pthread_mutex_t mutex;
};

struct puente
{
    int longitud_puente;
    int en_uso;
    pthread_mutex_t puente_lock;
};

struct info_autos
{
    int velocidad_auto_este;
    int velocidad_auto_oeste;
    int media_este;
    int media_oeste;
    struct puente_semaforo *semaforos;
    struct puente *puente;
};

int iniciar_semaforo();
int main_runner(struct info_autos *info);                                 //Esta funcion se encarga de crear y ejecutar threads que iniciaran la creacion de los semaforos y automoviles
void *create_semaforos(void *arg);                                        //Esta funcion encapsula la creacion del semaforo ubicado al este y oeste del puente
void *cambiar_semaforo_este(void *arg);                                   //Enciende el semaforo del oeste
void *cambiar_semaforo_oeste(void *arg);                                  //Enciende el semaforo del oeste
void *create_automoviles(void *arg);                                      //Funcion que crea los automoviles
void *automovil_este(void *arg);                                          //Crea los automoviles por que entran por el este
void *automovil_oeste(void *arg);                                         //Crea los automoviles que entran por el oeste
double ejecutar_integral(struct info_autos *info, char eleccion_media[]); //Funcion que ejecuta la integral exponencial encargada de decidir la velocidad con que se crean los automoviles;
void *crear_autos_este(void *arg);
void *crear_autos_oeste(void *arg);

int iniciar_semaforo()
{

    int longitud_puente;
    int media_este;
    int media_oeste;
    int velocidad_auto_este;
    int velocidad_auto_oeste;
    int tiempo_verde_este;
    int tiempo_verde_oeste;

    //Lectura de archivo de configuracion
    int aux;
    FILE *file;
    file = fopen("puente.conf", "r");

    fscanf(file, "LONGITUD_PUENTE=%d\n", &longitud_puente);
    fscanf(file, "MEDIA_ESTE=%d\n", &media_este);
    fscanf(file, "VELOCIDAD_AUTO_ESTE=%d\n", &velocidad_auto_este);
    fscanf(file, "K-ESTE=%d\n", &aux);
    fscanf(file, "TIEMPO_VERDE_ESTE=%d\n", &tiempo_verde_este);
    fscanf(file, "MEDIA_OESTE=%d\n", &media_oeste);
    fscanf(file, "VELOCIDAD_AUTO_OESTE=%d\n", &velocidad_auto_oeste);
    fscanf(file, "K-OESTE=%d\n", &aux);
    fscanf(file, "TIEMPO_VERDE_OESTE=%d\n", &tiempo_verde_oeste);
    fclose(file);

    //Struct donde se guardan los datos de los semaforos
    struct puente_semaforo *semaforos = malloc(sizeof(struct puente_semaforo));

    //Struct donde se guardan los datos del puente
    struct puente *puente = malloc(sizeof(struct puente));

    pthread_mutex_init(&semaforos->mutex, NULL);
    pthread_mutex_init(&puente->puente_lock, NULL);

    //Inicializacion de variables del puente
    puente->en_uso = 0;
    puente->longitud_puente = longitud_puente;

    //Inicializacion de variables de los semaforos
    semaforos->tiempo_verde_este = tiempo_verde_este;
    semaforos->tiempo_verde_oeste = tiempo_verde_oeste;
    semaforos->este = 0;
    semaforos->oeste = 1;
    semaforos->uso_timer = 0;

    //Inicializacion de varibales de struct donde se encaptulan las structs puente y semaforos
    struct info_autos *info = malloc(sizeof(struct info_autos));
    info->puente = puente;
    info->semaforos = semaforos;
    info->velocidad_auto_este = velocidad_auto_este;
    info->velocidad_auto_oeste = velocidad_auto_oeste;
    info->media_este = media_este;
    info->media_oeste = media_oeste;

    //Llamada a la funcion principal que iniciara la simulacion de los semaforos
    main_runner(info);

    free(info);
    free(semaforos);
    free(puente);
}

int main_runner(struct info_autos *info)
{

    //Threads encargados de crear los semaforos y los automoviles
    pthread_t thread_semaforos_creator, thread_automoviles_creator;

    pthread_create(&thread_semaforos_creator, NULL, create_semaforos, (void *)info->semaforos);
    pthread_create(&thread_automoviles_creator, NULL, create_automoviles, (void *)info);
    
    pthread_join(thread_semaforos_creator, NULL);
    pthread_join(thread_automoviles_creator, NULL);

    pthread_exit(0);
}

void *create_semaforos(void *arg)
{
    struct puente_semaforo *semaforos = (struct puente_semaforo *)arg;

    pthread_t thread_semaforo_este, thread_semaforo_oeste; //Un thread para cada semaforo

    pthread_create(&thread_semaforo_este, NULL, cambiar_semaforo_este, (void *)semaforos);
    pthread_create(&thread_semaforo_oeste, NULL, cambiar_semaforo_oeste, (void *)semaforos);

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

            sleep(semaforitos->tiempo_verde_este);

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

            sleep(semaforitos->tiempo_verde_oeste);

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

    pthread_t thread_crear_autos_este, thread_crear_autos_oeste;
    pthread_create(&thread_crear_autos_este, NULL, crear_autos_este, (void *)info);
    pthread_create(&thread_crear_autos_oeste, NULL, crear_autos_oeste, (void *)info);
    pthread_join(thread_crear_autos_este, NULL);
    pthread_join(thread_crear_autos_oeste, NULL);

    pthread_exit(0);
}

void *crear_autos_este(void *arg)
{

    struct info_autos *info = (struct info_autos *)arg;
    int total_autos = MAX_AUTOS;

    pthread_t automoviles_este[total_autos];

    for (int i = 0; i < total_autos; i++)
    {
        pthread_create(&automoviles_este[i], NULL, automovil_este, (void *)info);
    }

    for (int i = 0; i < total_autos; i++)
    {
        char media[] = "MEDIA_ESTE";
        int tiempo_creacion = (int) ejecutar_integral(info, media) * 1000000;
        pthread_join(automoviles_este[i], NULL);
        usleep(tiempo_creacion);
    }
    pthread_exit(0);
}

void *crear_autos_oeste(void *arg)
{

    struct info_autos *info = (struct info_autos *)arg;
    int total_autos = MAX_AUTOS;

    pthread_t automoviles_oeste[total_autos];

    for (int i = 0; i < total_autos; i++)
    {
        pthread_create(&automoviles_oeste[i], NULL, automovil_oeste, (void *)info);
    }

    for (int i = 0; i < total_autos; i++)
    {
        char media[] = "MEDIA_OESTE";
        int tiempo_creacion = (int) ejecutar_integral(info, media) * 1000000;
        pthread_join(automoviles_oeste[i], NULL);
        usleep(tiempo_creacion);
    }
    pthread_exit(0);
}

void *automovil_este(void *arg)
{
    struct info_autos *info = (struct info_autos *)arg;

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
    int velocidad_promedio_microseconds = (int)(info->puente->longitud_puente / info->velocidad_auto_este) * 1000000;
    usleep(velocidad_promedio_microseconds);
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
    int velocidad_promedio_microseconds = (int)(info->puente->longitud_puente / info->velocidad_auto_oeste) * 1000000;
    usleep(velocidad_promedio_microseconds);
    printf("Auto paso\n");
    pthread_mutex_unlock(&info->puente->puente_lock);
    pthread_exit(0);
}

double ejecutar_integral(struct info_autos *info, char eleccion_media[])
{
    double range = (0.9999999999999999 - 0);
    double div = RAND_MAX / range;
    double r = 0 + (rand() / div);
    double result = 0;
    if (eleccion_media = "MEDIA_ESTE")
    {
        result = -info->media_este* log(1 - r);
    }
    else if(eleccion_media = "MEDIA_OESTE"){
        result = -info->media_oeste* log(1 - r);
    }

    return result;
}
