#include <stdio.h>
int main(){
    int n, t;
    printf("Enter N: ");
    scanf("%d", &n);
    printf("Prime Numbers: ");
    for(int i = 2; i <= n; i++){
        t = 0;
        for(int j = 2; j < i; j++){
            if(i % j == 0){
                t++;
            }
        }
        if(t==0){
            printf("%d, ",i);
        }
    }
}