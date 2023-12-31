#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random>
#include<iomanip>
#include "scd.h"

using namespace std ;
using namespace scd ;

//**********************************************************************
// Variables globales

const int
   NUM_ITEMS = 40 ,   // número de items
	TAM_VEC   = 10 ,  // tamaño del buffer
   NUM_PROD = NUM_ITEMS / 5,
   NUM_CONS = NUM_ITEMS / 4,
   prodpprod = NUM_ITEMS / NUM_PROD,
   prodpcons = NUM_ITEMS / NUM_CONS;
unsigned  
   contador_producidos[NUM_ITEMS] = {0}, // contadores de verificación: para cada dato, número de veces que se ha producido.
   contador_consumidos[NUM_ITEMS] = {0}, // contadores de verificación: para cada dato, número de veces que se ha consumido.
   siguiente_dato       = 0 ,  // siguiente dato a producir en 'producir_dato' (solo se usa ahí)
   ultimo_libre = 0,
   buffer[TAM_VEC] = {0},
   producidos_por_productor[NUM_PROD] = {0},
   consumidos_por_consumidor[NUM_CONS] = {0};

Semaphore libres = TAM_VEC;
Semaphore ocupadas = 0;
Semaphore top_libre = 1;
//**********************************************************************
// funciones comunes a las dos soluciones (fifo y lifo)
//----------------------------------------------------------------------

unsigned producir_dato( int invocador)
{
   this_thread::sleep_for( chrono::milliseconds( aleatorio<80,150>() ));
   const unsigned dato_producido = siguiente_dato ;
   siguiente_dato++ ;
   contador_producidos[dato_producido] ++ ;
   cout << "producido: " << dato_producido << endl << endl << flush;
   producidos_por_productor[invocador]++;
   return dato_producido ;
}
//----------------------------------------------------------------------

void consumir_dato(unsigned dato, int invocador)
{
   assert( dato < NUM_ITEMS );
   contador_consumidos[dato] ++ ;
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,80>() ));

   cout << "                  consumido: " << dato << endl ;
   consumidos_por_consumidor[invocador]++;

}

void mostrar_buffer()
{
   // Pintar flecha de primero libre
   for (int i = 1; i < ultimo_libre; i++)
   {
      cout << "     ";
   }
   cout << "   top" << endl;

   for (int i = 1; i < ultimo_libre; i++)
   {
      cout << "     ";
   }
   cout << "    V" << endl;

   // Pintar ralla superior
   for (int i = 0; i < TAM_VEC; i++)
   {
      cout << "_____";
   }
   cout << "_" << endl;
   
   // Pintar contenido del buffer
   for (int i = 0; i < TAM_VEC; i++)
   {
      cout << "|" << setw(4) << buffer[i];
   }
   cout << "|" << endl;

   // Pintar ralla inferior
   for (int i = 0; i < TAM_VEC; i++)
   {
      cout << "_____";
   }
   cout << "_" << endl;
}
//----------------------------------------------------------------------

void test_contadores()
{
   bool ok = true ;
   cout << "comprobando contadores ...." ;
   for( unsigned i = 0 ; i < NUM_ITEMS ; i++ )
   {  if ( contador_producidos[i] != 1 )
      {  cout << "error: valor " << i << " producido " << contador_producidos[i] << " veces." << endl ;
         ok = false ;
      }
      if ( contador_consumidos[i] != 1 )
      {  cout << "error: valor " << i << " consumido " << contador_consumidos[i] << " veces" << endl ;
         ok = false ;
      }
   }

   for( int i = 0; i < NUM_PROD; i++){
      if (producidos_por_productor[i] != prodpprod)
      {
         cout << "error: el productor " << i << " ha producido " << producidos_por_productor[i] << " en lugar de " << prodpprod << endl;
         ok = false;
      } 
   }

   for (int i = 0; i < NUM_CONS; i++)
   {
      if (consumidos_por_consumidor[i] != prodpcons)
      {
         cout << "error: el consumidor " << i << " ha consumido " << consumidos_por_consumidor[i] << " en lugar de " << prodpcons << endl;
         ok = false;
      } 
   }
   
   if (ok)
      cout << endl << flush << "solución (aparentemente) correcta." << endl << flush ;
}

//----------------------------------------------------------------------

void  funcion_hebra_productora(int num_hebra, int paso)
{
   for( unsigned i = paso*num_hebra ; i < paso*(num_hebra+1); i++ )
   {
         libres.sem_wait();
         int dato = producir_dato(num_hebra) ;
         top_libre.sem_wait();
         buffer[ultimo_libre] = dato;
         ultimo_libre++;
         top_libre.sem_signal();
         ocupadas.sem_signal(); 
         mostrar_buffer();
   }
}

//----------------------------------------------------------------------

void funcion_hebra_consumidora(int num_hebra, int paso)
{
   for( unsigned i = paso*num_hebra ; i < paso*(num_hebra+1) ; i++ )
   {
         int dato;
         ocupadas.sem_wait();
         top_libre.sem_wait();
         dato = buffer[ultimo_libre-1];
         ultimo_libre--;
         top_libre.sem_signal();
         libres.sem_signal();
         consumir_dato(dato, num_hebra);
    }
}
//----------------------------------------------------------------------

int main()
{
   cout << "-----------------------------------------------------------------" << endl
        << "Problema de los productores-consumidores (solución MULTI LIFO)." << endl
        << "------------------------------------------------------------------" << endl
        << flush ;

   thread hebras_consumidoras[NUM_CONS];
   thread hebras_productoras[NUM_PROD];

   for (int i = 0; i < NUM_PROD; i++)
   {
      hebras_productoras[i] = thread(funcion_hebra_productora,i,prodpprod);
   }
   for (int i = 0; i < NUM_CONS; i++)
   {
      hebras_consumidoras[i] = thread(funcion_hebra_consumidora,i,prodpcons);
   }

   for (int i = 0; i < NUM_PROD; i++)
   {
      hebras_productoras[i].join();
   }
   for (int i = 0; i < NUM_CONS; i++)
   {
      hebras_consumidoras[i].join();
   }

   test_contadores();
}
