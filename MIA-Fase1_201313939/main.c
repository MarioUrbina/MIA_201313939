#include <stdio.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <stdbool.h>
#include "discos.h"

int CreacionDisco(char* name,char* size, char* unit, char*path, int ran);
int EDisco(char* path);
int marca=0;
montados montado[30];
int salir = 0;
char lectura[500];
char lectura2[1000];

int main(int argc, char *argv[])
{
   int num = 0;
   for (num = 0; num <50;num++){
       montado[num].estado_montados = "0";
       montado[num].estado_montados = "0";
       strcpy(montado[num].path_montados,"0");
   }
   while(salir == 0 ){
       int num = 0;
       memset (lectura,'\0',1000);
       printf("\n Porfavor Ingrese Instrucciones \n");
       fgets(lectura,1000,stdin);
       for( num=0;num<1000 && lectura[num]!= '\0';num++){
           if(lectura[num] == '\n'){
               lectura[num] = '\0';
           }else if(lectura[num]=='\\'){
               printf("Diagonal");
               int num2=num+1;
               while(lectura[num2]!='\n'){
                   lectura[num2]=' ';
                   num2++;
               }
                lectura[num2]=' ';
                        printf("\n Porfavor Ingrese Instrucciones siguiente\n");
             fgets(lectura2,500,stdin);
             strcat(lectura,lectura2);
           }else{

            lectura[num]= tolower(lectura[num]);
           }
       }

  printf("%c",lectura);
         char * poco = NULL;
          poco = strtok(lectura, " ");
  if(poco!=NULL){
        if(strcmp(poco,"salida")==0 || strcmp(poco,"SALIDA")==0 || strcmp(poco,"Salida")==0 ){
               salir=1;
                    }
   }
         if(poco!=NULL){
              analizador(poco);
          }

       }
       return 0;
}

int existDisk (char* path){

    int retorno = 0;

    printf("%s\n", path);

    FILE *exists = fopen(path, "rb+");

    if (exists == NULL){
        retorno = 0;
    }else{
        fclose(exists);
        retorno = 1;
    }

    return retorno;
}
int getInit(char* path, char type, DISCO auxMBR, char * size, char * fit, char* name){

    int start = 0;

    /* Caso 1: primera partición, extendida o primaria. Dado que es la primera
        * partición simplemente la ubicamos luego de nuestra estructura del MBR
        * es decir el tamaño del MBR del disco más 1.
        */

    if (auxMBR.primarias == 0 && auxMBR.extendidas == 0){
        printf("First partition\n");
        start = sizeof(MBR) + 1;
        return start;
    }

    /* Caso 2: primera partición lógica. Vamos a buscar a la tabla de particiones
         * el byte de inicio de la partición extendida y se asigna a la nueva partición
         * lógica
         */

    else if (type == 'l' || type == 'L'){

        if (auxMBR.logicas == 0){
            int o = 0;
            for (o = 0; o < 4; o++){
                if (auxMBR.part[o].type_particion == 'e' || auxMBR.part[o].type_particion == 'E'){

                    // Antes de ingresar debemos validar si la partición lógica es más pequeña que la extendida

                    if (auxMBR.part[o].size_particion < size){
                        printf("Erro, no Hay suficiente Espacio para trabajar.\n");
                        return -1;
                    }else{

                        start = auxMBR.part[o].part_start;
                        return start;
                    }
                }
            }
        }else{

            /* Caso 3: no sea la primera lógica. Debemos ir leyendo a lo largo de
                 * nuestra partición extendida hasta encontrar la última partición
                 * de tipo lógica se obtiene su inicio y su tamaño y se asigna a la
                 * nueva partición.
                 */

            ext aux_ebr, ref_ebr;
            ref_ebr.start = 0;

            FILE * ebr_reader = fopen(path, "rb+");
            fseek(ebr_reader, sizeof(DISCO) + 1, SEEK_SET);

            while (fread(&aux_ebr, sizeof(ext), 1, ebr_reader)){
                if (aux_ebr.next!=-1 && aux_ebr.status == '0' && aux_ebr.size == -1){
                    // Es el primer EBR
                    if (size <= aux_ebr.next){

                        fseek(ebr_reader, -1*sizeof(ext), SEEK_CUR);
                        aux_ebr.fit = fit;
                        strcpy(aux_ebr.name, name);
                        aux_ebr.size = size;
                        aux_ebr.status = '1';
                        fwrite(&aux_ebr, sizeof(ext),1,ebr_reader);
                        fclose(ebr_reader);
                        return -10;
                    }
                    // En medio
                }else if (aux_ebr.next != -1){
                    int free = 0;

                    free = aux_ebr.start + aux_ebr.size;
                    free = aux_ebr.next - free;

                    if (free >= size){
                        int next_ref = aux_ebr.next;
                        int pos = aux_ebr.start + aux_ebr.size;

                        fseek(ebr_reader, -1*sizeof(ext),SEEK_CUR);
                        aux_ebr.next = pos;
                        fwrite(&aux_ebr, sizeof(ext),1,ebr_reader);

                        fseek(ebr_reader, pos, SEEK_SET);
                        aux_ebr.start = pos;
                        aux_ebr.next = next_ref;
                        aux_ebr.fit = fit;
                        strcpy(aux_ebr.name, name);
                        aux_ebr.size = size;
                        aux_ebr.status = '1';
                        fwrite(&aux_ebr, sizeof(ext),1,ebr_reader);
                        fclose(ebr_reader);
                        return -10;
                    }
                }else{
                    // Hasta el fin
                    start = aux_ebr.start + aux_ebr.size;

                    /* Validamos si luego de la última partición lógica hay espacio suficiente
                         * para ingresar una nueva
                         */

                    int oo = 0;
                    for (oo = 0; oo < 4; oo++){
                        if (auxMBR.part[oo].type_particion == 'e' || auxMBR.part[oo].type_particion == 'E'){
                            int end_extended = auxMBR.part[oo].start + auxMBR.part[oo].size;
                            int free = end_extended = end_extended - start;
                            if (free < size){
                                printf("No hay Suficiente espacion el la particion extendida.\n");
                                fclose(ebr_reader);
                                return -1;
                            }else{
                                fclose(ebr_reader);
                                return start;
                            }
                        }
                    }

                }
            }


        }
    }else{

        /* Caso 4: no sea la primera partición. Vamos a recorrer la tabla de particiones
                * ubicada en el MBR hasta encontrar la partición que está hasta atrás del disco.
                */

        PARTICION aux_partition;
        aux_partition.part_start = 0;

        int w = 0;
        int ref = 0, free_space = 0;
        for (w = 0; w < 4; w++){

            ref = auxMBR.part[w].start_particion + auxMBR.part[w].size_particion;

            if (auxMBR.part[0].start_particion - (sizeof(DISCO) + 1)>=size){
                start = sizeof(DISCO) + 1;
                break;
            }else if (auxMBR.part[w+1].status_particion == '0'){
                start = auxMBR.part[w].start_particion + auxMBR.part[w].size_particion;
                break;
            }else{

                free_space = auxMBR.part[w+1].start_particion - ref;

                if (free_space >= size){
                    start = ref;
                    break;
                }
            }
        }
        return start;
    }
}
void bubble_sort(char * path){

    DISCO aux_mbr;

    FILE * sort_writer = fopen (path, "rb+");
    fseek(sort_writer, 0, SEEK_SET);

    fread(&aux_mbr, sizeof(DISCO),1, sort_writer);

    int x = 0;
    int y = 0;
    PARTICION aux;

    for (x= 0; x<4; x++){
        for (y = 0; y<3-x;y++){
            if (aux_mbr.part[y].start_particion>aux_mbr.part[y+1].start_particion){

                aux = aux_mbr.part[y];
                aux_mbr.part[y]=aux_mbr.part[y+1];
                aux_mbr.part[y+1]=aux;


            }
        }
    }

    for (x= 0; x<4; x++){
        for (y = 0; y<3-x;y++){
            if (aux_mbr.part[y].status_particion == '0'){

                aux = aux_mbr.part[y];
                aux_mbr.part[y]=aux_mbr.part[y+1];
                aux_mbr.part[y+1]=aux;


            }
        }
    }

    fseek(sort_writer, 0, SEEK_SET);
    fwrite(&aux_mbr, sizeof(DISCO),1,sort_writer);
    fclose(sort_writer);
}


int ejecutarFdisk(char *name, char *size, char *path, char *unit, char *type, char *fit){
    int i = 1;
    while(name[i] != '\"'){
       name[i-1]=name[i];
       i = i +1;
    }
    name[i-1]='\0';
    printf("name2:%s\n",name);

    i =1;
     while(path[i] != '\"'){
        path[i-1]=path[i];
        i = i +1;
     }
     path[i-1]='\0';
     printf("path2:%s\n",path);

     direccion = path;
     //strcpy(direccion, path);
     printf("direccion:%s\n", direccion);

        int flag = 0;
     int fix_size = 0;
     int get_start = 0;

     // Antes de iniciar verificamos si el disco existe.

     if(existDisk(path)==0){
         printf("Error el disco no existe\n");
         printf("------------------------------------------------------------------------------\n");
         return;
     }else{

         // Ya que el disco existe voy a leer su MBR para hacer validaciones.

         DISCO aux_mbr;

         FILE * bibliophile = fopen (path, "rb+");
         fseek(bibliophile, 0, SEEK_SET);
         fread(&aux_mbr, sizeof(MBR), 1, bibliophile);

         /* Validación 1: Luego de leer el MBR verificamos si la partición
              * que queremos ingresar es más pequeña que el tamaño libre del disco
              * para ello primero verificamos la unidad (B, K, M).
              *
              * Si la partición cabe en el disco cambiamos flag a 1, de lo contrario
              * permanece en 0.
              */
            int i;
         for (i =0; i<=4;i++){
                 if (unit == 'b' || unit == 'B'){
                     if (aux_mbr.part[i].sizeaux_particion >= size){
                         flag = 1;
                         fix_size = size;
                     }else{
                         flag = 0;
                     }
                 }else if (unit == 'k' || unit == 'K'){
                     if (aux_mbr.part[i].sizeaux_particion >= size * 1024){
                         flag = 1;
                         fix_size = size * 1024;
                     }else{
                         flag = 0;
                     }
                 }else if (unit == 'm' || unit == 'M'){
                     if (aux_mbr.part[i].sizeaux_particion >= size * 1024 * 1024){
                         flag = 1;
                         fix_size = size * 1024 * 1024;
                     }else{
                         flag = 0;
                     }
                 }else{
                     printf("Error, no puede usar %c como unidad.\n", unit);
                     return 0;
                 }
         }

         int rec = 0;
         for (rec = 0; rec<4;rec++){
             if (strcasecmp(aux_mbr.part[rec].name, name)==0 && aux_mbr.part[rec].status_particion == '1'){
                 printf("fdisk: you already have a partition named %s.\n", name);
                 return 0;
             }
         }

         fseek(bibliophile, sizeof(MBR) + 1, SEEK_SET);
         ext aux_ebr;
         while(fread(&aux_ebr, sizeof(ext),1,bibliophile)){

             if (strcasecmp(aux_ebr.name, name)==0 && aux_ebr.status == '1'){
                 printf("Error, el disco ya tiene una particion con el mismo nombre %s.\n", name);
                 return 0;
             }
         }

         // Examinamos los valores de flag para saber si seguir o no.

         if (flag == 0){
             printf("Error No hay espacio Suficiente \n");
             printf("------------------------------------------------------------------------------\n");
             return 0;
         }else{

             /* Validación 2: Ya que sabemos que nuestra partición cabe en el disco debemos
                  * hacer las respectivas restricciones sobre el número de particiones.
                  *
                  * Cuatro particiones primarias como máximo o tres primarias y una extendida,
                  * dentro de la extendida se podrán albergar n particiones lógicas, si hay
                  * espacio suficiente claro.
                  */

             if (type == 'p' || type == 'P'){
                 if (aux_mbr.primarias <= 3){

                     // Se procede a ingresar la partición primaria

                     if (aux_mbr.primarias == 3 && aux_mbr.extendidas == 1){
                         printf("Error Ya tiene 3 particiones primarias y 1 extendida\n");
                         printf("------------------------------------------------------------------------------\n");
                         return;
                     }else{
                         get_start = getInit(path, type, aux_mbr, fix_size, fit, name);
                         writePartition(fit, get_start, fix_size, type, name, path);
                     }
                 }else{
                     printf("Error Ya tiene 4 particiones.\n");
                     printf("------------------------------------------------------------------------------\n");
                     return;
                 }
             }else if (type == 'e' || type == 'E'){


                 if (aux_mbr.extendidas == 0 && aux_mbr.primarias <=3){

                     // Se procede a ingresar la partición extendida

                     get_start = getInit(path, type, aux_mbr, fix_size, fit, name);
                     writePartition(fit, get_start, fix_size, type, name, path);

                 }else{
                     printf("Error ya tiene particion extendida \n");
                     printf("------------------------------------------------------------------------------\n");
                     return;
                 }
             }else if (type == 'l' || type == 'L'){
                 if (aux_mbr.extendidas == 0){
                     printf("Error debe crear particion extendida Primero.\n");
                     printf("------------------------------------------------------------------------------\n");
                     return;
                 }else{

                     // Se procede a ingresar la partición lógica

                     get_start = getInit(path, type, aux_mbr, fix_size,fit, name);

                     if (get_start == -1 || get_start == -10){
                         return;
                     }else{
                         writePartition(fit, get_start, fix_size, type, name, path);
                     }
                 }
             }
         }
         fclose(bibliophile);
         bubble_sort(path);
     }



















     FILE * disk = fopen (direccion, "rb+");
     DISCO aux_mbr;
     fseek(disk, 0, SEEK_SET);
     fread(&aux_mbr, sizeof(MBR), 1, primary_read);

     /* Escritura de particiones primarias: dado que ya se validó anteriormente las cuestiones de
      * espacio y restricciones solamente debemos recorrer la tabla de particiones para
      * encontrar la posición que se encuentra vacía y aumentar el contador de
      * particiones primarias.
      */

     if (type == 'p' || type == 'P' || type == 'e' || type == 'E'){

         int l = 0;
         for (l = 0; l < 4; l++){
             if (aux_mbr.part[l].status_particion=='0'){
                 aux_mbr.part[l].fit_particion = fit;
                 strcpy(aux_mbr.part[l].name, name);
                 aux_mbr.part[l].size_particion = size;
                 aux_mbr.part[l].sizeaux_particion = aux_mbr.part[l].sizeaux_particion - aux_mbr.part[l].size_particion;
                 aux_mbr.part[l].start_particion = start;
                 aux_mbr.part[l].status_particion = '1';
                 aux_mbr.part[l].type_particion = type;
                 break;
             }
         }

         if (type == 'p' || type == 'P'){
             aux_mbr.primary_partitions++;
         }else{
             aux_mbr.extended_partition++;
         }


         fseek(primary_read, -1 * sizeof(MBR), SEEK_CUR);
         fwrite(&aux_mbr, sizeof(MBR), 1, primary_read);

         /* Escritura de la partición extendida: al igual que con las primarias ya se ha validado lo
          * necesario sobre espacio y restricciones así que debemos primero sobreescribir el MBR del disco
          * y agregar la partición y agregar el primer EBR al inicio de la misma.
          *
          * Se podría considerar la unión del proceso de escritura de lógicas y primarias.
          */

         if (type == 'e' || type == 'E'){

             EBR aux_ebr;

             fseek(primary_read, start, SEEK_SET);

             aux_ebr.part_fit = fit;
             strcpy(aux_ebr.part_name, "No logical partitions yet");
             aux_ebr.part_next = -1;
             aux_ebr.part_size = -1;
             aux_ebr.part_start = start;
             aux_ebr.part_status = '1';

             fwrite(&aux_ebr, sizeof(aux_ebr), 1, primary_read);
         }



     }else if (type == 'l' || type == 'L'){

         printf("Logical partition...\n");

         /* Escritura de particiones lógicas: a diferencia de las extendidas, las particiones lógicas no se
          * ubican dentro del MBR del disco, sino cada una cuenta con un EBR al inicio de su espacio, por
          * tanto para crearlas debemos escribir el EBR en el byte que obtuvimos con getStart.
          */

         EBR aux_ebr;

         /* Pueden haber dos tipos de escritura para las particiones lógicas, 1. Cuando se encuentra con
          * la primera partición a ingresar para la cual ya existe un EBR que modificar y 2. Cuando no es la
          * primera partición lógica por tanto debemos enlazar los EBR existentes.
          */

         if (aux_mbr.logic_partition == 0){

             fseek(primary_read, start, SEEK_SET);
             fread(&aux_ebr, sizeof(EBR), 1, primary_read);

             aux_ebr.part_fit = fit;
             strcpy(aux_ebr.part_name, name);
             aux_ebr.part_next = -1;
             aux_ebr.part_size = size;
             aux_ebr.part_start = start;
             aux_ebr.part_status = '1';

             fseek(primary_read, -1 * sizeof(EBR), SEEK_CUR);
             fwrite(&aux_ebr, sizeof(EBR), 1, primary_read);

         }else{

             fseek(primary_read, sizeof(MBR) + 1, SEEK_SET);

             while (fread(&aux_ebr, sizeof(EBR), 1, primary_read)){
                 if (aux_ebr.part_next == -1){
                     aux_ebr.part_next = start;
                     fseek(primary_read, -1 * sizeof(EBR), SEEK_CUR);
                     fwrite(&aux_ebr, sizeof(EBR), 1, primary_read);
                 }
             }

             fseek(primary_read, start, SEEK_SET);

             aux_ebr.part_fit = fit;
             strcpy(aux_ebr.part_name, name);
             aux_ebr.part_next = -1;
             aux_ebr.part_size = size;
             aux_ebr.part_start = start;
             aux_ebr.part_status = '1';

             fwrite(&aux_ebr, sizeof(EBR), 1, primary_read);
         }

         fseek(primary_read, 0, SEEK_SET);
         fread(&aux_mbr, sizeof(MBR),1,primary_read);

         aux_mbr.logic_partition++;
         fseek(primary_read, -1 * sizeof(MBR), SEEK_CUR);
         fwrite(&aux_mbr,sizeof(MBR),1,primary_read);
     }

     fclose(primary_read);
}

void analizador(char *instrucciones){
    char *i2 = NULL;
    char *cont = NULL;
    char instruccion[100];
    char* size=NULL;
    char* unit=NULL;
    char* path=NULL;
    char* name=NULL;
    int i, flag_size=0,flag_path=0,flag_name=0,flag_unit=0;


    if (instrucciones[0] == '#'){
        printf("se reconocio comentario\n");
    }
    else if(strcmp(instrucciones,"mkdisk")==0
            ||strcmp(instrucciones,"Mkdisk")==0
            || strcmp(instrucciones,"MKDISK")==0
            || strcmp(instrucciones,"MKdisk")==0) {

        printf("mkdisk");
        printf("-%s\n",instrucciones);
        instrucciones = strtok(NULL, " ");

        while(instrucciones!=NULL){
            printf("instruccion con cont:%s\n",instrucciones);
            memset(instruccion, '\0', 100);
            for (i =0; i <100 && instrucciones[i]!=':'; i = i +1){
                instruccion[i] = instrucciones[i];
            }
            printf("instruccion:%s\n", instruccion);
            cont = strstr(instrucciones,"::");
            strcpy(cont,&cont[2]);
            printf("cont------------..%s\n",cont);

            if (strcmp(instruccion, "-size")==0){
                printf("size:%s\n", cont);
                size = cont;
                flag_size=1;
            }
            else if (strcmp(instruccion, "-name")==0){
                printf("Nombre:%s\n", cont);
                name = cont;
                flag_name=1;
            }
            else if (strcmp(instruccion, "-path")==0){
                printf("Ruta:%s\n", cont);
                path= cont;
                flag_path=1;
            }
            else if (strcmp(instruccion, "+unit")==0){
                printf("Unidad:%s\n", cont);
                unit = cont;
                flag_unit=1;
            }
            else{
                printf("error: instruccion '%s' no existe\n",instruccion);
                return NULL;
            }

             i2 = instrucciones;
             char* ayuda= strstr(instrucciones,"\\");

             if(ayuda!=NULL){
                 printf("ayuda:%s\n",ayuda);
                 instrucciones = strtok(NULL, " ");
                 printf("inst:%s\n",instrucciones);
                /* while((strstr(instrucciones,"\"")==NULL)
                       && instrucciones!=NULL){
                     strcat(i2,instrucciones);
                     strcat(i2," ");
                     instrucciones = strtok(NULL, " ");
                 }
                 strcat(i2,instrucciones);*/
             }

             instrucciones = strtok( NULL, " ");
             printf("inst:%s\n",instrucciones);

        }
         printf("hola todos");
        if (flag_name == 0){
            printf("No Ingreso Nombre \n");
        }
        if (flag_size == 0){
            printf("No Ingreso Size\n");
        }
        if (flag_size == 0){
            printf("No Ingreso Path\n");
        }
        if (flag_unit == 0){
            unit = "M";
        }
       CreacionDisco(name,size,unit,path,marca);
        //marca++;
    }
    if(strcmp(instrucciones,"rmdisk")==0
            ||strcmp(instrucciones,"RMdisk")==0
            || strcmp(instrucciones,"RMDISK")==0) {
        char * pathE = NULL;
        int flag_pathE = 0;
        printf("rmdisk");
        instrucciones = strtok(NULL, " ");
        int i, flag_path=0;
        while(instrucciones!=NULL){
            printf("instruccion con cont:%s\n",instrucciones);
            memset(instruccion, '\0', 100);
            for (i =0; i <100 && instrucciones[i]!=':'; i = i +1){
                instruccion[i] = instrucciones[i];
            }
            printf("instruccion:%s\n", instruccion);
            cont = strstr(instrucciones,"::");
            strcpy(cont,&cont[2]);
            printf("cont------------..%s\n",cont);

            if (strcmp(instruccion, "-path")==0){
                printf("size:%s\n", cont);
                pathE = cont;
                flag_pathE=1;
             i2 = instrucciones;
             char* ayuda= strstr(instrucciones,"\\");

             if(ayuda!=NULL){
                 printf("ayuda:%s\n",ayuda);
                 instrucciones = strtok(NULL, " ");
                 printf("inst:%s\n",instrucciones);
                /* while((strstr(instrucciones,"\"")==NULL)
                       && instrucciones!=NULL){
                     strcat(i2,instrucciones);
                     strcat(i2," ");
                     instrucciones = strtok(NULL, " ");
                 }
                 strcat(i2,instrucciones);*/
             }
             instrucciones = strtok( NULL, " ");
        }
            else{
                printf("error: instruccion '%s' no existe\n",instruccion);
                return NULL;
            }

        if (flag_pathE == 0){
            printf("No Ingreso Path\n");
            }


        }
        i =1;
         while(cont[i] != '\"'){
            cont[i-1]=cont[i];
            i = i +1;
         }
         cont[i-1]='\0';
        printf(cont);
        EDisco(cont);
     }
    if(strcmp(instrucciones,"fdisk")==0
            ||strcmp(instrucciones,"Fdisk")==0
            || strcmp(instrucciones,"FDISK")==0) {
        char* size=NULL;
        char* unit=NULL;
        char* path=NULL;
        char* name=NULL;
        char* type=NULL;
        char* del=NULL;
        char* fit=NULL;

        int i, flag_size=0,flag_path=0,flag_name=0,flag_unit=0,flag_type=0,flag_fit=0,flag_delete=0;
        printf("fdisk");
        printf("-%s\n",instrucciones);
        instrucciones = strtok(NULL, " ");

        while(instrucciones!=NULL){
            printf("instruccion con cont:%s\n",instrucciones);
            memset(instruccion, '\0', 100);
            for (i =0; i <100 && instrucciones[i]!=':'; i = i +1){
                instruccion[i] = instrucciones[i];
            }
            printf("instruccion:%s\n", instruccion);
            cont = strstr(instrucciones,"::");
            strcpy(cont,&cont[2]);
            printf("cont------------..%s\n",cont);

            if (strcmp(instruccion, "-size")==0){
                printf("size:%s\n", cont);
                size = cont;
                flag_size=1;
            }
            else if (strcmp(instruccion, "-name")==0){
                printf("Nombre:%s\n", cont);
                name = cont;
                flag_name=1;
            }
            else if (strcmp(instruccion, "-path")==0){
                printf("Ruta:%s\n", cont);
                path= cont;
                flag_path=1;
            }

            else if (strcmp(instruccion, "+unit")==0){
                printf("Unidad:%s\n", cont);
                unit = cont;
                flag_unit=1;
            }
            else if (strcmp(instruccion, "+type")==0){
                printf("Unidad:%s\n", cont);
                type = cont;
                flag_type=1;
            }
            else if (strcmp(instruccion, "+fit")==0){
                printf("Unidad:%s\n", cont);
                fit = cont;
                flag_fit=1;
            }
            else if (strcmp(instruccion, "+delete")==0){
                printf("Unidad:%s\n", cont);
                del = cont;
                flag_delete=1;
            }
            else{
                printf("error: instruccion '%s' no existe\n",instruccion);
                return NULL;
            }
             i2 = instrucciones;
             char* ayuda= strstr(instrucciones,"\\");

             if(ayuda!=NULL){
                 printf("ayuda:%s\n",ayuda);
                 instrucciones = strtok(NULL, " ");
                 printf("inst:%s\n",instrucciones);
                /* while((strstr(instrucciones,"\"")==NULL)
                       && instrucciones!=NULL){
                     strcat(i2,instrucciones);
                     strcat(i2," ");
                     instrucciones = strtok(NULL, " ");
                 }
                 strcat(i2,instrucciones);*/
             }
             instrucciones = strtok( NULL, " ");
        }
        if (flag_name == 0){
            printf("No Ingreso Nombre \n");
        }
        if (flag_size == 0){
            printf("No Ingreso Size\n");
        }
        if (flag_path == 0){
            printf("No Ingreso Path\n");
        }
        if (flag_unit == 0){
            unit = "M";
        }
        if (flag_type == 0){
            type = "P";
        }
        if (flag_fit == 0){
            fit = "W";
        }
        if (flag_delete == 0){
        }
        ejecutarFdisk(name, size, path, unit, type, fit);
    }

}

int CreacionDisco(char *name,char* size, char* unit, char*path, int ran){
    printf ("Entro a Cread Disco");
     int aux=0;
     char *direccion;
     int i =1;
     printf("name:%s\n",name);
     printf("size:%s\n",size);
     printf("unit:%s\n",unit);
     printf("path:%s\n",path);
     while(name[i] != '\"'){
        name[i-1]=name[i];
        i = i +1;
     }
     name[i-1]='\0';
     printf("name2:%s\n",name);

     i =1;
      while(path[i] != '\"'){
         path[i-1]=path[i];
         i = i +1;
      }
      path[i-1]='\0';
      printf("path2:%s\n",path);

      direccion = path;
      //strcpy(direccion, path);
      printf("direccion:%s\n", direccion);

      strcat(direccion, name);
      printf("direccion:%s\n", direccion);

      if(strcmp(unit,"m")==0){
           aux =1;
      }else if(strcmp(unit,"k")==0){
            aux=2;
        }else{
            printf("Unidad Inexistente \n");
            return 0;}

      char *aux2;
       char aux3[200];
       i =1;
      aux2= direccion;
        /*crear carpeta si no existe
      for(i=0;i<200;i++){
          if(aux2[i]=='/'){
              aux3[i]=aux2[i];
              char *aux7= (char*)malloc(150);
              strcpy(aux7,"mkdir ");
              strcat(aux7,aux3);
              //printf("direccion: %s\n",aux7);
              system(aux7);
              free(aux7);
          }
          aux3[i]=aux2[i];
          if(aux2[i]=='\0'){
              break;
          }
      }

      for(i=0;i<200;i++){
          if(aux3[i]=='\n'){
              aux3[i]='\0';
          }
      }*/

      char buffer[1024];
       i=0;
      for(i=0;i<1024;i++){
          buffer[i]='0';
      }

       disco inf;
       inf.fecha = time(0);
       inf.sign = ran;
       inf.primarias = 0;
       inf.logicas = 0;
       inf.extendidas = 0;
       i=0;
       for(i=0;i<4;i++){
           inf.part[i].size_particion =0;
            inf.part[i].sizeaux_particion =0;
           inf.part[i].start_particion =0;
           inf.part[i].fit_particion = '0';
           inf.part[i].status_particion = '0';
           inf.part[i].type_particion = '0';

    int ii=0;
    for(ii=0;ii<8;ii++){
    inf.part[i].exten[ii].status='0';
    inf.part[i].exten[ii].fit='0';
    inf.part[i].exten[ii].next=0;
    inf.part[i].exten[ii].size=0;
    inf.part[i].exten[ii].sizeaux=0;
    inf.part[i].exten[ii].start=0;
    }
       }
       int cantidad=atoi(size);
       if(cantidad<1){
        printf("Tamano Invalido\n");
           return 0;
       }

       FILE *discos;
       discos = fopen(path,"wb+");

       if(aux==1){
           inf.size = cantidad*1048576;
            inf.size = cantidad*1048576;
           cantidad= cantidad*1024;
       }else if(aux==2){
           inf.size = cantidad*1024;
       }

       i=0;
       for(i=0;i<cantidad;i++){
           fwrite(&buffer,1024,1,discos);
       }

      fseek(discos,0,SEEK_SET);
       fwrite(&inf,sizeof(disco),1,discos);
       fclose(discos);
           printf("se creo el Disco\n");

     return 1;
    }

int EDisco(char* path){
    FILE* disco;
    disco = fopen(path,"rb+");
    if(disco== NULL){
      printf("no existe el Disco\n");
      return 0;
    }else{
        if(strcmp(path,"/home/luga")==0||strcmp(path,"/home/luga/")==0){
        printf("ingrese una direccion valida\n");
        }else{
            char com[5];
            printf("\n Desea eliminar el disco [s/n]: \n");
           fgets(com,5,stdin);
           if(com[0]=='s' || com[0]=='S'){
        char* aux;
        aux = (char*)malloc(160);
        strcpy(aux,"rm ");
        strcat(aux, path);
        system(aux);
        printf("Disco Eliminado con Exito\n");
          }else{
          printf("No se elimino el disco\n");
           }
    }}
    return 1;
}






