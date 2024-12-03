#include <memory.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

/**
 * @brief Define el tipo de dato de un bloque de memoria.
 */
typedef struct s_block* t_block;

void* base = NULL;
int method = 0;
int malloc_call = 0;
FILE* log_file = NULL;

/**
 * @brief Busca un bloque de memoria adecuado según el tamaño requerido.
 * @return Puntero al bloque encontrado o NULL si no se encuentra ninguno.
 */
t_block find_block(t_block* last, size_t size)
{
    t_block b = base;
    switch (method)
    {
    case 0:
        while (b && !(b->free && b->size >= size))
        {
            *last = b;
            b = b->next;
        }
        return b;
    case 1: {
        size_t dif = PAGESIZE;
        t_block best = NULL;
        while (b)
        {
            if (b->free)
            {
                if (b->size == size)
                    return b;
                if (b->size > size && (b->size - size) < dif)
                {
                    dif = b->size - size;
                    best = b;
                }
            }
            *last = b;
            b = b->next;
        }
        return best;
    }
    case 2: {
        size_t dif = 0;
        t_block best = NULL;
        while (b)
        {
            if (b->free)
            {
                if (b->size == size)
                    return b;
                if (b->size > size && (b->size - size) > dif)
                {
                    dif = b->size - size;
                    best = b;
                }
            }
            *last = b;
            b = b->next;
        }
        return best;
    }
    default:
        return NULL;
    }
}

/**
 * @brief Divide un bloque en dos, si es lo suficientemente grande.
 */
void split_block(t_block b, size_t s)
{
    if (b->size <= s + BLOCK_SIZE)
    {
        return;
    }
    t_block new;
    new = (t_block)(b->data + s);
    new->size = b->size - s - BLOCK_SIZE;
    new->next = b->next;
    new->free = 1;
    b->size = s;
    b->next = new;
}

/**
 * @brief Copia los datos de un bloque a otro.
 */
void copy_block(t_block src, t_block dst)
{
    int *sdata, *ddata;
    size_t i;
    if (!src->ptr || !dst->ptr)
    {
        return;
    }
    sdata = src->ptr;
    ddata = dst->ptr;

    for (i = 0; (i * 4) < src->size && (i * 4) < dst->size; i++)
        ddata[i] = sdata[i];
}

/**
 * @brief Obtiene el bloque de memoria asociado a un puntero.
 * @return Puntero al bloque correspondiente.
 */
t_block get_block(void* p)
{
    char* tmp;
    tmp = p;
    tmp -= BLOCK_SIZE;
    return (t_block)(tmp);
}

/**
 * @brief Verifica si una dirección de memoria es válida.
 * @return 1 si es válida, 0 si no lo es.
 */
int valid_addr(void* p)
{
    if (p == NULL || base == NULL)
    {

        return 0;
    }
    t_block b = get_block(p);
    t_block current = base;
    while (current)
    {
        if (current == b)
        {
            return (current->ptr == p);
        }
        current = current->next;
    }
    return INVALID_ADDR;
}

/**
 * @brief Fusiona bloques libres adyacentes para reducir fragmentación.
 * @return Puntero al bloque fusionado.
 */
t_block fusion(t_block b)
{
    if (b == NULL)
        return NULL; // Retorna NULL si el bloque inicial es inválido

    while (b->next && b->next->free)
    {
        // Validar que b->next y b->next->next sean válidos
        if (b->next == NULL || b->next->next == NULL)
            break;

        b->size += b->next->size;
        b->next = b->next->next;

        if (b->next)
            b->next->prev = b;
    }
    return b;
}

/**
 * @brief Extiende el heap asignando un nuevo bloque.
 * @return Puntero al nuevo bloque o NULL en caso de error.
 */
t_block extend_heap(t_block last, size_t s)
{
    t_block b;
    b = mmap(0, s, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (b == MAP_FAILED)
    {
        return NULL;
    }
    b->size = s;
    b->next = NULL;
    b->prev = last;
    b->ptr = b->data;
    if (last)
        last->next = b;

    b->free = 0;
    return b;
}

/**
 * @brief Obtiene el método de asignación actual.
 * @return El método actual (0, 1 o 2).
 */
int get_method()
{
    return method;
}

/**
 * @brief Establece el método de asignación.
 */
void set_method(int m)
{
    method = m;
}

/**
 * @brief Controla el método de asignación basado en un valor entero.
 */
void malloc_control(int m)
{
    if (m == 0)
    {
        set_method(0);
    }
    else if (m == 1)
    {
        set_method(1);
    }
    else if (m == 2)
    {
        set_method(2);
    }
    else
    {
        printf("Error: invalid method\n");
    }
}

/**
 * @brief Solicita memoria dinámica con el tamaño especificado.
 * @return Puntero al área de datos asignada o NULL en caso de error.
 */
void* my_malloc(size_t size)
{
    t_block b, last;
    size_t s;
    s = align(size);

    if (base)
    {
        last = base;
        b = find_block(&last, s);
        if (b)
        {
            if ((b->size - s) >= (BLOCK_SIZE + 4))
                split_block(b, s);
            b->free = 0;
        }
        else
        {
            b = extend_heap(last, s);
            if (!b)
                return (NULL);
        }
    }
    else
    {
        b = extend_heap(NULL, s);
        if (!b)
            return (NULL);
        base = b;
    }
    if (!malloc_call)
        log_handler("malloc", ALLOC_TYPE_MALLOC, b->data, size);
    malloc_call = 0;
    return (b->data);
}

/**
 * @brief Libera un bloque de memoria previamente asignado.
 */
void my_free(void* ptr)
{
    t_block b;
    if (valid_addr(ptr))
    {
        b = get_block(ptr);
        b->free = 1;
        fusion(b);
        if (!b->next)
        {
            if (!b->prev)
                base = NULL;
            else
                b->prev->next = NULL;
            brk((char*)b + b->size);
        }
        log_handler("free", ALLOC_TYPE_FREE, ptr, 0);
    }
}

/**
 * @brief Solicita memoria dinámica inicializada a cero.
 * @return Puntero a la memoria asignada o NULL en caso de error.
 */
void* my_calloc(size_t number, size_t size)
{
    size_t* new;
    size_t s4, i;

    if (!number || !size)
    {
        return (NULL);
    }
    malloc_call = 1;
    new = my_malloc(number * size);
    if (new)
    {
        s4 = align(number * size) << 2;
        for (i = 0; i < s4; i++)
            new[i] = 0;
    }
    log_handler("calloc", ALLOC_TYPE_CALLOC, new, 0);
    return (new);
}

/**
 * @brief Reasigna un bloque de memoria con un nuevo tamaño.
 * @return Puntero al bloque reasignado o NULL en caso de error.
 */
void* my_realloc(void* ptr, size_t size)
{
    size_t s;
    t_block b, new;
    void* newp;

    malloc_call = 1;

    if (!ptr)
    {
        newp = my_malloc(size);
        log_handler("realloc", ALLOC_TYPE_REALLOC, newp, 0);
        return newp;
    }

    if (valid_addr(ptr))
    {
        s = align(size);
        b = get_block(ptr);

        if (b->size >= s)
        {
            if (b->size - s >= (BLOCK_SIZE + 4))
                split_block(b, s);
        }
        else
        {
            if (b->next && b->next->free && (b->size + BLOCK_SIZE + b->next->size) >= s)
            {
                fusion(b);
                if (b->size - s >= (BLOCK_SIZE + 4))
                    split_block(b, s);
            }
            else
            {
                newp = my_malloc(s);
                if (!newp)
                    return (NULL);
                new = get_block(newp);
                copy_block(b, new);
                my_free(ptr);
                log_handler("realloc", ALLOC_TYPE_REALLOC, newp, 0);
                return (newp);
            }
        }
        log_handler("realloc", ALLOC_TYPE_REALLOC, ptr, 0);
        return (ptr);
    }
    return (NULL);
}

/**
 * @brief Verifica y muestra información sobre un bloque de memoria.
 */
void check_heap(void* data)
{
    if (data == NULL)
    {
        printf("Data is NULL\n");
        return;
    }

    t_block block = get_block(data);

    if (block == NULL)
    {
        printf("Block is NULL\n");
        return;
    }

    printf("\033[1;33mHeap check\033[0m\n");
    printf("Size: %zu\n", block->size);

    if (block->next != NULL)
    {
        printf("Next block: %p\n", (void*)(block->next));
    }
    else
    {
        printf("Next block: NULL\n");
    }

    if (block->prev != NULL)
    {
        printf("Prev block: %p\n", (void*)(block->prev));
    }
    else
    {
        printf("Prev block: NULL\n");
    }

    printf("Free: %d\n", block->free);

    if (block->ptr != NULL)
    {
        printf("Beginning data address: %p\n", block->ptr);
        printf("Last data address: %p\n", (void*)((char*)(block->ptr) + block->size));
    }
    else
    {
        printf("Data address: NULL\n");
    }

    printf("Heap address: %p\n", sbrk(0));

    // Check for inconsistencies
    t_block current = base;
    while (current)
    {
        // Check for adjacent free blocks
        if (current->free && current->next && current->next->free)
        {
            printf("Warning: Adjacent free blocks detected at %p and %p\n", (void*)current, (void*)current->next);
        }

        // Check for invalid block sizes
        if (current->size <= 0)
        {
            printf("Error: Invalid block size detected at %p\n", (void*)current);
        }

        current = current->next;
    }
}

/**
 * @brief Muestra un reporte del uso de memoria actual.
 */
void memory_usage()
{
    size_t total_allocated = 0;
    size_t total_free = 0;

    t_block current = base;
    while (current)
    {
        if (current->free)
        {
            total_free += current->size;
        }
        else
        {
            total_allocated += current->size;
        }
        printf("bloque de memoria reservado de: %ld bytes\n", current->size);
        printf("prev: %p\n", (void*)current->prev);
        printf("next: %p\n", (void*)current->next);
        current = current->next;
    }

    printf("\033[1;34mMemory Usage Report:\033[0m\n");
    printf("total memory used: %zu bytes\n", total_allocated + total_free);
    printf("Total allocated memory: %zu bytes\n", total_allocated);
    printf("Total free memory: %zu bytes\n", total_free);
}

/**
 * @brief Libera y fusiona todos los bloques de memoria no utilizados.
 */
void mem_trim()
{
    t_block current = base;
    while (current)
    {
        if (!current->free)
        {
            my_free(current->ptr); // Libera el bloque si no está marcado como libre
        }
        fusion(current);
        if (!current->next)
        {
            base = NULL;
        }
        current = current->next;
    }
}

/**
 * @brief Maneja el registro de llamadas a funciones de asignación de memoria.
 */
void log_handler(const char* func_name, alloc_type type, void* ptr, size_t size)
{
    if (log_file == NULL)
        log_file = fopen("log.txt", "a");

    if (log_file == NULL)
    {
        perror("Error al abrir el archivo de log");
        return;
    }
    log_function_call(func_name, type, ptr, size);
}

/**
 * @brief Registra en un archivo de log una llamada a una función relacionada con la memoria.
 *
 * Esta función escribe información detallada sobre una llamada a funciones de asignación
 * o liberación de memoria, incluyendo la hora del evento, el tipo de asignación, el puntero afectado
 * y el tamaño involucrado.
 *
 * @note La función asume que el archivo de log `log_file` está correctamente inicializado y abierto.
 */
void log_function_call(const char* func_name, alloc_type type, void* ptr, size_t size)
{
    // Obtener la hora actual para incluirla en el log
    time_t now = time(NULL);
    struct tm* t = localtime(&now);

    // Escribir el nombre de la función y la fecha/hora en el archivo
    fprintf(log_file, "[%04d-%02d-%02d %02d:%02d:%02d] Llamada a %s\n", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
            t->tm_hour, t->tm_min, t->tm_sec, func_name);

    // Agregar información adicional sobre la asignación
    fprintf(log_file, "Tipo de asignación: %d, Puntero: %p, Tamaño: %zu\n", type, ptr, size);
}

/**
 * @brief Cierra el archivo de log.
 */
void log_close()
{
    fclose(log_file);
}

/**
 * @brief Calcula el nivel de fragmentación externa.
 * @return Proporción de memoria libre no utilizable.
 */
double external_frag()
{
    t_block current = base;
    double total_free = 0;
    double total = 0;
    size_t maxs = 0;
    while (current)
    {
        if (!current->free && maxs < current->size)
            maxs = current->size;
        current = current->next;
    }
    current = base;
    while (current)
    {
        if (current->free && current->size < maxs)
        {
            total_free += current->size;
        }
        total += current->size;
        current = current->next;
    }
    double frag = (total_free / total) * 100;
    return frag;
}
