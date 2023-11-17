/**
 * @file prodcons_mu_fifo.cpp
 * @brief Solución al problema del productor-consumidor utilizando la estrategia FIFO.
 *
 * Este archivo contiene la implementación de un monitor en C++11 con semántica SU,
 * para el problema del productor/consumidor, con un único productor y consumidor. Opción FIFO.
 *
 * @author Marco Girela Vida
 * @date 11-11-2023
 */

#include <iostream>
#include <iomanip>
#include <cassert>
#include <random>
#include <thread>
#include "scd.h"

using namespace std ;
using namespace scd ;

/**
 * @brief Número de elementos a producir/consumir.
 */
constexpr int NUM_ITEMS = 20000;

/**
 * @brief Número de hilos productores.
 */
constexpr int NUM_PROD = 2000;

/**
 * @brief Número de hilos consumidores.
 */
constexpr int NUM_CONS = 2000;

const unsigned int CANTIDAD_DATOS_A_PRODUCIR = NUM_ITEMS/NUM_PROD;
const unsigned int CANTIDAD_DATOS_A_CONSUMIR = NUM_ITEMS/NUM_CONS;
/**
 * @brief Siguiente valor a devolver en 'producir_dato'.
 */
int siguiente_dato = 0 ;

/**
 * @brief Tiempo mínimo de espera en sleep_for.
 */
constexpr int min_ms = 5;

/**
 * @brief Tiempo máximo de espera en sleep_for.
 */
constexpr int max_ms = 20;

/**
 * @brief Mutex para la escritura en pantalla.
 */
mutex mtx;

/**
 * @brief Contadores de verificación para los elementos producidos.
 */
unsigned cont_prod[NUM_ITEMS] = {0};

/**
 * @brief Contadores de verificación para los elementos consumidos.
 */
unsigned cont_cons[NUM_ITEMS] = {0};

//**********************************************************************
// funciones comunes a las dos soluciones (fifo y lifo)
//----------------------------------------------------------------------

/**
 * @brief Función para producir datos.
 *
 * @return El dato producido.
 */
int producir_dato()
{
   
   this_thread::sleep_for( chrono::milliseconds( aleatorio<min_ms,max_ms>() ));
   mtx.lock();
   const int valor_producido = siguiente_dato ;
   siguiente_dato ++ ;
   cout << "hebra productora, produce " << valor_producido << endl << flush ;
   mtx.unlock();
   
   cont_prod[valor_producido]++ ;
   return valor_producido ;
}

//----------------------------------------------------------------------
/**
 * @brief Función para consumir datos.
 *
 * @param dato Dato que se va a consumir.
 */
void consumir_dato( unsigned valor_consumir )
{
   if ( NUM_ITEMS <= valor_consumir )
   {
      cout << " valor a consumir === " << valor_consumir << ", num_items == " << NUM_ITEMS << endl ;
      assert( valor_consumir < NUM_ITEMS );
   }
   cont_cons[valor_consumir] ++ ;
   this_thread::sleep_for( chrono::milliseconds( aleatorio<min_ms,max_ms>() ));
   mtx.lock();
   cout << "                  hebra consumidora, consume: " << valor_consumir << endl ;
   mtx.unlock();
}
//----------------------------------------------------------------------
/**
 * @brief Función para comprobar si se han producido y consumido todos los datos.
 */
void test_contadores()
{
   bool ok = true ;
   cout << "comprobando contadores ...." << endl ;

   for( unsigned i = 0 ; i < NUM_ITEMS ; i++ )
   {
      if ( cont_prod[i] != 1 )
      {
         cout << "error: valor " << i << " producido " << cont_prod[i] << " veces." << endl ;
         ok = false ;
      }
      if ( cont_cons[i] != 1 )
      {
         cout << "error: valor " << i << " consumido " << cont_cons[i] << " veces" << endl ;
         ok = false ;
      }
   }
   if (ok)
      cout << endl << flush << "solución (aparentemente) correcta." << endl << flush ;
}
/**
 * @class ProdConsSU1
 * @brief Clase para monitor buffer, versión FIFO, semántica SC, múltiples productores/consumidores.
 *
 * Esta clase implementa un monitor que gestiona un buffer de elementos. 
 * Los elementos son producidos por múltiples productores y consumidos por múltiples consumidores.
 * La estrategia de gestión del buffer es FIFO (First In, First Out), y la semántica es SC.
 */
class ProdConsSU1 : public HoareMonitor
{
 private:
 static const int           // constantes ('static' ya que no dependen de la instancia)
   TAM_BUFFER = 10;   //   núm. de entradas del buffer
 int                        // variables permanentes
   buffer[TAM_BUFFER],//   buffer de tamaño fijo, con los datos
   primera_libre ,          //   indice de celda de la próxima inserción
   ultima_ocupada ,         //   indice de celda de la próxima lectura        // MODIFICACIÓN - VARIABLE AÑADIDA
   contador_ocupadas;

 CondVar                    // colas condicion:
   lectores,                //  cola donde espera el consumidor (n>0)
   escritores ;                 //  cola donde espera el productor  (n<num_celdas_total)

 public:                    // constructor y métodos públicos
   ProdConsSU1();             // constructor
   int  leer();                // extraer un valor (sentencia L) (consumidor)
   void escribir(int valor); // insertar un valor (sentencia E) (productor)
} ;
// -----------------------------------------------------------------------------

ProdConsSU1::ProdConsSU1()
{
   primera_libre = 0 ;
   ultima_ocupada = 0;
   contador_ocupadas = 0;
   lectores = newCondVar();
   escritores = newCondVar();
}
// -----------------------------------------------------------------------------
// función llamada por el consumidor para extraer un dato

int ProdConsSU1::leer()
{
   // esperar bloqueado hasta que primera_libre != ultima_ocupada
   if ( contador_ocupadas == 0 )// ocupadas.empty()  MODIFICACIÓN - DUDA
      lectores.wait();
   
   //cout << "leer: ocup == " << primera_libre - ultima_ocupada << ", total == " << num_celdas_total << endl ;       // MODUFICACIÓN en la resta
   assert( contador_ocupadas > 0 );

   // hacer la operación de lectura, actualizando estado del monitor
   // Incremento (decremento respecto a primera libre) el valor de última ocupada, pues tras la lectura, hay una ocupada menos
   int valor = buffer[ultima_ocupada];                   // MODIFICACIÓN (se consulta ultima_ocupada)
   ultima_ocupada = (ultima_ocupada + 1) % TAM_BUFFER;   // MODIFICACIÓN
   contador_ocupadas--;
   
   // señalar al productor que hay un hueco libre, por si está esperando
   escritores.signal();

   // devolver valor
   return valor ;
}
// -----------------------------------------------------------------------------

void ProdConsSU1::escribir( int valor )
{
   // esperar bloqueado hasta que primera_libre < num_celdas_total
   if ( contador_ocupadas >= TAM_BUFFER )        // MODIFICACIÓN
      escritores.wait();

   //cout << "escribir: ocup == " << primera_libre << ", total == " << num_celdas_total << endl ;
   assert( contador_ocupadas < TAM_BUFFER );

   // hacer la operación de inserción, actualizando estado del monitor
   buffer[primera_libre] = valor ;
   primera_libre = (primera_libre + 1) % TAM_BUFFER;
   contador_ocupadas++;

   // señalar al consumidor que ya hay una celda ocupada (por si esta esperando)
   lectores.signal();
}

// -----------------------------------------------------------------------------
/**
 * @brief Función que ejecuta la hebra productora.
 *
 * Esta función es ejecutada por cada hebra productora. Produce una cantidad de números
 * y los escribe en el monitor.
 *
 * @param cantidad_numeros_a_producir Cantidad de números que la hebra productora debe producir.
 * @param monitor Monitor en el que la hebra productora debe escribir los números.
 */
void funcion_hebra_productora(const unsigned int cantidad_numeros_a_producir, MRef<ProdConsSU1> monitor )
{
   for( unsigned i = 0 ; i < cantidad_numeros_a_producir; i++ )
   {
      int valor = producir_dato(  ) ;
      monitor->escribir( valor );
   }
}

/**
 * @brief Función que ejecuta la hebra consumidora.
 *
 * Esta función es ejecutada por cada hebra consumidora. Consume una cantidad de números
 * del monitor.
 *
 * @param cantidad_numeros_a_producir Cantidad de números que la hebra consumidora debe consumir.
 * @param monitor Monitor del que la hebra consumidora debe leer los números.
 */
void funcion_hebra_consumidora(const unsigned int cantidad_numeros_a_producir, MRef<ProdConsSU1>  monitor )
{
   for( unsigned i = 0 ; i < cantidad_numeros_a_producir; i++ )
   {
      int valor = monitor->leer();
      consumir_dato( valor ) ;
   }
}
// -----------------------------------------------------------------------------

int main()
{
   cout << "--------------------------------------------------------------------" << endl
        << "Problema del productor-consumidor múltiples (Monitor SU, buffer FIFO). " << endl
        << "--------------------------------------------------------------------" << endl
        << flush ;

   // Declaraciones
   MRef<ProdConsSU1> monitor = Create<ProdConsSU1>() ;
   thread hebras_productoras[NUM_PROD];
   thread hebras_consumidoras[NUM_CONS];

   for (int i = 0; i < NUM_PROD; i++){
       hebras_productoras[i] = thread(funcion_hebra_productora, CANTIDAD_DATOS_A_PRODUCIR, monitor);
   }
   
   for (int i = 0; i < NUM_CONS; i++){
       hebras_consumidoras[i] = thread(funcion_hebra_consumidora, CANTIDAD_DATOS_A_CONSUMIR, monitor);
   }
   
   for (int i = 0; i < NUM_PROD; i++){
       hebras_productoras[i].join();
   }
   
   for (int i = 0; i < NUM_CONS; i++){
       hebras_consumidoras[i].join();
   }

   test_contadores();
}
