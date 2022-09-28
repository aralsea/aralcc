#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef enum {
    TK_RESERVED,             //記号
    TK_NUM,                  //整数
    TK_EOF                   // 終端
} Tokenkind;                 //トークンの種類を表すenum
typedef struct Token Token;  //これがないとTokenの初期化時に struct Token{*, *,
                             //*}と書く必要がある

struct Token {
    Tokenkind kind;
    Token *next;
    int val;    //整数の場合，その値
    char *str;  //トークンの文字列
};

// トークナイザの実装
Token *token;  //今見てるトークン

// エラーを報告するための関数
// printfと同じ引数を取る
void error(char *fmt, ...) {
    va_list ap;  //可変個の引数を扱うためのリスト
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// 次のトークンが期待している記号のときには、トークンを1つ読み進めて
// 真を返す。それ以外の場合には偽を返す。
bool consume(char op) {
    if (token->kind != TK_RESERVED || token->str[0] != op) return false;
    token = token->next;
    return true;
}

// 次のトークンが期待している記号のときには、トークンを1つ読み進める。
// それ以外の場合にはエラーを報告する。
void expect(char op) {
    if (token->kind != TK_RESERVED || token->str[0] != op)
        error("'%c'ではありません", op);
    token = token->next;
}

// 次のトークンが数値の場合、トークンを1つ読み進めてその数値を返す。
// それ以外の場合にはエラーを報告する。
int expect_number() {
    if (token->kind != TK_NUM) error("数ではありません");
    int val = token->val;
    token = token->next;
    return val;
}

//今見てるトークンがEOFかどうか
bool at_eof() {
    return token->kind == TK_EOF;
}

//新しいトークンを作成しcurに繋げ，その繋げたトークンへのポインタを返す
Token *new_token(Tokenkind kind, Token *cur, char *str) {
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    cur->next = tok;
    return tok;
}

Token *tokenize(char *p) {
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while (*p) {
        if (isspace(*p)) {
            p++;
            continue;
        }
        if (*p == '+' || *p == '-') {
            cur = new_token(TK_RESERVED, cur, p++);
            //,cur, p);
            // p++; と書いても同じ
            continue;
        }
        // Q. new_token()にpを代入するとEOFが遠くの方にあるのでは？
        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p);
            cur->val = strtol(p, &p, 10);
            continue;
        }

        error("トークナイズできません．");
    }
    new_token(TK_EOF, cur, p);
    return head.next;
}
int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "引数の個数が正しくありません．\n");
        return 1;
    }

    token = tokenize(argv[1]);

    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");

    printf("    mov rax, %d\n", expect_number());

    while (!at_eof()) {
        if (consume('+')) {
            printf("    add rax, %d\n", expect_number());
        }

        expect('-');
        printf("    sub rax, %d\n", expect_number());
    }
    printf("    ret\n");
    return 0;
}