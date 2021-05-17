#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

#define MAX_AUTOS_CARNAGE 1000
char MEDIA_ESTE_CARNAGE[] = "MEDIA_ESTE";
char MEDIA_OESTE_CARNAGE[] = "MEDIA_OESTE";

struct puente_carnage
{
    int sentido;        //1 = este 0 = oeste
    int sentido_actual; //1 = este 0 = oeste
    int longitud_puente;
    pthread_mutex_t *puente_lock;
};

struct info_autos_carnage
{
    int velocidad_auto_este;
    int velocidad_auto_oeste;
    int media_este;
    int media_oeste;
    struct puente_carnage *puente;
};

int iniciar_carnage();
int main_runner_carnage(struct info_autos_carnage *info); //Esta funcion se encarga de crear y ejecutar threads que iniciaran la creacion de los semaforos y automoviles
void *create_automoviles_carnage(void *arg);              //Funcion que crea los automoviles
void *automovil_este_carnage(void *arg);                  //Crea los automoviles por que entran por el este
void *automovil_oeste_carnage(void *arg);                 //Crea los automoviles que entran por el oeste
void *crear_autos_este_carnage(void *arg);
void *crear_autos_oeste_carnage(void *arg);
double ejecutar_integral_carnage(struct info_autos_carnage *info, char eleccion_media[]); //Funcion que ejecuta la integral exponencial encargada de decidir la velocidad con que se crean los automoviles;
int revisar_sentido_carnage(struct puente_carnage *puente);                               //funcion que recorre el arreglo de mutex revisando si hay un automovil
//usando el puente o no

int iniciar_carnage()
{

    int longitud_puente;
    int media_este;
    int media_oeste;
    int velocidad_auto_este;
    int velocidad_auto_oeste;

    //Lectura de archivo d21e configuracion
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
    struct puente_carnage *puente = (struct puente_carnage *)malloc(sizeof(struct puente_carnage));

    //Inicializacion de variables del puente
    puente->longitud_puente = longitud_puente;
    puente->sentido = -1;
    puente->sentido_actual = 1;
    puente->puente_lock = (pthread_mutex_t *)calloc(puente->longitud_puente, sizeof(pthread_mutex_t));
    //inicializacion de mutex del puente
    for (int i = 0; i < puente->longitud_puente; i++)
    {
        pthread_mutex_init(&puente->puente_lock[i], NULL);
    }

    //Inicializacion de varibales de struct donde se encaptulan las structs puente y semaforos
    struct info_autos_carnage *info = (struct info_autos_carnage *)malloc(sizeof(struct info_autos_carnage));
    info->puente = puente;
    info->velocidad_auto_este = velocidad_auto_este;
    info->velocidad_auto_oeste = velocidad_auto_oeste;
    info->media_este = media_este;
    info->media_oeste = media_oeste;

    //Llamada a la funcion principal que iniciara la simulacion de los semaforos
    main_runner_carnage(info);

    free(puente->puente_lock);
    free(info);
    free(puente);
}

int main_runner_carnage(struct info_autos_carnage *info)
{

    //Threads encargados de crear los semaforos y los automoviles
    pthread_t thread_semaforos_creator, thread_automoviles_creator;

    pthread_create(&thread_automoviles_creator, NULL, create_automoviles_carnage, (void *)info);

    pthread_join(thread_automoviles_creator, NULL);

    pthread_exit(0);
}

void *create_automoviles_carnage(void *arg)
{

    struct info_autos *info = (struct info_autos *)arg;

    pthread_t thread_crear_autos_este, thread_crear_autos_oeste;
    pthread_create(&thread_crear_autos_este, NULL, crear_autos_este_carnage, (void *)info);
    pthread_create(&thread_crear_autos_oeste, NULL, crear_autos_oeste_carnage, (void *)info);
    pthread_join(thread_crear_autos_este, NULL);
    pthread_join(thread_crear_autos_oeste, NULL);

    pthread_exit(0);
}

void *crear_autos_este_carnage(void *arg)
{
    struct info_autos_carnage *info = (struct info_autos_carnage *)arg;
    int total_autos = MAX_AUTOS_CARNAGE;
    pthread_t automoviles_este[total_autos];

    for (int i = 0; i < total_autos; i++)
    {
        char media[] = "MEDIA_ESTE";
        int tiempo_creacion = (int)ejecutar_integral_carnage(info, media) * 1000000;
        pthread_create(&automoviles_este[i], NULL, automovil_este_carnage, (void *)info);
        usleep(tiempo_creacion);
    }

    for (int i = 0; i < total_autos; i++)
    {

        pthread_join(automoviles_este[i], NULL);
    }
    pthread_exit(0);
}

void *crear_autos_oeste_carnage(void *arg)
{
    struct info_autos_carnage *info = (struct info_autos_carnage *)arg;
    int total_autos = MAX_AUTOS_CARNAGE;
    pthread_t automoviles_oeste[total_autos];

    for (int i = 0; i < total_autos; i++)
    {
        char media[] = "MEDIA_OESTE";
        int tiempo_creacion = (int)ejecutar_integral_carnage(info, media) * 1000000;
        pthread_create(&automoviles_oeste[i], NULL, automovil_oeste_carnage, (void *)info);
        usleep(tiempo_creacion);
    }
    for (int i = 0; i < total_autos; i++)
    {

        pthread_join(automoviles_oeste[i], NULL);
    }
    pthread_exit(0);
}

void *automovil_este_carnage(void *arg)
{
    struct info_autos_carnage *info = (struct info_autos_carnage *)arg;
    double duracion_total = (double)(info->puente->longitud_puente / info->velocidad_auto_este);
    double duracion_por_posicion_array = (double)(duracion_total / info->puente->longitud_puente); //Esta es la duracion
    //del vehiculo en cada espacio del array;
    double duracion_micro = (double)(duracion_por_posicion_array * 1000000);

start:
    printf("Auto '%ld' esperando este->oeste\n", pthread_self());
    while (info->puente->sentido == 0)
    {
    }

    info->puente->sentido = 1;

    for (int i = 0; i < info->puente->longitud_puente; i++)
    {
        if (i == 0)
            printf("PASA ESTE #%ld\n", pthread_self());

        pthread_mutex_lock(&info->puente->puente_lock[i]);
        printf("Auto '%ld' pasando '%d'este->oeste\n", pthread_self(), i);
        usleep((int)duracion_micro);
        pthread_mutex_unlock(&info->puente->puente_lock[i]);
    }

    info->puente->sentido = revisar_sentido_carnage(info->puente);

    pthread_exit(0);
}

void *automovil_oeste_carnage(void *arg)
{
    struct info_autos_carnage *info = (struct info_autos_carnage *)arg;
    double duracion_total = (double)(info->puente->longitud_puente / info->velocidad_auto_este);
    double duracion_por_posicion_array = (double)(duracion_total / info->puente->longitud_puente); //Esta es la duracion
    //del vehiculo en cada espacio del array.tos *)arg;
    double duracion_micro = (double)(duracion_por_posicion_array * 1000000);
    printf("Auto '%ld' esperando oeste->este\n", pthread_self());

start:
    
    while (info->puente->sentido == 1)
    {
    }

    info->puente->sentido = 0;

    for (int i = info->puente->longitud_puente - 1; i >= 0; i--)
    {   
        if (i == info->puente->longitud_puente - 1)
            printf("PASA OESTE #%ld\n", pthread_self());
        pthread_mutex_lock(&info->puente->puente_lock[i]);
        printf("Auto '%ld' pasando '%d'oeste->este\n", pthread_self(), i);
        usleep((int)duracion_micro);
        pthread_mutex_unlock(&info->puente->puente_lock[i]);
    }

    info->puente->sentido = revisar_sentido_carnage(info->puente);

    pthread_exit(0);
}

double ejecutar_integral_carnage(struct info_autos_carnage *info, char eleccion_media[])
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

int revisar_sentido_carnage(struct puente_carnage *puente)
{
    int puente_libre_counter = 0;
    for (int i = 0; i < puente->longitud_puente; i++)
    {
        if (pthread_mutex_trylock(&puente->puente_lock[i]) == 0)
        {
            pthread_mutex_unlock(&puente->puente_lock[i]);
            puente_libre_counter++;
        }
    }
    if (puente_libre_counter == puente->longitud_puente) //si la cantidad de espacios disponibles es igual a la longitud del puente, el puente esta libre de autos
        return -1;
    else
        return puente->sentido;
}

int es_ambulancia_carnage()
{
    int ganador = rand() % 100;
    if (ganador > 19 && ganador < 41) //Si ganador esta entre 20 y 40 el automivil es una ambulancia;
        return 1;
    return 0;
}
