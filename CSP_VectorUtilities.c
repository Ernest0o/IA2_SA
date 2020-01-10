#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "CSP_FlightUtilities.h"
    //--Linux Libraries
//#include <curses.h>
//#include <ncurses.h>
    //--Windows Libraries
#include <conio.h>
#include <stdbool.h>
#include <string.h>

#define num_max_vuelos 96 //El horario con más vuelos tiene 96
#define tamano_listas num_max_vuelos + 1 //1 espacio extra por seguridad
#define tamano_matriz_crews num_max_vuelos*20 //Máximo de crews que se van a generar

//---Función que convierte una hora hh:mm a decimal. Ej: "12:12" = 12.2 ---//
double horachar_a_decimal(char hora[6]){
        double resultado;
        int hora_int = atoi(hora);
        int minutos_int = atoi(hora+3);
        double minutos_float = (double)minutos_int/(double)60;
        resultado = hora_int + minutos_float;
    return resultado;
}

//---Función que convierte todo un vector de horas char a horas en decimal---//
void convierte_vector_horaschar_a_decimal(int num_vuelos, char vector_horas[tamano_listas][6], double vector_destino [tamano_listas]){
    for (int i = 0; i<num_vuelos; i++){
        vector_destino[i] = horachar_a_decimal(vector_horas[i]);
    }
}
//---Función que calcula las duraciones de todos los vuelos---//
void calcula_duraciones(int num_vuelos, double departure_decimal [tamano_listas], double arrival_decimal [tamano_listas], double duraciones[tamano_listas]){
    for (int i = 0; i<num_vuelos; i++){
        double llegada = arrival_decimal[i];
        double salida = departure_decimal[i];
        duraciones[i] = llegada - salida;
    }
}
//---Función que calcula el tamaño (número de elementos) de un vector---//
int calcula_tamano(int lista [tamano_listas]) {
    int size = 0;
    if (lista[0]<=0){
        size = 0;
    } else {
        while (lista[size] > 0){
            size = size+1;
        }
    }
    return size;
}
//---Función que calcula el número de renglones de una matriz---//
int calcula_renglones (int matriz [tamano_matriz_crews] [tamano_listas]){
    int renglones = 0;
    if (matriz[renglones][0]<=0){
        renglones = 0;
    } else {
        while (matriz[renglones][0]>0){
            renglones++;
        }
    }
    return renglones;
}
//---Función que encuentra la posición de un elemento en un vector---//
//  Ej. id_vuelo = [5, 3, 1, 4];
//  encuentra_posicion(1, 4, id_vuelo) dará como resultado 2 (pos. 0, pos. 1, pos 2.)
int encuentra_posicion (int value, int tamano, int * lista) {
    if (tamano > 0){
        int posicion = 0;
        while (posicion<tamano && lista[posicion] != value){
            posicion = posicion + 1;
        }
        return posicion;
    } else {
        return -1;
    }
}

//---Funciones que realizan el acomodo en orden ascendente de un vector---//
void swap(int *xp, int *yp){
    int temp = *xp;
    *xp = *yp;
    *yp = temp;
}
//-- arr es el vector, n es el tamaño del vector --/
void ordena_por_hora(int * arr, int n, int num_vuelos, int id_vuelo[tamano_listas], double departure_decimal [tamano_listas]){
   int i, j;
   bool swapped;
   for (i = 0; i < n-1; i++)
   {
     swapped = false;

     for (j = 0; j < n-i-1; j++){
        int posicion = encuentra_posicion(arr[j], num_vuelos, id_vuelo); //arr[j] - 1;
        int posicion2 = encuentra_posicion(arr[j+1], num_vuelos, id_vuelo); //arr[j+1]-1;
        if (departure_decimal[posicion]>departure_decimal[posicion2]){
           swap(&arr[j], &arr[j+1]);
           swapped = true;
        }
     }

     if (swapped == false)
        break;
   }
}
//---Función que agrega una ID de un vuelo al vector solucion---//
void agrega_a_solucion(int entero, int posicion, int solucion [tamano_listas]){
    solucion[posicion] = entero;

}
//---Función que copia al vector id_vuelos_restantes los vuelos que no han sido cubiertos
void copia_vuelos_restantes(int num_vuelos, int id_vuelo[tamano_listas], int id_vuelos_restantes[tamano_listas], int id_vuelos_restantes_temp[tamano_listas]){
    for (int i = 0; i<tamano_listas; i++){
        id_vuelos_restantes_temp[i] = id_vuelo[i];
        id_vuelos_restantes[i] = id_vuelo[i];
    }
}
//---Función que copia los contenidos del vector origen en el vector destino---//
void copia_dos_vectores(int vector_origen[tamano_listas], int vector_destino [tamano_listas]){

    memset(vector_destino, 0, tamano_listas);
    for (int i = 0; i<tamano_listas; i++){
        vector_destino[i] = vector_origen[i];
    }
}



//---Función que reacomoda los elementos de un vector de forma aleatoria (alternativa 2)---//
void reacomoda_vector_randomly(int vektor [tamano_listas], int tamanio_v){
    if (tamanio_v>1){
        for (int j = 0; j<random_number(1, tamanio_v/1.5); j++){
            int n1 = random_number(0, tamanio_v-1);
            int n2 = random_number(0, tamanio_v-1);
            swap(&vektor[n1], &vektor[n2]);
        }
    }
}

//--Función que llena v_d_tc_abase con los vuelos de vuelos_disponibles_tempc que se dirigen hacia la base--//
void filtra_tempc_a_base(int vuelos_disponibles_tempc [tamano_listas], int v_d_tc_abase [tamano_listas],
                          int id_vuelo [tamano_listas], char base [6], int num_vuelos, char destination [tamano_listas] [6]){
    int r = 0;
    memset(v_d_tc_abase, 0, tamano_listas); //Vacía el vector vuelos_from_bases con 0s
    for (int i = 0; i<calcula_tamano(vuelos_disponibles_tempc); i++){
        char destino[6];
        int pos_vuelo = encuentra_posicion(vuelos_disponibles_tempc[i], num_vuelos, id_vuelo);
        strcpy(destino, destination[pos_vuelo]);
        if (strcmp(destino, base) == 0){
            v_d_tc_abase[r] = vuelos_disponibles_tempc[i];
            r = r+1;
        }
    }
}
//---Función que compara dos vectores para comprobar si son iguales---//
bool compara_dos_vectores (int vector_1 [tamano_listas], int vector_2 [tamano_listas]){
    bool son_iguales;
    int i = 0;
    int size_1 = calcula_tamano(vector_1);
    int size_2 = calcula_tamano(vector_2);

    if (size_1 != size_2){
        son_iguales = false;
    } else {
        do {
            son_iguales = true;
            if (vector_1[i] != vector_2[i]){
                son_iguales = false;
            }
            i++;
        } while (son_iguales == true && i<size_1);
    }
    return son_iguales;
}
//---Función que revisa si un vector se encuentra presente en una matriz---//
bool revisa_si_esta_en_matriz (int la_matrix [tamano_listas] [tamano_listas], int renglones_matriz,
                               int vector_a_revisar [tamano_listas]){
    bool ya_esta;
    int i = 0;
    do{
        bool iguales;
        ya_esta = false;
        iguales = compara_dos_vectores(la_matrix[i], vector_a_revisar);
        if (iguales == true){
            ya_esta = true;
        }
        i++;
    } while (ya_esta == false && i<renglones_matriz);
    return ya_esta;
}
//---Función que suma los elementos de un vector que contiene enteros---//
//---Ej: v = {1, 3, 5, 9} . Suma = 18
int suma_elementos_de_vector (int vector [tamano_listas], int tamano_vector){
    int acumulador = 0;
    for (int i = 0; i<tamano_vector; i++){
        acumulador = acumulador + vector[i];
    }
    return acumulador;
}

