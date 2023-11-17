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
   NUM_ITEMS   = 40 ,         // número de items
	TAM_BUFFER  = 10 ,         // tamaño del buffer
   NUM_PROD = NUM_ITEMS / 10,
   NUM_CONS = NUM_ITEMS / 10;


unsigned  
   buffer[TAM_BUFFER]   = {0},

   contador_producidos[NUM_ITEMS] = {0},               // contadores de verificación: para cada dato, número de veces que se ha producido.
   contador_consumidos[NUM_ITEMS] = {0},               // contadores de verificación: para cada dato, número de veces que se ha consumido.

   producidos_por_productor[NUM_PROD]  = {0},
   consumidos_por_consumidor[NUM_CONS] = {0},


   siguiente_dato       = 0,                 // siguiente dato a producir en 'producir_dato' (solo se usa ahí)
   ultimo_dato_consumido    = 0,

   primero_libre        = 0,
   primera_ocupada      = 0;


Semaphore
   libres                  = TAM_BUFFER,
   ocupadas                = 0,

   primera_ocupada_en_uso  = 1,
   primera_libre_en_uso    = 1,

   escribiendo_en_ultimo_dato_consumido = 1;
   

//**********************************************************************
// funciones comunes a las dos soluciones (fifo y lifo)
//----------------------------------------------------------------------

unsigned producir_dato(int invocador)
{
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));

   const unsigned dato_producido = siguiente_dato ;
   siguiente_dato++ ;
   contador_producidos[dato_producido]++;
   producidos_por_productor[invocador]++;

   cout << "Productor " << invocador << " produjo: " << dato_producido << endl << endl << flush;

   return dato_producido ;
}
//----------------------------------------------------------------------

void consumir_dato( unsigned dato, int invocador)
{
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));

   assert( dato < NUM_ITEMS );

   escribiendo_en_ultimo_dato_consumido.sem_wait();
      ultimo_dato_consumido = ultimo_dato_consumido > dato ? ultimo_dato_consumido : dato;
   escribiendo_en_ultimo_dato_consumido.sem_signal();

   contador_consumidos[dato]++ ;
   consumidos_por_consumidor[invocador]++;

   cout << "                  Consumidor " << invocador << " consumio: " << dato << endl ;
}

//----------------------------------------------------------------------

void test_contadores()
{
   bool ok = true ;
   cout << "comprobando contadores ...." ;

   for( unsigned i = 0 ; i < NUM_ITEMS ; i++ )
   {
      if ( contador_producidos[i] != 1 )
      {  cout << "error: valor " << i << " producido " << contador_producidos[i] << " veces." << endl ;
         ok = false ;
      }

      if ( contador_consumidos[i] != 1 )
      {  cout << "error: valor " << i << " consumido " << contador_consumidos[i] << " veces" << endl ;
         ok = false ;
      }
   }
   
   if (ok)
      cout << endl << flush << "solución (aparentemente) correcta." << endl << flush ;
}

//----------------------------------------------------------------------

void  funcion_hebra_productora(int num_hebra)
{
   bool sigue_produciendo = true;

   while (sigue_produciendo)
   {
      unsigned dato = producir_dato(num_hebra);

      if (dato < NUM_ITEMS)
      {
         libres.sem_wait();
            primera_libre_en_uso.sem_wait();

               buffer[primero_libre] = dato;
               primero_libre = (primero_libre + 1) % TAM_BUFFER;

            primera_libre_en_uso.sem_signal();
         ocupadas.sem_signal();
      }
      else
         sigue_produciendo = false;
   }
}

//----------------------------------------------------------------------

void funcion_hebra_consumidora(int num_hebra)
{  
   bool continua_consumiendo = true;
   unsigned dato;

   while (continua_consumiendo)
   {
      escribiendo_en_ultimo_dato_consumido.sem_wait();
      cout << "CONSUMIDORA: " << num_hebra << " ultimo_cons: " << ultimo_dato_consumido << endl;
      cout << "Continua consumiendo = " << boolalpha << (ultimo_dato_consumido < NUM_ITEMS) << endl;
      escribiendo_en_ultimo_dato_consumido.sem_signal();

      if (ultimo_dato_consumido < NUM_ITEMS)
      {
         cout << "Se ha entrado en el if" << endl;
         ocupadas.sem_wait();
            cout << "Se ha salido del primer semáforo" << endl;
            primera_ocupada_en_uso.sem_wait();

                  dato = buffer[primera_ocupada];
                  primera_ocupada = (primera_ocupada + 1) % TAM_BUFFER;

            primera_ocupada_en_uso.sem_signal();
         libres.sem_signal();

         consumir_dato(dato, num_hebra);
      }
      else
         continua_consumiendo = false;
   }

   cout << "La hebra " << num_hebra << " terminó de consumir" << endl;
}
//----------------------------------------------------------------------

int main()
{
   cout << "-----------------------------------------------------------------" << endl
        << "Problema de los productores-consumidores (solución Multi FIFO)." << endl
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
