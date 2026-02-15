#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

//tokens
typedef enum {

    T_INT,
    T_PRINT,
    T_ID,//variable names
    T_NUM,//numeric literals
    
    T_PLUS,
    T_MINUS, 
    T_MULTIPLY,
    T_DIVIDE,   
    T_ASSIGN, 

    T_SEMICOLON,
    T_LPAREN,//(
    T_RPAREN,//)

    T_ENDFILE,
    T_ERROR

} Token_Type;

typedef struct {

    Token_Type type;
    char value[256];
    int Line_Number;

} Token;

//symbol table for variables
typedef struct {

    char name[256];
    int value;
    int IsDefined;

} Variable;

Variable variables[100];

int Var_Count = 0;

//global state
char *source;   
int pos = 0; 
Token Current_Token; 
int Line_Number = 1; 

//function prototypes
void Next_Token();
void Parse();
void Statement();
int Expression();
int Term();
int Factor();
void Error(const char *msg);
void Expect(Token_Type type, const char *msg);
int Get_Variable(const char *name);
void Set_Variable(const char *name, int value, int Line_Num);

//Error handling
void Error(const char *msg){

    printf("Error at line %d: %s\n", Current_Token.Line_Number, msg);
    exit(1);

}

//expect a specific token type
void Expect(Token_Type type, const char *msg){

    if (Current_Token.type != type){
        Error(msg);
 }

    Next_Token();
}

//symbol table get variable value
int Get_Variable(const char *name)
{
    for (int i = 0; i < Var_Count; i++)
    {
        if (strcmp(variables[i].name, name) == 0)
        {
            if (!variables[i].IsDefined)
            {
                char Err_Msg[300];
                sprintf(Err_Msg, "Variable '%s' used before initialization", name);
                Error(Err_Msg);
            }
            return variables[i].value;
        }
    }
    char Err_Msg[300];
    sprintf(Err_Msg, "Undefined variable '%s'", name);

    Error(Err_Msg);

    return 0;
}

//symbol table set variable value
void Set_Variable(const char *name, int value, int Line_Num) 
{
    //check if variable already exists
    for (int i = 0; i < Var_Count; i++) 
    {
        if (strcmp(variables[i].name, name) == 0) 
        {
            printf("Error at line %d: Variable '%s' already declared\n", Line_Num, name);
            exit(1);
        }
    }

    //add new variable
    strcpy(variables[Var_Count].name, name);
    variables[Var_Count].value = value;
    variables[Var_Count].IsDefined = 1;
    Var_Count++;
}

//tokenizer implementation
void Next_Token() 
{
    //skip whitespace and track line numbers
    while (source[pos] && isspace(source[pos])) 
    {
        if (source[pos] == '\n') Line_Number++;
        pos++;
    }
    
    if (source[pos] == '\0') 
    {
        Current_Token.type = T_ENDFILE;
        strcpy(Current_Token.value, "EOF");
        Current_Token.Line_Number = Line_Number;

        return;
    }
    
    //store line number for this token
    Current_Token.Line_Number = Line_Number;
    
    //check for keywords and identifiers
    if (isalpha(source[pos])) 
    {
        int start = pos;

        while (isalnum(source[pos]) || source[pos] == '_') pos++;

        int len = pos - start;
        strncpy(Current_Token.value, &source[start], len);
        Current_Token.value[len] = '\0';
        
        if (strcmp(Current_Token.value, "int") == 0)
            Current_Token.type = T_INT;
            
        else if (strcmp(Current_Token.value, "print") == 0)
            Current_Token.type = T_PRINT;

        else
            Current_Token.type = T_ID;

        return;
    }
    
    //check for numbers
    if (isdigit(source[pos])) 
    {
        int start = pos;

        while (isdigit(source[pos])) pos++;

        int len = pos - start;
        strncpy(Current_Token.value, &source[start], len);
        Current_Token.value[len] = '\0';
        Current_Token.type = T_NUM;

        return;
    }
    
    //single character tokens
    Current_Token.value[0] = source[pos];
    Current_Token.value[1] = '\0';
    
    switch (source[pos]) 
    {
        case '+': 
            Current_Token.type = T_PLUS; break;

        case '-': 
            Current_Token.type = T_MINUS; break;

        case '*': 
            Current_Token.type = T_MULTIPLY; break;

        case '/': 
            Current_Token.type = T_DIVIDE; break;

        case '=': 
            Current_Token.type =  T_ASSIGN; break;

        case ';': 
            Current_Token.type = T_SEMICOLON; break;

        case '(': 
            Current_Token.type = T_LPAREN; break;

        case ')': 
            Current_Token.type = T_RPAREN; break;

        default: 
            Current_Token.type = T_ERROR;
            char Err_Msg[100];
            sprintf(Err_Msg, "UnExpected character '%c'", source[pos]);
            Error(Err_Msg);
    }

    pos++;
}

//parser: factor - number| identifier| "(expression)"
int Factor() 
{
    if (Current_Token.type == T_NUM) 
    {
        int value = atoi(Current_Token.value);
        Next_Token();

        return value;
    }
    
    if (Current_Token.type == T_ID) 
    {
        char name[256];
        strcpy(name, Current_Token.value);
        Next_Token();

        return Get_Variable(name);
    }
    
    if (Current_Token.type == T_LPAREN) 
    {
        Next_Token();
        int value = Expression();
        Expect(T_RPAREN, "Expected ')'");

        return value;
    }
    
    Error("Expected number, identifier, or '('");

    return 0;
}

//parser: Term - Factor ((*|/)Factor)*
int Term() 
{
    int result = Factor();
    
    while (Current_Token.type == T_MULTIPLY || Current_Token.type == T_DIVIDE) 
    {
        Token_Type op = Current_Token.type;
        Next_Token();

        int right = Factor();
        
        if (op == T_MULTIPLY) 
        {
            result = result * right;
        } 
        else 
        {
            if (right == 0) 
            {
                Error("Division by zero");
            }

            result = result / right;
        }
    }
    
    return result;
}

//parser: expression- term ((+|-)Term)*
int Expression() 
{
    int result = Term();
    
    while (Current_Token.type == T_PLUS || Current_Token.type == T_MINUS) 
    {
        Token_Type op = Current_Token.type;
        Next_Token();
        int right = Term();
        
        if (op == T_PLUS)
        {
            result = result + right;
        } 
        else 
        {
            result = result - right;
        }
    }
    
    return result;
}

//parser: statement - declaration|printStmt
void Statement() 
{
    int StatementLine = Current_Token.Line_Number;  //track statement start line
    
    //declaration: 'int' identifier '=' expression ';'
    if (Current_Token.type == T_INT) 
    {
        Next_Token();
        
        if (Current_Token.type != T_ID) 
        {
            Error("Expected identifier after 'int'");
        }
        
        char name[256];
        strcpy(name, Current_Token.value);

        Next_Token();
        
        Expect (T_ASSIGN, "Expected '=' in declaration");
        
        int value = Expression();
        
        if (Current_Token.type != T_SEMICOLON) 
        {
            //report error at the statement line
            int savedLine = Current_Token.Line_Number;

            Current_Token.Line_Number = StatementLine;
            Error("Expected ';' at end of Statement");
            Current_Token.Line_Number = savedLine;
        }

        Next_Token();
        
        Set_Variable(name, value, StatementLine);
        return;
    }
    
    //printStmt: 'print' '(Expression)' ';"'
    if (Current_Token.type == T_PRINT) 
    {
        Next_Token();
        
        Expect (T_LPAREN, "Expected '(' after 'print'");
        
        int value = Expression();
        
        Expect (T_RPAREN, "Expected ')' after Expression");
        Expect (T_SEMICOLON, "Expected ';' at end of Statement");
        
        printf("%d\n", value);

        return;
    }
    
    Error ("Expected 'int' or 'print' Statement");
}

//parser: program - statement*
void Parse() 
{
    while (Current_Token.type != T_ENDFILE) 
    {
        Statement();
    }
}

int main(int argc, char *argv[]) 
{
    if (argc != 2)
    {
        printf("Usage: ./Parser inputfile\n");
        return 1;
    }
    
    //read source file
    FILE *file = fopen(argv[1], "r");
    
    if (!file) 
    {
        printf("Error: Cannot open file '%s'\n", argv[1]);
        return 1;
    }
    
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    source = malloc(size + 1);
    
    if (!source) 
    {
        printf("Error: Memory allocation failed\n");
        fclose(file);
        return 1;
    }
    
    fread(source, 1, size, file);
    source[size] = '\0';
    fclose(file);
    
    //start parsing
    Next_Token();
    Parse();
    
    free(source);
    return 0;
}