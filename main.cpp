/*
 * Programa demostrativo de manipulaciónprocesamiento de imágenes BMP en C++ usando Qt.
 *
 * Descripción:
 * Este programa realiza las siguientes tareas:
 * 1. Carga una imagen BMP desde un archivo (formato RGB sin usar estructuras ni STL).
 * 2. Modifica los valores RGB de los píxeles asignando un degradado artificial basado en su posición.
 * 3. Exporta la imagen modificada a un nuevo archivo BMP.
 * 4. Carga un archivo de texto que contiene una semilla (offset) y los resultados del enmascaramiento
 *    aplicados a una versión transformada de la imagen, en forma de tripletas RGB.
 * 5. Muestra en consola los valores cargados desde el archivo de enmascaramiento.
 * 6. Gestiona la memoria dinámicamente, liberando los recursos utilizados.
 *
 * Entradas:
 * - Archivo de imagen BMP de entrada ("I_O.bmp").
 * - Archivo de salida para guardar la imagen modificada ("I_D.bmp").
 * - Archivo de texto ("M1.txt") que contiene:
 *     • Una línea con la semilla inicial (offset).
 *     • Varias líneas con tripletas RGB resultantes del enmascaramiento.
 *
 * Salidas:
 * - Imagen BMP modificada ("I_D.bmp").
 * - Datos RGB leídos desde el archivo de enmascaramiento impresos por consola.
 *
 * Requiere:
 * - Librerías Qt para manejo de imágenes (QImage, QString).
 * - No utiliza estructuras ni STL. Solo arreglos dinámicos y memoria básica de C++.
 *
 * Autores: Augusto Salazar Y Aníbal Guerra
 * Fecha: 06/04/2025
 * Asistencia de ChatGPT para mejorar la forma y presentación del código fuente
 */

#include <fstream>
#include <iostream>
#include <QCoreApplication>
#include <QImage>
#include <algorithm>
#include <memory>
#include <cstdlib>

using namespace std;
unsigned char* loadPixels(QString input, int &width, int &height);
bool exportImage(unsigned char* pixelData, int width,int height, QString archivoSalida);
unsigned int* loadSeedMasking(const char* nombreArchivo, int &seed, int &n_pixels);
void aplicarXOREntreImagenes();
void revertirEnmascaramientoM2();
void revertirRotacion3Bits();
void reconstruirImagenOriginal();

int main() {

    // Definición de rutas de archivo de entrada (imagen original) y salida (imagen modificada)
    QString archivoEntrada = "I_O.bmp";
    QString archivoSalida = "I_D.bmp";

    // Variables para almacenar las dimensiones de la imagen
    int height = 0;
    int width = 0;

    // Carga la imagen BMP en memoria dinámica y obtiene ancho y alto
    unsigned char *pixelData = loadPixels(archivoEntrada, width, height);

    // Simula una modificación de la imagen asignando valores RGB incrementales
    // (Esto es solo un ejemplo de manipulación artificial)
    for (int i = 0; i < width * height * 3; i += 3) {
        pixelData[i] = i;     // Canal rojo
        pixelData[i + 1] = i; // Canal verde
        pixelData[i + 2] = i; // Canal azul
    }

    // Exporta la imagen modificada a un nuevo archivo BMP
    bool exportI = exportImage(pixelData, width, height, archivoSalida);

    // Muestra si la exportación fue exitosa (true o false)
    cout << exportI << endl;

    // Libera la memoria usada para los píxeles
    delete[] pixelData;
    pixelData = nullptr;

    // Variables para almacenar la semilla y el número de píxeles leídos del archivo de enmascaramiento
    int seed = 0;
    int n_pixels = 0;

    // Carga los datos de enmascaramiento desde un archivo .txt (semilla + valores RGB)
    unsigned int *maskingData = loadSeedMasking("M1.txt", seed, n_pixels);

    // Muestra en consola los primeros valores RGB leídos desde el archivo de enmascaramiento
    for (int i = 0; i < n_pixels * 3; i += 3) {
        cout << "Pixel " << i / 3 << ": ("
             << maskingData[i] << ", "
             << maskingData[i + 1] << ", "
             << maskingData[i + 2] << ")" << endl;
    }

    // Libera la memoria usada para los datos de enmascaramiento
    if (maskingData != nullptr){
        delete[] maskingData;
        maskingData = nullptr;
    }

    // Llamada a la nueva función para aplicar XOR entre dos imágenes
    aplicarXOREntreImagenes();
    revertirEnmascaramientoM2();
    revertirRotacion3Bits();
    reconstruirImagenOriginal();

    return 0; // Fin del programa
}

unsigned char* loadPixels(QString input, int &width, int &height){
    /*
 * @brief Carga una imagen BMP desde un archivo y extrae los datos de píxeles en formato RGB.
 *
 * Esta función utiliza la clase QImage de Qt para abrir una imagen en formato BMP, convertirla al
 * formato RGB888 (24 bits: 8 bits por canal), y copiar sus datos de píxeles a un arreglo dinámico
 * de tipo unsigned char. El arreglo contendrá los valores de los canales Rojo, Verde y Azul (R, G, B)
 * de cada píxel de la imagen, sin rellenos (padding).
 *
 * @param input Ruta del archivo de imagen BMP a cargar (tipo QString).
 * @param width Parámetro de salida que contendrá el ancho de la imagen cargada (en píxeles).
 * @param height Parámetro de salida que contendrá la altura de la imagen cargada (en píxeles).
 * @return Puntero a un arreglo dinámico que contiene los datos de los píxeles en formato RGB.
 *         Devuelve nullptr si la imagen no pudo cargarse.
 *
 * @note Es responsabilidad del usuario liberar la memoria asignada al arreglo devuelto usando `delete[]`.
 */

    // Cargar la imagen BMP desde el archivo especificado (usando Qt)
    QImage imagen(input);

    // Verifica si la imagen fue cargada correctamente
    if (imagen.isNull()) {
        cout << "Error: No se pudo cargar la imagen BMP." << std::endl;
        return nullptr; // Retorna un puntero nulo si la carga falló
    }

    // Convierte la imagen al formato RGB888 (3 canales de 8 bits sin transparencia)
    imagen = imagen.convertToFormat(QImage::Format_RGB888);

    // Obtiene el ancho y el alto de la imagen cargada
    width = imagen.width();
    height = imagen.height();

    // Calcula el tamaño total de datos (3 bytes por píxel: R, G, B)
    int dataSize = width * height * 3;

    // Reserva memoria dinámica para almacenar los valores RGB de cada píxel
    unsigned char* pixelData = new unsigned char[dataSize];

    // Copia cada línea de píxeles de la imagen Qt a nuestro arreglo lineal
    for (int y = 0; y < height; ++y) {
        const uchar* srcLine = imagen.scanLine(y);              // Línea original de la imagen con posible padding
        unsigned char* dstLine = pixelData + y * width * 3;     // Línea destino en el arreglo lineal sin padding
        memcpy(dstLine, srcLine, width * 3);                    // Copia los píxeles RGB de esa línea (sin padding)
    }

    // Retorna el puntero al arreglo de datos de píxeles cargado en memoria
    return pixelData;
}

bool exportImage(unsigned char* pixelData, int width,int height, QString archivoSalida){
    /*
 * @brief Exporta una imagen en formato BMP a partir de un arreglo de píxeles en formato RGB.
 *
 * Esta función crea una imagen de tipo QImage utilizando los datos contenidos en el arreglo dinámico
 * `pixelData`, que debe representar una imagen en formato RGB888 (3 bytes por píxel, sin padding).
 * A continuación, copia los datos línea por línea a la imagen de salida y guarda el archivo resultante
 * en formato BMP en la ruta especificada.
 *
 * @param pixelData Puntero a un arreglo de bytes que contiene los datos RGB de la imagen a exportar.
 *                  El tamaño debe ser igual a width * height * 3 bytes.
 * @param width Ancho de la imagen en píxeles.
 * @param height Alto de la imagen en píxeles.
 * @param archivoSalida Ruta y nombre del archivo de salida en el que se guardará la imagen BMP (QString).
 *
 * @return true si la imagen se guardó exitosamente; false si ocurrió un error durante el proceso.
 *
 * @note La función no libera la memoria del arreglo pixelData; esta responsabilidad recae en el usuario.
 */

    // Crear una nueva imagen de salida con el mismo tamaño que la original
    // usando el formato RGB888 (3 bytes por píxel, sin canal alfa)
    QImage outputImage(width, height, QImage::Format_RGB888);

    // Copiar los datos de píxeles desde el buffer al objeto QImage
    for (int y = 0; y < height; ++y) {
        // outputImage.scanLine(y) devuelve un puntero a la línea y-ésima de píxeles en la imagen
        // pixelData + y * width * 3 apunta al inicio de la línea y-ésima en el buffer (sin padding)
        // width * 3 son los bytes a copiar (3 por píxel)
        memcpy(outputImage.scanLine(y), pixelData + y * width * 3, width * 3);
    }

    // Guardar la imagen en disco como archivo BMP
    if (!outputImage.save(archivoSalida, "BMP")) {
        // Si hubo un error al guardar, mostrar mensaje de error
        cout << "Error: No se pudo guardar la imagen BMP modificada.";
        return false; // Indica que la operación falló
    } else {
        // Si la imagen fue guardada correctamente, mostrar mensaje de éxito
        cout << "Imagen BMP modificada guardada como " << archivoSalida.toStdString() << endl;
        return true; // Indica éxito
    }
}

unsigned int* loadSeedMasking(const char* nombreArchivo, int &seed, int &n_pixels) {
    // Inicializar n_pixels a 0
    n_pixels = 0;
    /*
 * @brief Carga la semilla y los resultados del enmascaramiento desde un archivo de texto.
 *
 * Esta función abre un archivo de texto que contiene una semilla en la primera línea y,
 * a continuación, una lista de valores RGB resultantes del proceso de enmascaramiento.
 * Primero cuenta cuántos tripletes de píxeles hay, luego reserva memoria dinámica
 * y finalmente carga los valores en un arreglo de enteros.
 *
 * @param nombreArchivo Ruta del archivo de texto que contiene la semilla y los valores RGB.
 * @param seed Variable de referencia donde se almacenará el valor entero de la semilla.
 * @param n_pixels Variable de referencia donde se almacenará la cantidad de píxeles leídos
 *                 (equivalente al número de líneas después de la semilla).
 *
 * @return Puntero a un arreglo dinámico de enteros que contiene los valores RGB
 *         en orden secuencial (R, G, B, R, G, B, ...). Devuelve nullptr si ocurre un error al abrir el archivo.
 *
 * @note Es responsabilidad del usuario liberar la memoria reservada con delete[].
 */

    // Abrir el archivo que contiene la semilla y los valores RGB
    ifstream archivo(nombreArchivo);
    if (!archivo.is_open()) {
        cerr << "No se pudo abrir el archivo: " << nombreArchivo << endl;
        return nullptr;
    }

    // Leer la semilla
    if (!(archivo >> seed)) {
        cerr << "Error leyendo la semilla del archivo: " << nombreArchivo << endl;
        archivo.close();
        return nullptr;
    }

    // Contar píxeles (tripletas RGB)
    int r, g, b;
    while (archivo >> r >> g >> b) {
        n_pixels++;
    }

    // Verificar si se leyeron píxeles correctamente
    if (n_pixels == 0) {
        cerr << "Advertencia: No se encontraron píxeles en el archivo: " << nombreArchivo << endl;
    }

    // Volver al inicio para leer los datos
    archivo.clear();
    archivo.seekg(0, ios::beg);
    if (!archivo) {
        cerr << "Error al reiniciar la lectura del archivo: " << nombreArchivo << endl;
        archivo.close();
        return nullptr;
    }

    // Leer semilla nuevamente
    archivo >> seed;

    // Reservar memoria para los datos RGB
    unsigned int* RGB = new unsigned int[n_pixels * 3];
    if (!RGB) {
        cerr << "Error al asignar memoria para los datos RGB" << endl;
        archivo.close();
        return nullptr;
    }

    // Leer los datos RGB
    for (int i = 0; i < n_pixels * 3; i += 3) {
        if (!(archivo >> r >> g >> b)) {
            cerr << "Error leyendo datos RGB en el pixel " << i/3 << endl;
            delete[] RGB;
            archivo.close();
            return nullptr;
        }
        RGB[i] = r;
        RGB[i + 1] = g;
        RGB[i + 2] = b;
    }

    archivo.close();

    // Mensaje de depuración
    cout << "Semilla: " << seed << endl;
    cout << "Cantidad de píxeles leídos correctamente: " << n_pixels << endl;

    return RGB;
}

/**
 * @brief Aplica XOR seguro entre imágenes con validación exhaustiva
 */
void aplicarXOREntreImagenes() {
    // 1. Cargar las imágenes
    int width_P3, height_P3;
    unsigned char* P3 = loadPixels("P3.bmp", width_P3, height_P3);
    if (!P3) {
        cerr << "Error: No se pudo cargar P3.bmp" << endl;
        return;
    }

    int width_IM, height_IM;
    unsigned char* IM = loadPixels("I_M.bmp", width_IM, height_IM);
    if (!IM) {
        cerr << "Error: No se pudo cargar I_M.bmp" << endl;
        delete[] P3;
        return;
    }

    // 2. Verificar dimensiones
    if (width_P3 != width_IM || height_P3 != height_IM) {
        cerr << "Error: Las imágenes deben tener las mismas dimensiones\n"
             << "P3: " << width_P3 << "x" << height_P3 << "\n"
             << "I_M: " << width_IM << "x" << height_IM << endl;
        delete[] P3;
        delete[] IM;
        return;
    }

    // 3. Aplicar XOR píxel por píxel
    const int totalBytes = width_P3 * height_P3 * 3;
    for (int i = 0; i < totalBytes; i++) {
        P3[i] = P3[i] ^ IM[i];
    }

    // 4. Guardar resultado
    if (!exportImage(P3, width_P3, height_P3, "P2_reconstruida.bmp")) {
        cerr << "Error al guardar P2_reconstruida.bmp" << endl;
    } else {
        cout << "P2_reconstruida.bmp creada exitosamente" << endl;
    }

    // 5. Liberar memoria
    delete[] P3;
    delete[] IM;
}

void revertirEnmascaramientoM2() {
    // 1. Cargar P2_reconstruida
    int width_P2, height_P2;
    unsigned char* P2 = loadPixels("P2_reconstruida.bmp", width_P2, height_P2);
    if (!P2) {
        cerr << "Error: No se pudo cargar P2_reconstruida.bmp" << endl;
        return;
    }

    // 2. Cargar máscara M
    int width_M, height_M;
    unsigned char* M = loadPixels("M.bmp", width_M, height_M);
    if (!M) {
        cerr << "Error: No se pudo cargar M.bmp" << endl;
        delete[] P2;
        return;
    }

    // 3. Cargar datos de enmascaramiento
    int seed, n_pixels;
    unsigned int* M2_data = loadSeedMasking("M2.txt", seed, n_pixels);
    if (!M2_data) {
        cerr << "Error: No se pudo cargar M2.txt" << endl;
        delete[] P2;
        delete[] M;
        return;
    }

    // 4. Revertir el enmascaramiento
    for (int i = 0; i < n_pixels; i++) {
        // Calcular posición en la imagen grande usando la semilla
        int pos = (seed + i) % (width_P2 * height_P2);

        // Calcular posición en la máscara (que es más pequeña)
        int pos_M = i % (width_M * height_M);

        // Revertir la operación para cada canal (R, G, B)
        for (int c = 0; c < 3; c++) {
            // Asumiendo que el enmascaramiento fue: M2 = P2 XOR M
            P2[pos * 3 + c] = M2_data[i * 3 + c] ^ M[pos_M * 3 + c];
        }
    }

    // 5. Guardar resultado
    if (!exportImage(P2, width_P2, height_P2, "P2_sin_enmascaramiento.bmp")) {
        cerr << "Error al guardar P2_sin_enmascaramiento.bmp" << endl;
    } else {
        cout << "P2_sin_enmascaramiento.bmp creada exitosamente" << endl;
    }

    // 6. Liberar memoria
    delete[] P2;
    delete[] M;
    delete[] M2_data;
}

void revertirRotacion3Bits() {
    int width, height;
    unsigned char* P2 = loadPixels("P2_sin_enmascaramiento.bmp", width, height);
    if (!P2) return;

    // Rotar 3 bits a la izquierda (reversa de rotación original a derecha)
    for (int i = 0; i < width * height * 3; i++) {
        P2[i] = (P2[i] << 3) | (P2[i] >> 5);
    }

    exportImage(P2, width, height, "P1_reconstruida.bmp");
    delete[] P2;
}

void reconstruirImagenOriginal() {
    // 1. Cargar P1 reconstruida
    int width_P1, height_P1;
    unsigned char* P1 = loadPixels("P1_reconstruida.bmp", width_P1, height_P1);
    if (!P1) return;

    // 2. Cargar IM
    int width_IM, height_IM;
    unsigned char* IM = loadPixels("I_M.bmp", width_IM, height_IM);
    if (!IM) {
        delete[] P1;
        return;
    }

    // 3. Verificar dimensiones
    if (width_P1 != width_IM || height_P1 != height_IM) {
        cerr << "Error: Dimensiones no coinciden (P1: "
             << width_P1 << "x" << height_P1
             << ", IM: " << width_IM << "x" << height_IM << ")" << endl;
        delete[] P1;
        delete[] IM;
        return;
    }

    // 4. Aplicar XOR con verificación de límites
    const int totalPixels = width_P1 * height_P1 * 3;
    for (int i = 0; i < totalPixels; i++) {
        // Verificación adicional (aunque ya está garantizado por loadPixels)
        if (i >= 0 && i < totalPixels) {
            P1[i] = static_cast<unsigned char>(P1[i] ^ IM[i]);
        }
    }

    // 5. Guardar imagen original reconstruida
    exportImage(P1, width_P1, height_P1, "IO_reconstruida.bmp");

    delete[] P1;
    delete[] IM;
}










