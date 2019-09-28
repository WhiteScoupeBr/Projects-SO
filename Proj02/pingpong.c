#include <stdio.h>
#include <stdlib.h>
#include "pingpong.h"

// operating system check
#if defined(_WIN32) || (!defined(__unix__) && !defined(__unix) && (!defined(__APPLE__) || !defined(__MACH__)))
#warning Este codigo foi planejado para ambientes UNIX (LInux, *BSD, MacOS). A compilacao e execucao em outros ambientes e responsabilidade do usuario.
#endif

#define STACKSIZE 32768
ucontext_t ContextMain;
task_t *taskAtual;
task_t *taskMain;


/*****************************************************/
void pingpong_init () {

	taskMain = (task_t*)(malloc(sizeof(task_t)));
	taskMain->tid = 0;
	taskMain->context = ContextMain;
	taskAtual = taskMain;

	setvbuf (stdout, 0, _IONBF, 0) ;
}

int task_create (task_t *task, void (*start_routine)(void *), void *arg){

	static int id=0;
	char *stack ;
	ucontext_t aux;

	id++;

	task->args = arg;
	task->tid = id;

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
	
	#ifdef DEBUG
	printf ("task_exit: tarefa %d sendo encerrada \n", taskAtual->tid) ;
	#endif
	ucontext_t *aux= &taskAtual->context;
	taskAtual= taskMain;
	swapcontext(aux, &taskMain->context);
}

int task_id (){
	return taskAtual->tid;
}
