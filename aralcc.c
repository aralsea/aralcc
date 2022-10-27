#include "aralcc.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "引数の個数が正しくありません．\n");
        return 1;
    }

    //トークナイズ&パースし，結果をcodeに保存
    user_input = argv[1];
    token = tokenize();
    locals = calloc(1, sizeof(LVar));
    locals->next = NULL;
    locals->name = "";
    locals->len = 0;
    locals->offset = 0;
    program();

    //アセンブリの最初の部分
    printf(".intel_syntax noprefix\n");
    /*printf(".globl main\n");
    printf("main:\n");

    //プロローグ
    printf("    push rbp\n");
    printf("    mov rbp, rsp\n");
    printf("    sub rsp, %d\n", locals->offset);

    // higher_address <- [stack_base, $, ..., $,
    // $, 開始時のRBPの値, a, b, ..., y, z, $, $, ...] ->lower_address
    //         ^rbp                    ^rsp
    */
    //先頭のstatementから順にアセンブリ生成
    jump_label = 0;
    for (int i = 0; code[i] != NULL; i++) {
        codegen_func(code[i]);
        //この時点でcode[i]の計算結果がスタックトップに残っているので，pop
        // printf("    pop rax\n");
    }

    //この時点で higher_address <- [stack_base, $, ..., $,
    // $, 開始時のRBPの値, a, b, ..., y, z, $, $, ...] ->lower_address
    //         ^rbp                    ^rsp
    //かつ rax に最後の式の計算結果が入ってる

    //最後の式の結果がRAXにあるので，それを返り値とする（retはraxに入ってる値をreturnする命令）
    //このとき「開始時のRBPの値」をスタックに残さないようにpopしてからretする
    /*printf("    mov rsp, rbp\n");
    printf("    pop rbp\n");
    printf("    ret\n");*/
    return 0;
}