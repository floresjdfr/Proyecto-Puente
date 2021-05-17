#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

#define MAX_AUTOS_OFICIAL 10
#define MAX_STACK_OFICIAL 20
char MEDIA_ESTE_OFICIAL[] = "MEDIA_ESTE";
char MEDIA_OESTE_OFICIAL[] = "MEDIA_OESTE";

struct oficial
{
    int k_este;  //Cantidad de autos que deja pasar el oficial del este
    int k_oeste; //Cantidad de autos que deja pasar el oficial del oeste
    int este;    //este=1 si los automoviles van a pasar por el este
    int oeste;   //oeste=1 si los automoviles van a pasar por el oeste
    int contador_este;
    int contador_oeste;
    int *control_autos_este;
    int *control_autos_oeste;
    pthread_mutex_t lock_oficial;
};

struct puente_oficial
{
    int longitud_puente;
    int sentido; //1 = este 0 = oeste
    int sentido_actual;
    pthread_mutex_t puente_check;
    pthread_mutex_t *puente_lock;
};

struct info_autos_oficial
{
    int velocidad_auto_este;
    int velocidad_auto_oeste;
    int media_este;
    int media_oeste;
    struct oficial *oficial;
    struct puente_oficial *puente;
};

void iniciar_oficial();
int main_runner_oficial(struct info_autos_oficial *info); //Esta funcion se encarga de crear y ejecutar threads que iniciaran la creacion de los semaforos y automoviles
void *create_oficial(void *arg);                          //Esta funcion encapsula la creacion del semaforo ubicado al este y oeste del puente
void *oficial_este(void *arg);                            //Enciende el semaforo del oeste
void *oficial_oeste(void *arg);                           //Enciende el semaforo del oeste
void *create_automoviles_oficial(void *arg);              //Funcion que crea los automoviles
void *automovil_este_oficial(void *arg);                  //Crea los automoviles por que entran por el este
void *automovil_oeste_oficial(void *arg);                 //Crea los automoviles que entran por el oeste
void *crear_autos_este_oficial(void *arg);
void *crear_autos_oeste_oficial(void *arg);
double ejecutar_integral_oficial(struct info_autos_oficial *info, char eleccion_media[]); //Funcion que ejecuta la integral exponencial encargada de decidir la velocidad con que se crean los automoviles;
int revisar_puente_en_uso_oficial(struct info_autos_oficial *info);

void iniciar_oficial()
{
    int longitud_puente;
    int media_este;
    int media_oeste;
    int velocidad_auto_este;
    int velocidad_auto_oeste;
    int k_este;
    int k_oeste;

    //Lectura de archivo de configuracion
    int aux;
    FILE *file;
    file = fopen("puente.conf", "r");

    fscanf(file, "LONGITUD_PUENTE=%d\n", &longitud_puente);
    fscanf(file, "MEDIA_ESTE=%d\n", &media_este);
    fscanf(file, "VELOCIDAD_AUTO_ESTE=%d\n", &velocidad_auto_este);
    fscanf(file, "K-ESTE=%d\n", &k_este);
    fscanf(file, "TIEMPO_VERDE_ESTE=%d\n", &aux);
    fscanf(file, "MEDIA_OESTE=%d\n", &media_oeste);
    fscanf(file, "VELOCIDAD_AUTO_OESTE=%d\n", &velocidad_auto_oeste);
    fscanf(file, "K-OESTE=%d\n", &k_oeste);
    fscanf(file, "TIEMPO_VERDE_OESTE=%d\n", &aux);
    fclose(file);

    //Struct donde se guardan los datos del oficial
    struct oficial *oficial = (struct oficial *)malloc(sizeof(struct oficial));

    //Struct donde se guardan los datos del puente
    struct puente_oficial *puente = (struct puente_oficial *)malloc(sizeof(struct puente_oficial));

    //Inicializacion de variables del puente
    puente->longitud_puente = longitud_puente;
    puente->sentido = 1;
    puente->sentido_actual = 1;
    pthread_mutex_init(&puente->puente_check, NULL);
    puente->puente_lock = calloc(puente->longitud_puente, sizeof(pthread_mutex_t));
    //inicializacion de mutex del puente
    for (int i = 0; i < puente->longitud_puente; i++)
    {
        pthread_mutex_init(&puente->puente_lock[i], NULL);
    }

    //Inicializacion de oficial de transito
    pthread_mutex_init(&oficial->lock_oficial, NULL);
    oficial->k_este = k_este;
    oficial->k_oeste = k_oeste;
    oficial->este = 1;
    oficial->oeste = 0;
    oficial->contador_este = 0;
    oficial->contador_oeste = 0;
    oficial->control_autos_este = calloc(sizeof(int), k_este);
    oficial->control_autos_oeste = calloc(sizeof(int), k_oeste);

    //Inicializacion de varibales de struct donde se encaptulan las structs puente y y oficial
    struct info_autos_oficial *info = (struct info_autos_oficial *)malloc(sizeof(struct info_autos_oficial));
    info->puente = puente;
    info->oficial = oficial;
    info->velocidad_auto_este = velocidad_auto_este;
    info->velocidad_auto_oeste = velocidad_auto_oeste;
    info->media_este = media_este;
    info->media_oeste = media_oeste;

    //Llamada a la funcion principal que iniciara la simulacion de los semaforos
    main_runner_oficial(info);

    free(puente->puente_lock);
    free(info);
    free(oficial->control_autos_este);
    free(oficial->control_autos_oeste);
    free(oficial);
    free(puente);
}

int main_runner_oficial(struct info_autos_oficial *info)
{
    struct info_autos_oficial *info_autos = info;
    //Threads encargados de crear los oficiales de transito y los automoviles
    pthread_t thread_oficial_creator, thread_automoviles_creator;

    pthread_create(&thread_oficial_creator, NULL, create_oficial, (void *)info_autos);
    pthread_create(&thread_automoviles_creator, NULL, create_automoviles_oficial, (void *)info_autos);

    pthread_join(thread_oficial_creator, NULL);
    pthread_join(thread_automoviles_creator, NULL);

    pthread_exit(0);
}

void *create_oficial(void *arg)
{
    struct info_autos_oficial *oficial = (struct info_autos_oficial *)arg;

    pthread_t thread_oficial_este, thread_oficial_oeste; //Un thread para cada semaforo

    pthread_create(&thread_oficial_este, NULL, oficial_este, (void *)oficial);
    pthread_create(&thread_oficial_oeste, NULL, oficial_oeste, (void *)oficial);

    pthread_join(thread_oficial_este, NULL);
    pthread_join(thread_oficial_oeste, NULL);

    pthread_exit(0);
}

void *oficial_este(void *arg)
{
    struct info_autos_oficial *info = (struct info_autos_oficial *)arg;
    while (1)
    {
        while (info->oficial->este == 0 && info->oficial->oeste == 1)
            ;
        pthread_mutex_lock(&info->oficial->lock_oficial);
        info->oficial->contador_este = 0;
        
        info->puente->sentido = 1;
        printf("Oficial deja pasar vehiculos este\n");
        pthread_mutex_unlock(&info->oficial->lock_oficial);

        while (info->oficial->contador_este < info->oficial->k_este)
            ;
        pthread_mutex_lock(&info->oficial->lock_oficial);
        info->oficial->este = 0;
        info->oficial->oeste == 1;
        pthread_mutex_unlock(&info->oficial->lock_oficial);
    }
    pthread_exit(0);
}

void *oficial_oeste(void *arg)
{
    struct info_autos_oficial *info = (struct info_autos_oficial *)arg;
    while (1)
    {
        while (info->oficial->este == 1 && info->oficial->oeste == 0)
            ;
        pthread_mutex_lock(&info->oficial->lock_oficial);
        info->oficial->contador_oeste = 0;
        info->puente->sentido = 0;
        printf("Oficial deja pasar vehiculos oeste\n");
        pthread_mutex_unlock(&info->oficial->lock_oficial);

        while (info->oficial->contador_oeste < info->oficial->k_oeste)
            ;
        pthread_mutex_lock(&info->oficial->lock_oficial);
        info->oficial->este = 1;
        info->oficial->oeste = 0;
        pthread_mutex_unlock(&info->oficial->lock_oficial);
    }
    pthread_exit(0);
}

void *create_automoviles_oficial(void *arg)
{

    struct info_autos_oficial *info = (struct info_autos_oficial *)arg;

    pthread_t thread_crear_autos_este, thread_crear_autos_oeste;
    pthread_create(&thread_crear_autos_este, NULL, crear_autos_este_oficial, (void *)info);
    pthread_create(&thread_crear_autos_oeste, NULL, crear_autos_oeste_oficial, (void *)info);
    pthread_join(thread_crear_autos_este, NULL);
    pthread_join(thread_crear_autos_oeste, NULL);

    pthread_exit(0);
}

void *crear_autos_este_oficial(void *arg)
{

    struct info_autos_oficial *info = (struct info_autos_oficial *)arg;
    int total_autos = MAX_AUTOS_OFICIAL;

    pthread_t automoviles_este[total_autos];

    for (int i = 0; i < total_autos; i++)
    {
        char media[] = "MEDIA_ESTE";
        int tiempo_creacion = (int)ejecutar_integral_oficial(info, media) * 1000000;
        pthread_create(&automoviles_este[i], NULL, automovil_este_oficial, (void *)info);
        usleep(tiempo_creacion);
    }

    for (int i = 0; i < total_autos; i++)
    {
        pthread_join(automoviles_este[i], NULL);
    }
    pthread_exit(0);
}

void *crear_autos_oeste_oficial(void *arg)
{

    struct info_autos_oficial *info = (struct info_autos_oficial *)arg;
    int total_autos = MAX_AUTOS_OFICIAL;

    pthread_t automoviles_oeste[total_autos];

    for (int i = 0; i < total_autos; i++)
    {
        pthread_create(&automoviles_oeste[i], NULL, automovil_oeste_oficial, (void *)info);
    }

    for (int i = 0; i < total_autos; i++)
    {
        char media[] = "MEDIA_OESTE";
        int tiempo_creacion = (int)ejecutar_integral_oficial(info, media) * 1000000;
        pthread_join(automoviles_oeste[i], NULL);
        usleep(tiempo_creacion);
    }
    pthread_exit(0);
}

void *automovil_este_oficial(void *arg)
{
    struct info_autos_oficial *info = (struct info_autos_oficial *)arg;
    double duracion_total = (double)(info->puente->longitud_puente / info->velocidad_auto_este);
    double duracion_por_posicion_array = (double)(duracion_total / info->puente->longitud_puente); //Esta es la duracion
                                                                                                   //del vehiculo en cada espacio del array;
    double duracion_micro = (double)(duracion_por_posicion_array * 1000000);

start:
    while (info->puente->sentido == 0)
        ;

    if (info->puente->sentido_actual == 0)
        if (revisar_puente_en_uso_oficial(info) == 0)
            info->puente->sentido_actual = 1;
        else
            goto start;

    for (int i = 0; i < info->puente->longitud_puente; i++)
    {
        if (i == 0)
        {
            pthread_mutex_lock(&info->puente->puente_lock[i]);
            pthread_mutex_lock(&info->oficial->lock_oficial);
            info->oficial->contador_este++;
            printf("PASA ESTE #%d\n", info->oficial->contador_este);
            pthread_mutex_unlock(&info->oficial->lock_oficial);
            printf("Auto '%ld' pasando '%d'este->oeste\n", pthread_self(), i);
            usleep((int)duracion_micro);
            pthread_mutex_unlock(&info->puente->puente_lock[i]);
        }
        else
        {
            pthread_mutex_lock(&info->puente->puente_lock[i]);
            printf("Auto '%ld' pasando '%d'este->oeste\n", pthread_self(), i);
            usleep((int)duracion_micro);
            pthread_mutex_unlock(&info->puente->puente_lock[i]);
        }
    }
    pthread_exit(0);
}

void *automovil_oeste_oficial(void *arg)
{
    struct info_autos_oficial *info = (struct info_autos_oficial *)arg;
    double duracion_total = (double)(info->puente->longitud_puente / info->velocidad_auto_este);
    double duracion_por_posicion_array = (double)(duracion_total / info->puente->longitud_puente); //Esta es la duracion
    //del vehiculo en cada espacio del array.tos *)arg;
    double duracion_micro = (double)(duracion_por_posicion_array * 1000000);
    int auto_id;

start:
    while (info->puente->sentido == 1)
        ;

    if (info->puente->sentido_actual == 1)
        if (revisar_puente_en_uso_oficial(info) == 0)
            info->puente->sentido_actual = 0;
        else
            goto start;

    for (int i = info->puente->longitud_puente-1; i >= 0; i--)
    {
        if (i == info->puente->longitud_puente-1)
        {
            pthread_mutex_lock(&info->puente->puente_lock[i]);
            pthread_mutex_lock(&info->oficial->lock_oficial);
            info->oficial->contador_oeste++;
            printf("PASA OESTE #%d\n", info->oficial->contador_oeste);
            pthread_mutex_unlock(&info->oficial->lock_oficial);
            printf("Auto '%ld' pasando '%d'oeste->este\n", pthread_self(), i);
            usleep((int)duracion_micro);
            pthread_mutex_unlock(&info->puente->puente_lock[i]);
        }
        else
        {
            pthread_mutex_lock(&info->puente->puente_lock[i]);
            printf("Auto '%ld' pasando '%d'oeste->este\n", pthread_self(), i);
            usleep((int)duracion_micro);
            pthread_mutex_unlock(&info->puente->puente_lock[i]);
        }
    }
    pthread_exit(0);
}

int revisar_puente_en_uso_oficial(struct info_autos_oficial *info)
{
    int puente_libre_counter = 0;
    for (int i = 0; i < info->puente->longitud_puente; i++)
    {
        if (pthread_mutex_trylock(&info->puente->puente_lock[i]) == 0)
        {
            pthread_mutex_unlock(&info->puente->puente_lock[i]);
            puente_libre_counter++;
        }
    }
    if (puente_libre_counter == info->puente->longitud_puente) //si la cantidad de espacios disponibles es igual a la longitud del puente, el puente esta libre de autos
        return 0;
    else
        return 1;
}

int revisar_puente_en_uso_oeste(struct info_autos_oficial *info)
{
    int puente_libre = 0;
    for (int i = 0; i < info->oficial->k_oeste; i++)
    {
        puente_libre += info->oficial->control_autos_oeste[i];
    }
    if (puente_libre == 0) //si la cantidad de espacios disponibles es igual a la longitud del puente, el puente esta libre de autos
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

double ejecutar_integral_oficial(struct info_autos_oficial *info, char eleccion_media[])
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