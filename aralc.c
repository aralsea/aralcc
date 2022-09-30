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

//エラーメッセージ
char *user_input;  //入力されたソースコード

//エラー箇所の報告
void error_at(char *loc, char *fmt, ...) {
    /** locがエラー該当箇所，fmtはフォーマット文字列 **/
    va_list ap;
    va_start(ap, fmt);

    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, " ");  // pos個の空白を出力
    fprintf(stderr, "^ ");
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
        error_at(token->str, "'%c'ではありません", op);
    token = token->next;
}

// 次のトークンが数値の場合、トークンを1つ読み進めてその数値を返す。
// それ以外の場合にはエラーを報告する。
int expect_number() {
    if (token->kind != TK_NUM) error_at(token->str, "数ではありません");
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

Token *tokenize() {
    char *p = user_input;
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while (*p) {
        if (isspace(*p)) {
            p++;
            continue;
        }
        if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' ||
            *p == ')') {
            cur = new_token(TK_RESERVED, cur, p++);
            //,cur, p);
            // p++; と書いても同じ
            continue;
        }
        // Q. new_token()にpを代入するとEOFが遠くの方にあるのでは？
        // A.はい．pは「そのトークンの始まる位置」なので
        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p);
            cur->val = strtol(p, &p, 10);
            continue;
        }

        error_at(p, "トークナイズできません．");
    }
    new_token(TK_EOF, cur, p);
    return head.next;
}
//トークナイザここまで

//パーサ
typedef enum { ND_ADD, ND_SUB, ND_MUL, ND_DIV, ND_NUM } NodeKind;

typedef struct Node Node;

struct Node {
    NodeKind kind;
    Node *lhs;  //左の子
    Node *rhs;  //右の子
    int val;    // kind=ND_NUMのとき，その値
};

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_num(int val) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    return node;
}

Node *expr();
Node *mul();
Node *unary();    //\pm primary
Node *primary();  // num or (expr)

Node *expr() {
    //今見てるトークンからexprの構文木を作成する
    //返り値はその木の根を表すNode
    Node *node = mul();
    while (1) {
        if (consume('+'))
            node = new_node(ND_ADD, node, mul());
        else if (consume('-'))
            node = new_node(ND_SUB, node, mul());
        else
            return node;
    }
}

Node *mul() {
    Node *node = unary();
    while (1) {
        if (consume('*'))
            node = new_node(ND_MUL, node, unary());
        else if (consume('/'))
            node = new_node(ND_DIV, node, unary());
        else
            return node;
    }
}
Node *unary() {
    if (consume('+')) {
        return primary();
    }
    if (consume('-')) {
        return new_node(ND_SUB, new_node_num(0), primary());
    }
    return primary();
}

Node *primary() {
    if (consume('(')) {
        Node *node = expr();
        expect(')');
        return node;
    }
    return new_node_num(expect_number());
}
//パーサ終わり

//アセンブリジェネレータ
void gen(Node *node) {
    // node を根とする構文木からアセンブリを生成する
    // 計算結果はスタックトップに保存される

    if (node->kind == ND_NUM) {
        printf("    push %d\n", node->val);
        return;
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("    pop rdi\n");
    printf("    pop rax\n");
    switch (node->kind) {
        case ND_ADD:
            printf("  add rax, rdi\n");
            break;
        case ND_SUB:
            printf("  sub rax, rdi\n");
            break;
        case ND_MUL:
            printf("  imul rax, rdi\n");
            break;
        case ND_DIV:
            printf("  cqo\n");
            printf("  idiv rdi\n");
            break;
    }

    printf("  push rax\n");
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "引数の個数が正しくありません．\n");
        return 1;
    }

    user_input = argv[1];
    token = tokenize();
    Node *node = expr();

    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");

    gen(node);

    printf("    pop rax\n");
    printf("    ret\n");
    return 0;
}