#include "uart.h"
#include "string.h"

void input(char *command) {
    int index = 0;
    char tmp;
    do {
        tmp = uart_getc();
        command[index] = (tmp != '\n')? tmp : '\0';
        uart_send(tmp);
        index++;
    } while(tmp != '\n');
}

int slice_str(char *input, char *output, char *filename, int length) {
    for (int i = 0; i < length; i++) {
        if (input[i] == '\0') {
            output[i] = '\0';
            return 0;
        }
        else {
            output[i] = input[i];
        }
        if (i == length-1) output[i+1] = '\0';
    }
    

    int index = 0;
    while (1) {
        if (!input[length+1+index]) {
            filename[index] = '\0';
            return 1;
        }
        filename[index] = input[length+1+index];
        index++;
    }
}

int atoi(char* s) {
    int i = 0;
    int num = 0;
    int negative = 0;

    while(s[i] != '\0') {
        if(i == 0 && s[i] == '-') {
            negative = 1;
        }
        else {
            int digit = s[i] - '0';
            num = num*10 + digit;
        }
        i++;
    }
    
    if(negative) return num *= -1;
    return num;
}


void itoa(int num, char *s) {
    int i = 0;
    if(num == 0)
        s[i++] = '0';
    while(num != 0) {
        int digit = num % 10;
        s[i++] = digit + '0';
        num /= 10;
    }
    s[i] = '\0';
    
    // reverse s
    for(int j = i-1; j >= i/2; j--) {
        char temp = s[j];
        s[j] = s[i-1-j];
        s[i-1-j] = temp;
    }
}

void print(int x) {
    char str[20];
    itoa(x, str);
    uart_puts(str);
    uart_puts("\n");
}