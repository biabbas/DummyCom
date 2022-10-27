#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include "lexer.h"
#include "execute.h"
#include "tasks.h"
#include "string"

using namespace std;
/*            Global Variables              */
LexicalAnalyzer lexer;

// Pointers to linked list that store variable declarations.
Var First = NULL, Last = NULL;
//This is 1, if no type errors detected in input.
bool NoTypeErrors = 1;
bool TypeErrorAtLine[20] = {0};

bool TreeStored = 0;
Node FirstTree;
//Used to arrange messages .
int NoofMnode = 0;
Mnode MesArray[30];
/*            Global Variables              */   


/* ******************************************************************           
                              Functions
                                                                    *************************/

//Code to insert a node into Mnode Array.
void InsertToMnode(Node root,int no){
    if(root == NULL)
    return;
    if(root->isTerminal){
    if (isdigit(root->word[0]))
    {
        MesArray[NoofMnode].mes = "NUM "+ (char)34 + root->word + (char)34;
    }
    else
       MesArray[NoofMnode].mes = "ID "+ (char)34 + root->word + (char)34;
    }
    else
    MesArray[NoofMnode].mes = root->word;
    MesArray[NoofMnode].no = no;
    NoofMnode++;
    InsertToMnode(root->left,no*10+1);
    InsertToMnode(root->right,no*10+2);
}   
//Sorting MesArray in ascending order based on no.                                                         
void quicksort(Mnode Tray[30], int first, int last)
{

    int i, j, pivot;
    Mnode temp;

    if (first < last)
    {

        pivot = first;

        i = first;

        j = last;

        while (i < j)
        {

            while (Tray[i].no <= Tray[pivot].no && i < last)

                i++;

            while (Tray[j].no > Tray[pivot].no)

                j--;

            if (i < j)
            {

                temp = Tray[i];

                Tray[i] = Tray[j];

                Tray[j] = Tray[i];
            }
        }

        temp = Tray[pivot];

        Tray[pivot] = Tray[j];

        Tray[j] = temp;

        quicksort(Tray, first, j - 1);

        quicksort(Tray, j + 1, last);
    }
}   

/* Prints SNYTAX Error into both files*/
void SyntaxError(){
    fstream file1, file2;
    file1.open("1txt.2expected", ios::out );
    if(file1){

        file1 << "SNYATX EORRR !!!" << endl;
        }
    file2.open("1txt.2expected", ios::out );
    if(file2){

        file2 << "SNYATX EORRR !!!" << endl;
        }
    exit(0);
}
/*Function to register a type Error*/
void typeError(int line_no){
    NoTypeErrors = 0;
    TypeErrorAtLine[line_no] = 1;
}

/* If next token is of expected type returns token else Syntax error is raised.*/
Token expect(TokenType temp){
Token t = lexer.GetToken();
if(t.token_type != temp){
    SyntaxError();
}
return t;
}

/* Creates a linked list and stores variable information*/
void PushVariable(string name, TokenType TypeofVariable){
    //Code to check for multiple declarations
    Var temp = First;
        while (temp!= NULL)
        {
            if(temp->name == name)
            SyntaxError();
            temp = temp->next;
        }
    //end    
    
    temp = (Var)malloc(sizeof(struct variable));
        temp->name = name;
        if(TypeofVariable == SCALAR)
        temp->IsScalar = 1;
        else
        temp->IsScalar = 0;
        temp->next = NULL;
    if(First == NULL){
        First = temp;
        Last = First;
    }
    else{
        Last->next = temp;
    }
}
//Returns yes if passed argument is a operator in (+,-,*,*)
bool isOperator(TokenType tType){
    if (tType == PLUS || tType == MINUS || tType == MULT || tType == DIV) 
    return 1;
    else
    return 0;
}
//Precedence Lookup function
int ip(char x){
    switch (x) {
        case '#' :
        case '[' :
        case '(' : return 0;break;
        case '+' :
        case '-' : return 1;break;
        case '*' :
        case '/' : return 2;break;
        default: printf("Parsing Error:\nCheck Parse expression",x);
        exit(0);
    }
}

//This function is to assign type from declaration sections
VariableType assign_type_from_declaration(string scanLex){
   Var temp = First;
   while (temp!=NULL)
   {
    if (temp->name == scanLex)
    {
        if (temp->IsScalar)
        {
            return scalar;
        }
        else
        return array;
    }
    
    temp = temp->next;
   }

    return error;
}
//Assigns type to root nodes.
VariableType assign_type_after_operation(VariableType L,string symbol,VariableType R,int lineno){
    if(L == error || R == error)
    return error;
    if (symbol == "[]")
        if (L== array || R == scalar)
            return scalar;
    
    else if (symbol == "+" || symbol == "-")
        if (L == R)
        return L;

    else if (symbol == "*")
        if( L==R && R!=error)
        return scalar;
        
    else if (symbol == "/")
        if (L == R == scalar)
            return scalar;
    
    else if (symbol == "=")
    {
        if ( L == R )
        return L;

        else if(L == scalar && R == array)
        return array;
        
    }
    else
    SyntaxError();

    typeError(lineno);
    return error;
}

VariableType assign_type_afterdot(VariableType LST,int lineno){
 if (LST == array)
    return array;
 typeError(lineno);
 return error;
 
}
//Used to create nodes with operators which is not a terminal
Node createNodewithSymbol(string symbol){
    Node tree = (Node)malloc(sizeof(struct node));
    tree->isTerminal = 0;
    tree->word = symbol;
    tree->left = NULL;
    tree->right = NULL;
    return tree;
}



//This functions converts a id token into Node struct; and returns its address;
Node createTerminalNode(Token t){
    Node idNode = (Node)malloc(sizeof(struct node));
    idNode->isTerminal = 1;
    idNode->word = t.lexeme;
    if (t.token_type == NUM)
    {
        idNode->AssignedType = scalar;
    }
    else
    idNode->AssignedType = assign_type_from_declaration(t.lexeme);
    
    idNode->left = NULL;
    idNode->right = NULL;
    return idNode;
}

/*********888*************      Parsing Functions          ************888************/


void parse_idlist(TokenType TypeofVariable){
Token variable  = expect(ID);
PushVariable(variable.lexeme, TypeofVariable);

Token next = lexer.peek(1);
if(next.token_type == ID)
parse_idlist(TypeofVariable);
}

void parse_scalar_decl(){
    expect( SCALAR );
    parse_idlist(SCALAR);
}
void parse_array_decl(){
    expect( ARRAY );
    parse_idlist(ARRAY);
}

void parse_decl_section(){
  parse_scalar_decl();
  parse_array_decl();
}

/*88888888**************    Code involved in creating AST
     AST consists of Node datatype. This Datatype is defined in tasks.h
                                                                 *88888888*************/
//Next token after this parse_expr is always RBRAC. Returns a node that is root node for a Tree.
Node parse_variable_access_expr(){


//stacks.    
Node DigStk[10],OpStk[10];
//StackIndex
int j=0,topD=-1,topO= 0;
VariableType left,right;

OpStk[0] = (Node)malloc(sizeof(struct node));
OpStk[0]->word = "#";
Token curToken = lexer.GetToken();
if (curToken.token_type!=ID && curToken.token_type != NUM && curToken.token_type!= LPAREN)
SyntaxError();

while(true){
Node temp = NULL;

if(curToken.token_type == ID || curToken.token_type == NUM){
    temp = createTerminalNode(curToken);
    DigStk[++topD] = temp; 
}
else{
     if (curToken.token_type == LBRAC){
        Token nextoken = lexer.peek(1);
        if(nextoken.token_type==DOT){
            expect(DOT);
            expect(RBRAC);
            temp = createNodewithSymbol("[.]");
            temp->left = DigStk[topD];
            temp->AssignedType = assign_type_afterdot(DigStk[topD]->AssignedType,curToken.line_no);
            DigStk[topD] = temp;
        }

        else{
        temp = createNodewithSymbol("[]");
        OpStk[++topO]=temp;
        }
     } 
        else if (curToken.token_type == RBRAC){
            Token next = lexer.peek(1);
            if (next.token_type == EQUAL || next.token_type == SEMICOLON){
            goto finish;
            break;
            }
            while (OpStk[topO]->word!="[]") {
            right = DigStk[topD]->AssignedType;
            OpStk[topO]->right = DigStk[topD--];
            left = DigStk[topD]->AssignedType;
            OpStk[topO]->left = DigStk[topD--];
            OpStk[topO]->AssignedType = assign_type_after_operation(left,OpStk[topO]->word,right,curToken.line_no);
            DigStk[++topD] = OpStk[topO--];
            }
           //Code to build a[]expr node.
            right = DigStk[topD]->AssignedType;
            OpStk[topO]->right = DigStk[topD--];
            left = DigStk[topD]->AssignedType;
            OpStk[topO]->left = DigStk[topD--];
            OpStk[topO]->AssignedType = assign_type_after_operation(left,OpStk[topO]->word,right,curToken.line_no);
            DigStk[++topD] = OpStk[topO--];
            

        }
    else if (curToken.token_type == LPAREN){
        temp = createNodewithSymbol("(");
        OpStk[++topO]=temp;
     } 
        else if (curToken.token_type == RPAREN){
            while (OpStk[topO]->word!="(") {
            right = DigStk[topD]->AssignedType;
            OpStk[topO]->right = DigStk[topD--];
            left = DigStk[topD]->AssignedType;
            OpStk[topO]->left = DigStk[topD--];
            OpStk[topO]->AssignedType = assign_type_after_operation(left,OpStk[topO]->word,right,curToken.line_no);
            DigStk[++topD] = OpStk[topO--];
            }

            free(OpStk[topO--]);
            } 
        
        else if(isOperator(curToken.token_type)){
            temp = createNodewithSymbol(curToken.lexeme);
            if(ip(OpStk[topO]->word[0]) < ip(temp->word[0])){
            OpStk[++topO] = temp;
            }
                
            else{
            while (ip(OpStk[topO]->word[0]) >= ip(temp->word[0])) {
            right = DigStk[topD]->AssignedType;
            OpStk[topO]->right = DigStk[topD--];
            left = DigStk[topD]->AssignedType;
            OpStk[topO]->left = DigStk[topD--];
            OpStk[topO]->AssignedType = assign_type_after_operation(left,OpStk[topO]->word,right,curToken.line_no);
            DigStk[++topD] = OpStk[topO--];
                }
            OpStk[++topO] = temp;  
            }
        }
        else
        SyntaxError();
    }
    curToken = lexer.GetToken();//Increment current token to next token.
}
finish:
while (OpStk[topO]->word!= "#")
{
        right = DigStk[topD]->AssignedType;
        OpStk[topO]->right = DigStk[topD--];
        left = DigStk[topD]->AssignedType;
        OpStk[topO]->left = DigStk[topD--];
        OpStk[topO]->AssignedType = assign_type_after_operation(left,OpStk[topO]->word,right,curToken.line_no);
        DigStk[++topD] = OpStk[topO--];
      }
if(topD != 0 || topO !=0)
SyntaxError();

lexer.UngetToken(1);//To unget  ']' in case of variable access expression


return DigStk[topD];

    



}

/*Next token after this parse_expr is always semicolon. 
This function returns a expression tree root node */
Node parse_expr(){


//stacks.    
Node DigStk[10],OpStk[10];
//StackIndex
int j=0,topD=-1,topO= 0;
VariableType left,right;

OpStk[0] = (Node)malloc(sizeof(struct node));
OpStk[0]->word = "#";
Token curToken = lexer.GetToken();
if (curToken.token_type!=ID && curToken.token_type != NUM && curToken.token_type!= LPAREN)
SyntaxError();

while(curToken.token_type != SEMICOLON){
Node temp = NULL;

if(curToken.token_type == ID || curToken.token_type == NUM){
    temp = createTerminalNode(curToken);
    DigStk[++topD] = temp; 
}
else{
     if (curToken.token_type == LBRAC){
        Token nextoken = lexer.peek(1);
        if(nextoken.token_type==DOT){
            expect(DOT);
            expect(RBRAC);
            temp = createNodewithSymbol("[.]");
            temp->left = DigStk[topD];
            temp->AssignedType = assign_type_afterdot(DigStk[topD]->AssignedType,curToken.line_no);
            DigStk[topD] = temp;
        }

        else{
        temp = createNodewithSymbol("[]");
        OpStk[++topO]=temp;
        }
     } 
        else if (curToken.token_type == RBRAC){
            while (OpStk[topO]->word!="[]") {
            right = DigStk[topD]->AssignedType;
            OpStk[topO]->right = DigStk[topD--];
            left = DigStk[topD]->AssignedType;
            OpStk[topO]->left = DigStk[topD--];
            OpStk[topO]->AssignedType = assign_type_after_operation(left,OpStk[topO]->word,right,curToken.line_no);
            DigStk[++topD] = OpStk[topO--];
            }
           //Code to build a[]expr node.
            right = DigStk[topD]->AssignedType;
            OpStk[topO]->right = DigStk[topD--];
            left = DigStk[topD]->AssignedType;
            OpStk[topO]->left = DigStk[topD--];
            OpStk[topO]->AssignedType = assign_type_after_operation(left,OpStk[topO]->word,right,curToken.line_no);
            DigStk[++topD] = OpStk[topO--];
            

        }
    else if (curToken.token_type == LPAREN){
        temp = createNodewithSymbol("(");
        OpStk[++topO]=temp;
     } 
        else if (curToken.token_type == RPAREN){
            while (OpStk[topO]->word!="(") {
            right = DigStk[topD]->AssignedType;
            OpStk[topO]->right = DigStk[topD--];
            left = DigStk[topD]->AssignedType;
            OpStk[topO]->left = DigStk[topD--];
            OpStk[topO]->AssignedType = assign_type_after_operation(left,OpStk[topO]->word,right,curToken.line_no);
            DigStk[++topD] = OpStk[topO--];
            }

            free(OpStk[topO--]);
            } 
        
        else if(isOperator(curToken.token_type)){
            temp = createNodewithSymbol(curToken.lexeme);
            if(ip(OpStk[topO]->word[0]) < ip(temp->word[0])){
            OpStk[++topO] = temp;
            }
                
            else{
            while (ip(OpStk[topO]->word[0]) >= ip(temp->word[0])) {
            right = DigStk[topD]->AssignedType;
            OpStk[topO]->right = DigStk[topD--];
            left = DigStk[topD]->AssignedType;
            OpStk[topO]->left = DigStk[topD--];
            OpStk[topO]->AssignedType = assign_type_after_operation(left,OpStk[topO]->word,right,curToken.line_no);
            DigStk[++topD] = OpStk[topO--];
                }
            OpStk[++topO] = temp;  
            }
        }
        else
        SyntaxError();
    }
    //Increment Token
    curToken = lexer.GetToken();
}
while (OpStk[topO]->word!= "#")
{
        right = DigStk[topD]->AssignedType;
        OpStk[topO]->right = DigStk[topD--];
        left = DigStk[topD]->AssignedType;
        OpStk[topO]->left = DigStk[topD--];
        OpStk[topO]->AssignedType = assign_type_after_operation(left,OpStk[topO]->word,right,curToken.line_no);
        DigStk[++topD] = OpStk[topO--];
      }
if(topD != 0 || topO !=0)
SyntaxError();

lexer.UngetToken(1);//To unget  SEMICOLON Token.


return DigStk[topD];

    


/****  ****88********************/
}



//Returns a node with Left hand side of equation.  Variableaccess = Id | Id[.] | Id[expr]                                                       
Node parse_variable_access(){
   Node Tree,tempWithID;
   Token t1 = expect(ID);
   tempWithID = createTerminalNode(t1);
   Token t2 = lexer.peek(1);
   if (t2.token_type == EQUAL )
   {
    return tempWithID;
   }
   else if (t2.token_type == LBRAC)
   {
    Token t3 = lexer.peek(2);
    if (t3.token_type == DOT)
    {
        expect(LBRAC);
        expect(DOT);
        expect(RBRAC);
    Tree = createNodewithSymbol("[.]");
    Tree->left = tempWithID;
    Tree->AssignedType = assign_type_afterdot(tempWithID->AssignedType,t1.line_no);
    return Tree;
    }
    else{
       expect(LBRAC);
       Node RST =  parse_variable_access_expr();
       expect(RBRAC);
       Tree = createNodewithSymbol("[]");
       Tree->AssignedType = assign_type_after_operation(tempWithID->AssignedType,Tree->word,RST->AssignedType,t1.line_no);
       Tree->left = tempWithID;
       Tree->right = RST;
       return Tree;
    }
    
   }
   else
   SyntaxError();
   

   return NULL;
}

void parse_assign_stmt(){
    Node left, right,root;
    left = parse_variable_access();
    Token te = expect(EQUAL);
    right = parse_expr();
    root = createNodewithSymbol("=");
    root->left = left;
    root->right = right;
    root->AssignedType = assign_type_after_operation(left->AssignedType,root->word,right->AssignedType,te.line_no);
    expect(SEMICOLON);
    //Include code to store all root tree corresponding to each statement.
    if (TreeStored = 0)
    {
        FirstTree = root;
        TreeStored = 1;
    }
}


void parse_output_stmt(){
expect(OUTPUT);
parse_variable_access();
expect(SEMICOLON);
}

void parse_stmt(){
Token t = lexer.peek(1);
if (t.token_type == ID)
{
    parse_assign_stmt();
}
else if (t.token_type == OUTPUT)
{
    parse_output_stmt();
}
else
SyntaxError();

}


void parse_stmtList(){
parse_stmt();
Token next = lexer.peek(1);
if(next.token_type != RBRACE)   //Follow set of Statement List
parse_stmtList();
}

void parse_block(){
expect(LBRACE);
parse_stmtList();
expect(RBRACE);
}

void parse_program(){
    parse_decl_section();
    parse_block();
}
void parse_input(){
    parse_program();
    expect( END_OF_FILE );
}



/*********888*************    End of  Parsing Functions          ************888************/


// Task 1
void parse_and_generate_AST()
{
	cout << "1" << endl;
}

// Task 2
void parse_and_type_check()
{
	cout << "2" << endl;
}

// Task 3
instNode* parse_and_generate_statement_list()
{
    cout << "3" << endl;

    // The following is the hardcoded statement list 
    // generated for a specific program
    // you should replace this code with code that parses the
    // input and generayes a statement list
    // 
    // program
    // SCALAR a b c d
    // ARRAY x y z
    // a = 1;
    // b = 2;
    // c = 3;
    // d = (a+b)*(b+c);
    // OUTPUT d;
    // x[a+b] = d;
    // OUTPUT x[3];
    //
    //  a will be at location 0
    //  b will be at location 1
    //  c will be at location 2
    //  d will be at location 3
    //  x will be at location 4 - 13
    //  y will be at location 14 - 23
    //  z will be at location 24 - 33
    //  t1 will be at location 34 : intermediate value for (a+b)
    //  t2 will be at location 35 : intermediate value for (b+c)
    //  t3 will be at location 36 : intermediate value (a+b)*(b+c)
    //  t4 will be at location 37 : intermediate value for a+b index of array
    //  t5 will be at location 38 : intermediate value for addr of x[a+b] =
    //                              address_of_x + value of a+b =
    //                              4 + value of a+b
    //  t6 will be at location 39 : intermediate value for addr of x[3] =
    //                              address_of_x + value of 3 =
    //                              4 + value of 3 (computation is not done at
    //                              compile time)
    //
    instNode * i01 = new instNode();
    i01->lhsat=DIRECT; i01->lhs = 0;    // a
    i01->iType=ASSIGN_INST;             // =
    i01->op1at=IMMEDIATE; i01->op1 = 1; // 1
    i01->oper = OP_NOOP;                // no operator

    instNode * i02 = new instNode();
    i02->lhsat=DIRECT; i02->lhs = 1;    // b
    i02->iType=ASSIGN_INST;             // =
    i02->op1at=IMMEDIATE; i02->op1 = 2; // 2
    i02->oper = OP_NOOP;                // no operator

    i01->next = i02;

    instNode * i03 = new instNode();
    i03->lhsat=DIRECT; i03->lhs = 2;    // c
    i03->iType=ASSIGN_INST;             // =
    i03->op1at=IMMEDIATE; i03->op1 = 3; // 3
    i03->oper = OP_NOOP;                // no operator

    i02->next = i03;


    instNode * i1 = new instNode();
    i1->lhsat=DIRECT; i1->lhs = 34; // t1
    i1->iType=ASSIGN_INST;          // =
    i1->op1at=DIRECT; i1->op1 = 0;  // a
    i1->oper = OP_PLUS;             // +
    i1->op2at=DIRECT; i1->op2 = 1;  // b

    i03->next = i1;

    instNode * i2 = new instNode();
    i2->lhsat=DIRECT; i2->lhs = 35; // t2
    i2->iType=ASSIGN_INST;          // =
    i2->op1at=DIRECT; i2->op1 = 1;  // b
    i2->oper = OP_PLUS;             // +
    i2->op2at=DIRECT; i2->op2 = 2;  // c

    i1->next = i2;

    instNode * i3 = new instNode();
    i3->lhsat=DIRECT; i3->lhs = 36; // t3
    i3->iType=ASSIGN_INST;          // =
    i3->op1at=DIRECT; i3->op1 = 34;  // t1
    i3->oper = OP_MULT;             // *
    i3->op2at=DIRECT; i3->op2 = 35;  // t2


    i2->next = i3;                  // i3 should be after i1 and i2

    instNode * i4 = new instNode();
    i4->lhsat=DIRECT; i4->lhs = 3;  // d
    i4->iType=ASSIGN_INST;          // =
    i4->op1at=DIRECT; i4->op1 = 36; // t3
    i4->oper = OP_NOOP;             // no operator


    i3->next = i4;

    instNode * i5 = new instNode();
    i5->iType=OUTPUT_INST;          // OUTPUT
    i5->op1at=DIRECT; i5->op1 = 3;  // d

    i4->next = i5;

    instNode * i6 = new instNode();
    i6->lhsat=DIRECT; i6->lhs = 37; // t4
    i6->iType=ASSIGN_INST;          // =
    i6->op1at=DIRECT; i6->op1 = 0;  // a
    i6->oper = OP_PLUS;             // +
    i6->op2at=DIRECT; i6->op2 = 1;  // b
    i5->next = i6;

    instNode * i7 = new instNode();
    i7->lhsat=DIRECT; i7->lhs = 38;    // t5
    i7->iType=ASSIGN_INST;             // =
    i7->op1at=IMMEDIATE; i7->op1 = 4;  // address of x = 4 available
                                       // at compile time
    i7->oper = OP_PLUS;                // +
    i7->op2at=DIRECT; i7->op2 = 37;    // t5 (contains value of a+b

    i6->next = i7;

    instNode * i8 = new instNode();
    i8->lhsat=INDIRECT; i8->lhs = 38;  // x[a+b]
    i8->iType=ASSIGN_INST;             // =
    i8->op1at=DIRECT; i8->op1 = 3;     // d
    i8->oper = OP_NOOP;

    i7->next = i8;

    instNode * i9 = new instNode();
    i9->lhsat=DIRECT; i9->lhs = 39;    // t6 will contain address of x[3]
    i9->iType=ASSIGN_INST;             // =
    i9->op1at=IMMEDIATE; i9->op1 = 4;  // address of x = 4 available
                                       // at compile time
    i9->oper = OP_PLUS;                // +
    i9->op2at=IMMEDIATE; i9->op2 = 3;  // 3

    i8->next = i9;

    instNode * i10 = new instNode();
    i10->iType=OUTPUT_INST;              // OUTPUT
    i10->op1at=INDIRECT; i10->op1 = 39;  // x[3] by providing its
                                         // address indirectly through
                                         // t6

    i9->next = i10;


    instNode* code = i01;

    return code;
}
