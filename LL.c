#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

struct Node;
typedef struct Node{
    int data;
    struct Node* next;
}Node;


int main()
{
    Node* listhead = malloc(sizeof(Node));
    listhead->data = 5;
    listhead->next = NULL;


    for(int i = 4; i > 0; i--)
    {
        Node* tmp = listhead;
        listhead = malloc(sizeof(Node));
        listhead->data = i;
        listhead->next = tmp;
    }

    FILE * fp = fopen("LLoutput.txt","w");

    fprintf(fp, "LinkedList");
    # ifdef _OPENMP 
	printf("Compiled by an OpenMP-compliant implementation.\n");
    # endif
    #pragma omp parallel 
    { 
        #pragma omp single
        { 
            Node* p = listhead; 
            while (p != NULL) 
            {
                #pragma omp task firstprivate(p)
                { 
                    char c = p->data + '0';
                    printf("%c\n", c);
                } 
                p = p->next; 
            } 
        } 
    }
    fclose(fp);
    return 0;
}
