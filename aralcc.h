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
    TK_RETURN,               // return
    TK_IF,                   // if
    TK_ELSE,                 // else
    TK_WHILE,                // while
    TK_FOR,                  // for
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
char *expect_ident();
bool at_eof();
bool is_alphabet(char c);
bool is_alphabet_num(char c);
Token *new_token(Tokenkind kind, Token *cur, char *str, int len);
Token *tokenize();
//トークナイザここまで

//パーサ
typedef enum {
    ND_ADD,       // +
    ND_SUB,       // -
    ND_MUL,       // *
    ND_DIV,       // /
    ND_NUM,       // 数
    ND_EQ,        // ==
    ND_NEQ,       // !=
    ND_LEQ,       // <
    ND_LNEQ,      // <=
    ND_ASSIGN,    //代入の'='
    ND_LVAR,      //ローカル変数
    ND_RETURN,    // return
    ND_IF,        // if
    ND_ELSE,      // else
    ND_WHILE,     // while
    ND_FOR,       // for
    ND_BLOCK,     // {statement statement ...}
    ND_FUNCCALL,  // 関数呼び出し
    ND_ADDR,      //ポインタの&
    ND_DEREF,     //ポインタの*

} NodeKind;

typedef struct Node Node;
struct Node {
    NodeKind kind;
    Node *next;  // {...}内のstatementのroot nodeの場合，その次のstatementのroot
                 // nodeをもつ
    Node *lhs;  //左の子
    Node *rhs;  //右の子

    int val;     // kind = ND_NUM のとき，その値
    int offset;  // kind = ND_LVAR
                 // のとき，そのローカル変数のベースポインタからのoffset
    char *funcname;  // kind = ND_FUNCのとき，その名前，番兵付き
    Node *arg[6];    //引数を格納する配列
    int argnum;      //引数の個数

    /*if文，while文，for文用のノード*/
    Node *condition;
    Node *then;
    Node *els;
    Node *init;
    Node *inc;

    // blockの中身，ひとつ次のstatementはbody->nextで参照する
    Node *body;
};

typedef struct LVar LVar;
struct LVar {
    LVar *next;  //次のローカル変数
    char *name;  //変数名
    int len;     //変数名の長さ
    int offset;  // RBPからのオフセット
};

typedef struct Function Function;
struct Function {
    char *name;    //関数名
    Node *node;    //{...}の中身
    LVar *locals;  //関数内ローカル変数の一覧
    int argnum;    //引数の個数
};

Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(int val);

void program();
Node *statement();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();    //\pm primary
Node *primary();  // num or ident or (expr)

LVar *find_lvar(Token *tok);
//パーサここまで

//アセンブリコードジェネレータ
void gen_lval(Node *node);
void codegen(Node *node);
void codegen_func(Function *func);
//アセンブリコードジェネレータここまで

//グローバル変数宣言
extern char *user_input;
extern Token *token;
extern Function *code
    [100];  //セミコロン区切りの文を表す木の値を，100個まで保存する配列，最後にはNULLポインタが入ってる．
extern LVar *locals;
extern int jump_label;  //アセンブリにおけるジャンプ先をラベルするためのカウンタ
//グローバル変数宣言ここまで
