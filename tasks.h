#include "execute.h"
#include "string"
#include "lexer.h"
using namespace std;

enum VariableType{
    scalar,
    array,
    error
};

typedef struct node
{
    bool isTerminal;
    VariableType AssignedType;
    string word;//Lexeme
    struct node* left;
    struct node* right;
    
}*Node;

//Used To store Variable data. 
typedef struct variable{
    string name;
    bool IsScalar;// 1 for scalar variables and 0 for array variables
    struct variable* next;
} *Var;

typedef struct MesNode{
    int no;
    string mes;
}Mnode;

void parse_input();
void parse_and_generate_AST();
void parse_and_type_check();
instNode* parse_and_generate_statement_list();

