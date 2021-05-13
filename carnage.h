#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

#define MAX_AUTOS 10;
char MEDIA_ESTE[] = "MEDIA_ESTE";
char MEDIA_OESTE[] = "MEDIA_OESTE";

struct puente
{
    int longitud_puente;
    int en_uso;
    int este;
    int oeste;
    pthread_mutex_t puente_lock;
};

struct info_autos
{
    int velocidad_auto_este;
    int velocidad_auto_oeste;
    int media_este;
    int media_oeste;
    struct puente *puente;
};

int iniciar_semaforo();
int main_runner(struct info_autos *info); //Esta funcion se encarga de crear y ejecutar threads que iniciaran la creacion automoviles
void *create_automoviles(void *arg);      //Funcion que crea los automoviles
void *automovil_este(void *arg);          //Crea los automoviles por que entran por el este
void *automovil_oeste(void *arg);         //Crea los automoviles que entran por el oeste
void *crear_autos_este(void *arg);
void *crear_autos_oeste(void *arg);
double ejecutar_integral(struct info_autos *info, char eleccion_media[]); //Funcion que ejecuta la integral exponencial encargada de decidir la velocidad con que se crean los automoviles;

int iniciar_semaforo()
{
    int longitud_puente;
    int media_este;
    int media_oeste;
    int velocidad_auto_este;
    int velocidad_auto_oeste;

    //Lectura de archivo de configuracion
    int aux;
    FILE *file;
    file = fopen("puente.conf", "r");

    fscanf(file, "LONGITUD_PUENTE=%d\n", &longitud_puente);
    fscanf(file, "MEDIA_ESTE=%d\n", &media_este);
    fscanf(file, "VELOCIDAD_AUTO_ESTE=%d\n", &velocidad_auto_este);
    fscanf(file, "K-ESTE=%d\n", &aux);
    fscanf(file, "TIEMPO_VERDE_ESTE=%d\n", &aux);
    fscanf(file, "MEDIA_OESTE=%d\n", &media_oeste);
    fscanf(file, "VELOCIDAD_AUTO_OESTE=%d\n", &velocidad_auto_oeste);
    fscanf(file, "K-OESTE=%d\n", &aux);
    fscanf(file, "TIEMPO_VERDE_OESTE=%d\n", &aux);
    fclose(file);


    //Struct donde se guardan los datos del puente
    struct puente *puente = (struct puente*)malloc(sizeof(struct puente));

    pthread_mutex_init(&puente->puente_lock, NULL);

    //Inicializacion de variables del puente
    puente->en_uso = 0;
    puente->longitud_puente = longitud_puente;
    puente->este = 0;
    puente->oeste = 1;

    //Inicializacion de varibales de struct donde se encaptulan las structs puente y semaforos
    struct info_autos *info = (struct info_autos*)malloc(sizeof(struct info_autos));
    info->puente = puente;
    info->velocidad_auto_este = velocidad_auto_este;
    info->velocidad_auto_oeste = velocidad_auto_oeste;
    info->media_este = media_este;
    info->media_oeste = media_oeste;
    

    //Llamada a la funcion principal que iniciara la simulacion de los semaforos
    main_runner(info);

    free(info);
    free(puente);
}


int main_runner(struct info_autos *info)
{

    //Thread encargado de crear los automoviles
    pthread_t thread_automoviles_creator;

    pthread_create(&thread_automoviles_creator, NULL, create_automoviles, (void *)info);

    pthread_join(thread_automoviles_creator, NULL);

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
        int tiempo_creacion = (int)ejecutar_integral(info, media) * 1000000;
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
        int tiempo_creacion = (int)ejecutar_integral(info, media) * 1000000;
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
        result = -info->media_este * log(1 - r);
    }
    else if (eleccion_media = "MEDIA_OESTE")
    {
        result = -info->media_oeste * log(1 - r);
    }

    return result;
}