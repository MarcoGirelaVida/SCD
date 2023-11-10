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
	tam_vec   = 10 ,   // tamaño del buffer
   n_prod = NUM_ITEMS / 5,
   n_cons = NUM_ITEMS / 4,
   prodpprod = NUM_ITEMS / n_prod,
   prodpcons = NUM_ITEMS / n_cons;
unsigned  
   cont_prod[NUM_ITEMS] = {0}, // contadores de verificación: para cada dato, número de veces que se ha producido.
   cont_cons[NUM_ITEMS] = {0}, // contadores de verificación: para cada dato, número de veces que se ha consumido.
   siguiente_dato       = 0 ,  // siguiente dato a producir en 'producir_dato' (solo se usa ahí)
   primero_libre = 0 ,
   ultimo_ocupado = 0 ,
   buffer[tam_vec] = {0},
   producidos_por_productor[n_prod] = {0},
   consumidos_por_consumidor[n_cons] = {0};

Semaphore libres = tam_vec,
            ocupadas = 0,
            ultimo_ocupado_libre = 1,
            primero_libre_libre = 1;
//**********************************************************************
// funciones comunes a las dos soluciones (fifo y lifo)
//----------------------------------------------------------------------

unsigned producir_dato(int invocador)
{
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));
   const unsigned dato_producido = siguiente_dato ;
   siguiente_dato++ ;
   cont_prod[dato_producido] ++ ;
   cout << "producido: " << dato_producido << endl << endl << flush;
   producidos_por_productor[invocador]++;
   return dato_producido ;
}
//----------------------------------------------------------------------

void consumir_dato( unsigned dato, int invocador)
{
   assert( dato < NUM_ITEMS );
   cont_cons[dato] ++ ;
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));

   cout << "                  consumido: " << dato << endl ;
   consumidos_por_consumidor[invocador]++;

}

void mostrar_buffer()
{
   // Pintar flecha de primero libre
   for (int i = 1; i < primero_libre; i++)
   {
      cout << "     ";
   }
   cout << "   primero libre" << endl;

   for (int i = 1; i < primero_libre; i++)
   {
      cout << "     ";
   }
   cout << "    V" << endl;

   // Pintar ralla superior
   for (int i = 0; i < tam_vec; i++)
   {
      cout << "_____";
   }
   cout << "_" << endl;
   
   // Pintar contenido del buffer
   for (int i = 0; i < tam_vec; i++)
   {
      cout << "|" << setw(4) << buffer[i];
   }
   cout << "|" << endl;

   // Pintar ralla inferior
   for (int i = 0; i < tam_vec; i++)
   {
      cout << "_____";
   }
   cout << "_" << endl;

   for (int i = 1; i < ultimo_ocupado; i++)
   {
      cout << "     ";
   }
   cout << "    A" << endl;
   for (int i = 1; i < ultimo_ocupado; i++)
   {
      cout << "     ";
   }
   cout << "   Ultimo ocupado" << endl;

}
//----------------------------------------------------------------------

void test_contadores()
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

   for( int i = 0; i < n_prod; i++){
      if (producidos_por_productor[i] != prodpprod)
      {
         cout << "error: el productor " << i << " ha producido " << producidos_por_productor[i] << " en lugar de " << prodpprod << endl;
         ok = false;
      } 
   }

   for (int i = 0; i < n_cons; i++)
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
      int dato = producir_dato(num_hebra) ;
      libres.sem_wait();
      primero_libre_libre.sem_wait();
      buffer[primero_libre] = dato;
      primero_libre = (primero_libre + 1) % tam_vec;
      primero_libre_libre.sem_signal();
      ocupadas.sem_signal();
      mostrar_buffer();
   }
}

//----------------------------------------------------------------------

void funcion_hebra_consumidora(int num_hebra, int paso)
{
   for( unsigned i = paso*num_hebra ; i < paso*(num_hebra+1) ; i++ )
   {
      int dato ;
      ocupadas.sem_wait();
      ultimo_ocupado_libre.sem_wait();
      dato = buffer[ultimo_ocupado];
      ultimo_ocupado = (ultimo_ocupado + 1) % tam_vec;
      ultimo_ocupado_libre.sem_signal();
      libres.sem_signal();
      consumir_dato( dato, num_hebra) ;
    }
}
//----------------------------------------------------------------------

int main()
{
   cout << "-----------------------------------------------------------------" << endl
        << "Problema de los productores-consumidores (solución Multi FIFO ?)." << endl
        << "------------------------------------------------------------------" << endl
        << flush ;

   thread hebras_consumidoras[n_cons];
   thread hebras_productoras[n_prod];

   for (int i = 0; i < n_prod; i++)
   {
      hebras_productoras[i] = thread(funcion_hebra_productora,i,prodpprod);
   }
   for (int i = 0; i < n_cons; i++)
   {
      hebras_consumidoras[i] = thread(funcion_hebra_consumidora,i,prodpcons);
   }

   for (int i = 0; i < n_prod; i++)
   {
      hebras_productoras[i].join();
   }
   for (int i = 0; i < n_cons; i++)
   {
      hebras_consumidoras[i].join();
   }

   test_contadores();
}
