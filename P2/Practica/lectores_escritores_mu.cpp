// ************************************************************************* //
/**
 * @file lectores_escritores_mu.cpp
 * @brief Fichero con el ejercicio de la práctica 3 de SCD : Implementación de Lectores-Escritores con monitores
 * Consta de 5 partes:
 * 
 * @section sec_global_vars 1. Variables Globales
 * Contiene las variables globales del programa:  
 * @ref NUM_ESCRITORES "NUM_ESCRITORES"  
 * @ref NUM_LECTORES "NUM_LECTORES"   
 * 
 * @section sec_class_lec_esc 2. Clase LecturaEscritura
 * Contiene el monitor Lectura Escritura que proporcionará las variables y funciones necesarias para acceder a la lectura-escritura garantizando la exclusión mutua  
 * @ref LecturaEscritura "Clase Lectura Escritura"  
 * 
 * @section sec_func_hebras 3. Funciones a Ejecutar por cada Hebra.
 * Contiene las funciones que ejecutará cada actor:  
 * Función que ejecutará cada lector: @ref funcion_hebra_lectora "funcion_hebra_lectora"
 * Función que ejecutará cada escritor: @ref funcion_hebra_escritora "funcion_hebra_escritora"
 * 
 * @section sec_test 4. Test para comprobar que la ejecución se ha dado correctamente
 * @ref test_lectores_escritores "test_lectores_escritores"  
 * 
 * @section sec_main 5. Main
 * Contiene el main del programa  
 * @ref main "main"  
 * 
 * @author Marco Girela Vida
 * @date 11-11-2023
*/
// ************************************************************************* //
/**
 * @brief Dependencias: scd.h, isotream, thread, cassert, chrono
*/
#include "scd.h"
#include <iostream>
#include <thread>
#include <cassert>
#include <chrono>

using namespace std;
using namespace scd;

// ************************************************************************* //
/**
 * @brief Indica el número de lectores que tendrá el programa
 * @pre 0 <= NUM_LECTORES
*/
constexpr unsigned short int NUM_LECTORES = 10;

/**
 * @brief Indica el número de escritores que tendrá el programa
 * @pre 0 <= NUM_ESCRITORES
*/
constexpr unsigned short int NUM_ESCRITORES = 10;

// ************************************************************************* //
/**
 * @brief Representa el punto de escritura-lectura
*/
class LecturaEscritura : public HoareMonitor
{
private:
    /**
     * @brief Número de lectores que hay leyendo actualmente
     * @pre 0 <= num_lectores_leyendo <= num_lectores
    */
    unsigned short int num_lectores_leyendo;

    /**
     * @brief Indica si hay algún escritor escribiendo
    */
    bool escritor_escribiendo;
    
    /**
     * @brief Cola de los lectores que están esperando para poder leer
    */
    CondVar esperando_lectura;

    /**
     * @brief Cola de los escritores que están esperando para poder escribir
    */
    CondVar esperando_escritura;

public:
    /**
     * @brief Constructor por defecto
     * @post Genera una instancia de la clase LecturaEscritura con ningún escritor escribiendo ni ningún lector leyendo
     * @return Objeto de la clase mostrador 
    */
    LecturaEscritura();

    /**
     * @brief Inicializa el proceso de lectura de todos los lectores en espera si no hay ningún escritor escribiend
    */
    void ini_lectura();

    /**
     * @brief Finaliza el proceso de lectura de un lector.
    */
    void fin_lectura();

    /**
     * @brief Inicializa el proceso de escritura de un escritor siempre que no haya alguien leyendo o escribiendo
    */
    void ini_escritura();

    /**
     * @brief Finaliza el proceso de escritura de un escritor
    */
    void fin_escritura();
};

LecturaEscritura::LecturaEscritura()
{
    num_lectores_leyendo = 0;
    escritor_escribiendo = false;
    esperando_escritura = newCondVar();
    esperando_lectura = newCondVar();
}

void LecturaEscritura::ini_lectura()
{
    if (escritor_escribiendo)
        esperando_lectura.wait();
    
    num_lectores_leyendo++;
    esperando_lectura.signal();
}

void LecturaEscritura::fin_lectura()
{
    num_lectores_leyendo--;
    if (num_lectores_leyendo == 0)
        esperando_escritura.signal();
}

void LecturaEscritura::ini_escritura()
{
    if (escritor_escribiendo || num_lectores_leyendo)
        esperando_escritura.wait();
    escritor_escribiendo = true;
}

void LecturaEscritura::fin_escritura()
{
    escritor_escribiendo = false;
    if (!esperando_lectura.empty())
        esperando_lectura.signal();
    else
        esperando_escritura.signal();
}

// ************************************************************************* //
/**
 * @brief Función que ejecuta la hebra de cada lector
 * @param lectura_escritura monitor de lectura_escritura de donde leerá
*/
void funcion_hebra_lectora(MRef<LecturaEscritura> lectura_escritura, const unsigned int num_lector)
{
    while (true)
    {
        chrono::milliseconds duracion_lectura( aleatorio<100,300>() );
        chrono::milliseconds duracion_lectura_resto( aleatorio<200,600>() );

        lectura_escritura->ini_lectura();
        cout << "Lector leyendo por: " << duracion_lectura.count() << endl;
        this_thread::sleep_for(duracion_lectura);
        cout << "Lector " << num_lector << " ha finalizado de leer" << endl;
        lectura_escritura->fin_lectura();

        this_thread::sleep_for(duracion_lectura_resto);
    }
}

/**
 * @brief Función que ejecuta la hebra de cada escritor
 * @param lectura_escritura monitor de lectura_escritura donde escribirá
*/
void funcion_hebra_escritora(MRef<LecturaEscritura> lectura_escritura, const unsigned short int num_escritor)
{
    while (true)
    {
        chrono::milliseconds duracion_escritura( aleatorio<100,300>() );
        chrono::milliseconds duracion_escritura_resto( aleatorio<100,300>() );

        lectura_escritura->ini_escritura();
        cout << "                   Escritor " << num_escritor << " escribiendo por: " << duracion_escritura.count() << endl;
        this_thread::sleep_for(duracion_escritura);
        cout << "                   Escitor " << num_escritor << " ha finalizado de escribir " << endl;
        lectura_escritura->fin_escritura();

        this_thread::sleep_for(duracion_escritura_resto);
    }
}

// ************************************************************************* //
/**
 * @brief Testea que cada lector ¿?
 * @post ¿?
*/
void test_lectores_escritores(){}

// ************************************************************************* //
/**
 * @brief main
 * @note Se crean las hebras, se ejecutan las mismas y se muestra en pantalla el resultado de los test correspondientes si los hubiese
*/
int main()
{
   MRef<LecturaEscritura> lectura_escritura = Create<LecturaEscritura>();

   //---------------------------//

   cerr << "-----------------------------------------------------------------" << endl;
   cout << "Problema de los lectores-escritores con monitores." << endl;
   cout << "------------------------------------------------------------------" << endl;
   cout << flush ;

   //---------------------------//

   thread hebras_escritores[NUM_ESCRITORES];
   thread hebras_lectores[NUM_LECTORES];

   for (size_t i = 0; i < NUM_ESCRITORES; i++)
      hebras_escritores[i] = thread(funcion_hebra_escritora, lectura_escritura, i);
   for (size_t i = 0; i < NUM_LECTORES; i++)
      hebras_lectores[i] = thread(funcion_hebra_lectora, lectura_escritura, i);

   //---//
   
   for (size_t i = 0; i < NUM_ESCRITORES; i++)
      hebras_escritores[i].join();
   
   for (size_t i = 0; i < NUM_LECTORES; i++)
      hebras_lectores[i].join();
   
   //---------------------------//

   //test_lectores_escritores();

   return 0;
}