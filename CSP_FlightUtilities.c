#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include "CSP_VectorUtilities.h"
#include "struct_definition.h"
    //--Linux Libraries
//#include <curses.h>
//#include <ncurses.h>
    //--Windows Libraries
#include <conio.h>
#include <stdbool.h>
#include <string.h>
#include <windows.h>

#define num_max_vuelos 96 //El horario con más vuelos tiene 96
#define tamano_listas num_max_vuelos + 1 //1 espacio extra por seguridad
#define max_crew_resets 26
//---Función que llena "vuelos_from_bases" con las id de los vuelos que parten de las bases B1 y B2---//
void filtra_vuelos_bases(int tamano_vec, char B1 [6], char B2 [6], char origin[tamano_listas][6], int id_vector[tamano_listas],
                            int vuelos_from_bases [tamano_listas], int num_vuelos, int id_vuelo[tamano_listas]) {

    int r = 0;
    memset(vuelos_from_bases, 0, tamano_listas); //Vacía el vector vuelos_from_bases con 0s
    for (int i = 0; i<tamano_vec; i++){
        char partida[6];
        int pos_vuelo = encuentra_posicion(id_vector[i], num_vuelos, id_vuelo);
        strcpy(partida, origin[pos_vuelo]);
        if (strcmp(partida, B1) == 0 || strcmp(partida, B2) == 0){
            vuelos_from_bases[r] = id_vector[i];
            r = r+1;
        }
    }
}
//---Funciones que reajustan el valor decimal de las hora de partida y llegada. Si salen al día siguiente, se les
//---aumenta 24 h a su valor, es decir 01.50 se convierte en 25.5 (1)---//
void horas_departure_reajuste(int num_vuelos, double departure_decimal [tamano_listas], double lo_mas_temprano){
    for (int i = 0; i<num_vuelos; i++){
        if (departure_decimal[i] < lo_mas_temprano){
            departure_decimal[i] = departure_decimal[i] + 24;
        }
    }
}
//---Funciones que reajustan el valor decimal de las horas de partida y llegada. Si salen al día siguiente, se les
//---aumenta 24 h a su valor, es decir 01.50 del día sig. se convierte en 25.5 (2)---//
void horas_arrival_reajuste(int num_vuelos, double arrival_decimal [tamano_listas], double lo_mas_temprano){
    for (int i = 0; i<num_vuelos; i++){
        if (arrival_decimal[i] < lo_mas_temprano){
            arrival_decimal[i] = arrival_decimal[i] + 24;
        }
    }
}


//---Función que da un número aleatorio entre un rango máximo y mínimo, con dichos valores incluidos---//
int random_number(int min_num, int max_num){

    int result = 0, low_num = 0, hi_num = 0;

    if (min_num < max_num){
        low_num = min_num;
        hi_num = max_num + 1; // include max_num in output
    } else {
        low_num = max_num + 1; // include max_num in output
        hi_num = min_num;
    }
    result = (rand() % (hi_num - low_num)) + low_num;
    return result;

}

//---Función que realiza la selección del siguiente vuelo---//
typedef struct returns_siguiente_vuelo Struct;
Struct siguiente_vuelo(int ultimo_vuelo, int num_vuelos, double tiempo_espera_min, double tiempo_espera_max,
                       double tiempo_total_max, double tiempo_vuelo_max, int id_vuelo[tamano_listas], int solucion [tamano_listas],
                       char origin [tamano_listas] [6], char destination[tamano_listas] [6],
                       int id_vuelos_restantes[tamano_listas], double departure_decimal[tamano_listas],
                       double arrival_decimal [tamano_listas], double horas_de_trabajo, double horas_de_vuelo,
                       double horas_de_espera, double duraciones[tamano_listas],  int vuelos_disponibles_tempc_backup [tamano_listas],
                       int num_crew_resets){

    struct returns_siguiente_vuelo ret; //ret es la estructura que tendrá los valores de salida de la función
    ret.fin_tripulacion = false; //Si es true el crew ha terminado su trabajo.
    ret.atascado = false;   //Hay un atasco
    ret.hay_retorno = false; //Hay retorno a base
    ret.imposible_generar_crew = false;
    int posicion = encuentra_posicion(ultimo_vuelo, num_vuelos, id_vuelo);
    int posicion_vuelo1 = encuentra_posicion(solucion[0], num_vuelos, id_vuelo);
    char base_tripulacion [6]; //La base de la que sale la tripulación actual.
    strcpy(base_tripulacion, origin[posicion_vuelo1]);
    int vuelos_disponibles_temp [tamano_listas] = {0}; //Vuelos filtrados por origin
    int vuelos_disponibles_tempb [tamano_listas] = {0}; //Vuelos filtrados por tiempo
    int vuelos_disponibles_tempc [tamano_listas] = {0}; //Vuelos filtrados por tiempo no excedido
    int vuelos_tempc_a_base [tamano_listas];
    double arrival_ultimo_vuelo = arrival_decimal[posicion]; //Hora de llegada del último vuelo
    char lugar_ultimo_vuelo [6]; //Lugar de llegada del último vuelo.
    strcpy(lugar_ultimo_vuelo, destination[posicion]);

    //--Forward cheking filtra los dominios (vuelos) con base en las restricciones. Comenzando por la restriccion
    //que dice: El siguiente vuelo siempre debe partir de donde llega el anterior.
    int r = 0;
    for (int i = 0; i<num_vuelos; i++){
        int vuelo = id_vuelo[i];
        int posi = encuentra_posicion(vuelo, num_vuelos, id_vuelo);
        char partida [6]; //El lugar de partida del siguiente vuelo
        strcpy(partida, origin[posi]); //partida = origin[posi]
        if (strcmp(partida, lugar_ultimo_vuelo) == 0){
            vuelos_disponibles_temp[r] = vuelo;
            //printf("\nVuelo %d sale de %s", vuelos_disponibles_temp[r], partida);
            r = r+1;
        }
    }
    int size_vuelos_disponibles_temp = calcula_tamano(vuelos_disponibles_temp);

    //Después filtrará con la restriccion que dice: La hora de salida del siguiente vuelo, tiene que ser mayor
    //o igual a la hora de llegada del vuelo anterior más el tiempo mínimo de espera; y menor que la hora de
    //llegada del vuelo anterior más el tiempo máximo de espera.
    r = 0;
    for (int i = 0; i<size_vuelos_disponibles_temp; i++){
        int position = encuentra_posicion(vuelos_disponibles_temp[i], num_vuelos, id_vuelo);
        double hr_salida = departure_decimal [position]; //La hora de salida del siguiente vuelo
        if (hr_salida >= arrival_ultimo_vuelo + tiempo_espera_min &&
            hr_salida <= arrival_ultimo_vuelo + tiempo_espera_max){
            vuelos_disponibles_tempb[r] = vuelos_disponibles_temp[i];
            r = r+1;
        }
    }
    int size_vuelos_disponibles_tempb = calcula_tamano(vuelos_disponibles_tempb);
    ordena_por_hora(vuelos_disponibles_tempb, size_vuelos_disponibles_tempb, num_vuelos, id_vuelo, departure_decimal); //Los reordena por hora de salida.
    //Después, el algoritmo filtrará una vez más los vuelos, esta vez, los filtrará de tal manera que sólo
    //estarán disponibles los que, al elegirlos, la tripulación no se exceda de su tiempo límite de trabajo.
    r = 0;
    for (int i= 0; i<size_vuelos_disponibles_tempb; i++){
        int position = encuentra_posicion(vuelos_disponibles_tempb[i], num_vuelos, id_vuelo);
        double tiempo_espera = departure_decimal[position] - arrival_ultimo_vuelo;
        double tiempo_total_con_vuelo = horas_de_trabajo + tiempo_espera + duraciones[position];
        if (tiempo_total_con_vuelo <= tiempo_total_max){
            vuelos_disponibles_tempc[r] = vuelos_disponibles_tempb[i];
            r=r+1;
        }
    }

    int size_vuelos_disponibles_tempc = calcula_tamano(vuelos_disponibles_tempc);

    if (size_vuelos_disponibles_tempc <= 0){ //Si es 0, que revise si la tripulación está en su base.
        if (strcmp(base_tripulacion, lugar_ultimo_vuelo) == 0){ //Si está en su base, que ahí termine su jornada.
            ret.next_flight = 0;
            ret.hay_retorno = true;
            ret.fin_tripulacion = true;
        } else { //Si no está en su base, hay un atasco
            if (calcula_tamano(solucion)>1){
                ret.atascado = true;
                ret.fin_tripulacion = false;
                ret.next_flight = 0;
                ret.hay_retorno = false;
            } else { //Si sólo han cubierto un vuelo, es imposible generar crew
                ret.atascado = true;
                ret.fin_tripulacion = false;
                ret.next_flight = 0;
                ret.imposible_generar_crew = true;
            }
        }


    } else if (size_vuelos_disponibles_tempc > 0){ //Si sí hay vuelos disponibles, el FC continúa.

    //El algoritmo manejará las restricciones de tiempo máximo de vuelo y tiempo máximo de trabajo

        double horas_restantes = tiempo_total_max-horas_de_trabajo;
        double horas_vuelo_restantes = tiempo_vuelo_max - horas_de_vuelo;

        if (horas_restantes>=3 && horas_vuelo_restantes>=2 && num_crew_resets <= max_crew_resets){ //Si le quedan más de 3 horas restantes...
            int random;
            if (size_vuelos_disponibles_tempc>1){
                random = random_number(0, size_vuelos_disponibles_tempc-1);
            } else {
                random = 0;
            }
            ret.next_flight = vuelos_disponibles_tempc[random]; //El vuelo siguiente será cualquiera de la lista.
            ret.atascado = false;
            ret.fin_tripulacion = false;
        } else { //Si le quedan menos de 3 horas disponibles
            filtra_tempc_a_base(vuelos_disponibles_tempc, vuelos_tempc_a_base, id_vuelo, base_tripulacion, num_vuelos, destination); //
            int num_tempc_base = calcula_tamano(vuelos_tempc_a_base);
            if (num_tempc_base <=0){ //Si no encontró ningún vuelo que lo regrese a su base, entonces:
                if (strcmp(base_tripulacion, lugar_ultimo_vuelo) == 0){ //Si está en su base, que ahí termine su jornada.
                    ret.next_flight = 0;
                    ret.atascado = false;
                    ret.fin_tripulacion = true;
                    ret.hay_retorno = false;
                } else {
                    ret.atascado = true;
                    ret.fin_tripulacion = false;
                    ret.next_flight = 0;
                }
            } else { //Si hay por lo menos un vuelo que lo lleve a base
                int random2;
                if (num_tempc_base>1){
                    random2 = random_number(0, num_tempc_base-1);
                } else {
                    random2 = 0;
                }
                ret.next_flight = vuelos_tempc_a_base[random2];
                ret.atascado = false;
                ret.fin_tripulacion = true;
                ret.hay_retorno = false;
            }
        }
    }
    //--Etapa del backup: Le avisa al main que hay forma de regresar a base, para que realice un backup//
    if (ret.next_flight>0 && ret.atascado == false && ret.fin_tripulacion == false && size_vuelos_disponibles_tempc>1){ //Si eligió un vuelo válido..
        ret.hay_retorno = false; //Se pondrá en 1 cuando la búsqueda encuentre un vuelo que lo lleve a su base
        filtra_tempc_a_base(vuelos_disponibles_tempc, vuelos_tempc_a_base, id_vuelo, base_tripulacion,
                            num_vuelos, destination);
        if (calcula_tamano(vuelos_tempc_a_base)>0){
            ret.hay_retorno = true;
        }

        //--Backup de la lista de vuelos disponibles c, ordenados random
        if (ret.hay_retorno == true && size_vuelos_disponibles_tempc>1){

            for (int i = 0; i<size_vuelos_disponibles_tempc; i++){
                vuelos_disponibles_tempc_backup[i] = vuelos_disponibles_tempc[i];
            }
            int size_vuelos_disponibles_tempc_backup = calcula_tamano(vuelos_disponibles_tempc_backup);
            reacomoda_vector_randomly(vuelos_disponibles_tempc_backup, size_vuelos_disponibles_tempc_backup);
            ret.next_flight = vuelos_disponibles_tempc_backup[0];
            //Eliminando el vuelo elegido del backup para que no lo vuelva a elegir.
            int re;
            for (re = 0; re < size_vuelos_disponibles_tempc_backup-1; re++){
                vuelos_disponibles_tempc_backup[re] = vuelos_disponibles_tempc_backup[re+1];
            }
            for (; re < tamano_listas; re++){
                vuelos_disponibles_tempc_backup[re] = 0;
            }
            size_vuelos_disponibles_tempc_backup = calcula_tamano(vuelos_disponibles_tempc_backup);
        }

    }

    return ret;
}
