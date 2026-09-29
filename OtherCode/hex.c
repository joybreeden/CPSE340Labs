#include <stdio.h>

int main(){
    int num;
    printf("Enter Hex Number in 0x00 format: ");
    scanf("%x",&num); 
      
    for(int i =0; i < 8; i++){
        printf("%d", num %2);
        num = num /2;
    }
}
