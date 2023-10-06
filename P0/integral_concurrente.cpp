#include <iostream>
#include <chrono>
#include <future>
#include <thread>
#include <cmath>

using namespace std;
using namespace std::chrono;


double f(double x)
{
    return 4.0/(1+pow(x,2));
}


double calcular_integral_secuencial(long inicio, long final)
{
    double suma = 0.0 ;

    for( long j = inicio ; j < (final-inicio) ; j++ )
    {
        const double xj = double(j+0.5)/(final-inicio);
        suma += f(xj);
    }

    return suma/(final-inicio);
}


double calcular_integral_concurrente(long muestras, int n_hebras)
{
    long paso = muestras / n_hebras;

    future <double> futuros[n_hebras];

    for( int i = 0 ; i < n_hebras ; i++ )
        futuros[i] = async( launch::async, calcular_integral_secuencial, i*paso, i*paso+paso ) ;

    double suma = 0;
    for (int i=0; i < n_hebras; i++)
        suma += futuros[i].get();

    return suma;
}


int main(int argc, char * argv[])
{
    const long muestras = 1000000000;
    const int n_hebras = stoi(argv[1]);
    const double resultado_real = 3.14159265358979312;


    time_point<steady_clock> instante_inicio1 = steady_clock::now();
    const double resultado_secuencial = calcular_integral_secuencial(0, muestras);
    time_point<steady_clock> instante_final1 = steady_clock::now() ;
    duration<float,micro> tiempo_secuencial = instante_final1 - instante_inicio1 ;


    time_point<steady_clock> instante_inicio2 = steady_clock::now();
    const double resultado_concurrente = calcular_integral_concurrente(muestras, n_hebras);
    time_point<steady_clock> instante_final2 = steady_clock::now() ;
    duration<float,micro> tiempo_concurrente = instante_final2 - instante_inicio2 ;

    double porcentaje_mejora_tiempo = (tiempo_concurrente/tiempo_secuencial)*100;
    cout << "Número de muestras (m) :" << muestras << endl;
    cout << "Número de hebras (n) :" << n_hebras << endl;
    cout << "Valor de PI :" << resultado_real << endl << endl;
    cout << "Resultado secuencial :" << resultado_secuencial << endl;
    cout << "Tiempo secuencial :" << tiempo_secuencial.count() << " ms" << endl << endl;
    cout << "Resultado concurrente :" << resultado_concurrente << endl;
    cout << "Tiempo concurrente :" << tiempo_concurrente.count() << " ms" << endl << endl;;

    cout << "Porcentaje t.conc/t.sec. :" << 100-porcentaje_mejora_tiempo << " %" << endl;
}