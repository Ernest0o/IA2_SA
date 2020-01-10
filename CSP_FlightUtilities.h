#ifndef CSP_FLIGHTUTILITIES_H_INCLUDED
#define CSP_FLIGHTUTILITIES_H_INCLUDED
#include <stdbool.h>
#include "struct_definition.h"
#define num_max_vuelos 96 //El horario con más vuelos tiene 96
#define tamano_listas num_max_vuelos + 1 //1 espacio extra por seguridad


void filtra_vuelos_bases(int tamano_vec, char B1 [6], char B2 [6], char origin[tamano_listas][6], int id_vector[tamano_listas],
                            int vuelos_from_bases [tamano_listas], int num_vuelos, int id_vuelo[tamano_listas]);


void horas_departure_reajuste(int num_vuelos, double departure_decimal [tamano_listas], double lo_mas_temprano);

void horas_arrival_reajuste(int num_vuelos, double arrival_decimal [tamano_listas], double lo_mas_temprano);

int random_number(int min_num, int max_num);

typedef struct returns_siguiente_vuelo Struct;
Struct siguiente_vuelo(int ultimo_vuelo, int num_vuelos, double tiempo_espera_min, double tiempo_espera_max,
                       double tiempo_total_max, double tiempo_vuelo_max, int id_vuelo[tamano_listas], int solucion [tamano_listas],
                       char origin [tamano_listas] [6], char destination[tamano_listas] [6],
                       int id_vuelos_restantes[tamano_listas], double departure_decimal[tamano_listas],
                       double arrival_decimal [tamano_listas], double horas_de_trabajo, double horas_de_vuelo,
                       double horas_de_espera, double duraciones[tamano_listas],  int vuelos_disponibles_tempc_backup [tamano_listas],
                       int num_crew_resets);

#endif // CSP_FLIGHTUTILITIES_H_INCLUDED
