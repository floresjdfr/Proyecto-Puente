#include "semaforo.h"

int elegir_opcion();

int main(int argc, char const *argv[])
{
    int opcion_elegida = elegir_opcion();
    switch (opcion_elegida)
    {
    case 2:
        iniciar_semaforo();
        break;
    
    default:
        break;
    }
    
    
    return 0;
}


int elegir_opcion(){
    puts("1. Carnage");
    puts("2. Semaforo");
    puts("3. Oficial de transito");

    int opcion;
    scanf("%d", &opcion);
    return opcion;
}