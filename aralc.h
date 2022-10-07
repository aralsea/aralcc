#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//トークナイザ
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
    int len;    //トークンの文字列の長さ
};

void error_at(char *loc, char *fmt, ...);
bool consume(char *op);
void expect(char *op);
int expect_number();
bool at_eof();
Token *new_token(Tokenkind kind, Token *cur, char *str, int len);
Token *tokenize();
//トークナイザここまで

//パーサ
typedef enum {
    ND_ADD,
    ND_SUB,
    ND_MUL,
    ND_DIV,
    ND_NUM,
    ND_EQ,
    ND_NEQ,
    ND_LEQ,

    ND_LNEQ,

} NodeKind;

typedef struct Node Node;

struct Node {
    NodeKind kind;
    Node *lhs;  //左の子
    Node *rhs;  //右の子
    int val;    // kind=ND_NUMのとき，その値
};
Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(int val);
Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();    //\pm primary
Node *primary();  // num or (expr)
//パーサここまで

//アセンブリコードジェネレータ
void codegen(Node *node);
//アセンブリコードジェネレータここまで

//グローバル変数宣言
extern char *user_input;
extern Token *token;
//グローバル変数宣言ここまで
