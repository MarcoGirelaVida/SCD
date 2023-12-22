// -----------------------------------------------------------------------------
// Marco Girela Vida
// Sistemas concurrentes y Distribuidos.
// Práctica 4. Implementación de sistemas de tiempo real.
//
//   Datos de las tareas:
//   ------------
//   Ta.  T    C
//   ------------
//   A  500  100
//   B  500  150
//   C  1000 200
//   D  2000 240
//  -------------
//
//  Planificación (con Ts == 500 ms)
//  *--------------*-------------*-------------*------------*
//  | A B C +50ms  | A B D +10ms | A B C +50ms | A B +250ms |
//  *--------------*-------------*-------------*------------*
//  
//
// Archivo: ejecutivo2.cpp
// -----------------------------------------------------------------------------

/*
1- ¿cual es el mínimo tiempo de espera que queda al final de las
iteraciones del ciclo secundario con tu solución?
   En la primera iteración queda un tiempo de espera teórico de 50ms.
   En la segunda, uno de 10ms.
   En la tercera 50ms.
   Y por último en la cuarta 250ms.

   Por tanto el tiempo mínimo de espera es 10ms en la segunda iteración

2- ¿sería planificable si la tarea D tuviese un tiempo cómputo de 250ms?
   En teoría sí puesto que sería capaz de entrar en los 500ms que constituyen
una iteración, no obstante en la práctica (al menos la simulación que estamos haciendo nosotros)
no sería posible puesto que los sleep_until no son precisos y siempre tienen un pequeño excedente
que en este caso, por pequeño que fuese, supodría vulnerar la ejecución de D o A (que es el proceso que le sucede).
*/
#include <string>
#include <iostream> // cout, cerr
#include <thread>
#include <chrono>   // utilidades de tiempo
#include <ratio>    // std::ratio_divide
#include <numeric>

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
                  T_B(150) ,
                  T_C(200) ,
                  T_D(240) ;

void TareaA() { Tarea( "A", T_A );  }
void TareaB() { Tarea( "B", T_B );  }
void TareaC() { Tarea( "C", T_C );  }
void TareaD() { Tarea( "D", T_D );  }

// -----------------------------------------------------------------------------
// implementación del ejecutivo cíclico:

int main( int argc, char *argv[] )
{
   // Ts = duración del ciclo secundario (en unidades de milisegundos, enteros)
   const milliseconds Ts_ms (500);
   /*
   const millisecons T_total_ms
   (
      std::lcm<int>
      (
         std::lcm<int>
         (
               std::lcm<int>
               (
                  static_cast<int>(T_A.count()),
                  static_cast<int>(T_B.count())
               ),
               static_cast<int>(T_C.count())
         ),
         static_cast<int>(T_D.count())
      )
   );
   */
   cout << "El tiempo de duración del ciclo secundario es: " << Ts_ms.count() << " milisegundos." << endl;

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
            case 1 : TareaA(); TareaB(); TareaC(); T_esperado += T_C;   break ;
            case 2 : TareaA(); TareaB(); TareaD(); T_esperado += T_D;   break ;
            case 3 : TareaA(); TareaB(); TareaC(); T_esperado += T_C;   break ;
            case 4 : TareaA(); TareaB();                                break ;
         }

         // Calculo cual era el tiempo esperado y cuál ha sido el real y lo muestro
         time_point<steady_clock> fin_sec = steady_clock::now();
         milliseconds_f duracion_ciclo_real = fin_sec - ini_sec;
         cout << "Se ha tardado " << duracion_ciclo_real.count() << " milisegundos en lugar de los " << T_esperado.count() << " esperados." << endl;
         cout << "No obstante le han sobrado: " << (Ts_ms - duracion_ciclo_real).count() << " milisegundos." << endl;

         // calcular el siguiente instante de inicio del ciclo secundario
         ini_sec += Ts_ms;
         // esperar hasta el inicio de la siguiente iteración del ciclo secundario
         sleep_until( ini_sec );
      }
   }
}
