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

constexpr unsigned int
   NUM_ITEMS = 40 ,   // número de items
	TAM_VEC   = 10 ,   // tamaño del buffer
   NUM_PROD = NUM_ITEMS / 5,
   NUM_CONS = NUM_ITEMS / 4;

unsigned  
   cont_prod[NUM_ITEMS] = {0}, // contadores de verificación: para cada dato, número de veces que se ha producido.
   cont_cons[NUM_ITEMS] = {0}, // contadores de verificación: para cada dato, número de veces que se ha consumido.
   siguiente_dato       = 0 ,  // siguiente dato a producir en 'producir_dato' (solo se usa ahí)
   primero_libre = 0 ,
   ultimo_ocupado = 0 ,
   buffer[TAM_VEC] = {0},
   producidos_por_productor[NUM_PROD] = {0},
   consumidos_por_consumidor[NUM_CONS] = {0};

bool FINALIZA_CONSUMIDORES = false;

Semaphore libres = TAM_VEC,
            ocupadas = 0,
            ultimo_ocupado_libre = 1,
            primero_libre_libre = 1,
            espera_confirmacion = 1;
//**********************************************************************
// funciones comunes a las dos soluciones (fifo y lifo)
//----------------------------------------------------------------------

unsigned producir_dato(int invocador)
{
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));
   const unsigned dato_producido = siguiente_dato ;
   siguiente_dato++ ;
   cont_prod[dato_producido] ++;

   cout << "Productor " << invocador << " produjo: " << dato_producido << endl << endl << flush;
   producidos_por_productor[invocador]++;
   return dato_producido ;
}
//----------------------------------------------------------------------

void consumir_dato( unsigned dato, int invocador)
{
   assert( dato < NUM_ITEMS );
   cont_cons[dato] ++ ;
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));
   cout << "                  Consumidor " << invocador << " consumio: " << dato << endl ;
   consumidos_por_consumidor[invocador]++;
}

/*
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
*/
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
   
   if (ok)
      cout << endl << flush << "solución (aparentemente) correcta." << endl << flush ;
}

//----------------------------------------------------------------------

void  funcion_hebra_productora(int num_hebra)
{
   bool finaliza = false;

   do
   {
      cout << "produce produce " << endl;
      int dato = producir_dato(num_hebra);

      finaliza = dato >= NUM_ITEMS;
      
      if (!finaliza)
      {
         libres.sem_wait();
            primero_libre_libre.sem_wait();
               buffer[primero_libre] = dato;
               primero_libre = (primero_libre + 1) % TAM_VEC;
            primero_libre_libre.sem_signal();
         ocupadas.sem_signal();
      }
      cout << "/produce produce " << endl;
   } while (!finaliza);
   cout << "hey hey produce" << endl;
}

//----------------------------------------------------------------------

void funcion_hebra_consumidora(int num_hebra)
{  
   while (!FINALIZA_CONSUMIDORES)
   {
      cout << "consume consume" << endl;
      int dato;

      cout << "consume consume 4" << endl;
      espera_confirmacion.sem_wait();
      if (!FINALIZA_CONSUMIDORES)
      {
         cout << "consume consume 2" << endl;
         ocupadas.sem_wait();
         cout << "conume consume 3" << endl;
            ultimo_ocupado_libre.sem_wait();

               dato = buffer[ultimo_ocupado];
               FINALIZA_CONSUMIDORES = !(dato < NUM_ITEMS);
               espera_confirmacion.sem_signal();
               ultimo_ocupado = (ultimo_ocupado + 1) % TAM_VEC;

            ultimo_ocupado_libre.sem_signal();
             cout << "/conume consume 3" << endl;
         libres.sem_signal();
         cout << "/conume consume 2" << endl;

         if (!FINALIZA_CONSUMIDORES) consumir_dato( dato, num_hebra);
         cout << "/consume consume" << endl;
      }
      
   }
   cout << "hey hey consume " << endl;
}
//----------------------------------------------------------------------

int main()
{
   cout << "-----------------------------------------------------------------" << endl
        << "Problema de los productores-consumidores (solución Multi FIFO ?)." << endl
        << "------------------------------------------------------------------" << endl
        << flush ;

   thread hebras_consumidoras[NUM_CONS];
   thread hebras_productoras[NUM_PROD];

   for (int i = 0; i < NUM_PROD; i++)
   {
      hebras_productoras[i] = thread(funcion_hebra_productora,i);
   }
   for (int i = 0; i < NUM_CONS; i++)
   {
      hebras_consumidoras[i] = thread(funcion_hebra_consumidora,i);
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

   unsigned int total_producido = 0;
   unsigned int total_consumido = 0;
   for (size_t i = 0; i < NUM_PROD; i++)
   {
      total_producido += producidos_por_productor[i];
      cout << "Productor: " << i << " ha producido: " << producidos_por_productor[i] << " elementos." << endl;
   }

   for (size_t i = 0; i < NUM_CONS; i++)
   {
      total_consumido += consumidos_por_consumidor[i];
      cout << "Consumidor: " << i << " ha consumido: " << consumidos_por_consumidor[i] << " elementos." << endl;
   }
   
   cout << "Se han producido: " << total_producido << " items." << endl;
   cout << "Se han consumido: " << total_consumido << " items." << endl;
   cout << "En total deberían ser: " << NUM_ITEMS << " elementos." << endl;
}
