#ifndef SIM_ALLOC_H
#define SIM_ALLOC_H

#include <stddef.h> // Para size_t

// Definición de constantes
#define SEMILLA_ALEATORIA 42
#define NUM_ITERACIONES 5000
#define TAMANO_MAXIMO_ASIGNACION 1024
#define MAX_PUNTEROS_ACTIVOS 1000
#define TIEMPO_ESPERA_SEGUNDOS 1

/**
 * @brief Obtiene la métrica de fragmentación externa utilizando el método First Fit.
 *
 * @return El valor de la fragmentación externa calculada por el método First Fit.
 */
double get_frag_first_fit();

/**
 * @brief Obtiene la métrica de fragmentación externa utilizando el método Best Fit.
 *
 * @return El valor de la fragmentación externa calculada por el método Best Fit.
 */
double get_frag_best_fit();

/**
 * @brief Obtiene la métrica de fragmentación externa utilizando el método Worst Fit.
 *
 * @return El valor de la fragmentación externa calculada por el método Worst Fit.
 */
double get_frag_worst_fit();

/**
 * @brief Genera datos para las iteraciones del simulador.
 *
 * Llena los arrays globales `acciones` y `tamanos` con valores aleatorios
 * para simular operaciones de asignación (malloc) y liberación (free).
 */
void generar_datos(void);

/**
 * @brief Ejecuta el simulador con una política específica.
 *
 * @param metodo Indica la política a utilizar:
 *               - 0: First Fit
 *               - 1: Best Fit
 *               - 2: Worst Fit
 */
void simulador(int metodo);

/**
 * @brief Punto de entrada principal para el simulador.
 *
 * Inicializa los datos aleatorios y ejecuta iterativamente las simulaciones
 * para diferentes políticas de asignación de memoria.
 *
 * @return 0 si la ejecución es exitosa.
 */
void* init_sim(void* arg);

#endif // SIM_ALLOC
