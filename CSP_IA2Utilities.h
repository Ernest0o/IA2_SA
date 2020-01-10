#ifndef CSP_IA2UTILITIES_H_INCLUDED
#define CSP_IA2UTILITIES_H_INCLUDED

#define num_max_vuelos 96 //El horario con más vuelos tiene 96
#define tamano_listas num_max_vuelos + 1 //1 espacio extra por seguridad
#define tamano_matriz_crews num_max_vuelos*20 //Máximo de crews que se van a generar

void coloca_1s_aleatorios (bool vector_a_colocarle [tamano_matriz_crews], int tamano_vector, int num_unos);

void solucion_bool_a_decimal(bool solucion [tamano_matriz_crews], int sol_por_crew [tamano_listas], int total_crews,
                             int matriz_de_crews [tamano_matriz_crews] [tamano_listas],
                            double vector_costos_crew[tamano_matriz_crews], int vector_auxiliar_crews_random [tamano_matriz_crews]);

double calcula_costo_solucion(int solucion_actual_por_num_crew[tamano_listas], double vector_costos_crew[tamano_matriz_crews],
                              int matriz_de_crews [tamano_matriz_crews] [tamano_listas], int num_vuelos, int id_vuelo [tamano_listas],
                              double penalizacion_por_vuelo_no_cubierto, double penalizacion_por_deadhead);

void backup_best_solucion (int solucion_actual_por_num_crew [tamano_listas], int mejor_solucion_por_num_crew [tamano_listas],
                           double *costo_best_sol, double costo_sol_pres);

void copia_dos_vectores_bool(bool vector_origen[tamano_matriz_crews], bool vector_destino [tamano_matriz_crews]);

void bit_flip (bool vekctor [tamano_matriz_crews], int pos_flip, int total_crews);

bool eleccion_por_simulated_annealing (double costo_solucion_actual, double costo_solucion_futura, double temperatura);

void recuento_y_creacion_csv(int solucion_actual_por_num_crew[tamano_listas], double vector_costos_crew[tamano_matriz_crews],
                              int matriz_de_crews [tamano_matriz_crews] [tamano_listas], int num_vuelos, int id_vuelo [tamano_listas],
                              double penalizacion_por_vuelo_no_cubierto, double penalizacion_por_deadhead, char nombre_archivo_csv [120],
                              double costo, int seed, char unidades_moneda [10], double time_taken);

#endif // CSP_IA2UTILITIES_H_INCLUDED
