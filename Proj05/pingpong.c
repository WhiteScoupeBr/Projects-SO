#include <stdio.h>
#include <stdlib.h>
#include "pingpong.h"
#include "queue.h"
#include <signal.h>
#include <sys/time.h>

// operating system check
#if defined(_WIN32) || (!defined(__unix__) && !defined(__unix) && (!defined(__APPLE__) || !defined(__MACH__)))
#warning Este codigo foi planejado para ambientes UNIX (LInux, *BSD, MacOS). A compilacao e execucao em outros ambientes e responsabilidade do usuario.
#endif

#define STACKSIZE 32768


ucontext_t contextMain;
task_t *taskAtual;
task_t *taskMain;
task_t *pronta,*suspensa,*terminada;
task_t dispatcher;

struct itimerval timer;
struct sigaction action ;


/*****************************************************/

void tratador ();


task_t * scheduler(){
<<<<<<< Updated upstream

	pronta=pronta->next;
	pronta->quantum=20;
	return pronta;

}

void task_yield(){
	
	task_switch(&dispatcher);

}

void tratador(int signum){
	taskAtual->quantum--;
	if(taskAtual->quantum==0){
		task_yield();
	}
}

void dispatcher_body (){ // dispatcher é uma tarefa

   pronta=pronta->prev;
   task_t* next;
   while ( queue_size((queue_t*) pronta) > 0 )
   {
      next = scheduler() ; // scheduler é uma função
      if (next)
      {
         task_switch (next) ;
      }
   }
   printf("Dispacther");
 task_exit(0) ; // encerra a tarefa dispatcher
}

void pingpong_init () {

	taskMain = (task_t*)(malloc(sizeof(task_t)));
	taskMain->tid = 0;
	taskMain->context = contextMain;
	taskAtual = taskMain;

	task_create(&dispatcher,dispatcher_body,"Dispatcher");


	action.sa_handler = tratador ;
   sigemptyset (&action.sa_mask) ;
   action.sa_flags = 0 ;
   if (sigaction (SIGINT, &action, 0) < 0)
   {
      perror ("Erro em sigaction: ") ;
      exit (1) ;
   }
  // ajusta valores do temporizador
	timer.it_value.tv_usec = 0 ;      // primeiro disparo, em micro-segundos
	timer.it_value.tv_sec  = 0 ;      // primeiro disparo, em segundos
	timer.it_interval.tv_usec = 1000 ;   // disparos subsequentes, em micro-segundos
	timer.it_interval.tv_sec  = 0 ;   // disparos subsequentes, em segundos

	// arma o temporizador ITIMER_REAL (vide man setitimer)
	if (setitimer (ITIMER_REAL, &timer, 0) < 0)
	{
		perror ("Erro em setitimer: ") ;
		exit (1) ;
	}

	setvbuf (stdout, 0, _IONBF, 0) ;
}

int task_create (task_t *task, void (*start_routine)(void *), void *arg){

	static int id=0;
	char *stack ;

	id++;

	task->args = arg;
	task->tid = id;
	task->state= 2;
	task->prio =0;
	task->prioD = 0;
	task->quantum=20;
	getcontext (&task->context);

	stack = malloc (STACKSIZE) ;
	if (stack){
		task->context.uc_stack.ss_sp = stack ;
		task->context.uc_stack.ss_size = STACKSIZE;
		task->context.uc_stack.ss_flags = 0;
		task->context.uc_link = 0;
	}
	else{
		perror ("Erro na criação da pilha: ");
		exit (1);
	}
	
	makecontext (&task->context,(void *)(*start_routine), 1, arg);

	if(task!=&dispatcher)
		queue_append((queue_t**)&pronta,(queue_t*)task);


	return id;
}

int task_switch (task_t *task){

	if (task){
		ucontext_t *aux= &taskAtual->context;
		taskAtual= task;
		taskAtual->state=4;
		swapcontext(aux, &task->context);
		
	}
	else
		return -1;
}

void task_exit (int exit_code){
	
		
	ucontext_t *aux= &taskAtual->context;
	taskAtual->state=5;
	if(taskAtual==&dispatcher){
		taskAtual=taskMain;
	}
	else{
		queue_remove((queue_t**)&pronta,(queue_t*)taskAtual);
		queue_append((queue_t**)&terminada,(queue_t*)taskAtual);
		
		pronta=pronta->prev;
		taskAtual=&dispatcher;
	}

	swapcontext(aux, &taskAtual->context);
}

int task_id (){
	return taskAtual->tid;
}

void task_setprio (task_t *task, int prio){
	task_t * ptr = task;
	if(prio>-20&&prio<20){
		if(ptr==NULL){
			taskAtual->prio=prio;
			taskAtual->prioD=prio;
			return;
		}
		else{
			ptr->prio=prio;
			ptr->prioD=prio;
			return;
		}
	}
	else
	{
		printf("Prioridade invalida");
	}
}

int task_getprio (task_t *task){
	task_t * ptr = task;
	if(ptr==NULL){
		return taskAtual->prio;
	}
	else{
		return ptr->prio;
	}
	
}

/*
void task_suspend(task_t *task, task_t **queue){

	if(task==NULL){
		if(queue_size((queue_t*) queue)!=NULL){
			queue_remove ((queue_t**) &exec, (queue_t*) &taskAtual) ;
		}
		queue_append ((queue_t **) &queue, (queue_t*) &taskAtual);
		taskAtual->state = 3;
	}
	else{
		if(queue_size((queue_t*) queue)!=NULL){
			queue_remove ((queue_t**) &exec, (queue_t*) &task) ;
		}
		queue_append ((queue_t **) &queue, (queue_t*) &task);
		task->state = 3;
	}
	
}

void task_resume (task_t *task){

	queue_remove ((queue_t**) &suspensa, (queue_t*) &task) ;

	queue_append ((queue_t **) &pronta, (queue_t*) &task);
	task->state=2;
}
*/

