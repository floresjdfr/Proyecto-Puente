#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <math.h>


#define MAX_AUTOS 1000
char MEDIA_ESTE[] = "MEDIA_ESTE";
char MEDIA_OESTE[] = "MEDIA_OESTE";

struct semaforo
{
    int tiempo_verde_este;
    int tiempo_verde_oeste;
    int este;
    int oeste;
    int uso_timer;
    pthread_mutex_t mutex;
    struct puente *puente;
};

struct puente
{
    int sentido;        //1 = este 0 = oeste
    int sentido_actual; //1 = este 0 = oeste
    int longitud_puente;
    pthread_mutex_t *puente_lock;
};

struct info_autos
{
    int velocidad_auto_este;
    int velocidad_auto_oeste;
    int media_este;
    int media_oeste;
    struct semaforo *semaforos;
    struct puente *puente;
};

int iniciar_semaforo();
int main_runner(struct info_autos *info); //Esta funcion se encarga de crear y ejecutar threads que iniciaran la creacion de los semaforos y automoviles
void *create_semaforos(void *arg);        //Esta funcion encapsula la creacion del semaforo ubicado al este y oeste del puente
void *cambiar_semaforo_este(void *arg);   //Enciende el semaforo del oeste
void *cambiar_semaforo_oeste(void *arg);  //Enciende el semaforo del oeste
void *create_automoviles(void *arg);      //Funcion que crea los automoviles
void *automovil_este(void *arg);          //Crea los automoviles por que entran por el este
void *automovil_oeste(void *arg);         //Crea los automoviles que entran por el oeste
void *crear_autos_este(void *arg);
void *crear_autos_oeste(void *arg);
double ejecutar_integral(struct info_autos *info, char eleccion_media[]); //Funcion que ejecuta la integral exponencial encargada de decidir la velocidad con que se crean los automoviles;
int revisar_puente_en_uso_semaforo(struct puente *puente);

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
    struct semaforo *semaforos = malloc(sizeof(struct semaforo));

    //Struct donde se guardan los datos del puente
    struct puente *puente = malloc(sizeof(struct puente));

    //Inicializacion de variables del puente
    puente->longitud_puente = longitud_puente;
    puente->sentido = 1;
    puente->sentido_actual = 1;
    puente->puente_lock = calloc(puente->longitud_puente, sizeof(pthread_mutex_t));
    //inicializacion de mutex del puente
    for (int i = 0; i < puente->longitud_puente; i++)
    {
        pthread_mutex_init(&puente->puente_lock[i], NULL);
    }

    //Inicializacion de variables de los semaforos
    semaforos->tiempo_verde_este = tiempo_verde_este;
    semaforos->tiempo_verde_oeste = tiempo_verde_oeste;
    semaforos->este = 0;
    semaforos->oeste = 1;
    semaforos->uso_timer = 0;
    semaforos->puente = puente;

    pthread_mutex_init(&semaforos->mutex, NULL);

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

    free(puente->puente_lock);
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
    struct semaforo *semaforos = (struct semaforo *)arg;

    pthread_t thread_semaforo_este, thread_semaforo_oeste; //Un thread para cada semaforo

    pthread_create(&thread_semaforo_este, NULL, cambiar_semaforo_este, (void *)semaforos);
    pthread_create(&thread_semaforo_oeste, NULL, cambiar_semaforo_oeste, (void *)semaforos);

    pthread_join(thread_semaforo_este, NULL);
    pthread_join(thread_semaforo_oeste, NULL);

    pthread_exit(0);
}

void *cambiar_semaforo_este(void *arg)
{
    struct semaforo *semaforitos = (struct semaforo *)arg;
    while (1)
    {
        if (semaforitos->este == 0 && semaforitos->oeste == 1 && semaforitos->uso_timer == 0)
        {
            pthread_mutex_lock(&semaforitos->mutex);
            printf("Semaforo 'este' encendido y 'oeste' apagado.\n");
            semaforitos->este = 1;
            semaforitos->oeste = 0;
            semaforitos->uso_timer = 1;
            semaforitos->puente->sentido = 1;
            

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
    struct semaforo *semaforitos = (struct semaforo *)arg;
    while (1)
    {
        if (semaforitos->este == 1 && semaforitos->oeste == 0 && semaforitos->uso_timer == 0)
        {
            pthread_mutex_lock(&semaforitos->mutex);
            printf("Semaforo 'este' apagado y 'oeste' encendido.\n");
            semaforitos->este = 0;
            semaforitos->oeste = 1;
            semaforitos->uso_timer = 1;
            semaforitos->puente->sentido = 0;
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
        char media[] = "MEDIA_ESTE";
        int tiempo_creacion = (int)ejecutar_integral(info, media) * 1000000;
        usleep(tiempo_creacion);
    }

    for (int i = 0; i < total_autos; i++)
    {
        pthread_join(automoviles_este[i], NULL);   
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
        char media[] = "MEDIA_OESTE";
        int tiempo_creacion = (int)ejecutar_integral(info, media) * 1000000;
        usleep(tiempo_creacion);
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
    double duracion_total = (double)(info->puente->longitud_puente / info->velocidad_auto_este);
    double duracion_por_posicion_array = (double)(duracion_total / info->puente->longitud_puente); //Esta es la duracion
    //del vehiculo en cada espacio del array;
    double duracion_micro = (double)(duracion_por_posicion_array * 1000000);

start:
    while (info->puente->sentido == 0)
    {
    }
    if (info->puente->sentido_actual == 0)
        if (revisar_puente_en_uso_semaforo(info->puente) == 0)
            info->puente->sentido_actual = 1;
        else
            goto start;

    for (int i = 0; i < info->puente->longitud_puente; i++)
    {
        if (i == 0)
            printf("PASA ESTE #%ld\n", pthread_self());
        pthread_mutex_lock(&info->puente->puente_lock[i]);
        printf("Auto '%ld' pasando '%d'este->oeste\n", pthread_self(), i);
        usleep((int)duracion_micro);
        pthread_mutex_unlock(&info->puente->puente_lock[i]);
    }
    pthread_exit(0);
}

void *automovil_oeste(void *arg)
{
    struct info_autos *info = (struct info_autos *)arg;
    double duracion_total = (double)(info->puente->longitud_puente / info->velocidad_auto_este);
    double duracion_por_posicion_array = (double)(duracion_total / info->puente->longitud_puente); //Esta es la duracion
    //del vehiculo en cada espacio del array.tos *)arg;
    double duracion_micro = (double)(duracion_por_posicion_array * 1000000);

start:
    while (info->puente->sentido == 1)
    {
    }
    if (info->puente->sentido_actual == 1)
        if (revisar_puente_en_uso_semaforo(info->puente) == 0)
            info->puente->sentido_actual = 0;
        else
            goto start;

    for (int i = info->puente->longitud_puente - 1; i >= 0; i--)
    {
        if (i == info->puente->longitud_puente - 1)
            printf("PASA OESTE #%ld \n", pthread_self());
        pthread_mutex_lock(&info->puente->puente_lock[i]);
        printf("Auto '%ld' pasando '%d'oeste->este\n", pthread_self(), i);
        usleep((int)duracion_micro);
        pthread_mutex_unlock(&info->puente->puente_lock[i]);
    }
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

int revisar_puente_en_uso_semaforo(struct puente *puente)
{
    int puente_libre_counter = 0;
    for (int i = 0; i < puente->longitud_puente; i++)
    {
        int lock_izquierda = pthread_mutex_trylock(&puente->puente_lock[i]);//revisa la disponibilidad del puente de izquierda a derecha
        int lock_derecha = pthread_mutex_trylock(&puente->puente_lock[(puente->longitud_puente-1)-i]); //revisa la disponibilidad del puente de derecha a izquierda
        if (lock_izquierda == 0 && lock_derecha == 0)
        {
            pthread_mutex_unlock(&puente->puente_lock[i]);
            pthread_mutex_unlock(&puente->puente_lock[(puente->longitud_puente-1)-i]);
            puente_libre_counter++;
            continue;
        }
        if (lock_izquierda == 0){
            pthread_mutex_unlock(&puente->puente_lock[i]);
        }
        else if (lock_derecha == 0){
            pthread_mutex_unlock(&puente->puente_lock[(puente->longitud_puente-1)-i]);
        }
    }
    if (puente_libre_counter == (puente->longitud_puente)) //si la cantidad de espacios disponibles es igual a la longitud del puente, el puente esta libre de autos
        return 0;
    else
        return 1;
}

int es_ambulancia()
{
    int ganador = rand() % 100;
    if (ganador > 19 && ganador < 41) //Si ganador esta entre 20 y 40 el automivil es una ambulancia;
        return 1;
    return 0;
}
