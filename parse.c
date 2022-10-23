#include "aralcc.h"

//パーサ
Node *code[100];
LVar *locals;  //ローカル変数を表す連結リスト

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

/*
program    = *statement
statement  = expr ";"
           | "return" expr ";"
           | "if" "(" expr ")" statement ("else" statement)?
           | "while" "(" expr ")" statement
           | "for" "(" expr? ";" expr? ";" expr? ")" statement
expr       = assign
assign     = equality ("=" assign)?
equality   = relational ("==" relational | "!=" relational)*
relational = add ("<" add | "<=" add | ">" add | ">=" add)*
add        = mul ("+" mul | "-" mul)*
mul        = unary ("*" unary | "/" unary)*
unary      = ("+" | "-")? primary
primary    = num | ident | "(" expr ")"
*/

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

void *program() {
    //トークンの列から構文木を作成し，セミコロン区切りごとにcodeに入れる
    int i = 0;
    while (!at_eof()) {
        code[i++] = statement();
    }
    code[i] = NULL;  //最後にはNULLポインタを入れる
}
Node *statement() {
    Node *node;
    if (consume("return")) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_RETURN;
        node->lhs = expr();
        expect(";");
    } else if (consume("if")) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_IF;

        expect("(");
        node->condition = expr();
        expect(")");

        node->then = statement();

        if (consume("else")) {
            node->els = statement();
        }
    } else {
        node = expr();
        expect(";");
    }
    return node;
}
Node *expr() {
    //今見てるトークンからexprの構文木を作成する
    //返り値はその木の根を表すNode
    Node *node = assign();
    return node;
}
Node *assign() {
    Node *node = equality();
    while (1) {
        if (consume("=")) {
            node = new_node(ND_ASSIGN, node, assign());
        } else {
            return node;
        }
    }
}
Node *equality() {
    Node *node = relational();
    while (1) {
        if (consume("==")) {
            node = new_node(ND_EQ, node, relational());
        } else if (consume("!=")) {
            node = new_node(ND_NEQ, node, relational());
        } else {
            return node;
        }
    }
}
Node *relational() {
    Node *node = add();
    while (1) {
        if (consume("<"))
            node = new_node(ND_LNEQ, node, add());
        else if (consume("<="))
            node = new_node(ND_LEQ, node, add());
        else if (consume(">"))
            node = new_node(ND_LNEQ, add(), node);
        else if (consume(">="))
            node = new_node(ND_LEQ, add(), node);
        else
            return node;
    }
}

Node *add() {
    Node *node = mul();
    while (1) {
        if (consume("+"))
            node = new_node(ND_ADD, node, mul());
        else if (consume("-"))
            node = new_node(ND_SUB, node, mul());
        else
            return node;
    }
}

Node *mul() {
    Node *node = unary();
    while (1) {
        if (consume("*"))
            node = new_node(ND_MUL, node, unary());
        else if (consume("/"))
            node = new_node(ND_DIV, node, unary());
        else
            return node;
    }
}
Node *unary() {
    if (consume("+")) {
        return primary();
    }
    if (consume("-")) {
        return new_node(ND_SUB, new_node_num(0), primary());
    }
    return primary();
}

Node *primary() {
    if (consume("(")) {
        Node *node = expr();
        expect(")");
        return node;
    }

    Token *tok = consume_ident();
    if (tok != NULL) {
        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_LVAR;

        LVar *lvar = find_lvar(tok);
        if (lvar != NULL) {
            node->offset = lvar->offset;
        } else {
            // localsの先頭に新しい変数名を記録
            lvar = calloc(1, sizeof(LVar));
            lvar->next = locals;
            lvar->name = tok->str;
            lvar->len = tok->len;
            lvar->offset = locals->offset + 8;
            node->offset = lvar->offset;
            locals = lvar;
        }
        return node;
    }
    return new_node_num(expect_number());
}

LVar *find_lvar(Token *tok) {
    // tokが表す変数名が既に現れているかどうか確認
    //現れていればそこへのポインタを返す
    //そうでないならNULL

    for (LVar *var = locals; var != NULL; var = var->next) {
        if (tok->len == var->len &&
            strncmp(tok->str, var->name, tok->len) == 0) {
            return var;
        }
    }
    return NULL;
}
//パーサ終わり