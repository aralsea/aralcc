#include "aralc.h"

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

// 次のトークンが期待している記号（文字列）のときには、トークンを1つ読み進めて
// 真を返す。それ以外の場合には偽を返す。
bool consume(char *op) {
    if (token->kind != TK_RESERVED || strlen(op) != token->len ||
        memcmp(op, token->str, token->len))
        return false;
    token = token->next;
    return true;
}

// 次のトークンが期待している記号のときには、トークンを1つ読み進める。
// それ以外の場合にはエラーを報告する。
void expect(char *op) {
    if (token->kind != TK_RESERVED || strlen(op) != token->len ||
        memcmp(op, token->str, token->len))
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
Token *new_token(Tokenkind kind, Token *cur, char *str, int len) {
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;
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
        if (strncmp(p, ">=", 2) == 0 || strncmp(p, "<=", 2) == 0 ||
            strncmp(p, "==", 2) == 0 || strncmp(p, "!=", 2) == 0) {
            cur = new_token(TK_RESERVED, cur, p, 2);
            p += 2;
            continue;
        }
        if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' ||
            *p == ')' || *p == '<' || *p == '>') {
            cur = new_token(TK_RESERVED, cur, p++, 1);
            //,cur, p);
            // p++; と書いても同じ
            continue;
        }
        // Q. new_token()にpを代入するとEOFが遠くの方にあるのでは？
        // A.はい．pは「そのトークンの始まる位置」なので
        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p, 0);
            char *q = p;
            cur->val = strtol(p, &p, 10);
            cur->len = p - q;
            continue;
        }

        error_at(p, "トークナイズできません．");
    }
    new_token(TK_EOF, cur, p, 0);
    return head.next;
}
//トークナイザここまで
