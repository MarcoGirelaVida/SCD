#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random>
#include "scd.h"

using namespace std ;
using namespace scd ;

//**********************************************************************
// Variables globales

const unsigned 
   num_items = 40 ,   // número de items
	tam_vec   = 10 ;   // tamaño del buffer
unsigned  
   cont_prod[num_items] = {0}, // contadores de verificación: para cada dato, número de veces que se ha producido.
   cont_cons[num_items] = {0}, // contadores de verificación: para cada dato, número de veces que se ha consumido.
   siguiente_dato       = 0 ,  // siguiente dato a producir en 'producir_dato' (solo se usa ahí)
   primero_libre = 0 ,
   ultimo_ocupado = 0 ,
   buffer[tam_vec] = {0};

Semaphore libres = tam_vec;
Semaphore ocupadas = 0;
//**********************************************************************
// funciones comunes a las dos soluciones (fifo y lifo)
//----------------------------------------------------------------------

unsigned producir_dato()
{
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));
   const unsigned dato_producido = siguiente_dato ;
   siguiente_dato++ ;
   cont_prod[dato_producido] ++ ;
   cout << "producido: " << dato_producido << endl << flush ;
   return dato_producido ;
}
//----------------------------------------------------------------------

void consumir_dato( unsigned dato )
{
   assert( dato < num_items );
   cont_cons[dato] ++ ;
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));

   cout << "                  consumido: " << dato << endl ;

}

void mostrar_buffer()
{
   // Pintar flecha de primero libre
   for (int i = 1; i < tam_vec; i++)
   {
      cout << "   ";
   }
   cout << "  V Primero Libre" << endl;

   // Pintar ralla superior
   for (int i = 0; i < tam_vec; i++)
   {
      cout << "___";
   }
   cout << endl;
   
   // Pintar contenido del buffer
   for (int i = 0; i < tam_vec; i++)
   {
      cout << "|" << buffer[i];
   }
   cout << "|" << endl;

   // Pintar ralla inferior
   for (int i = 0; i < tam_vec; i++)
   {
      cout << "___";
   }
   cout << endl;
   
   // Pintar flecha de ultimo ocupado
   for (int i = 1; i < tam_vec; i++)
   {
      cout << "   ";
   }
   cout << "  A Ultimo ocupado" << endl;
   
}
//----------------------------------------------------------------------

void test_contadores()
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

void  funcion_hebra_productora(  )
{
   for( unsigned i = 0 ; i < num_items ; i++ )
   {
      int dato = producir_dato() ;
      libres.sem_wait();
      buffer[primero_libre] = dato;
      primero_libre = (primero_libre + 1) % tam_vec;
      ocupadas.sem_signal();
      mostrar_buffer();
   }
}

//----------------------------------------------------------------------

void funcion_hebra_consumidora(  )
{
   for( unsigned i = 0 ; i < num_items ; i++ )
   {
      int dato ;
      ocupadas.sem_wait();
      dato = buffer[ultimo_ocupado];
      ultimo_ocupado = (ultimo_ocupado + 1) % tam_vec;
      libres.sem_signal();
      consumir_dato( dato ) ;
    }
}
//----------------------------------------------------------------------

int main()
{
   cout << "-----------------------------------------------------------------" << endl
        << "Problema de los productores-consumidores (solución LIFO o FIFO ?)." << endl
        << "------------------------------------------------------------------" << endl
        << flush ;

   thread hebra_productora ( funcion_hebra_productora ),
          hebra_consumidora( funcion_hebra_consumidora );

   hebra_productora.join() ;
   hebra_consumidora.join() ;

   test_contadores();
}
