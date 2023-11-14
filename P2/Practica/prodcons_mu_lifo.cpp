// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Seminario 2. Introducción a los monitores en C++11.
//
// Archivo: prodcons1_su.cpp
//
// Ejemplo de un monitor en C++11 con semántica SU, para el problema
// del productor/consumidor, con productor y consumidor únicos.
// Opcion LIFO
//
// Historial:
// Creado el 30 Sept de 2022. (adaptado de prodcons2_su.cpp)
// 20 oct 22 --> paso este archivo de FIFO a LIFO, para que se corresponda con lo que dicen las transparencias
// -----------------------------------------------------------------------------------


#include <iostream>
#include <iomanip>
#include <cassert>
#include <random>
#include <thread>
#include "scd.h"

using namespace std ;
using namespace scd ;

/**
 * @var NUM_ITEMS
 * @brief Número de items a producir/consumir.
 */
constexpr int NUM_ITEMS = 2000000;

/**
 * @var NUM_HEBRAS
 * @brief Número de hebras a utilizar.
 */
constexpr int NUM_HEBRAS = 2000;

/**
 * @var siguiente_dato
 * @brief Siguiente valor a devolver en 'producir_dato'.
 */
int siguiente_dato = 0;

/**
 * @var min_ms
 * @brief Tiempo mínimo de espera en sleep_for.
 */
constexpr int min_ms = 5;

/**
 * @var max_ms
 * @brief Tiempo máximo de espera en sleep_for.
 */
constexpr int max_ms = 20;

/**
 * @var mtx
 * @brief Mutex para la escritura en pantalla.
 */
mutex mtx;

/**
 * @var cont_prod
 * @brief Contadores de verificación para los datos producidos.
 */
unsigned cont_prod[NUM_ITEMS] = {0};

/**
 * @var cont_cons
 * @brief Contadores de verificación para los datos consumidos.
 */
unsigned cont_cons[NUM_ITEMS] = {0};

//**********************************************************************
// funciones comunes a las dos soluciones (fifo y lifo)
//----------------------------------------------------------------------
/**
 * @brief Función que produce un dato.
 *
 * Esta función genera un dato para ser consumido posteriormente.
 * El dato producido es simplemente un contador que se incrementa después de cada producción.
 *
 * @return El dato producido.
 */
int producir_dato(  )
{
   // Se ha modificado la posición del mutex para garantizar la exclusión mutua sobre todas las
   // operaciones que involucren a "siguiente dato"
   // También había pensado en poner esta función como un método del monitor, pero entonces
   // no se podría dar el tiempo de espera concurrentemente
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
 * @brief Función que consume un dato.
 *
 * Esta función consume un dato. Verifica que el dato a consumir sea válido y luego lo consume.
 *
 * @param valor_consumir El valor a consumir.
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
 * @brief Función que verifica los contadores de producción y consumo.
 *
 * Esta función verifica que cada dato producido haya sido consumido exactamente una vez.
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

// *****************************************************************************
/**
 * @brief Clase que implementa un monitor para el problema del productor/consumidor.
 *
 * Esta clase implementa un monitor que proporciona una solución al problema del productor/consumidor.
 * El monitor utiliza semántica de señalización urgente (SU) y permite múltiples productores y consumidores.
 */
class ProdConsSU1 : public HoareMonitor
{
 private:
 static const int           // constantes ('static' ya que no dependen de la instancia)
   TAM_BUFFER = 10;   //   núm. de entradas del buffer
 int                        // variables permanentes
   buffer[TAM_BUFFER],//   buffer de tamaño fijo, con los datos
   primera_libre ;          //   indice de celda de la próxima inserción ( == número de celdas ocupadas)

 CondVar                    // colas condicion:
   ocupadas,                //  cola donde espera el consumidor (n>0)
   libres ;                 //  cola donde espera el productor  (n<num_celdas_total)

 public:                    // constructor y métodos públicos
   ProdConsSU1() ;             // constructor
   int  leer();   // extraer un valor (sentencia L) (consumidor)
   void escribir( int valor ); // insertar un valor (sentencia E) (productor)
} ;
// -----------------------------------------------------------------------------

ProdConsSU1::ProdConsSU1(  )
{
   primera_libre = 0 ;
   ocupadas      = newCondVar();
   libres        = newCondVar();
}
// -----------------------------------------------------------------------------
// función llamada por el consumidor para extraer un dato

int ProdConsSU1::leer()
{
   // esperar bloqueado hasta que 0 < primera_libre
   if ( primera_libre == 0 )
      ocupadas.wait();

   //cout << "leer: ocup == " << primera_libre << ", total == " << num_celdas_total << endl ;
   assert( 0 < primera_libre  );

   // hacer la operación de lectura, actualizando estado del monitor
   primera_libre-- ;
   const int valor = buffer[primera_libre] ;
   
   // señalar al productor que hay un hueco libre, por si está esperando
   libres.signal();

   // devolver valor
   return valor ;
}
// -----------------------------------------------------------------------------

void ProdConsSU1::escribir( int valor )
{
   // esperar bloqueado hasta que primera_libre < num_celdas_total
   if ( primera_libre == TAM_BUFFER )
      libres.wait();

   //cout << "escribir: ocup == " << primera_libre << ", total == " << num_celdas_total << endl ;
   assert( primera_libre < TAM_BUFFER );

   // hacer la operación de inserción, actualizando estado del monitor
   buffer[primera_libre] = valor ;
   primera_libre++ ;

   // señalar al consumidor que ya hay una celda ocupada (por si esta esperando)
   ocupadas.signal();
}
// *****************************************************************************
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
   for( size_t i = 0 ; i < cantidad_numeros_a_producir; i++ )
   {
      int valor = producir_dato(  ) ;
      monitor->escribir( valor );
   }
}
// -----------------------------------------------------------------------------
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
   for( size_t i = 0 ; i < cantidad_numeros_a_producir ; i++ )
   {
      int valor = monitor->leer();
      consumir_dato( valor ) ;
   }
}
// -----------------------------------------------------------------------------
/**
 * @brief Función principal.
 *
 * Esta función crea un monitor y varias hebras productoras y consumidoras.
 * Las hebras producen y consumen datos utilizando el monitor.
 * Finalmente, la función verifica que todos los datos producidos hayan sido consumidos.
 */

int main()
{
   cout << "--------------------------------------------------------------------" << endl
        << "Problema del productor-consumidor múltiple (Monitor SU, buffer LIFO). " << endl
        << "--------------------------------------------------------------------" << endl
        << flush ;

   // Declaraciones
   MRef<ProdConsSU1> monitor = Create<ProdConsSU1>() ;
   thread hebras_productoras[NUM_HEBRAS];
   thread hebras_consumidoras[NUM_HEBRAS];
   const unsigned int cantidad_de_ejecuciones_por_hebra = NUM_ITEMS / NUM_HEBRAS;

   // Lógica para los casos en los que el número de items no es una división entera con numero de hebras
   bool items_son_divisibles_entre_hebras = (NUM_ITEMS % NUM_HEBRAS) == 0;
   const unsigned int num_hebras_con_cantidad_ejecuciones_normal = items_son_divisibles_entre_hebras ? NUM_HEBRAS : (NUM_HEBRAS - 1);

   // DEBUG
   /*
   cout << "Cantidad ejecuciones por hebra:" << cantidad_de_ejecuciones_por_hebra << endl;
   cout << "Numero de hebras con cantidad de ejecuciones normal: " << num_hebras_con_cantidad_ejecuciones_normal << endl;
   cout << "Numero de hebras: " << NUM_HEBRAS << endl;
   cout << "Numero de items: " << NUM_ITEMS << endl;
   cout << endl;
   */

   // Inicialización hebras
   for (size_t i = 0; i < num_hebras_con_cantidad_ejecuciones_normal; i++)
   {
      hebras_productoras[i] = thread(funcion_hebra_productora, cantidad_de_ejecuciones_por_hebra, monitor);
      hebras_consumidoras[i] = thread(funcion_hebra_consumidora, cantidad_de_ejecuciones_por_hebra, monitor);
   }
   // Lógica de inicialización para los casos donde num_items / num_hebras no es una división entera
   if (num_hebras_con_cantidad_ejecuciones_normal != NUM_HEBRAS)
   {
      hebras_productoras[NUM_HEBRAS - 1] = thread(funcion_hebra_productora, cantidad_de_ejecuciones_por_hebra + 1, monitor);
      hebras_consumidoras[NUM_HEBRAS - 1] = thread(funcion_hebra_consumidora, cantidad_de_ejecuciones_por_hebra + 1, monitor);
   }
   
   // Finalización hebras
   for (size_t i = 0; i < NUM_HEBRAS; i++)
   {
      hebras_productoras[i].join();
      hebras_consumidoras[i].join();
   }

   test_contadores() ;
}
