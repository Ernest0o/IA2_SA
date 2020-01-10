#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "CSP_VectorUtilities.h"
#include "CSP_FlightUtilities.h"
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
#define tamano_matriz_crews num_max_vuelos*20 //Máximo de crews que se van a generar
#define euler 2.71828

//---Función que coloca la cantidad de num_unos en un vector booleano de forma aleatoria---//
void coloca_1s_aleatorios (bool vector_a_colocarle [tamano_matriz_crews], int tamano_vector, int num_unos){
    int i = 0;
    if (tamano_vector>num_unos){
        while (i<num_unos){
            int random = random_number(0, tamano_vector-1);
            if (vector_a_colocarle[random] != true){
                vector_a_colocarle[random] = true;
                i++;
            }
        }
    }

}

//---Función que pasa un vector binario a un vector decimal---//
void solucion_bool_a_decimal(bool solucion [tamano_matriz_crews], int sol_por_crew [tamano_listas], int total_crews,
                             int matriz_de_crews [tamano_matriz_crews] [tamano_listas],
                            double vector_costos_crew[tamano_matriz_crews], int vector_auxiliar_crews_random [tamano_matriz_crews]){
    int aux = 0;
    memset(sol_por_crew, 0, tamano_listas);
    for (int r = 0; r<total_crews; r++){
        if (solucion[r] == true){
            //printf("\nCrew %d: ", r+1);
            sol_por_crew[aux] = vector_auxiliar_crews_random[r];
            aux++;
        }
    }
    for(; aux<tamano_listas; aux++){
        sol_por_crew[aux] = 0;
    }
}
//---Función que calcula el costo total de una solución---//
//--Costo de solución = sum(costo_crews) + penalizaciones_por_vuelos_sin_cubrir + penalizaciones_por_deadhead--//
double calcula_costo_solucion(int solucion_actual_por_num_crew[tamano_listas], double vector_costos_crew[tamano_matriz_crews],
                              int matriz_de_crews [tamano_matriz_crews] [tamano_listas], int num_vuelos, int id_vuelo [tamano_listas],
                              double penalizacion_por_vuelo_no_cubierto, double penalizacion_por_deadhead){

    double costo_total_solucion = 0; //Costo de solución = sum(costo_crews) + penalizaciones_por_vuelos_sin_cubrir + penalizaciones_por_deadhead
    double costo_sol_crews = 0; //sum(costo_crews)

    //a) Calcula el costo total de los crews elegidos
    int num_crews_de_solucion = calcula_tamano(solucion_actual_por_num_crew);
    for (int i = 0; i<num_crews_de_solucion; i++){
        costo_sol_crews = costo_sol_crews + vector_costos_crew[solucion_actual_por_num_crew[i]-1];
    }
    int veces_vuelos_cubiertos [tamano_listas] = {0}; //El valor del vector en la posición i indica las veces cubiertas del vuelo en la misma posición
    //EJ: id_vuelo = {5,9,3} veces_vuelos_cubiertos = {1, 2, 0}, el vuelo en la posición [1], o sea vuelo 9, se encuentra cubierto 2 veces.
    //Llenado de vector veces_vuelos_cubiertos//
    for (int i = 0; i<num_crews_de_solucion; i++){
        int pos_crew = solucion_actual_por_num_crew[i]-1;
        for (int j = 0; j<calcula_tamano(matriz_de_crews[pos_crew]); j++){
            int vuelo = matriz_de_crews[pos_crew][j];
            int pos_vuelo = encuentra_posicion(vuelo, num_vuelos, id_vuelo);
            veces_vuelos_cubiertos[pos_vuelo]++;
        }
    }
    //b) Penalizaciones por vuelos no cubiertos
    int sol_vuelos_no_cubiertos = 0;
    for (int i = 0; i<num_vuelos; i++){
        if (veces_vuelos_cubiertos[i]<1){
            sol_vuelos_no_cubiertos++;
        }
    }
    double costo_pen_sincubir = penalizacion_por_vuelo_no_cubierto*sol_vuelos_no_cubiertos;

    //c) Penalizaciones por deadheading
    int sol_num_dedheads = 0;
    for (int i = 0; i<num_vuelos; i++){
        if (veces_vuelos_cubiertos[i]>1){
            sol_num_dedheads = sol_num_dedheads + veces_vuelos_cubiertos[i]-1;
        }
    }
    double costo_pen_deadheads = penalizacion_por_deadhead*sol_num_dedheads;
    costo_total_solucion = costo_sol_crews + costo_pen_deadheads + costo_pen_sincubir;

    return costo_total_solucion;
}

//---Función que realiza el backup de la solución encontrada a la mejor solución
void backup_best_solucion (int solucion_actual_por_num_crew [tamano_listas], int mejor_solucion_por_num_crew [tamano_listas],
                           double *costo_best_sol, double costo_sol_pres){

    *costo_best_sol = costo_sol_pres;
    memset(mejor_solucion_por_num_crew, 0, tamano_listas);
    copia_dos_vectores(solucion_actual_por_num_crew, mejor_solucion_por_num_crew);
}

//--Función que copia los contenidos del vector origen bool, al vector destino--//
void copia_dos_vectores_bool(bool vector_origen[tamano_matriz_crews], bool vector_destino [tamano_matriz_crews]){

    memset(vector_destino, 0, tamano_matriz_crews);
    for (int i = 0; i<tamano_matriz_crews; i++){
        vector_destino[i] = vector_origen[i];
    }
}

//--Función que realiza el movimiento bitflip en un elemento del vector--//
void bit_flip(bool vector [tamano_matriz_crews], int pos_flip, int total_crews){
    if (vector[pos_flip] == true){
        vector[pos_flip] = false;
    } else {
        vector[pos_flip] = true;
    }
}

//--Función que elige mediante la probabilidad, si una solución peor a la anterior es elegida o no--//
bool eleccion_por_simulated_annealing (double costo_solucion_actual, double costo_solucion_futura, double temperatura){
    bool la_eliges;
    double delta_evaluacion = costo_solucion_futura - costo_solucion_actual;
    double potencia = -delta_evaluacion/temperatura;
    double probabilidad = pow(euler,potencia);
    double aleatorio = (random_number(0, 100))/100.0000;
    if (probabilidad>= aleatorio){
            la_eliges = true;
    } else {
            la_eliges = false;
    }
    return la_eliges;
}

//--Función que muestra en pantalla la info de la mejor solución, y crea el archivo .csv de salida--//
void recuento_y_creacion_csv(int solucion_actual_por_num_crew[tamano_listas], double vector_costos_crew[tamano_matriz_crews],
                              int matriz_de_crews [tamano_matriz_crews] [tamano_listas], int num_vuelos, int id_vuelo [tamano_listas],
                              double penalizacion_por_vuelo_no_cubierto, double penalizacion_por_deadhead, char nombre_archivo_csv [120],
                              double costo, int seed, char unidades_moneda [10], double time_taken){

    int veces_vuelos_cubiertos [tamano_listas] = {0}; //El valor del vector en la posición i indica las veces cubiertas del vuelo en la misma posición
    int vuelos_no_cubiertos [tamano_listas] = {0}; //Contiene las ID de los vuelos que no fueron cubiertos
    int vuelos_deadhead [tamano_listas] = {0}; //Contiene las ID de los vuelos que presentan deadhead
    //EJ: id_vuelo = {5,9,3} veces_vuelos_cubiertos = {1, 2, 0}, el vuelo en la posición [1], o sea vuelo 9, se encuentra cubierto 2 veces.
    //Llenado de vector veces_vuelos_cubiertos//
    int num_crews_de_solucion = calcula_tamano(solucion_actual_por_num_crew);

    for (int i = 0; i<num_crews_de_solucion; i++){
        int pos_crew = solucion_actual_por_num_crew[i]-1;
        for (int j = 0; j<calcula_tamano(matriz_de_crews[pos_crew]); j++){
            int vuelo = matriz_de_crews[pos_crew][j];
            int pos_vuelo = encuentra_posicion(vuelo, num_vuelos, id_vuelo);
            veces_vuelos_cubiertos[pos_vuelo]++;
        }
    }
    //b) vuelos no cubiertos
    int pos = 0;
    for (int i = 0; i<num_vuelos; i++){
        if (veces_vuelos_cubiertos[i]<1){
            vuelos_no_cubiertos [pos] = id_vuelo[i];
            pos++;
        }
    }
    int num_vuelos_sin_cubrir = calcula_tamano(vuelos_no_cubiertos);
    printf("\n\nVuelos sin cubrir: %d. ID: ", num_vuelos_sin_cubrir);
    for (int i = 0; i<num_vuelos_sin_cubrir; i++){
        printf("%d ", vuelos_no_cubiertos[i]);
    }
    double pen_por_no_cubiertos = num_vuelos_sin_cubrir*penalizacion_por_vuelo_no_cubierto;
    printf("    $Penalizacion por vuelos sin cubrir: %.2f %s", pen_por_no_cubiertos, unidades_moneda);
    //c) Deadheadings
    int sol_num_dedheads = 0;
    int pos_2 = 0;
    for (int i = 0; i<num_vuelos; i++){
        if (veces_vuelos_cubiertos[i]>1){
            sol_num_dedheads = sol_num_dedheads + veces_vuelos_cubiertos[i]-1;
            vuelos_deadhead [pos_2] = id_vuelo[i];
            pos_2++;
        }
    }
    int num_vuelos_deadhead = calcula_tamano(vuelos_deadhead);
    printf("\nDeadheadings: %d. ID de vuelos con deadhead: ", sol_num_dedheads);
    for (int i = 0; i<num_vuelos_deadhead; i++){
        printf("%d ", vuelos_deadhead[i]);
    }
    double pen_por_deadheads = sol_num_dedheads*penalizacion_por_deadhead;
    printf("    $Penalizacion por deadheadings: %.2f %s", pen_por_deadheads, unidades_moneda);
    printf("\n\n                ~~ Costo total de la solucion: %.2f %s ~~", costo, unidades_moneda);
    //--Creación de archivo .csv de salida--//

    FILE *fp;
    char costo_char [250];
    sprintf(costo_char, "C:\\Users\\Ernesto\\Documents\\ITSE\\Tesis\\Programming\\IA2_SA\\Resultados\\%d_", (int)costo);
    strcat(costo_char, nombre_archivo_csv);
    strcpy(nombre_archivo_csv, costo_char);
    fp=fopen(nombre_archivo_csv,"w+");

    fprintf(fp,"Crew,ID_vuelos_cubiertos,");

    for (int r = 0; r<calcula_tamano(solucion_actual_por_num_crew); r++){

        fprintf(fp,"\nT%d,",r+1);

        for (int c = 0; c<calcula_tamano(matriz_de_crews[solucion_actual_por_num_crew[r]-1]); c++){
            int ide = matriz_de_crews[solucion_actual_por_num_crew[r]-1][c];
            fprintf(fp, "%d,", ide);
        }
        fprintf(fp, "$%.2f,", vector_costos_crew[solucion_actual_por_num_crew[r]-1]);
    }
    fprintf(fp, "\nNum_vuelos_sin_cubrir,%d,", num_vuelos_sin_cubrir);
    fprintf(fp, "$%.2f,", pen_por_no_cubiertos);

    fprintf(fp, "\nID_vuelos_sin_cubrir,");
    for(int x = 0; x < num_vuelos_sin_cubrir; x++){
        fprintf(fp, "%d,", vuelos_no_cubiertos[x]);
    }

    fprintf(fp, "\nNum_deadheads,%d,", sol_num_dedheads);
    fprintf(fp, "$%.2f,", pen_por_deadheads);

    fprintf(fp, "\nID_vuelos_con_deadhead,");
    for (int l = 0; l<num_vuelos_deadhead; l++){
        fprintf(fp, "%d,", vuelos_deadhead[l]);
    }

    fprintf(fp, "\nCosto_solucion,$%.2f,", costo);
    fprintf(fp, "\nSemilla,%u,", seed);
    fprintf(fp, "\nTiempo_de_ejecucion,%.6f,", time_taken);
    fclose(fp);

    printf("\n\n%s creado.", nombre_archivo_csv);

}
