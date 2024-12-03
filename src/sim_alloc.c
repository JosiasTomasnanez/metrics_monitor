#include "memory.h" // Tu biblioteca personalizada
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // Para sleep()
// Definición de constantes
#define SEMILLA_ALEATORIA 73        /**< Semilla para la generación de números aleatorios. */
#define NUM_ITERACIONES 25          /**< Número de iteraciones por simulación. */
#define TAMANO_MAXIMO_ASIGNACION 50 /**< Tamaño máximo de bloques de memoria a asignar. */
#define MAX_PUNTEROS_ACTIVOS 10     /**< Máximo número de punteros activos simultáneamente. */
#define TIEMPO_ESPERA_SEGUNDOS 5    /**< Tiempo de espera entre iteraciones en segundos. */

sem_t sem; /**< Semáforo para sincronización en el acceso a métricas. */
// Variables globales para fragmentación
double frag0 = 0; /**< Fragmentación externa para First Fit. */
double frag1 = 0; /**< Fragmentación externa para Best Fit. */
double frag2 = 0; /**< Fragmentación externa para Worst Fit. */
/**
 * @brief Obtiene la métrica de fragmentación externa utilizando el método First Fit.
 * @return El valor de la fragmentación externa calculada.
 */
double get_frag_first_fit()
{
    sem_wait(&sem);
    double f = frag0;
    sem_post(&sem);
    return f;
}

/**
 * @brief Obtiene la métrica de fragmentación externa utilizando el método Best Fit.
 * @return El valor de la fragmentación externa calculada.
 */
double get_frag_best_fit()
{
    sem_wait(&sem);
    double f = frag1;
    sem_post(&sem);
    return f;
}

/**
 * @brief Obtiene la métrica de fragmentación externa utilizando el método Worst Fit.
 * @return El valor de la fragmentación externa calculada.
 */
double get_frag_worst_fit()
{
    sem_wait(&sem);
    double f = frag2;
    sem_post(&sem);
    return f;
}

/**
 * @brief Genera datos aleatorios para las acciones y tamaños de bloques de memoria.
 */
void generar_datos(int* acciones, size_t* tamanos)
{
    for (int i = 0; i < NUM_ITERACIONES; i++)
    {
        acciones[i] = rand() % 2; // 0: malloc, 1: free
        tamanos[i] = (rand() % TAMANO_MAXIMO_ASIGNACION) + 1;
    }
}

/**
 * @brief Simula la asignación y liberación de memoria para una política específica.
 */
void simulador(int metodo, int* acciones, size_t* tamanos, double* frag)
{
    set_method(metodo);
    void* punteros[MAX_PUNTEROS_ACTIVOS];
    size_t asignaciones_activas = 0;

    for (int i = 0; i < NUM_ITERACIONES; i++)
    {
        if (acciones[i] == 0 && asignaciones_activas < MAX_PUNTEROS_ACTIVOS)
        {
            void* bloque = my_malloc(tamanos[i]);
            if (bloque)
            {
                punteros[asignaciones_activas++] = bloque;
            }
        }
        else if (acciones[i] == 1 && asignaciones_activas > 0)
        {
            size_t indice = rand() % asignaciones_activas;
            my_free(punteros[indice]);
            punteros[indice] = punteros[--asignaciones_activas];
        }
    }

    // Liberar memoria restante
    for (size_t i = 0; i < asignaciones_activas; i++)
    {
        my_free(punteros[i]);
    }

    // Calcular y almacenar fragmentación
    sem_wait(&sem);
    *frag = external_frag();
    sem_post(&sem);
    mem_trim();
}

/**
 * @brief Inicializa la simulación de fragmentación externa.
 * @return Siempre retorna NULL.
 */
void* init_sim(void* arg)
{
    (void)arg;
    srand(SEMILLA_ALEATORIA); // Semilla fija para reproducibilidad
    sem_init(&sem, 0, 1);
    // Arreglos para las iteraciones
    int acciones[NUM_ITERACIONES];
    size_t tamanos[NUM_ITERACIONES];

    while (1)
    {
        // para habilitar la prueba , hay que descomentar la siguiente linea
        break;
        // nuestra lib no soporta mucho tiempo haciendo tanto malloc y free debido a la limitacion del sistema
        //  Generar los mismos datos para las tres políticas
        generar_datos(acciones, tamanos);

        // Ejecutar simuladores con los mismos datos
        simulador(0, acciones, tamanos, &frag0); // First Fit
        simulador(1, acciones, tamanos, &frag1); // Best Fit
        simulador(2, acciones, tamanos, &frag2); // Worst Fit
        sleep(TIEMPO_ESPERA_SEGUNDOS);           // Espera para la próxima iteración
    }
    return NULL;
}
