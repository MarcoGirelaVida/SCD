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
   cout << "-----------------------------------------------------------------" << endl
        << "Problema de los productores-consumidores multiple monitores (solución LIFO)." << endl
        << "------------------------------------------------------------------" << endl
        << flush ;

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



/*******************************************/
/*******************************************/
/* 
//PRODCONS LIFO CON SEMÁFOROS
#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random>
#include <atomic> 
#include "scd.h"

//PARA COMPILAR -> g++ -std=c++11 -pthread -I./include -o prod_cons1_dinamico ./bin//prod_cons1_dinamico.cpp ./bin/scd.cpp

using namespace std ;
using namespace scd ;

//**********************************************************************
// Variables globales
const int NUM_ITEMS = 20000;
const int NUM_PROD = 10; //Número de productores
const int NUM_CONS = 10; //Número de consumidores
const unsigned int CONSUMIDOS_POR_CONSUMIDOR = NUM_ITEMS/NUM_CONS;
const unsigned int PRODUCTOS_POR_PRODUCTOR = NUM_ITEMS/NUM_PROD;

mutex mtx; // Declaración cerrojo usado para insertar-extraer (pues estamos en una pila)
mutex output;

const unsigned 
	tam_vec   = 10 ;   // tamaño del buffer

//atomic<int> producidos(NUM_ITEMS); //Declaración variables atómicas usadas como contador de productos producidos y consumidos
//atomic<int> consumidos(0); //Declaración variables atómicas usadas como contador de productos producidos y consumidos

unsigned  
   cont_prod[NUM_ITEMS] = {0}, // contadores de verificación: para cada dato, número de veces que se ha producido.
   cont_cons[NUM_ITEMS] = {0}, // contadores de verificación: para cada dato, número de veces que se ha consumido.
        
   siguiente_dato       = 0 ;// siguiente dato a producir en 'producir_dato' (solo se usa ahí)
   int productos[tam_vec]; 

   Semaphore libres = tam_vec; // semáforo que indica si hay posiciones libres en las cuales insertar
   Semaphore ocupadas = 0; // semáforo que indica si hay posiciones ocupadas que leer
   
   int primera_libre = 0; //indica la posición de lectura/escritura de la pila

//**********************************************************************
// funciones comunes a las dos soluciones (fifo y lifo)
//----------------------------------------------------------------------

unsigned producir_dato(int indice)
{
   this_thread::sleep_for( chrono::milliseconds( aleatorio<700,900>() ));//tiempo en producir

      const unsigned dato_producido = siguiente_dato ; 
      
      siguiente_dato++ ; 
      cont_prod[dato_producido] ++ ; //señala que ya se ha producido dicho dato (no se producirán más así)
      
   output.lock();
      cout << "producido: " << dato_producido << endl << flush ;
   output.unlock();

   return dato_producido;
}
//----------------------------------------------------------------------

void consumir_dato( unsigned dato, int indice )
{
   assert( dato < NUM_ITEMS ); //da error si dato es mayor que el NUM_ITEMS
   cont_cons[dato] ++;
   
   this_thread::sleep_for( chrono::milliseconds( aleatorio<5,5>() )); //tiempo que tarda en consumir

   output.lock();
   cout << "                  consumido: " << dato << endl ;
   output.unlock();
}

//----------------------------------------------------------------------

void test_contadores() //test para comprobar que no se ha producido/consumido de más
{
   bool ok = true ;
   cout << "comprobando contadores ...." ;
   for( unsigned i = 0 ; i < NUM_ITEMS ; i++ )
   {  if ( cont_prod[i] != 1 )
      {  cout << "error: valor " << i << " producido " << cont_prod[i] << " veces." << endl ;
         ok = false ;
      }
      if ( cont_cons[i] != 1 )
      {  cout << "error: valor " << i << " consumido " << cont_cons[i] << " veces" << endl ;
         ok = false ;
      }
   }
   if (ok)
      cout << endl << flush << "solución (aparentemente) correcta." << endl << flush ;
}

//----------------------------------------------------------------------

void  funcion_hebra_productora(unsigned indice)
{     
     for (size_t i = indice*PRODUCTOS_POR_PRODUCTOR; i < PRODUCTOS_POR_PRODUCTOR*(indice+1); i++){
        //producidos--; // está garantizada la em de este contador -> tipo atómico
        int dato = producir_dato(indice) ;
        
        sem_wait(libres);     // decrementa el valor de libres, si es 0 espera
         mtx.lock();          // cerrojo que garantiza que dos o más hebras no accedan a la misma posiciónd e memoria/variable a la vez
            productos[primera_libre] = dato;
            primera_libre++;
         mtx.unlock();
        sem_signal(ocupadas); //aumenta el valor de ocupados
              
      }   
}

//----------------------------------------------------------------------

void funcion_hebra_consumidora(unsigned indice)
{     
   for (size_t i = indice*CONSUMIDOS_POR_CONSUMIDOR; i < CONSUMIDOS_POR_CONSUMIDOR*(indice+1); i++){
      //consumidos++;
      int dato ;
      
      sem_wait(ocupadas);     //decrementa el valor, si es 0 espera
         mtx.lock();
            primera_libre--;
            dato = productos[primera_libre]; 
         mtx.unlock();       
      sem_signal(libres);     //aumenta el valor, pues se ha consumido un producto
      
      consumir_dato(dato, indice) ;
    }   
}

//----------------------------------------------------------------------
int main()
{
   cout << "-----------------------------------------------------------------" << endl
        << "Problema de los productores-consumidores dinámico (solución LIFO)." << endl
        << "------------------------------------------------------------------" << endl
        << flush ;
   
   thread hebras_productoras[NUM_PROD];
   thread hebras_consumidoras[NUM_CONS];
   
   for (size_t i = 0; i < NUM_PROD; i++){
       hebras_productoras[i] = thread(funcion_hebra_productora, i);
   }
   
   for (size_t i = 0; i < NUM_CONS; i++){
       hebras_consumidoras[i] = thread(funcion_hebra_consumidora, i);
   }
   
   for (size_t i = 0; i < NUM_PROD; i++){
       hebras_productoras[i].join();
   }
   
   for (size_t i = 0; i < NUM_CONS; i++){
       hebras_consumidoras[i].join();
   }

   test_contadores();
}
*/