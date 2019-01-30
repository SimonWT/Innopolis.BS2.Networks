#include <stdio.h>
#include <unistd.h>
#include <string.h>

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
        char com[10];
        int command_num=0;
        int arg_chars[100];

        printf("%s  -  %i \n",buf,  strlen(buf));

        while(buf[i]!=' ' && buf[i]!=EOF && (strlen(buf) > i )) {
                com[i] = buf[i];
                i++;
        }

        printf("Validate: %s\n", com );

        //compare strings
        if(strcmp(com, "push")==0) {
                command_num = 1;
        }else if(strcmp(com, "peek")==0) {
                command_num = 2;
        }else if(strcmp(com, "display")==0) {
                command_num = 3;
        }else if(strcmp(com, "empty")==0) {
                command_num = 4;
        }else if(strcmp(com, "create")==0) {
                command_num = 5;
        }else if(strcmp(com, "stack_size")==0) {
                command_num = 6;
        }else if(strcmp(com, "pop")==0) {
                command_num = 7;
        }else{
                printf("[Error]: Unknown command\n");
        }

        //strcmp()
        if(strlen(buf)>=i) {
                i++;
                int j = 0;
                while(strlen(buf)>i) {
                        arg_chars[j] = buf[i];
                        i++;
                        j++;
                }
        }

        *arg = atoi(arg_chars);
        return command_num;
}

void client(int pipefd[]){
        //reading commands
        char buf[50];
        int comm;
        int arg;
        char pipebuf[50];

        close(pipefd[0]);
        printf("Enter your command: \n");

        scanf("%s", buf);
        printf("Scanned: %s\n", buf );
        comm = validate(buf, &arg);
        printf("Comm num = %i\n", comm);
        write(pipefd[1], comm, 1);
        write(pipefd[1], arg, 4);

}

void server(int pipefd[]){

        char buf[50];
        close(pipefd[1]);
        read(pipefd[0], buf, 50);
        int comm = atoi(buf[0]);
        int arg;
        char arg_chars[49];

        //Stack
        struct stack stack;
        stack.capacity = 0;

        int i=0;
        if(strlen(buf)>=i) {
                i++;
                int j = 0;
                while(strlen(buf)>i) {
                        arg_chars[j] = buf[i];
                        i++;
                        j++;
                }
        }

        arg = atoi(arg_chars);
        printf("arg = %i\n",arg);
        switch ( comm ) {
        case 1:
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

        //printf("Recived: %c\n", buf[0]);
}


int main(){
        int pid;
        int pipefd[2];

        pipe(pipefd);

        pid = fork();

        if(pid!=0) {
                //parent
                printf("hello, im parent\n");
                client(pipefd);

        }else{
                //child
                printf("hello, im child\n");
                server(pipefd);
        }
        return 0;
}
