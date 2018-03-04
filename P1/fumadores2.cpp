#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "Semaphore.h"
#include <vector>


using namespace std ;
using namespace SEM ;

//numero de fumadores
const int num_fum = 2;

// semáforos compartidos
Semaphore fumador[num_fum] = {0,0};
Semaphore estanquero = 0;
Semaphore sum_necesita = 1;
Semaphore sum_disponible = 0;

// Vector que simboliza el paquete de n suministros
const int n_sum = 10;
int paquete[10];


//**********************************************************************
// plantilla de función para generar un entero aleatorio uniformemente
// distribuido entre dos valores enteros, ambos incluidos
// (ambos tienen que ser dos constantes, conocidas en tiempo de compilación)
//----------------------------------------------------------------------

template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}

//----------------------------------------------------------------------
// función que ejecuta la hebra del estanquero

//función que genera el ingrediente de cada fumador
inline int producir ()
{
  return aleatorio<0,num_fum-1>(); 
}


void funcion_hebra_estanquero(  )
{
  while( true ) {
    sem_wait( sum_disponible );           //Espera a que haya suministros
    
    for ( int i = 0; i < n_sum; ++i ) {
      cout << " Estanquero sirve porducto: " << paquete[i] << endl;
      
      sem_signal(fumador[ paquete[i] ]);  //Avisa al fumador

      sem_wait(estanquero);               //Espera a que recojan el producto
    }


    sem_signal( sum_necesita );           //Espera a que haya nueva mercancia
  }
}

//-----------------------------------------------------------------------
// Funcion para la hebra suministradora

void funcion_hebra_suministradora(  )
{
  while( true ) {

    //Produce un paquete de tamaño n_sum
    for ( int i = 0; i < n_sum; ++i ) {
      paquete[i] = producir();
    }

    sem_wait( sum_necesita );         //Espera a que le encargen el pedido    
    sem_signal( sum_disponible );     //Avisa al estanquero de que está listo
    cout << "*********** Ha llegado el paquete. ***********" << endl;    
  }
    
}


//-------------------------------------------------------------------------
// Función que simula la acción de fumar, como un retardo aleatoria de la hebra

void fumar( int num_fumador )
{

   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_fumar( aleatorio<20,200>() );

   // informa de que comienza a fumar

    cout << "Fumador " << num_fumador << "  :"
          << " empieza a fumar (" << duracion_fumar.count() << " milisegundos)" << endl;

   // espera bloqueada un tiempo igual a ''duracion_fumar' milisegundos
   this_thread::sleep_for( duracion_fumar );

   // informa de que ha terminado de fumar

    cout << "Fumador " << num_fumador << "  : termina de fumar, comienza espera de ingrediente." << endl;

}

//----------------------------------------------------------------------
// función que ejecuta la hebra del fumador
void  funcion_hebra_fumador( int num_fumador )
{
  while( true )
    {
      sem_wait(fumador[num_fumador]);            //Espera a que llegue su ingrediente
      cout << "Fumador " << num_fumador << " retira el producto." << endl;
      sem_signal(estanquero);                    //Avisa que lo recogió
      fumar(num_fumador);                        //Fuma
    }
}

//----------------------------------------------------------------------

int main()
{
  cout << "--------------------------------------------------------" << endl
        << "Problema de los fumadores." << endl
        << "--------------------------------------------------------" << endl
        << flush ;

  //Generamos las hebras de los procesos
  thread hebra_fumador[num_fum ];
  thread hebra_estanquero ( funcion_hebra_estanquero);
  thread hebra_suministradora ( funcion_hebra_suministradora);

  //inicializamos el vector de fumadores
  for ( int i = 0; i < num_fum; ++i )
    hebra_fumador[i] = thread ( funcion_hebra_fumador, i );


  //Esperamos al final de la ejecución de los procesos
  //      (esta parte realmente es innecesaria pues las hebra corren un bucle infinito)   
  for ( int i = 0; i < num_fum; ++i )
    hebra_fumador[i].join();

  hebra_estanquero.join();
  hebra_suministradora.join();

   cout << "-------------------------------------------------------" << endl
	<< "FIN"
	<< "-------------------------------------------------------" << endl;
}
