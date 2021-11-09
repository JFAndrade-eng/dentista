#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<semaphore.h>
#include<unistd.h>
#define NTHREADS 50


int dentista_ocupado = 0;

int numCliente=0, ncadeiras=0, tcadeiras;

pthread_mutex_t lock_cadeira;
pthread_mutex_t lock_dentista;
pthread_mutex_t lock_mudanca_cliente;

// pthread_cond_t proximo;
sem_t proximo;
sem_t liberado;
sem_t entrei;


void* dentista(void* arg){
   int i,*n = (int*)arg;
   for(i=0;i<*n;i++){

      printf("------------------\nDentista eguando\n------------------\n");

      sem_post(&proximo);
      //Cliente Chegou
      // if (ncadeiras < tcadeiras) {
      //    pthread_mutex_lock(&lock_cadeira);
      //       ncadeiras++;
      //    pthread_mutex_unlock(&lock_cadeira);
      // }
      sem_wait(&entrei);
      printf("Dentista atendendo cliente %d\n",numCliente);
      usleep(100);

      //Acabou o atendimento
      printf("Acabou o atendimento do dentista no cliente %d\n",numCliente);
      sem_post(&liberado);
      pthread_mutex_lock(&lock_dentista);
         dentista_ocupado = 0;
      pthread_mutex_unlock(&lock_dentista);

   }
}

void* cliente(void* arg){
   int n = *((int *)arg);
   if(dentista_ocupado == 1){// ALTERE ESTA CONDICAO: DENTISTA OCUPADO
      if(ncadeiras > 0) { // ALTERAR ESTA CONDICAO: Se existem cadeiras livres
         pthread_mutex_lock(&lock_cadeira);
            printf("Cliente %d sentando na cadeira %d da fila de espera\n",n,ncadeiras--);
         pthread_mutex_unlock(&lock_cadeira);
      }
      else{
         printf("Consultorio lotado. Cliente %d ficou puto e foi embora\n",n);
         // Vai embora - como representar isso?
      }
   }
   // SEÇÃO CRÍTICA
   // RECEBEU SINAL DE QUE ESTÁ LIVRE
   sem_wait(&proximo);
   pthread_mutex_lock(&lock_dentista);
      dentista_ocupado = 1;
   pthread_mutex_unlock(&lock_dentista);

   pthread_mutex_lock(&lock_mudanca_cliente);
      numCliente = n;
      printf("Cliente %d sendo atendido\n",numCliente);
   pthread_mutex_unlock(&lock_mudanca_cliente);
   sem_post(&entrei);

   // Acabou o Atendimento - como representar isso?
   sem_wait(&liberado);
   printf("Cliente %d indo embora\n",numCliente);
   return NULL;
}

int main(int argc,char* argv[]){
   pthread_t tdentista,clientes[NTHREADS];
   int ids[NTHREADS],i,ncli;

   pthread_mutex_init(&lock_cadeira, NULL);
   pthread_mutex_init(&lock_dentista, NULL);
   pthread_mutex_init(&lock_mudanca_cliente, NULL);
   // pthread_cond_init(&proximo, NULL);
   sem_init(&proximo, 0, 0);
   sem_init(&liberado, 0, 0);
   sem_init(&entrei, 0, 0);

   if(argc<3){
      fprintf(stderr,"Uso %s <nclientes> <ncadeiras>\n",argv[0]);
      return 1;
   }
   ncli = atoi(argv[1]);
   tcadeiras = atoi(argv[2]);
   ncadeiras = tcadeiras;
   pthread_create(&tdentista,NULL,dentista,&ncli);
   for(i=0;i<atoi(argv[1]);i++){
      ids[i]=i;
      pthread_create(&clientes[i],NULL,cliente,&ids[i]);
   }

   for(i=0;i<atoi(argv[1]);i++){
      pthread_join(clientes[i],NULL);
   }
   pthread_join(tdentista,NULL);
   return 0;
}
