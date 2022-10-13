#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//トークナイザ
typedef enum {
    TK_RESERVED,             //記号
    TK_IDENT,                //識別子
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
Token *consume_ident();
void expect(char *op);
int expect_number();
bool at_eof();
bool is_alphabet(char c);
bool is_alphabet_num(char c);
Token *new_token(Tokenkind kind, Token *cur, char *str, int len);
Token *tokenize();
//トークナイザここまで

//パーサ
typedef enum {
    ND_ADD,     // +
    ND_SUB,     // -
    ND_MUL,     // *
    ND_DIV,     // /
    ND_NUM,     // 数
    ND_EQ,      // ==
    ND_NEQ,     // !=
    ND_LEQ,     // <
    ND_LNEQ,    // <=
    ND_ASSIGN,  //代入の'='
    ND_LVAR,    //ローカル変数

} NodeKind;

typedef struct Node Node;

struct Node {
    NodeKind kind;
    Node *lhs;   //左の子
    Node *rhs;   //右の子
    int val;     // kind = ND_NUM のとき，その値
    int offset;  // kind = ND_LVAR
                 // のとき，そのローカル変数のベースポインタからのoffset
};
Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(int val);

void *program();
Node *statement();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();    //\pm primary
Node *primary();  // num or ident or (expr)

typedef struct LVar LVar;

struct LVar {
    LVar *next;  //次のローカル変数
    char *name;  //変数名
    int len;     //変数名の長さ
    int offset;  // RBPからのオフセット
};
LVar *find_lvar(Token *tok);
//パーサここまで

//アセンブリコードジェネレータ
void gen_lval(Node *node);
void codegen(Node *node);
//アセンブリコードジェネレータここまで

//グローバル変数宣言
extern char *user_input;
extern Token *token;
extern Node *code[100];
extern LVar *locals;
//セミコロン区切りの文を表す木の値を，100個まで保存する配列，最後にはNULLポインタが入ってる．
//グローバル変数宣言ここまで
