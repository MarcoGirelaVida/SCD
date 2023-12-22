// -----------------------------------------------------------------------------
// Marco Girela Vida
// Sistemas concurrentes y Distribuidos.
// Práctica 4. Implementación de sistemas de tiempo real.
//
// Archivo: ejecutivo1-compr.cpp
// -----------------------------------------------------------------------------

#include <string>
#include <iostream> // cout, cerr
#include <thread>
#include <chrono>   // utilidades de tiempo
#include <ratio>    // std::ratio_divide

using namespace std ;
using namespace std::chrono ;
using namespace std::this_thread ;

// tipo para duraciones en segundos y milisegundos, en coma flotante:
//typedef duration<float,ratio<1,1>>    seconds_f ;
typedef duration<float,ratio<1,1000>> milliseconds_f ;

// -----------------------------------------------------------------------------
// tarea genérica: duerme durante un intervalo de tiempo (de determinada duración)

void Tarea( const std::string & nombre, milliseconds tcomputo )
{
   cout << "   Comienza tarea " << nombre << " (C == " << tcomputo.count() << " ms.) ... " ;
   sleep_for( tcomputo );
   cout << "fin." << endl ;
}

// -----------------------------------------------------------------------------
// tareas concretas del problema:
const milliseconds
                  T_A(100),
                  T_B(80) ,
                  T_C(50) ,
                  T_D(40) ,
                  T_E(20) ;

void TareaA() { Tarea( "A", T_A );  }
void TareaB() { Tarea( "B", T_B );  }
void TareaC() { Tarea( "C", T_C );  }
void TareaD() { Tarea( "D", T_D );  }
void TareaE() { Tarea( "E", T_E );  }

// -----------------------------------------------------------------------------
// implementación del ejecutivo cíclico:

int main( int argc, char *argv[] )
{
   // Ts = duración del ciclo secundario (en unidades de milisegundos, enteros)
   const milliseconds Ts_ms( 250 );

   // ini_sec = instante de inicio de la iteración actual del ciclo secundario
   time_point<steady_clock> ini_sec = steady_clock::now();

   while( true ) // ciclo principal
   {
      cout << endl
           << "---------------------------------------" << endl
           << "Comienza iteración del ciclo principal." << endl ;
      milliseconds T_esperado;
      for( int i = 1 ; i <= 4 ; i++ ) // ciclo secundario (4 iteraciones)
      {
         T_esperado = T_A + T_B;
         cout << endl << "Comienza iteración " << i << " del ciclo secundario." << endl ;

         switch( i )
         {
            case 1 : TareaA(); TareaB(); TareaC(); T_esperado += T_C;                     break ;
            case 2 : TareaA(); TareaB(); TareaD(); TareaE(); T_esperado += (T_D + T_E);   break ;
            case 3 : TareaA(); TareaB(); TareaC(); T_esperado += T_C;                     break ;
            case 4 : TareaA(); TareaB(); TareaD(); T_esperado += T_D;                     break ;
         }
         time_point<steady_clock> fin_sec = steady_clock::now(); // Registro el tiempo de finalización

         // calcular el siguiente instante de inicio del ciclo secundario
         ini_sec += Ts_ms;
         // esperar hasta el inicio de la siguiente iteración del ciclo secundario
         sleep_until( ini_sec );

         // Calculo cual era el tiempo esperado y cuál ha sido el real y lo muestro
         milliseconds_f duracion_ciclo_250ms = fin_sec - (ini_sec - Ts_ms);
         cout << "Se ha tardado " << duracion_ciclo_250ms.count() << " milisegundos en lugar de los " << T_esperado.count() << " esperados." << endl;
      }
   }
}
