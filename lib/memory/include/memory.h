/**
 * @file memory.h
 * @brief Memory management library with custom allocation functions.
 *
 * Esta biblioteca define funciones de asignación de memoria dinámica
 * y gestión de bloques para crear un asignador de memoria personalizado.
 */

#pragma once

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

/**
 * @brief Macro para alinear una cantidad de bytes al siguiente múltiplo de 8.
 *
 * @param x Cantidad de bytes a alinear.
 */
#define align(x) (((((x) - 1) >> 3) << 3) + 8)

/** Tamaño mínimo de un bloque de memoria. */
#define BLOCK_SIZE 40
/** Tamaño de página en memoria. */
#define PAGESIZE 4096
/** Política de asignación First Fit. */
#define FIRST_FIT 0
/** Política de asignación Best Fit. */
#define BEST_FIT 1
/** Política de asignación Worst Fit. */
#define WORST_FIT 2
/** Tamaño del bloque */
#define DATA_START 1
/** Dirección inválida */
#define INVALID_ADDR 0

/**
 * @struct s_block
 * @brief Estructura para representar un bloque de memoria.
 *
 * Contiene la información necesaria para gestionar la asignación y
 * liberación de un bloque de memoria.
 */
struct s_block
{
    size_t size;           /**< Tamaño del bloque de datos. */
    struct s_block* next;  /**< Puntero al siguiente bloque en la lista enlazada. */
    struct s_block* prev;  /**< Puntero al bloque anterior en la lista enlazada. */
    int free;              /**< Indicador de si el bloque está libre (1) o ocupado (0). */
    void* ptr;             /**< Puntero a la dirección de los datos almacenados. */
    char data[DATA_START]; /**< Área donde comienzan los datos del bloque. */
};

/** Tipo de puntero para un bloque de memoria. */
typedef struct s_block* t_block;

/**
 * @enum alloc_type
 * @brief Enumeración que define los tipos de operaciones de memoria.
 *
 * Esta enumeración se utiliza para identificar el tipo de operación
 * de asignación o liberación de memoria que se está realizando.
 */
typedef enum
{
    ALLOC_TYPE_MALLOC,  /**< Asignación de memoria con malloc. */
    ALLOC_TYPE_CALLOC,  /**< Asignación de memoria con calloc. */
    ALLOC_TYPE_REALLOC, /**< Redimensionamiento de memoria con realloc. */
    ALLOC_TYPE_FREE     /**< Liberación de memoria con free. */
} alloc_type;

/**
 * @brief Obtiene el bloque que contiene una dirección de memoria dada.
 *
 * @param p Puntero a la dirección de datos.
 * @return t_block Puntero al bloque de memoria correspondiente.
 */
t_block get_block(void* p);

/**
 * @brief Verifica si una dirección de memoria es válida.
 *
 * @param p Dirección de memoria a verificar.
 * @return int Retorna 1 si la dirección es válida, 0 en caso contrario.
 */
int valid_addr(void* p);

/**
 * @brief Encuentra un bloque libre que tenga al menos el tamaño solicitado.
 *
 * @param last Puntero al último bloque.
 * @param size Tamaño solicitado.
 * @return t_block Puntero al bloque encontrado, o NULL si no se encuentra ninguno.
 */
t_block find_block(t_block* last, size_t size);

/**
 * @brief Expande el heap para crear un nuevo bloque de memoria.
 *
 * @param last Último bloque del heap.
 * @param s Tamaño del nuevo bloque.
 * @return t_block Puntero al nuevo bloque creado.
 */
t_block extend_heap(t_block last, size_t s);

/**
 * @brief Divide un bloque de memoria en dos, si el tamaño solicitado es menor que el bloque disponible.
 *
 * @param b Bloque a dividir.
 * @param s Tamaño del nuevo bloque.
 */
void split_block(t_block b, size_t s);

/**
 * @brief Fusiona un bloque libre con su siguiente bloque si también está libre.
 *
 * @param b Bloque a fusionar.
 * @return t_block Puntero al bloque fusionado.
 */
t_block fusion(t_block b);

/**
 * @brief Copia el contenido de un bloque de origen a un bloque de destino.
 *
 * @param src Bloque de origen.
 * @param dst Bloque de destino.
 */
void copy_block(t_block src, t_block dst);

/**
 * @brief Asigna un bloque de memoria del tamaño solicitado.
 *
 * @param size Tamaño en bytes del bloque a asignar.
 * @return void* Puntero al área de datos asignada.
 */
void* my_malloc(size_t size);

/**
 * @brief Libera un bloque de memoria previamente asignado.
 *
 * @param p Puntero al área de datos a liberar.
 */
void my_free(void* p);

/**
 * @brief Asigna un bloque de memoria para un número de elementos, inicializándolo a cero.
 *
 * @param number Número de elementos.
 * @param size Tamaño de cada elemento.
 * @return void* Puntero al área de datos asignada e inicializada.
 */
void* my_calloc(size_t number, size_t size);

/**
 * @brief Cambia el tamaño de un bloque de memoria previamente asignado.
 *
 * @param p Puntero al área de datos a redimensionar.
 * @param size Nuevo tamaño en bytes.
 * @return void* Puntero al área de datos redimensionada.
 */
void* my_realloc(void* p, size_t size);

/**
 * @brief Verifica el estado del heap y detecta bloques libres consecutivos.
 *
 * @param data Información adicional para la verificación.
 */
void check_heap(void* data);

/**
 * @brief Configura el modo de asignación de memoria (First Fit o Best Fit).
 *
 * @param mode Modo de asignación (0 para First Fit, 1 para Best Fit).
 */
void malloc_control(int mode);

/**
 * @brief Establece el método de asignación de memoria.
 *
 * Esta función permite seleccionar el método de asignación de memoria
 * que se utilizará al buscar bloques libres. Los métodos disponibles son:
 * - 0: Primer ajuste (First Fit)
 * - 1: Mejor ajuste (Best Fit)
 *
 * @param m Un entero que representa el método de asignación deseado.
 *          Debe ser 0 o 1. Cualquier otro valor es inválido.
 */
void set_method(int m);

/**
 * @brief Reporta el uso de memoria actual.
 *
 * Esta función recorre todos los bloques de memoria gestionados por el
 * administrador de memoria personalizado y calcula el tamaño total de
 * memoria asignada y la cantidad de memoria libre. Luego, imprime un
 * informe detallado del uso de memoria.
 *
 * El informe incluye:
 * - Total de memoria asignada en bytes.
 * - Total de memoria libre en bytes.
 *
 * Esta función es útil para monitorear la eficiencia del uso de memoria
 * y detectar posibles problemas de fragmentación o fugas de memoria.
 */
void memory_usage();

/**
 * @brief Finaliza y cierra los registros de depuración.
 *
 * Esta función limpia cualquier recurso relacionado con los registros
 * de depuración y finaliza su uso. Debe llamarse al terminar la ejecución
 * del programa.
 */
void log_close();

/**
 * @brief Maneja el registro de una operación de memoria.
 *
 * Esta función registra información sobre una operación de asignación
 * o liberación de memoria, incluyendo el nombre de la función, tipo
 * de operación, puntero involucrado y tamaño del bloque.
 *
 * @param func_name Nombre de la función que realizó la operación.
 * @param type Tipo de operación de memoria (malloc, calloc, realloc, free).
 * @param ptr Puntero a la memoria afectada.
 * @param size Tamaño de la memoria involucrada, en bytes.
 */
void log_handler(const char* func_name, alloc_type type, void* ptr, size_t size);

/**
 * @brief Registra la llamada a una función de gestión de memoria.
 *
 * Esta función almacena información sobre las funciones de gestión
 * de memoria que han sido llamadas, incluyendo el tipo de asignación,
 * el puntero de memoria y el tamaño solicitado o liberado.
 *
 * @param func_name Nombre de la función llamada.
 * @param type Tipo de asignación realizada.
 * @param ptr Puntero a la memoria afectada.
 * @param size Tamaño de la memoria afectada.
 */
void log_function_call(const char* func_name, alloc_type type, void* ptr, size_t size);

/**
 * @brief Reduce el tamaño del heap para recuperar memoria al sistema operativo.
 *
 * Esta función intenta reducir el tamaño del heap si existen bloques libres
 * al final de la memoria asignada, devolviendo esa memoria al sistema operativo.
 */
void mem_trim();

/**
 * @brief Muestra un mensaje de depuración.
 *
 * Esta función toma un mensaje en formato de cadena y lo imprime,
 * útil para fines de depuración durante la ejecución del programa.
 *
 * @param message El mensaje de depuración a mostrar.
 */
void debug_message(const char* message);

/**
 * @brief Libera un bloque de memoria gestionado.
 *
 * Esta función es una implementación personalizada para liberar bloques
 * de memoria previamente asignados y manejar su estado dentro de la lista
 * de bloques.
 *
 * @param ptr Puntero al área de memoria a liberar.
 */
void custom_free(void* ptr);

/**
 * @brief Calcula la fragmentación externa de la memoria.
 *
 * Esta función evalúa la cantidad de memoria que está libre pero no
 * disponible para nuevas asignaciones debido a la fragmentación.
 *
 * @return double Porcentaje de fragmentación externa.
 */
double external_frag();
