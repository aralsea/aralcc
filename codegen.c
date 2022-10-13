#include "aralcc.h"

//アセンブリジェネレータ
void gen_lval(Node *node) {
    // nodeを左辺値と解釈し，それが表すアドレスをスタックにpushする
    if (node->kind != ND_LVAR) {
        fprintf(stderr, "代入の左辺値が変数ではありません．");
    }
    printf("    mov rax, rbp\n");
    printf("    sub rax, %d\n", node->offset);
    printf("    push rax\n");
}
void codegen(Node *node) {
    // node
    // を根とする構文木から，「その式が表す値の計算結果をスタックトップに保存する」アセンブリを生成する
    //代入式の場合は,「代入を実行し，さらに代入される値をスタックトップに保存する」アセンブリを生成する
    switch (node->kind) {
        case ND_NUM:
            printf("    push %d\n", node->val);
            return;
        case ND_LVAR:
            gen_lval(
                node);  //この時点で今見てるnodeを左辺値として解釈した時のアドレスがスタックトップにいる
            printf("    pop rax\n");  //そのアドレスをraxにpop
            printf("    mov rax, [rax]\n");  // raxにそのアドレスにいる値を代入
            printf("    push rax\n");
            return;
        case ND_ASSIGN:
            gen_lval(node->lhs);  //左辺のアドレスがスタックトップにくる
            codegen(
                node->rhs);  //右辺の計算結果がさらにスタックトップにpushされる
            printf("    pop rdi\n");
            printf("    pop rax\n");
            printf("    mov [rax], rdi\n");
            printf("    push rdi\n");
            return;
    }

    codegen(node->lhs);
    codegen(node->rhs);

    printf("    pop rdi\n");
    printf("    pop rax\n");
    switch (node->kind) {
        case ND_ADD:
            printf("    add rax, rdi\n");
            break;
        case ND_SUB:
            printf("    sub rax, rdi\n");
            break;
        case ND_MUL:
            printf("    imul rax, rdi\n");
            break;
        case ND_DIV:
            printf("    cqo\n");
            printf("    idiv rdi\n");
            break;
        case ND_EQ:
            printf("    cmp rax, rdi\n");
            printf("    sete al\n");
            printf("    movzb rax, al\n");
            break;
        case ND_NEQ:
            printf("    cmp rax, rdi\n");
            printf("    setne al\n");
            printf("    movzb rax, al\n");
            break;
        case ND_LNEQ:
            printf("    cmp rax, rdi\n");
            printf("    setl al\n");
            printf("    movzb rax, al\n");
            break;
        case ND_LEQ:
            printf("    cmp rax, rdi\n");
            printf("    setle al\n");
            printf("    movzb rax, al\n");
            break;
    }

    printf("    push rax\n");
}