#include <stdio.h>
#include <stdlib.h>
#include "pingpong.h"
#include "queue.h"

// operating system check
#if defined(_WIN32) || (!defined(__unix__) && !defined(__unix) && (!defined(__APPLE__) || !defined(__MACH__)))
#warning Este codigo foi planejado para ambientes UNIX (LInux, *BSD, MacOS). A compilacao e execucao em outros ambientes e responsabilidade do usuario.
#endif

#define STACKSIZE 32768
ucontext_t contextMain;
task_t *taskAtual;
task_t *taskMain;
task_t *nova,*pronta,*exec,*suspensa,*terminada;
task_t dispatcher;


/*****************************************************/

task_t * scheduler(){

    pronta=pronta->next; 
    return pronta;
}


void dispatcher_body (){ // dispatcher é uma tarefa

   //int userTasks =  ;
   task_t* next;
   while ( queue_size((queue_t*) pronta) > 0 )
   {
      next = scheduler() ; // scheduler é uma função
      if (next)
      {
 
         task_switch (next) ;
      }
   }
   printf("ENCERRANDO O DISPA\n");
 task_exit(0) ; // encerra a tarefa dispatcher
}

void pingpong_init () {

	setvbuf (stdout, 0, _IONBF, 0) ;

	taskMain = (task_t*)(malloc(sizeof(task_t)));
	taskMain->tid = 0;
	taskMain->context = contextMain;
	taskAtual = taskMain;

	task_create(&dispatcher,dispatcher_body, "Dispatcher");

	
}

int task_create (task_t *task, void (*start_routine)(void *), void *arg){

	static int id=0;
	char *stack ;

	id++;

	task->args = arg;
	task->tid = id;
	task->state=2;
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

	task->prev=NULL;
	task->next=NULL;

	queue_append ((queue_t **) &pronta, (queue_t*) task) ;

	#ifdef DEBUG
	printf("task_create: criou tarefa %d\n", taskAtual->tid);
	#endif

	return id;
}

int task_switch (task_t *task){

	if (task){
		ucontext_t *aux= &taskAtual->context;
		taskAtual= task;
		swapcontext(aux, &task->context);

		#ifdef DEBUG
		printf ("task_switch: trocando contexto %d -> %d\n", taskAtual->tid, task->tid ) ;
		#endif
	}
	else
		return -1;
}

void task_exit (int exit_code){
	
	/*#ifdef DEBUG
	printf ("task_exit: tarefa %d sendo encerrada \n", taskAtual->tid) ;
	#endif*/

	ucontext_t *aux= &taskAtual->context;

	if(taskAtual!=&dispatcher){
		queue_remove ((queue_t**) &pronta, (queue_t*) taskAtual) ;
		pronta=pronta->prev;
		//queue_append ((queue_t **) &terminada, (queue_t*) &taskAtual);
		taskAtual=&dispatcher;	
	}
	else {

		taskAtual=taskMain;
	}
	
	swapcontext(aux,&taskAtual->context);	
}

int task_id (){
	return taskAtual->tid;
}

void task_yield(){
	
	task_switch(&dispatcher);

}



/*void task_suspend(task_t *task, task_t **queue){

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