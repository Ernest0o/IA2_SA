#ifndef STRUCT_DEFINITION_H_INCLUDED
#define STRUCT_DEFINITION_H_INCLUDED
#include <stdbool.h>
struct returns_siguiente_vuelo {
    int next_flight;
    bool fin_tripulacion;
    bool atascado;
    bool hay_retorno;
    bool imposible_generar_crew;
};

#endif // STRUCT_DEFINITION_H_INCLUDED
