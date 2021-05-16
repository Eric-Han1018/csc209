#include <stdio.h>

int main(void){
    char phone[11];
    int num;
    scanf("%s %d", phone, &num);
    if(num >= 0 && num <= 9){
        printf("%c\n", phone[num]);
    }
    else if(num == -1){
        printf("%s\n", phone);
    }
    else{
        printf("ERROR\n");
    }

    while(scanf("%d", &num) != EOF){
        if(num >= 0 && num <= 9){
            printf("%c\n", phone[num]);
        }
        else if(num == -1){
            printf("%s\n", phone);
        }
        else{
            printf("ERROR\n");
        }
    }
    
return 0;
}