#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define ARCHIVO_DATOS "biblioteca.dat"
#define ARCHIVO_CSV   "catalogo.csv"

#define TAM_ISBN   14   /* 13 + nul */
#define TAM_TITULO 128
#define TAM_AUTOR   80

/* ======== Estructuras ========= */
typedef struct {
    char isbn[TAM_ISBN];
    char titulo[TAM_TITULO];
    char autor[TAM_AUTOR];
    int  anio;
} Libro;

/* ======== Variables globales ========= */
static Libro *catalogo      = NULL; /* arreglo dinámico */
static size_t cantidad      = 0;    /* libros actuales   */
static size_t capacidad     = 0;    /* tamaño del buffer */

/* ======== Prototipos ========= */
void cargarDatos(void);
void guardarDatos(void);
void exportarCSV(void);

void menu(void);
void agregarLibro(void);
void listarLibros(void);
void buscarLibro(void);
void editarLibro(void);
void eliminarLibro(void);
void estadisticas(void);

/* utilidades */
int  buscarISBN(const char *isbn);
void ordenarPorTitulo(void);
void imprimirLibro(const Libro *l, int idx);
void asegurarCapacidad(void);
char *strtolower(char *s);
int  compararTitulo(const void *a, const void *b);

/* ===================================================== */
int main(void)
{
    cargarDatos();
    menu();
    guardarDatos();
    free(catalogo);
    return 0;
}

/* ---------------- Persistencia ---------------- */
void cargarDatos(void)
{
    FILE *fp = fopen(ARCHIVO_DATOS, "rb");
    if (!fp) {
        /* primera vez: reservar capacidad inicial */
        capacidad = 100;
        catalogo = calloc(capacidad, sizeof(Libro));
        return;
    }
    fread(&cantidad, sizeof(size_t), 1, fp);
    capacidad = (cantidad > 0) ? cantidad : 100;
    catalogo = calloc(capacidad, sizeof(Libro));
    fread(catalogo, sizeof(Libro), cantidad, fp);
    fclose(fp);
}

void guardarDatos(void)
{
    FILE *fp = fopen(ARCHIVO_DATOS, "wb");
    if (!fp) {
        perror("Error al guardar datos");
        return;
    }
    fwrite(&cantidad, sizeof(size_t), 1, fp);
    fwrite(catalogo, sizeof(Libro), cantidad, fp);
    fclose(fp);
    printf("\nDatos guardados en '%s'.\n", ARCHIVO_DATOS);
}

void exportarCSV(void)
{
    FILE *fp = fopen(ARCHIVO_CSV, "w");
    if (!fp) {
        perror("Error al exportar CSV");
        return;
    }
    fprintf(fp, "ISBN,Titulo,Autor,Anio\n");
    for (size_t i = 0; i < cantidad; ++i) {
        fprintf(fp, "%s,\"%s\",\"%s\",%d\n",
                catalogo[i].isbn,
                catalogo[i].titulo,
                catalogo[i].autor,
                catalogo[i].anio);
    }
    fclose(fp);
    printf("\nCatálogo exportado a '%s'.\n", ARCHIVO_CSV);
}

/* ---------------- Utilidades internas ---------------- */
void asegurarCapacidad(void)
{
    if (cantidad < capacidad) return;
    capacidad *= 2;
    catalogo = realloc(catalogo, capacidad * sizeof(Libro));
}

int buscarISBN(const char *isbn)
{
    for (size_t i = 0; i < cantidad; ++i) {
        if (strcmp(catalogo[i].isbn, isbn) == 0) return (int)i;
    }
    return -1;
}

char *strtolower(char *s)
{
    for (char *p = s; *p; ++p) *p = (char)tolower((unsigned char)*p);
    return s;
}

int compararTitulo(const void *a, const void *b)
{
    const Libro *la = a, *lb = b;
    return strcasecmp(la->titulo, lb->titulo);
}

void ordenarPorTitulo(void)
{
    qsort(catalogo, cantidad, sizeof(Libro), compararTitulo);
}

void imprimirLibro(const Libro *l, int idx)
{
    printf("[%03d] ISBN:%s | '%s' - %s (%d)\n",
           idx, l->isbn, l->titulo, l->autor, l->anio);
}

/* ---------------- Menú ---------------- */
void menu(void)
{
    int opcion;
    do {
        printf("\n===== SISTEMA AVANZADO DE BIBLIOTECA =====\n");
        printf("1. Agregar libro\n");
        printf("2. Listar libros\n");
        printf("3. Buscar libro\n");
        printf("4. Editar libro\n");
        printf("5. Eliminar libro\n");
        printf("6. Exportar catálogo a CSV\n");
        printf("7. Estadísticas\n");
        printf("8. Guardar y salir\n");
        printf("Seleccione una opción: ");
        if (scanf("%d", &opcion) != 1) {
            fprintf(stderr, "Entrada inválida.\n");
            while (getchar() != '\n');
            continue;
        }
        switch (opcion) {
            case 1: agregarLibro(); break;
            case 2: listarLibros(); break;
            case 3: buscarLibro(); break;
            case 4: editarLibro(); break;
            case 5: eliminarLibro(); break;
            case 6: exportarCSV(); break;
            case 7: estadisticas(); break;
            case 8: puts("Saliendo..."); break;
            default: puts("Opción no válida.");
        }
    } while (opcion != 8);
}

/* ------------ Operaciones CRUD ------------ */
void agregarLibro(void)
{
    asegurarCapacidad();

    Libro nuevo;
    printf("\n— Agregar nuevo libro —\n");
    printf("ISBN (13 dígitos): ");
    scanf("%13s", nuevo.isbn);
    getchar();

    if (buscarISBN(nuevo.isbn) != -1) {
        puts("ISBN duplicado. Operación cancelada.");
        return;
    }

    printf("Título: ");
    fgets(nuevo.titulo, TAM_TITULO, stdin);
    nuevo.titulo[strcspn(nuevo.titulo, "\n")] = '\0';

    printf("Autor: ");
    fgets(nuevo.autor, TAM_AUTOR, stdin);
    nuevo.autor[strcspn(nuevo.autor, "\n")] = '\0';

    printf("Año de publicación: ");
    scanf("%d", &nuevo.anio);

    catalogo[cantidad++] = nuevo;
    ordenarPorTitulo();
    puts("Libro agregado exitosamente.");
}

void listarLibros(void)
{
    if (cantidad == 0) {
        puts("\nCatálogo vacío.");
        return;
    }
    puts("\n— Catálogo —");
    for (size_t i = 0; i < cantidad; ++i) imprimirLibro(&catalogo[i], (int)i);
}

void buscarLibro(void)
{
    if (cantidad == 0) {
        puts("\nCatálogo vacío.");
        return;
    }
    printf("\nBuscar por:\n  1. ISBN\n  2. Título\n  3. Autor\n> ");
    int op; scanf("%d", &op); getchar();

    char buffer[TAM_TITULO];
    bool encontrado = false;

    if (op == 1) {
        printf("ISBN: "); scanf("%13s", buffer);
        int idx = buscarISBN(buffer);
        if (idx != -1) { imprimirLibro(&catalogo[idx], idx); } else puts("No encontrado.");
        return;
    }

    printf("Ingrese término de búsqueda: ");
    fgets(buffer, sizeof(buffer), stdin);
    buffer[strcspn(buffer, "\n")] = '\0';
    char criterio[TAM_TITULO]; strncpy(criterio, buffer, sizeof(criterio)); strtolower(criterio);

    for (size_t i = 0; i < cantidad; ++i) {
        char campo[TAM_TITULO];
        if (op == 2) {
            strncpy(campo, catalogo[i].titulo, sizeof(campo));
        } else if (op == 3) {
            strncpy(campo, catalogo[i].autor, sizeof(campo));
        } else { puts("Opción inválida."); return; }
        strtolower(campo);
        if (strstr(campo, criterio)) {
            imprimirLibro(&catalogo[i], (int)i);
            encontrado = true;
        }
    }
    if (!encontrado) puts("Sin coincidencias.");
}

void editarLibro(void)
{
    if (cantidad == 0) { puts("Catálogo vacío."); return; }
    char isbn[TAM_ISBN];
    printf("\nISBN del libro a editar: ");
    scanf("%13s", isbn); getchar();
    int idx = buscarISBN(isbn);
    if (idx == -1) { puts("No encontrado."); return; }

    Libro *l = &catalogo[idx];
    puts("Libro seleccionado:"); imprimirLibro(l, idx);

    char buffer[TAM_TITULO];
    printf("Nuevo título (enter p/ mantener): ");
    fgets(buffer, TAM_TITULO, stdin);
    if (buffer[0] != '\n') {
        buffer[strcspn(buffer, "\n")] = '\0'; strncpy(l->titulo, buffer, TAM_TITULO);
    }

    printf("Nuevo autor (enter p/ mantener): ");
    fgets(buffer, TAM_AUTOR, stdin);
    if (buffer[0] != '\n') {
        buffer[strcspn(buffer, "\n")] = '\0'; strncpy(l->autor, buffer, TAM_AUTOR);
    }

    printf("Nuevo año (0 p/ mantener): ");
    int nuevoAnio; scanf("%d", &nuevoAnio); getchar();
    if (nuevoAnio != 0) l->anio = nuevoAnio;

    ordenarPorTitulo();
    puts("Libro actualizado.");
}

void eliminarLibro(void)
{
    if (cantidad == 0) { puts("Catálogo vacío."); return; }
    char isbn[TAM_ISBN];
    printf("ISBN del libro a eliminar: ");
    scanf("%13s", isbn);
    int idx = buscarISBN(isbn);
    if (idx == -1) { puts("No encontrado."); return; }

    /* desplazamiento → borrado físico */
    memmove(&catalogo[idx], &catalogo[idx + 1], (cantidad - idx - 1) * sizeof(Libro));
    --cantidad;
    puts("Libro eliminado.");
}

/* -------------- Estadísticas ---------------- */
void estadisticas(void)
{
    printf("\nTotal de libros: %zu\n", cantidad);

    /* contar autores más frecuentes (simple, O(n²) para ≤ 1000) */
    if (cantidad == 0) return;
    typedef struct { char autor[TAM_AUTOR]; int count; } Pair;
    Pair top[5] = {0};

    for (size_t i = 0; i < cantidad; ++i) {
        bool sumado = false;
        for (int j = 0; j < 5; ++j) {
            if (top[j].count && strcmp(top[j].autor, catalogo[i].autor) == 0) {
                top[j].count++; sumado = true; break;
            }
        }
        if (!sumado) {
            /* busca slot vacío o reemplaza al menor */
            int minIdx = 0;
            for (int j = 1; j < 5; ++j) if (top[j].count < top[minIdx].count) minIdx = j;
            if (top[minIdx].count < 1) {
                strncpy(top[minIdx].autor, catalogo[i].autor, TAM_AUTOR);
                top[minIdx].count = 1;
            }
        }
    }

    puts("Autor(es) más frecuentes:");
    for (int i = 0; i < 5; ++i) {
        if (top[i].count)
            printf("  - %s (%d libro%s)\n", top[i].autor, top[i].count, top[i].count==1?"":"s");
    }
}
