#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include "CSP_VectorUtilities.h"
#include "CSP_FlightUtilities.h"
#include "struct_definition.h"
#include "CSP_IA2Utilities.h"
    //--Linux Libraries
//#include <curses.h>
//#include <ncurses.h>
    //--Windows Libraries
#include <conio.h>
#include <stdbool.h>
#include <string.h>
#include <windows.h>

    //---Constantes del programa---//
#define costo_hora_vuelo 1000 // sobre unidad de hora
#define costo_hora_espera 0.75*costo_hora_vuelo // sobre unidad de hora

#define tiempo_espera_min 0.5 //0.5 horas = 30 minutos
#define tiempo_espera_max 4 //horas
#define tiempo_total_max 12 //horas
#define tiempo_vuelo_max 8 //horas

#define penalizacion_por_varado 7.0*costo_hora_vuelo //Costo de penalización si la tripulación no regresa a su base
#define penalizacion_por_deadhead 7.0*costo_hora_vuelo //Costo de penalización por cada tripulación extra en un vuelo
#define penalizacion_por_vuelo_no_cubierto 1.7*penalizacion_por_deadhead //Costo de penalización por cada vuelo no cubierto
#define unidades_costo "USD" // Moneda utilizada, simple estética

#define B1 "IST" //Bases de las que partirán tripulaciones
#define B2 "ANK" // "

#define num_max_vuelos 96 //El horario con más vuelos tiene 96
#define tamano_listas num_max_vuelos + 1 //1 espacio extra por seguridad
#define tamano_matriz_crews num_max_vuelos*20 //Máximo de crews que se pueden generar
    //---Nombres de las columnas del archivo .csv, modificar en caso de que sean diferentes---//
#define nombre_col_id "Flight_id" //Nombre de la columna donde están las ID de los vuelos
#define nombre_col_origin "origin" //Nombre de la columna donde está la cd. origen
#define nombre_col_destination "destination" //Nombre de la columna donde está la cd. destino
#define nombre_col_departure "departure" //Nombre de la columna donde está la hora de salida
#define nombre_col_arrival "arrival" //Nombre de la columna donde está la hora de llegada

    //---Nombre del archivo a leer por default si no se escribe ninguno manualmente---//
#define archivo_por_default "38_vuelos.csv"
    //---Parámetros del algoritmo---//
//Las veces máximas que se reconstruyen crews si se repiten, si se llega al máximo,
//se detiene la creación de crews para ese vuelo inicial:
#define max_reconstrucciones 2 //NO modificable
//Máximo de reconstrucción de crews si se llega a un reinicio por atascos:
#define max_num_crew_resets 50 //NO modificable
//Máximo número de atascos antes de que ocurra un crew_reset:
#define max_num_atascos 15 //NO modificable
//Valor inicial de temperatura:
#define temperatura_inicial 100*costo_hora_vuelo //NO modificable

    //Parámetros modificables//
//Máximo número de crews generados por vuelo inicial:
#define max_num_crews_por_vuelo 4
//Coeficiente para reducción de temperatura:
#define coeficiente_temperatura 0.9
//Máximo de iteraciones por default si no se ingresa manualmente:
#define max_iteraciones_default 10000

    //---Vectores donde se almacena la información de los vuelos---//
char id_vuelo_char [tamano_listas] [8];
char origin [tamano_listas] [6];
char destination [tamano_listas] [6];
char departure [tamano_listas] [6];
char arrival [tamano_listas] [6];

int id_vuelo[tamano_listas]; //Como las ID son números enteros, podemos convertir de char a int, por facilidad.
int id_vuelo_ordenada[tamano_listas]; //ID de los vuelos ordenados por hora de salida
double departure_decimal [tamano_listas]; //Horas de partida convertidas a decimales. Ej: 05:00 = 5.00
double arrival_decimal [tamano_listas]; //Horas de llegada convertidas a decimales. Ej: 12:45 = 12.75
double duraciones [tamano_listas]; //Incluye las duraciones de cada uno de los vuelos
int vuelos_from_bases [tamano_listas]; //Incluye las ID de los vuelos que parten de las bases B1 y B2

double horas_de_vuelo, horas_de_vuelo_backup; //De la tripulación en cuestión
double horas_de_trabajo, horas_de_trabajo_backup; //"
double horas_de_espera, horas_de_espera_backup; //"

int solucion [tamano_listas]; //Por tripulación
int solucion_backup [tamano_listas]; //"
int vuelos_disponibles_tempc_backup [tamano_listas] = {0}; //Backup para el GBJ
int id_vuelos_restantes[tamano_listas];
int id_vuelos_restantes_backup [tamano_listas];
int id_vuelos_restantes_temp[tamano_listas]; //Lista Auxiliar
int id_vuelos_restantes_temp_backup [tamano_listas];
int id_vuelos_restantes_backup_antes_crew [tamano_listas]; //Backup que se guarda antes de comenzar cada tripulación
int id_vuelos_restantes_temp_antes_crew [tamano_listas]; //"
int solucion_matriz [tamano_listas] [tamano_listas]; //Matriz que contiene en cada renglón los vuelos cubiertos por cada tripulación
int crew_matriz [tamano_listas] [tamano_listas]; //Matriz que contiene distintos crews generados que parten de un mismo vuelo
double vector_costos_crew [tamano_matriz_crews]; //Vector que contiene los costos de cada crew generado
int matriz_de_crews [tamano_matriz_crews] [tamano_listas]; //Matriz que contiene el total de crews generados
int crews_por_vuelo [tamano_listas]; //Total de crews generado por vuelo de inicio.
bool solucion_actual [tamano_matriz_crews] = {false}; //Vector binario de solución
bool solucion_inicial [tamano_matriz_crews] = {false}; //Solución inicial binaria
bool solucion_futura [tamano_matriz_crews] = {false};
int solucion_inicial_por_num_crew [tamano_listas] = {0};
int solucion_actual_por_num_crew [tamano_listas] = {0};
int solucion_futura_por_num_crew [tamano_listas] = {0};
int mejor_solucion_por_num_crew [tamano_listas] = {0}; //Backup de la mejor solución encontrada hasta el momento
int pos_solucion, pos_solucion_backup;
int next_vuelo_backup;
int num_vuelos_restantes;
bool flag_reset_crew;
char file_name[120]; //Nombre del archivo a abrir
char nombre_archivo_csv [120]; //Nombre del archivo csv a crear
int num_atascos;
int num_vuelos_bases;
int posi_vuelo_mas_temprano;
int vuelo_inicial;
int num_vuelos; //Número de vuelos del archivo.
int num_crew_resets; //conteo de reseteos de tripulación
int num_crews; //Número de tripulaciones generadas
int crews_sin_retorno; //Número de tripulaciones que no volvieron a base
int max_iteraciones; //Número de iteraciones máximas en la ejecución
unsigned int seed; //Semilla para los números aleatorios
char seed_char [50]; //semilla ingresada manualmente, originalmente en char
char iteraciones_char [50]; //iteraciones ingresadas manualmente.
struct returns_siguiente_vuelo sig_vuelo;

float costo_crew;
float costo_solucion;

double costo_solucion_inicial, costo_solucion_actual, costo_solucion_futura;
double costo_mejor_solucion;
double temperatura;
struct timeval start, end;

    //---Parsing del archivo .csv a los vectores (p1)---//
const char* getfield(char* line, int num) { //Función para el parsing del .csv
    const char* tok;
    for (tok = strtok(line, ","); //"," Carácter que separa a los renglones (modificar también abajo)
            tok && *tok;
            tok = strtok(NULL, ",\n")) //Modificar también aquí, dejar el \n
    {
        if (!--num)
            return tok;
    }
    return NULL;
}

    //---Parsing del archivo .csv a los vectores (p2)---//
int get_file_vuelos(char archivo[120], int columna){
    FILE* horario = fopen(archivo, "r");
    int reng = 0; //Num. renglon 0...n
    char line[1024]; //Renglon completo
    char nombre_columna [20];
    while (fgets(line, 1024, horario)){

        char * tmp = strdup(line);
        char valor_leido [30];
        strcpy(valor_leido, getfield(tmp,columna));

        if (reng < 1){
            strcpy(nombre_columna, valor_leido); // nombre_columna = valor_leido;
        } else {
        if (strcmp(nombre_columna, nombre_col_id) == 0){
            strcpy(id_vuelo_char[reng-1], valor_leido);
            id_vuelo[reng-1] = atoi(id_vuelo_char[reng-1]);
        } else if (strcmp(nombre_columna, nombre_col_origin)== 0){
            strcpy(origin[reng-1], valor_leido);
        } else if (strcmp(nombre_columna, nombre_col_destination) == 0){
            strcpy(destination[reng-1], valor_leido);
        } else if (strcmp(nombre_columna, nombre_col_departure) == 0){
            strcpy(departure[reng-1],valor_leido);
        } else if (strcmp(nombre_columna, nombre_col_arrival) == 0){
            strcpy(arrival[reng-1],valor_leido);
        }
        free(tmp); //Libera el espacio en memoria
        }
        reng = reng + 1;

    }
    return reng = reng-1; //El último incremento se descarta.
}

    //---Elige el primer vuelo de la tripulación---//
void primer_vuelo(int start_flight){
    vuelo_inicial = start_flight;
    memset(solucion, 0, tamano_listas); //Limpia vector de cualquier solución anterior, con 0s
    memset(solucion_backup, 0, tamano_listas); //"
    memset(vuelos_disponibles_tempc_backup, 0, tamano_listas);
    horas_de_vuelo = 0; //Limpia las horas de trabajo de la tripulación en cuestión
    horas_de_trabajo = 0; //"
    horas_de_espera = 0; //"

    pos_solucion = 0; //Posición en el vector de solución
    horas_de_vuelo = duraciones[encuentra_posicion(vuelo_inicial, num_vuelos, id_vuelo)]; //Añadimos primera duración a las horas totales de vuelo
    horas_de_trabajo = horas_de_vuelo + horas_de_espera; //Añadimos las horas de vuelo a las horas totales de trabajo
    agrega_a_solucion(vuelo_inicial, pos_solucion, solucion); //Se agrega el vuelo inicial a la posición 0 del vector solución

}
    //---Reinicia la construcción del pairing---//
void hard_reset(){
    copia_vuelos_restantes(num_vuelos, id_vuelo, id_vuelos_restantes, id_vuelos_restantes_temp);
    copia_dos_vectores(id_vuelos_restantes, id_vuelos_restantes_backup_antes_crew); //Realiza un backup antes de empezar a acomodar.
    copia_dos_vectores(id_vuelos_restantes_temp, id_vuelos_restantes_temp_antes_crew); //"
    num_crews = 0;
    costo_solucion = 0;
    crews_sin_retorno = 0;
    memset(solucion_matriz, 0, sizeof solucion_matriz);
}
    //---Genera un pairing completo a partir del vuelo start_flight---//
bool genera_crew(int start_flight){
        flag_reset_crew = false;
        num_atascos = 0; //"
        sig_vuelo.fin_tripulacion = false;
        sig_vuelo.imposible_generar_crew = false;
        copia_dos_vectores(id_vuelos_restantes, id_vuelos_restantes_backup_antes_crew); //Realiza un backup antes de empezar a acomodar.
        copia_dos_vectores(id_vuelos_restantes_temp, id_vuelos_restantes_temp_antes_crew); //"
        primer_vuelo(start_flight); //Elige el primer vuelo de la tripulación
        while (sig_vuelo.fin_tripulacion == false && sig_vuelo.imposible_generar_crew == false) { //Mientras la tripulación no esté finalizada
            if (num_atascos > max_num_atascos){ //Si se ha atascado más de 30 veces, que ponga flag_reset_crew en 1
                flag_reset_crew = true;
                num_atascos = 0;
            }
            if (flag_reset_crew ==  true){ //Si flag_reset_crew = 1, que reinicie la tripulación
                    num_crew_resets = num_crew_resets + 1;
                if (num_crew_resets <= max_num_crew_resets){
                    primer_vuelo(start_flight);
                    flag_reset_crew = false;
                } else {
                    num_crew_resets = 0;
                    hard_reset();
                    sig_vuelo.imposible_generar_crew = true;
                }

            }
            int vuelo_anterior;
            if (sig_vuelo.imposible_generar_crew == false){
            vuelo_anterior = solucion[pos_solucion];
            //--Se elige el siguiente vuelo de la tripulación--//
            sig_vuelo = siguiente_vuelo(vuelo_anterior, num_vuelos, tiempo_espera_min, tiempo_espera_max, tiempo_total_max,
                                        tiempo_vuelo_max, id_vuelo, solucion, origin, destination, id_vuelos_restantes, departure_decimal,
                                        arrival_decimal, horas_de_trabajo, horas_de_vuelo, horas_de_espera, duraciones, vuelos_disponibles_tempc_backup,
                                        num_crew_resets);
            }
            if (sig_vuelo.hay_retorno == true && sig_vuelo.fin_tripulacion == false && sig_vuelo.imposible_generar_crew == false){ //Si hay retorno, se realiza el backup
                    //---Backup----//
                        memset(id_vuelos_restantes_backup, 0, tamano_listas); //Limpia los backup antes de copiar
                        memset(solucion_backup, 0, tamano_listas); //idem
                        horas_de_espera_backup = horas_de_espera;
                        horas_de_trabajo_backup = horas_de_trabajo;
                        horas_de_vuelo_backup = horas_de_vuelo;
                        pos_solucion_backup = pos_solucion;

                        for (int i = 0; i<tamano_listas; i++){ //copia las listas.
                            solucion_backup[i] = solucion[i];
                            id_vuelos_restantes_backup[i] = id_vuelos_restantes[i];
                            id_vuelos_restantes_temp_backup[i] = id_vuelos_restantes_temp[i];
                        }
                       for (int i = 0; i<tamano_listas; i++){ //copia las listas.
                            id_vuelos_restantes_backup[i] = id_vuelos_restantes[i];
                        }
                        for (int i = 0; i<tamano_listas; i++){ //copia las listas.
                            id_vuelos_restantes_temp_backup[i] = id_vuelos_restantes_temp[i];
                        }

            }
            if (sig_vuelo.atascado == true && sig_vuelo.imposible_generar_crew == false ){ //Que incremente el número de atascos
                num_atascos = num_atascos+1;
                int i;
                int solucion_backup_size= calcula_tamano(solucion_backup);
                for (i = 0; i<tamano_listas; i++){
                    solucion[i] = solucion_backup[i];
                }
                for (i = 0; i<tamano_listas; i++){
                    id_vuelos_restantes[i] = id_vuelos_restantes_backup[i];
                }
                for (i = 0; i<tamano_listas; i++){
                    id_vuelos_restantes_temp[i] = id_vuelos_restantes_temp_backup[i];
                }

                num_vuelos_restantes = calcula_tamano(id_vuelos_restantes);
                vuelo_anterior = solucion[solucion_backup_size-1];
                horas_de_espera = horas_de_espera_backup;
                horas_de_trabajo = horas_de_trabajo_backup;
                horas_de_vuelo = horas_de_vuelo_backup;
                pos_solucion = pos_solucion_backup; //o pos_solucion_backup -1 ?
                //Elegir el primer vuelo de los que están en la lista vuelos_c_backup y luego eliminarlo de esta
                //setear banderas dependiendo de la solución y que calcule si ya se pasó de tiempo,etc.
                //posteriormente continuar con el algoritmo como lo haría normalmente.
                int size_vuelos_disponibles_tempc_backup = calcula_tamano(vuelos_disponibles_tempc_backup);
                if (size_vuelos_disponibles_tempc_backup>0){
                    sig_vuelo.next_flight = vuelos_disponibles_tempc_backup[0];
                    sig_vuelo.atascado = false;
                    sig_vuelo.fin_tripulacion = false;
                    sig_vuelo.hay_retorno = false;
                    flag_reset_crew = false;
                    int re;
                    for (re = 0; re < size_vuelos_disponibles_tempc_backup-1; re++){
                        vuelos_disponibles_tempc_backup[re] = vuelos_disponibles_tempc_backup[re+1];
                    }
                    for (; re < tamano_listas; re++){
                        vuelos_disponibles_tempc_backup[re] = 0;
                    }
                } else {
                    sig_vuelo.next_flight = 0;
                    sig_vuelo.atascado = true;
                    sig_vuelo.fin_tripulacion = false;
                    sig_vuelo.hay_retorno = false;
                    flag_reset_crew = true;
                }
            }

            if (sig_vuelo.atascado == false && sig_vuelo.next_flight > 0 && sig_vuelo.imposible_generar_crew == false){ //Si eligió un vuelo válido, entonces:
                vuelo_anterior = solucion[pos_solucion];
                pos_solucion = pos_solucion + 1;
                agrega_a_solucion(sig_vuelo.next_flight, pos_solucion, solucion);

                int pos_next_vuelo = encuentra_posicion(sig_vuelo.next_flight, num_vuelos, id_vuelo);
                int pos_vuelo_anterior = encuentra_posicion(vuelo_anterior, num_vuelos, id_vuelo);
                double tiempo_espera = departure_decimal[pos_next_vuelo] - arrival_decimal [pos_vuelo_anterior];
                horas_de_espera = horas_de_espera + tiempo_espera;
                double duracion_next_vuelo = duraciones[pos_next_vuelo];
                horas_de_trabajo = horas_de_trabajo + tiempo_espera + duracion_next_vuelo;
                horas_de_vuelo = horas_de_vuelo + duracion_next_vuelo;

                if (horas_de_trabajo >= tiempo_total_max || horas_de_vuelo >= tiempo_vuelo_max){

                    if (strcmp(destination[encuentra_posicion(sig_vuelo.next_flight, num_vuelos, id_vuelo)], origin[encuentra_posicion(solucion[0], num_vuelos, id_vuelo)]) == 0){
                        sig_vuelo.fin_tripulacion = true;
                    } else {
                        num_atascos++;
                        sig_vuelo.fin_tripulacion = false;
                        flag_reset_crew = true;
                    }
                }

            }
        }
        int num_vuelos_cubiertos = calcula_tamano(solucion);
        if (num_vuelos_cubiertos>0 && sig_vuelo.imposible_generar_crew == false){
            num_crew_resets = 0;
            costo_crew = (horas_de_espera*costo_hora_espera) + (horas_de_vuelo*costo_hora_vuelo);
        }
    hard_reset();
    return sig_vuelo.imposible_generar_crew;
}
    //---Genera un set de tripulaciones que parten de un vuelo inicial---//
void genera_set_de_crews (int vuelo_1, int pos_vuelo_en_vuelos_from_bases){
    gettimeofday(&start, NULL); //Tiempo al inicio de la ejecución
    int num_reconstrucciones = 0;
    int r = 0;
    int crews_generados = 0;
    bool crew_noposible = false; //si es imposible crear un crew, se pondrá en verdadero
    memset(crew_matriz, 0, tamano_listas);
        do{
            crew_noposible = genera_crew (vuelo_1);
            while (revisa_si_esta_en_matriz(crew_matriz, r, solucion) == true && num_reconstrucciones <= max_reconstrucciones
                    && crew_noposible == false){
                crew_noposible = genera_crew(vuelo_1);
                num_reconstrucciones++;
            }
            if (num_reconstrucciones<=max_reconstrucciones && sig_vuelo.imposible_generar_crew == false){
                copia_dos_vectores(solucion, crew_matriz[r]);
                crews_generados = r+1;
                vector_costos_crew[calcula_renglones(matriz_de_crews) + r] = costo_crew;
            }
                r++;
        } while (num_reconstrucciones <= max_reconstrucciones && sig_vuelo.imposible_generar_crew == false &&
                 crews_generados< max_num_crews_por_vuelo);

    if (crew_noposible == true) {
        crews_generados = 0;
        crew_noposible = false;
    }
    crews_por_vuelo[pos_vuelo_en_vuelos_from_bases] = crews_generados;
}
int main(int argc, char ** argv) { //argc es el número de parámetros, argv tiene los parámetros
    //Ejecución: IA2_SA.exe nombre_archivo.csv num_iteraciones semilla
    //--Lectura del archivo .csv--//
    if (argc >= 2){
        strcpy(file_name, argv[1]);
        printf("Archivo copiado: %s", file_name);
        if (argc >= 3){
            strcpy(iteraciones_char, argv[2]); //Núm. iteraciones manualmente
            max_iteraciones = atoi(iteraciones_char); //Char a entero
            if (argc == 4){
            strcpy(seed_char, argv[3]); //Seed ingresada manualmente
            seed = atoi(seed_char); //Char a entero
            } else {
                seed = (unsigned int)time(NULL); //Seed aleatoria
            }
        } else {
            max_iteraciones = max_iteraciones_default;
            seed = (unsigned int)time(NULL); //Seed aleatoria
        }
    }
    else { //Si no se especifica nada, todo por default
        printf("Archivo no especificado, se tomara el default: ''%s''\n", archivo_por_default);
        strcpy(file_name,archivo_por_default); //Archivo por defecto
        seed = (unsigned int)time(NULL); //Seed aleatoria
        max_iteraciones = max_iteraciones_default; //Iteraciones por default
    }

    for (int colmn = 1; colmn<= 5; colmn++){ //Lee del archivo file_name de las columnas 1 a 5
        num_vuelos = get_file_vuelos(file_name, colmn);
    }


    //(a)~~Estas líneas sólo se necesitan hacer una vez en cada ejecución del programa...
    //seed = 1572594195; //seed manual
    srand(seed); //Establece la semilla para los números aleatorios.
    strcpy(nombre_archivo_csv,"Solucion_");
    strcat(nombre_archivo_csv, file_name);
    //--Conversión de horas de salida y llegada char, a flotante--//
    convierte_vector_horaschar_a_decimal(num_vuelos, departure, departure_decimal);
    convierte_vector_horaschar_a_decimal(num_vuelos, arrival, arrival_decimal);
    copia_vuelos_restantes(num_vuelos, id_vuelo, id_vuelos_restantes, id_vuelos_restantes_temp);
    //--Genera la lista "vuelos_from_bases" que tiene las ID de los vuelos que salen de B1 y B2, y calcula su tamaño--//
    filtra_vuelos_bases(num_vuelos, B1, B2, origin, id_vuelo, vuelos_from_bases, num_vuelos, id_vuelo);

    num_vuelos_bases = calcula_tamano(vuelos_from_bases);
    //--Ordena por hora de salida los vuelos que parten de las bases, en esa misma lista--//
    ordena_por_hora(vuelos_from_bases, num_vuelos_bases, num_vuelos, id_vuelo, departure_decimal);
    //--Ordena las ID de los vuelos por hora de salida--//
    copia_dos_vectores(id_vuelo, id_vuelo_ordenada);
    ordena_por_hora(id_vuelo_ordenada, num_vuelos, num_vuelos, id_vuelo, departure_decimal);
    //--Reajusta las horas de llegada y salida para que no haya problema si hay vuelos que salen o llegan en la madrugada del día siguiente--//
    posi_vuelo_mas_temprano = encuentra_posicion(vuelos_from_bases[0], num_vuelos, id_vuelo);
    double depart_vuelo_mas_temprano = departure_decimal[posi_vuelo_mas_temprano]; //Hora de partida del vuelo que sale más temprano
    //printf("\nEl vuelo %d sale mas temprano, posicion %d, sale a las: %.2f", vuelos_from_bases[0], posi_vuelo_mas_temprano, depart_vuelo_mas_temprano);
    horas_departure_reajuste(num_vuelos, departure_decimal, depart_vuelo_mas_temprano);
    horas_arrival_reajuste(num_vuelos, arrival_decimal, depart_vuelo_mas_temprano);
    //--Calcula las duraciones de cada uno de los vuelos--//
    calcula_duraciones(num_vuelos, departure_decimal, arrival_decimal, duraciones);
    //--Descomentar para ver información completa de los vuelos--//
    /*printf("\nHorario de %d vuelos.", num_vuelos);
    for (int i = 0; i<num_vuelos; i++){
        printf("\nID: %d Ori: %s Dest: %s Dep: %s = %.2f Arr: %s = %.2f Dur: %.4f", id_vuelo[i], origin[i], destination[i],
                departure[i], departure_decimal[i], arrival[i], arrival_decimal[i], duraciones[i]);
    } */
    num_crew_resets = 0;
    num_crews = 0;
    costo_solucion = 0;
    crews_sin_retorno = 0;
    memset(solucion_matriz, 0, sizeof solucion_matriz);
    strcpy(nombre_archivo_csv,"Solucion_");
    strcat(nombre_archivo_csv, file_name);
    //...~~(a)//

    //--Comienza la generación de pairings--//
    for (int v = 0; v<num_vuelos_bases; v++){
        genera_set_de_crews(vuelos_from_bases[v], v);
        //--Una vez generado el set de crews, que los pase a la matriz global de crews--//
        int renklones = calcula_renglones(matriz_de_crews);
        for (int i = renklones; i < renklones + crews_por_vuelo[v]; i++){
            copia_dos_vectores(crew_matriz[i-renklones], matriz_de_crews[i]);
        }
    }
    int total_crews = suma_elementos_de_vector(crews_por_vuelo, num_vuelos_bases);
    int vector_auxiliar_crews_random [tamano_matriz_crews] = {0};
    for (int x = 0; x<total_crews; x++){
        vector_auxiliar_crews_random[x] = x+1;
    }

    reacomoda_vector_randomly(vector_auxiliar_crews_random, total_crews);

    //--Comienza la magia de la técnica de búsqueda--//
    //-1. Genera solución inicial, aleatoria-//

    coloca_1s_aleatorios(solucion_inicial, total_crews, num_vuelos/6);
    solucion_bool_a_decimal(solucion_inicial, solucion_inicial_por_num_crew, total_crews, matriz_de_crews, vector_costos_crew, vector_auxiliar_crews_random);

    //-2. Evalúa el costo de la solución generada-//
    costo_solucion_inicial = calcula_costo_solucion(solucion_actual_por_num_crew, vector_costos_crew, matriz_de_crews, num_vuelos, id_vuelo,
                           penalizacion_por_vuelo_no_cubierto, penalizacion_por_deadhead);
    costo_solucion_actual = costo_solucion_inicial;
    //printf("\nCosto sol: %.2f", costo_solucion_actual);
    //-3. Al ser la única solución hasta el momento, que la guarde en "mejor solución encontrada"-//
    backup_best_solucion(solucion_inicial_por_num_crew, mejor_solucion_por_num_crew, &costo_mejor_solucion, costo_solucion_inicial);
    //costo_mejor_solucion = costo_solucion_actual;
    //printf("\nCosto mejor solucion (main): %.2f", costo_mejor_solucion);
    //-4. Comienza el algoritmo de simulated annealing-//
    //La siguiente solución se generará mediante el movimiento elegido: bit flip, 1->0 o 0->1, 1 mov. a la vez.
    int pos_bit = 0;
    temperatura = temperatura_inicial;
    int iter;
    copia_dos_vectores_bool(solucion_inicial, solucion_actual); //SA = SI
    for (iter = 0; (iter< max_iteraciones) ; iter++){ //&& pos_bit<total_crews
    solucion_bool_a_decimal(solucion_actual, solucion_actual_por_num_crew, total_crews, matriz_de_crews, vector_costos_crew, vector_auxiliar_crews_random);
    solucion_bool_a_decimal(solucion_inicial, solucion_inicial_por_num_crew, total_crews, matriz_de_crews, vector_costos_crew, vector_auxiliar_crews_random);
    copia_dos_vectores_bool(solucion_actual, solucion_futura); //SF = SA
    bit_flip(solucion_futura, pos_bit, total_crews); //SF modificada con bitflip.
    pos_bit++;
    solucion_bool_a_decimal(solucion_futura, solucion_futura_por_num_crew, total_crews, matriz_de_crews, vector_costos_crew, vector_auxiliar_crews_random);
    costo_solucion_futura = calcula_costo_solucion(solucion_futura_por_num_crew, vector_costos_crew, matriz_de_crews,
                                                   num_vuelos, id_vuelo, penalizacion_por_vuelo_no_cubierto, penalizacion_por_deadhead);
    //printf("\nCosto sol. fut: %.2f | Costo sol. act: %.2f", costo_solucion_futura, costo_solucion_actual);
    if (costo_solucion_futura<costo_mejor_solucion){ //Si la solución encontrada es mejor que la mejor hasta el momento, sustituir.
        backup_best_solucion(solucion_futura_por_num_crew, mejor_solucion_por_num_crew, &costo_mejor_solucion,
                              costo_solucion_futura);
    }
    if (compara_dos_vectores(solucion_inicial_por_num_crew, solucion_futura_por_num_crew) == false){ //Si la solución encontrada no es igual a la anterior.
        if (costo_solucion_futura<costo_solucion_actual || pos_bit >= total_crews - 1){ //Si la solución encontrada es mejor a la anterior, la elijo
            copia_dos_vectores_bool(solucion_actual, solucion_inicial); //SI = SA
            costo_solucion_inicial = costo_solucion_actual;
            copia_dos_vectores_bool(solucion_futura, solucion_actual); //SA = SF
            costo_solucion_actual = costo_solucion_futura;
            pos_bit = 0;
        } else { //Si no, que entre la probabilidad
            bool la_eliges;
            la_eliges = eleccion_por_simulated_annealing(costo_solucion_actual, costo_solucion_futura, temperatura);
            if (la_eliges == true){
                copia_dos_vectores_bool(solucion_actual, solucion_inicial); //SI = SA
                costo_solucion_inicial = costo_solucion_actual;
                copia_dos_vectores_bool(solucion_futura, solucion_actual); //SA = SF
                costo_solucion_actual = costo_solucion_futura;
                pos_bit = 0;
            }

        }
    } else {
    //printf("\nSolucion repetida, descartada.");
    }
    temperatura = temperatura*coeficiente_temperatura; //Enfriado de la temperatura.
    }
    printf("\nMejor solucion encontrada, precio: %.2f ", costo_mejor_solucion);
    int num_crews_best_sol = calcula_tamano(mejor_solucion_por_num_crew);
    printf("\nMejor solucion formada por %d crews: ", num_crews_best_sol);
    for (int i = 0; i<num_crews_best_sol; i++){
        printf("%d ", mejor_solucion_por_num_crew[i]);
    }
    for (int r = 0; r<calcula_tamano(mejor_solucion_por_num_crew); r++){
        printf("\nCrew %d: ", mejor_solucion_por_num_crew[r]);
        for (int c = 0; c<calcula_tamano(matriz_de_crews[mejor_solucion_por_num_crew[r]-1]); c++){
            printf("%d ", matriz_de_crews[mejor_solucion_por_num_crew[r]-1][c]);
        }
        printf("    $%.2f %s", vector_costos_crew[mejor_solucion_por_num_crew[r]-1], unidades_costo);
    }
    gettimeofday(&end, NULL); //Tiempo a la finalización del algoritmo
    double time_taken;
    time_taken = (end.tv_sec - start.tv_sec) * 1e6;
    time_taken = (time_taken + (end.tv_usec -
                              start.tv_usec)) * 1e-6;
    recuento_y_creacion_csv(mejor_solucion_por_num_crew, vector_costos_crew, matriz_de_crews, num_vuelos, id_vuelo,
                           penalizacion_por_vuelo_no_cubierto, penalizacion_por_deadhead, nombre_archivo_csv,
                           costo_mejor_solucion, seed, unidades_costo, time_taken);

    printf("\n\nSeed utlizada: %u\n", seed);
    printf("\n%d iteraciones realizadas\n", iter);
    printf("\nTiempo de ejecucion: %.6f s\n", time_taken);
    //Descomentar lo de abajo si se desean visualizar todos los crews creados:
    /*printf("\ncrews totales generados: %d\n", total_crews);
    for (int r = 0; r<calcula_renglones(matriz_de_crews); r++){
        printf("Crew %d: ", r+1);
        for (int c = 0; c<calcula_tamano(matriz_de_crews[r]); c++){
            printf("%d ", matriz_de_crews[r][c]);
        }
        printf("---> Costo %.2f %s", vector_costos_crew[r], unidades_costo);
        printf("\n");
    }*/
}


