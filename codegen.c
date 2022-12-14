#include "aralcc.h"

int jump_label;
char *funcname;
char *argreg[6] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

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
    //代入式の場合は,「代入を実行し，さらに代入された値をスタックトップに保存する」アセンブリを生成する
    //ただしreturn文のときはスタックトップをpopしてret文にjmpする

    switch (node->kind) {
        case ND_BLOCK:
            Node *cur = node->body;
            while (cur != NULL) {
                codegen(cur);
                if (cur->next != NULL) {
                    printf("    pop rax\n");
                }
                cur = cur->next;
            }
            return;
        case ND_RETURN:
            codegen(node->lhs);
            //ここで終わってしまうとこのgen()が再帰で呼ばれたときにさらに処理が進んでしまう
            //例えば"a=1; return a; return
            // a+1;"のとき2個目のreturnを見に行ってしまう
            //そこで以下にret文を出力することで打ち切る（アセンブリの出力は続くが，実行は最初のret文で終わる）
            // main関数が出力するret文と被るが，ret文は一回しか通らないのでとりあえずok
            printf("    pop rax\n");
            printf("    jmp .Lend.%s\n", funcname);
            return;
        case ND_IF: {
            int label = jump_label++;
            if (node->els != NULL) {
                codegen(
                    node->condition);  //この時点でスタックトップにconditionの計算結果がある
                printf("    pop rax\n");
                printf("    cmp rax, 0\n");
                printf("    je .Lelse%d\n", label);

                codegen(node->then);  // thenの計算結果
                printf("    jmp .Lend%d\n", label);

                printf(".Lelse%d:\n", label);
                codegen(node->els);

                printf(".Lend%d:\n", label);

            } else {
                codegen(node->condition);
                printf("    pop rax\n");
                printf("    cmp rax, 0\n");
                printf("    je .Lend%d\n", label);
                codegen(node->then);  // thenの計算結果
                printf(".Lend%d:\n", label);
            }
            return;
        }
        case ND_WHILE: {
            int label = jump_label++;

            printf(".Lbegin%d:\n", label);
            codegen(node->condition);
            printf("    pop rax\n");
            printf("    cmp rax, 0\n");
            printf("    je .Lend%d\n", label);
            codegen(node->then);  // thenの計算結果
            printf("    jmp .Lbegin%d\n", label);
            printf(".Lend%d:\n", label);

            return;
        }
        case ND_FOR: {
            int label = jump_label++;

            if (node->init != NULL) {
                codegen(node->init);
            }

            printf(".Lbegin%d:\n", label);
            codegen(node->condition);
            printf("    pop rax\n");
            printf("    cmp rax, 0\n");
            printf("    je .Lend%d\n", label);
            codegen(node->then);
            if (node->inc) {
                codegen(node->inc);
            }
            printf("    jmp .Lbegin%d\n", label);
            printf(".Lend%d:\n", label);

            return;
        }
        case ND_NUM:
            printf("    push %d\n", node->val);
            return;
        case ND_LVAR:
            gen_lval(
                node);  //この時点で今見てるnodeを左辺値として解釈した時のアドレスがスタックトップにいる
            printf("    pop rax\n");  //そのアドレスをraxにpop
            printf(
                "    mov rax, [rax]\n");  // raxにそのアドレスにいる値を代入，未定義変数なら何が入ってるかわからない
            printf("    push rax\n");
            return;
        case ND_FUNCCALL:
            for (int i = 0; i < node->argnum; i++) {
                codegen(node->arg[i]);
                //この時点でスタックトップにはarg[i]の値が入ってる
                // x86-64
                // linuxでは初めの6個の引数は指定されたレジスタに格納された状態で関数呼び出しをすると決まっている(ABI参照)
                printf("    pop %s\n", argreg[i]);
            }

            printf(
                "    call %s\n",
                node->funcname);  // x86-64のABIによると，返り値はraxに入ってる
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
        case ND_ADDR:
            gen_lval(node->lhs);
            return;
        case ND_DEREF:
            codegen(node->lhs);
            printf("    pop rax\n");
            printf("    mov rax, [rax]\n");
            printf("    push rax\n");
            return;
    }

    //以下二項演算子をまとめたもの
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

void codegen_func(Function *func) {
    printf(".globl %s\n", func->name);
    printf("%s:\n", func->name);

    //プロローグ
    printf("    push rbp\n");
    printf("    mov rbp, rsp\n");
    printf("    sub rsp, %d\n", func->locals->offset);
    // higher_address <- [stack_base, $, ..., $,
    // $, 開始時のRBPの値, a, b, ..., y, z, $, $, ...] ->lower_address
    //         ^rbp                    ^rsp

    //引数の値をレジスタから取り出してスタックの該当箇所に保存
    for (int i = 0; i < func->argnum; i++) {
        printf("    mov rax, rbp\n");
        printf("    sub rax, %d\n", (i + 1) * 8);
        printf("    mov [rax], %s\n", argreg[i]);
    }

    //関数本体のアセンブリを生成
    funcname = func->name;
    codegen(func->node);

    //この時点で higher_address <- [stack_base, $, ..., $,
    // $, 開始時のRBPの値, a, b, ..., y, z, $, $, ...] ->lower_address
    //         ^rbp                    ^rsp
    //かつ rax に最後の式の計算結果が入ってる

    //最後の式の結果がRAXにあるので，それを返り値とする（retはraxに入ってる値をreturnする命令）
    //このとき「開始時のRBPの値」をスタックに残さないようにpopしてからretする
    printf(".Lend.%s:\n", funcname);
    printf("    mov rsp, rbp\n");
    printf("    pop rbp\n");
    printf("    ret\n");
}