#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

struct stack {
        int data[50];
        int capacity;
        int size;
};

void push(struct stack *stack, int element){
        stack->size++;
        int size = stack->size;
        int capacity = stack->capacity;
        if(capacity == 0) {
                printf("Stack is not created\n");
        }else if (capacity > size) {
                stack->data[size-1] = element;
        }else{
                //Reallocate data[]
                printf("Stack is full\n");
        }
}

void pop(struct stack *stack){
        int size = stack->size;
        int capacity = stack->capacity;
        if(capacity == 0) {
                printf("Stack is not created\n");
        }else if(size == 0 ) {
                printf("Stack is empty\n" );
        }else{
                size--;
                stack->size = size;
        }
}

int peek(struct stack *stack){
        return stack->data[stack->data[stack->size-1]];
}

int empty(struct stack *stack){
        return stack->size==0;
}

int stack_size(struct stack *stack){
        return stack->size;
}

void create(struct stack *stack){
        stack->capacity=50;
        stack->size=0;
}

void display(struct stack *stack){
        printf("Stack: ");
        for (int i=0; i<stack->size; i++) {
                printf("%i ", stack->data[i]);
        }
        printf("\n");
}

int validate(char buf[], int* arg){
        int i=0;
        int command_num=0;

        //compare strings
        if(strcmp(buf, "push")==0) {
                command_num = 1;
        }else if(strcmp(buf, "peek")==0) {
                command_num = 2;
        }else if(strcmp(buf, "display")==0) {
                command_num = 3;
        }else if(strcmp(buf, "empty")==0) {
                command_num = 4;
        }else if(strcmp(buf, "create")==0) {
                command_num = 5;
        }else if(strcmp(buf, "stack_size")==0) {
                command_num = 6;
        }else if(strcmp(buf, "pop")==0) {
                command_num = 7;
        }else{
                printf("[Error]: Unknown command\n");
        }
        return command_num;
}



void server(int pipefd[], int parent_pid){
        char buf;
        close(pipefd[1]);

        while(1) {
                pause();
                printf("Server...........\n");
                read(pipefd[0], buf, 1);
                int comm = atoi(buf);
                printf("Recived comm = %i; Char = %c\n", comm, buf );
                int arg;

                //Stack
                struct stack stack;
                stack.capacity = 0;

                switch ( comm ) {
                case 1:
                        printf("what to push?: \n");
                        scanf("%i\n", arg);
                        printf("Pushing: %i»\n", arg);
                        push(&stack, arg);
                        break;
                case 2:
                        printf("Peek: %i\n", peek(&stack));
                        break;
                case 3:
                        display(&stack);
                        break;
                case 4:
                        printf( "is Empty? %i\n", empty(&stack));
                        break;
                case 5:
                        printf( "Create stack...\n");
                        create(&stack);
                        break;
                case 6:
                        printf( "Stack size: %i\n", stack_size(&stack));
                        break;
                case 7:
                        printf( "Pop: %i\n", peek(&stack));
                        pop(&stack);
                        break;
                default:
                        printf( "Неправильный ввод.\n" );
                }
                kill( parent_pid, SIGCONT );
        }
}

void client(int pipefd[], int child_pid){
        //reading commands
        char buf[50];
        int comm;
        int arg;
        char pipebuf[50];
        close(pipefd[0]);
        while(1) {
                printf("Enter your command: \n");

                scanf("%s", buf);
                printf("Scanned: %s\n", buf );
                comm = validate(buf, &arg);
                printf("Comm num = %i\n", comm);

                write(pipefd[1], comm, 1);
                pause();
        }

}


int main(){
        int pid;
        int pipefd[2];

        pipe(pipefd);

        pid = fork();

        if(pid!=0) {
                //parent
                printf("hello, im parent\n");
                client(pipefd, pid);

        }else{
                //child
                printf("hello, im child\n");
                server(pipefd, getpid());
        }
        return 0;
}
