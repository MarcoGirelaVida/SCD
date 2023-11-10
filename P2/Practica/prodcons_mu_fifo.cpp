// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Seminario 2. Introducción a los monitores en C++11.
//
// Archivo: prodcons1_su_fifo.cpp
//
// Ejemplo de un monitor en C++11 con semántica SU, para el problema
// del productor/consumidor, con productor y consumidor únicos.
// Opcion FIFO
// -----------------------------------------------------------------------------------


#include <iostream>
#include <iomanip>
#include <cassert>
#include <random>
#include <thread>
#include "scd.h"

using namespace std ;
using namespace scd ;

constexpr int
   NUM_ITEMS = 2000000,   // número de items a producir/consumir
   NUM_HEBRAS_PROD = 2000,
   NUM_HEBRAS_CONS = 2000;
int
   siguiente_dato = 0 ; // siguiente valor a devolver en 'producir_dato'
   
constexpr int               
   min_ms    = 5,     // tiempo minimo de espera en sleep_for
   max_ms    = 20 ;   // tiempo máximo de espera en sleep_for


mutex
   mtx ;                 // mutex de escritura en pantalla
unsigned
   cont_prod[NUM_ITEMS] = {0}, // contadores de verificación: producidos
   cont_cons[NUM_ITEMS] = {0}; // contadores de verificación: consumidos

//**********************************************************************
// funciones comunes a las dos soluciones (fifo y lifo)
//----------------------------------------------------------------------

int producir_dato(  )
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
// clase para monitor buffer, version FIFO, semántica SC, multiples prod/cons

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
   ProdConsSU1() ;             // constructor
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
// *****************************************************************************
// funciones de hebras

void funcion_hebra_productora(const unsigned int cantidad_numeros_a_producir, MRef<ProdConsSU1> monitor )
{
   for( unsigned i = 0 ; i < cantidad_numeros_a_producir; i++ )
   {
      int valor = producir_dato(  ) ;
      monitor->escribir( valor );
   }
}
// -----------------------------------------------------------------------------

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
   thread hebras_productoras[NUM_HEBRAS_PROD];
   thread hebras_consumidoras[NUM_HEBRAS_CONS];
   
   const unsigned int cantidad_de_ejecuciones_por_hebra_prod = NUM_ITEMS / NUM_HEBRAS_PROD;
   // Lógica para los casos en los que el número de items no es una división entera con numero de hebras
   bool items_son_divisibles_entre_hebras_prod = (NUM_ITEMS % NUM_HEBRAS_PROD) == 0;
   const unsigned int num_hebras_con_cantidad_ejecuciones_normal_prod = items_son_divisibles_entre_hebras_prod ? NUM_HEBRAS_PROD : (NUM_HEBRAS_PROD - 1);

   const unsigned int cantidad_de_ejecuciones_por_hebra_cons = NUM_ITEMS / NUM_HEBRAS_CONS;
   // Lógica para los casos en los que el número de items no es una división entera con numero de hebras
   bool items_son_divisibles_entre_hebras_cons = (NUM_ITEMS % NUM_HEBRAS_CONS) == 0;
   const unsigned int num_hebras_con_cantidad_ejecuciones_normal_cons = items_son_divisibles_entre_hebras_cons ? NUM_HEBRAS_CONS : (NUM_HEBRAS_CONS - 1);
   // DEBUG
   /*
   cout << "Cantidad ejecuciones por hebra:" << cantidad_de_ejecuciones_por_hebra << endl;
   cout << "Numero de hebras con cantidad de ejecuciones normal: " << num_hebras_con_cantidad_ejecuciones_normal << endl;
   cout << "Numero de hebras: " << NUM_HEBRAS << endl;
   cout << "Numero de items: " << NUM_ITEMS << endl;
   cout << endl;
   */

   // Inicialización hebras
   for (size_t i = 0; i < num_hebras_con_cantidad_ejecuciones_normal_prod; i++)
      hebras_productoras[i] = thread(funcion_hebra_productora, cantidad_de_ejecuciones_por_hebra_prod, monitor);
   // Lógica de inicialización para los casos donde num_items / num_hebras no es una división entera
   if (num_hebras_con_cantidad_ejecuciones_normal_prod != NUM_HEBRAS_PROD)
      hebras_productoras[NUM_HEBRAS_PROD - 1] = thread(funcion_hebra_productora, cantidad_de_ejecuciones_por_hebra_prod + 1, monitor);
   
   for (size_t i = 0; i < num_hebras_con_cantidad_ejecuciones_normal_cons; i++)
      hebras_consumidoras[i] = thread(funcion_hebra_consumidora, cantidad_de_ejecuciones_por_hebra_cons, monitor);
   // Lógica de inicialización para los casos donde num_items / num_hebras no es una división entera
   if (num_hebras_con_cantidad_ejecuciones_normal_cons != NUM_HEBRAS_CONS)
      hebras_consumidoras[NUM_HEBRAS_CONS - 1] = thread(funcion_hebra_consumidora, cantidad_de_ejecuciones_por_hebra_cons + 1, monitor);
   


   // Finalización hebras
   for (size_t i = 0; i < NUM_HEBRAS_PROD; i++)
      hebras_productoras[i].join();
   for (size_t i = 0; i < NUM_HEBRAS_CONS; i++)
      hebras_consumidoras[i].join();

   test_contadores() ;
}
