#include <stdbool.h>
#ifndef CSP_VECTORUTILITIES_H_INCLUDED
#define CSP_VECTORUTILITIES_H_INCLUDED

#define num_max_vuelos 96 //El horario con más vuelos tiene 96
#define tamano_listas num_max_vuelos + 1 //1 espacio extra por seguridad
#define tamano_matriz_crews num_max_vuelos*20 //Máximo de crews que se van a generar

double horachar_a_decimal(char hora[6]);

void convierte_vector_horaschar_a_decimal(int num_vuelos, char vector_horas[tamano_listas][6],
                                          double vector_destino [tamano_listas]);

void calcula_duraciones(int num_vuelos, double departure_decimal [tamano_listas], double arrival_decimal [tamano_listas],
                         double duraciones[tamano_listas]);

int calcula_tamano(int lista [tamano_listas]);

int calcula_renglones (int matriz [tamano_matriz_crews] [tamano_listas]);

int encuentra_posicion (int value, int tamano, int * lista);

void ordena_por_hora(int * arr, int n, int num_vuelos, int id_vuelo[tamano_listas], double departure_decimal [tamano_listas]);

void agrega_a_solucion(int entero, int posicion, int solucion [tamano_listas]);

void copia_vuelos_restantes(int num_vuelos, int id_vuelo[tamano_listas], int id_vuelos_restantes[tamano_listas], int id_vuelos_restantes_temp[tamano_listas]);

void copia_dos_vectores(int vector_origen[tamano_listas], int vector_destino [tamano_listas]);

void reacomoda_vector_randomly(int vektor [tamano_listas], int tamanio_v);

void filtra_tempc_a_base(int vuelos_disponibles_tempc [tamano_listas], int v_d_tc_abase [tamano_listas],
                          int id_vuelo [tamano_listas], char base [6], int num_vuelos, char destination [tamano_listas] [6]);

bool compara_dos_vectores (int vector_1 [tamano_listas], int vector_2 [tamano_listas]);

bool revisa_si_esta_en_matriz (int la_matrix [tamano_listas] [tamano_listas], int renglones_matriz,
                               int vector_a_revisar [tamano_listas]);

int suma_elementos_de_vector (int vecktor [tamano_listas], int tamano_vector);

#endif // CSP_VECTORUTILITIES_H_INCLUDED
