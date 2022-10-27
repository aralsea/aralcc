#include "aralcc.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "引数の個数が正しくありません．\n");
        return 1;
    }

    //トークナイズ&パースし，結果をcodeに保存
    user_input = argv[1];
    token = tokenize();
    program();

    //アセンブリの最初の部分
    printf(".intel_syntax noprefix\n");

    //先頭のfunctionから順にアセンブリ生成
    jump_label = 0;
    for (int i = 0; code[i] != NULL; i++) {
        codegen_func(code[i]);
    }
    return 0;
}