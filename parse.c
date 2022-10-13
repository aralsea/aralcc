#include "aralcc.h"

//パーサ
Node *code[100];

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
    Node *node = expr();
    expect(";");
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
        node->offset = (tok->str[0] - 'a' + 1) * 8;
        return node;
    }
    return new_node_num(expect_number());
}
//パーサ終わり