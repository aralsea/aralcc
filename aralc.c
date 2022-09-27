#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv){
    if(argc != 2){
        fprintf(stderr, "引数の個数が正しくありません．\n");
        return 1;
    }

    char *p = argv[1];

    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");
    printf("    mov rax, %ld\n", strtol(p, &p, 10));
    //strtol(p, &p, 10)は「pを前から見ていって数として解釈できなくなるところにあたったら10進数変換し，文字列のそれ以降を再びpに格納する」という意味．第二引数はchar **型でないといけない．
    while(*p){
        //ヌル文字に当たるまでインクリメントし続ける
        if(*p == '+'){
            p++;
            printf("    add rax, %ld\n", strtol(p, &p, 10));
            continue;
        }
        if(*p == '-'){
            p++;
            printf("    sub rax, %ld\n", strtol(p, &p, 10));
            continue;
        }

        fprintf(stderr, "予期しない文字です: '%c'\n", *p);
        return 1;
    }
    printf("    ret\n");
    return 0;
}