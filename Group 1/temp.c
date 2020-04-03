#include<stdio.h>
#include<stdlib.h>

int search(int arr[], int n){
    for(int i=0; i<100000; i++){
        if(arr[i] == n)
            return 1;
    }
    return 0;
}

void insert(int arr[], int n){
    for(int i=0; i<100000; i++){
        if(arr[i] == 0){
            arr[i] = n;
            break;
        }
    }
    return;
}

int main(){
    // FILE *input, *output;
    // input =  fopen("CC1.txt", "r");
    // output = fopen("address.txt", "w");

    int distinct[100000] = {0};

    // while(!feof(input)){
    //     int n;
    //     fscanf(input, "%x", &n);
    //     n = n>>10;
    //     fprintf(output, "%x\n", n);
    // }

    // fclose(input);
    // fclose(output);

    FILE* output;
    output = fopen("address.txt", "r");
    int count = 0;

    while(!feof(output)){
        int n;
        count++;
        fscanf(output, "%x", &n);
        if(search(distinct, n) == 0){
            insert(distinct, n);
        }
    }

    int dist = 0;
    printf("\n");
    for(int i=0; i<100000; i++){
        if(distinct[i] != 0){
            dist++;
            printf("%x\t", distinct[i]);
        }
    }
    printf("\n");


    printf("dist = %d, count = %d\n", dist, count);
    printf("%f\n", ((float)dist/(float)count)*100);

}