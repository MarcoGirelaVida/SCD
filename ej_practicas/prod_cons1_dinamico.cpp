#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random>
#include <atomic> 
#include "scd.h"

//PARA COMPILAR -> g++ -std=c++11 -pthread -I./include -o prod_cons1_dinamico ./bin//prod_cons1_dinamico.cpp ./bin/scd.cpp

//VARIACION: PRODUCTOR-CONSUMIDOR MULT LIFO CON REPARTO DINÁMICO DEL TRABAJO
//REVISADISIMO

using namespace std ;
using namespace scd ;

//**********************************************************************
// Variables globales

const int NUM_PROD = 8; //Número de productores
const int NUM_CONS = 5; //Número de consumidores

mutex mtx; // Declaración cerrojo usado para insertar-extraer (pues estamos en una pila)

const unsigned 
   num_items = 40 ,   // número de items
	tam_vec   = 10 ;   // tamaño del buffer

atomic<int> producidos(num_items); //Declaración variables atómicas usadas como contador de productos producidos y consumidos
atomic<int> consumidos(0); //Declaración variables atómicas usadas como contador de productos producidos y consumidos

unsigned  
   cont_prod[num_items] = {0}, // contadores de verificación: para cada dato, número de veces que se ha producido.
   cont_cons[num_items] = {0}, // contadores de verificación: para cada dato, número de veces que se ha consumido.
        
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
   
   cout << "producido: " << dato_producido << endl << flush ;
   
   return dato_producido;
}
//----------------------------------------------------------------------

void consumir_dato( unsigned dato, int indice )
{
   assert( dato < num_items ); //da error si dato es mayor que el num_items
   cont_cons[dato] ++;
   
   this_thread::sleep_for( chrono::milliseconds( aleatorio<5,5>() )); //tiempo que tarda en consumir
   cout << "                  consumido: " << dato << endl ;
}

//----------------------------------------------------------------------

void test_contadores() //test para comprobar que no se ha producido/consumido de más
{
   bool ok = true ;
   cout << "comprobando contadores ...." ;
   for( unsigned i = 0 ; i < num_items ; i++ )
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

void  funcion_hebra_productora(int indice)
{     
      while (producidos > 0){ //mientras todavía no se han producido todos los items
        producidos--; // está garantizada la em de este contador -> tipo atómico
        int dato = producir_dato(indice) ;
        
        sem_wait(libres);// decrementa el valor de libres, si es 0 espera
        mtx.lock(); // cerrojo que garantiza que dos o más hebras no accedan a la misma posiciónd e memoria/variable a la vez
        productos[primera_libre] = dato;
        primera_libre++;
        mtx.unlock();
        sem_signal(ocupadas); //aumenta el valor de ocupados
              
      }   
}

//----------------------------------------------------------------------

void funcion_hebra_consumidora(int i)
{     
    while (consumidos < num_items){
      consumidos++;
      int dato ;
      
      sem_wait(ocupadas);//decrementa el valor, si es 0 espera
      mtx.lock();
      primera_libre--;
      dato = productos[primera_libre]; 
      mtx.unlock();       
      sem_signal(libres);//aumenta el valor, pues se ha consumido un producto
      
      consumir_dato(dato, i) ;
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
   
   for (int i = 0; i < NUM_PROD; i++){
       hebras_productoras[i] = thread(funcion_hebra_productora, i);
   }
   
   for (int i = 0; i < NUM_CONS; i++){
       hebras_consumidoras[i] = thread(funcion_hebra_consumidora, i);
   }
   
   for (int i = 0; i < NUM_PROD; i++){
       hebras_productoras[i].join();
   }
   
   for (int i = 0; i < NUM_CONS; i++){
       hebras_consumidoras[i].join();
   }

   test_contadores();
}
