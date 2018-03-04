#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random>
#include "Semaphore.h"
#include <atomic>
using namespace std ;
using namespace SEM ;

//**********************************************************************
// variables compartidas

const int num_items = 89 ,   // número de items
  tam_vec   = 10,// tamaño del buffer
  num_prod  = 3,
  num_con   = 4;


unsigned  cont_prod[num_items] = {0}, // contadores de verificación: producidos
          cont_cons[num_items] = {0}; // contadores de verificación: consumidos


int primera_libre = 0,
  primera_ocupada = 0 ,
  buffer[tam_vec];
atomic<int> consumidos (0),
  producidos (0);
  
// semáforos compartidos
Semaphore puede_escribir = tam_vec ,
          puede_leer  = 0 ; // 1 si hay valor pendiente de leer

//Candados
mutex mtx_prod, mtx_con, mtx_prods, mtx_cons;


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

//**********************************************************************
// funciones comunes a las dos soluciones (fifo y lifo)
//----------------------------------------------------------------------

int producir_dato()
{
   static int contador = 0 ;
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));

   cout << "producido: " << contador << endl << flush ;

   cont_prod[contador] ++ ;
   return contador++ ;
}
//----------------------------------------------------------------------

void consumir_dato( unsigned dato )
{
   assert( dato < num_items );
   cont_cons[dato] ++ ;
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));

   cout << "                  consumido: " << dato << endl ;
   
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

void  funcion_hebra_productora( int num_hebra )
{
  
  int cont = producidos++;
  

  
  while(cont < num_items)
    {
      //produce el dato de modo concurrente
      int dato = producir_dato() ;
      
      //espera a que sea posible la inserción
      sem_wait( puede_escribir ) ;

      mtx_prod.lock();
      buffer[primera_libre] = dato ; // escribe el valor
      primera_libre = (primera_libre+1)%tam_vec; //actualiza el valor del indice (cola circular)
      mtx_prod.unlock();
      
      cout << "escrito: " << dato << endl ;

      sem_signal( puede_leer ) ;
      //envía señal para que se pueda proceder con la lectura

      int cont = producidos++;
     }
}

//----------------------------------------------------------------------

void funcion_hebra_consumidora( int num_hebra )
{

  int cont = consumidos++;
   
  
  while (cont < num_items)
  {
     //espera hasta que haya algo que leer
      sem_wait( puede_leer ) ;

      mtx_con.lock();
      int dato = buffer[primera_ocupada] ; // lee el valor generado
      primera_ocupada = (primera_ocupada+1)%tam_vec; // incrementa el valor del lecto (cola circular)
      mtx_con.unlock();
      
      cout << "leído: " << dato << endl;

      sem_signal( puede_escribir ) ;
      //Envía señal de que se puede sobreescribir una casilla del buffer

      
      consumir_dato( dato ) ;

      
      int cont = consumidos++;
  }
}
//----------------------------------------------------------------------

int main()
{
   cout << "--------------------------------------------------------" << endl
        << "Problema de los productores-consumidores (solución FIFO)." << endl
        << "--------------------------------------------------------" << endl
        << flush ;

   
   thread hebra_productora [num_prod],
     hebra_consumidora[num_con];

   for (int i = 0; i < num_prod; ++i) 
     hebra_productora[i] = thread( funcion_hebra_productora, i );

   for (int i = 0; i < num_con; ++i)
     hebra_consumidora[i] = thread( funcion_hebra_consumidora, i );

   for (int i = 0; i < num_prod; ++i) 
     hebra_productora[i].join() ;
   
   for (int i = 0; i < num_con; ++i)
   hebra_consumidora[i].join() ;

   test_contadores();

   return 0;
}
