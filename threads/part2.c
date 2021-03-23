/*
*	Hoang Ho
*	ID 1001654608
*	CSE 3320-003
*	Due 03/28/2021 11:59 PM
*/

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <semaphore.h>

#define INITIAL_CUSTOMERS 1

#ifndef NUM_THREADS
#define NUM_THREADS 4
#endif

#define NONSHARED 1

sem_t customer_checked_out, customers_in_line;    
int customers_waiting = INITIAL_CUSTOMERS ;            

void * CustomerProducer( void * arg ) 
{
  printf( "CustomerProducer created\n" );

  while( 1 )
  {
    // Only produce a new customer if we check out an exiting customer
    sem_wait( &customer_checked_out );

    int new_customers = rand( ) % 10; 
    customers_waiting += new_customers; 

    printf( "Adding %d customers to the line\n", new_customers ); 
    printf( "%d customer waiting in line\n", customers_waiting );

    // Notify the cashiers that we've added a new customer to the lineA
    int i;
    for( i = 0; i < new_customers; i++ )
    {
      sem_post( &customers_in_line );
    }

    // Sleep a little bit so we can read the output on the screen
    sleep( 2 );

  }
}

void * Cashier( void * arg ) 
{

  printf( "Cashier created\n" );

  while( 1 )
  {
    // Wait here for a customer to appear in line
    sem_wait( &customers_in_line );

    customers_waiting --;

    // Check to make sure we haven't reduced the customer count
    // to below 0.  If we have then crash
    assert( customers_waiting >= 0 );

    printf( "Checking out customer. %d customers left in line\n", customers_waiting );

    sem_post( &customer_checked_out );

    // Sleep a little bit so we can read the output on the screen
    sleep( 1 );
  }

}

int main( int argc, char *argv[] ) 
{
  time_t t;

  srand( ( unsigned int ) time( & t ) );

  pthread_t producer_tid;  
  pthread_t cashier_tid [ NUM_CASHIERS ];  

  sem_init( & customer_checked_out, NONSHARED, 0 );  
  sem_init( & customers_in_line,    NONSHARED, INITIAL_CUSTOMERS );   

  pthread_create( & producer_tid, NULL, CustomerProducer, NULL );

  int i;
  for( i = 0; i < NUM_CASHIERS; i++ )
  {
    pthread_create( & cashier_tid[i], NULL, Cashier, NULL );
  }

  pthread_join( producer_tid, NULL );
  for( i = 0; i < NUM_CASHIERS; i++ )
  {
    pthread_join( cashier_tid[i], NULL );
  }

}
