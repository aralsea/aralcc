#include "aralcc.h"

//パーサ
Function *code[100];
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
function = ident "("(ident ("," ident)*)?")" "{" statement "}"
statement  = expr ";"
           | "{" statement* "}"
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
primary    = num | ident ("(" (expr ( "," expr)*)? ")")? | "(" expr ")"
//fは変数，f()は関数
*/

void program();
Function *function();
Node *statement();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();    //\pm primary
Node *primary();  // num or ident or (expr)

void program() {
    //トークンの列から構文木を作成し，セミコロン区切りごとにcodeに入れる
    int i = 0;
    while (!at_eof()) {
        code[i++] = function();
    }
    code[i] = NULL;  //最後にはNULLポインタを入れる
}

Function *function() {
    Function *func = calloc(1, sizeof(Function));

    func->name = expect_ident();
    func->argnum = 0;

    locals = calloc(1, sizeof(LVar));
    locals->next = NULL;
    locals->name = "";
    locals->len = 0;
    locals->offset = 0;

    expect("(");
    if (!consume(")")) {
        //引数を6個まで受け取る
        int count = 0;
        while (1) {
            Token *tok = consume_ident();
            if (tok == NULL) {
                error_at(token->str, "引数が必要です．");
            }
            LVar *lvar = find_lvar(tok);
            if (lvar != NULL) {
                error_at(tok->str, "関数の引数が重複しています．");
            }
            lvar = calloc(1, sizeof(LVar));
            lvar->next = locals;
            lvar->name = tok->str;
            lvar->len = tok->len;
            lvar->offset = locals->offset + 8;
            locals = lvar;
            count++;
            if (count == 6 && consume(",")) {
                error_at(token->str, "関数の引数は6個以下でないといけません．");
            }
            if (consume(",")) {
                continue;
            } else if (consume(")")) {
                break;
            } else {
                error_at(token->str, "引数の宣言が正しくありません．");
            }
        }
        func->argnum = count;
    }

    func->node = statement();

    if (func->node->kind != ND_BLOCK) {
        error_at(token->str, "関数本体を{...}で記述してください．");
    }

    func->locals = locals;

    return func;
}
Node *statement() {
    Node *node;
    if (consume("{")) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_BLOCK;

        Node head;
        head.next = NULL;

        Node *cur = &head;
        while (!consume("}")) {
            if (token->kind == TK_EOF) {
                error_at(token->str, "} が閉じていません．");
            }
            cur->next = statement();
            cur = cur->next;
        }
        node->body = head.next;
    } else if (consume("return")) {
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
    } else if (consume("while")) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_WHILE;

        expect("(");
        node->condition = expr();
        expect(")");

        node->then = statement();
    } else if (consume("for")) {
        // for(init; condition; inc) statement
        node = calloc(1, sizeof(Node));
        node->kind = ND_FOR;

        expect("(");
        if (!consume(";")) {
            node->init = expr();
        }
        expect(";");
        if (!consume(";")) {
            node->condition = expr();
        } else {
            // conditionが指定されてないときは常に真を表す整数1が入ってるとみなす
            node->condition = new_node_num(1);
        }
        expect(";");
        if (!consume(")")) {
            node->inc = expr();
        }
        expect(")");

        node->then = statement();
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
        if (consume("(")) {
            //関数のとき
            node->kind = ND_FUNCCALL;
            node->funcname = calloc(tok->len + 1, sizeof(char));
            strncpy(node->funcname, tok->str, tok->len);

            if (consume(")")) {
                node->argnum = 0;
            } else {
                //引数を6個まで受け取る
                int count = 0;
                while (1) {
                    node->arg[count] = expr();
                    count++;
                    if (count == 6 && consume(",")) {
                        error_at(token->str,
                                 "関数の引数は6個以下でないといけません．");
                    }
                    consume(",");
                    if (consume(")")) {
                        break;
                    }
                }
                node->argnum = count;
            }
        } else {
            //変数のとき
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