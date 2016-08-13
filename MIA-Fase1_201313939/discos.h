#ifndef DISCOS_H
#define DISCOS_H
#include <time.h>

#define name_length 25
#define path_length 50
#define id_length 4

typedef struct MONTADOS{

    char path_montados[path_length];
    char name_montados[name_length];
    int disco_montados;
    char id_montados[4];
    char estado_montados;
    char uso_montados;

}montados;


typedef struct EXTENSION{
    char status;
    char fit;
    int start;
    int size;
    int sizeaux;
    int next;
    char name[16];

}ext;

typedef struct PARTICION{

    char status_particion; //1 activa, 0 caida
    char type_particion;
    char fit_particion;
    int start_particion;
    int size_particion;
    int sizeaux_particion;
    char name[name_length];
    ext exten[8];
}particion;

typedef struct DISCO{

    int size;
    time_t fecha;
    int sign;
    particion part[4];
    char name[name_length];
    int primarias;
    int extendidas;
    int logicas;

}disco;


#endif // DISCOS_H
