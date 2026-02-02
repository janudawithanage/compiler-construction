#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Token types
typedef enum {
    TOKEN_INT,      // "int" keyword
    TOKEN_PRINT,    // "print" keyword
    TOKEN_IDENT,    // variable names
    TOKEN_NUMBER,   // numeric literals
    TOKEN_PLUS,     // +
    TOKEN_MINUS,    // -
    TOKEN_MULTIPLY, // *
    TOKEN_DIVIDE,   // /
    TOKEN_ASSIGN,   // =
    TOKEN_SEMICOLON,// ;
    TOKEN_LPAREN,   // (
    TOKEN_RPAREN,   // )
    TOKEN_EOF,      // end of file
    TOKEN_ERROR     // error token
} TokenType;

typedef struct {
    TokenType type;
    char value[256];
    int lineNumber;
} Token;

// Symbol table for variables
typedef struct {
    char name[256];
    int value;
    int isDefined;
} Variable;

Variable variables[100];
int varCount = 0;

// Global state
char *source;
int pos = 0;
Token currentToken;
int lineNumber = 1;

// Function prototypes
void nextToken();
void parse();
void statement();
int expression();
int term();
int factor();
void error(const char *msg);
void expect(TokenType type, const char *msg);
int getVariable(const char *name);
void setVariable(const char *name, int value);

// Error handling
void error(const char *msg) {
    printf("Error at line %d: %s\n", currentToken.lineNumber, msg);
    exit(1);
}

// Expect a specific token type
void expect(TokenType type, const char *msg) {
    if (currentToken.type != type) {
        error(msg);
    }
    nextToken();
}

// Symbol table: get variable value
int getVariable(const char *name) {
    for (int i = 0; i < varCount; i++) {
        if (strcmp(variables[i].name, name) == 0) {
            if (!variables[i].isDefined) {
                char errMsg[300];
                sprintf(errMsg, "Variable '%s' used before initialization", name);
                error(errMsg);
            }
            return variables[i].value;
        }
    }
    char errMsg[300];
    sprintf(errMsg, "Undefined variable '%s'", name);
    error(errMsg);
    return 0;
}

// Symbol table: set variable value
void setVariable(const char *name, int value) {
    // Check if variable already exists
    for (int i = 0; i < varCount; i++) {
        if (strcmp(variables[i].name, name) == 0) {
            char errMsg[300];
            sprintf(errMsg, "Variable '%s' already declared", name);
            error(errMsg);
        }
    }
    // Add new variable
    strcpy(variables[varCount].name, name);
    variables[varCount].value = value;
    variables[varCount].isDefined = 1;
    varCount++;
}

// Tokenizer implementation
void nextToken() {
    // Skip whitespace and track line numbers
    while (source[pos] && isspace(source[pos])) {
        if (source[pos] == '\n') lineNumber++;
        pos++;
    }
    
    if (source[pos] == '\0') {
        currentToken.type = TOKEN_EOF;
        strcpy(currentToken.value, "EOF");
        currentToken.lineNumber = lineNumber;
        return;
    }
    
    // Store line number for this token
    currentToken.lineNumber = lineNumber;
    
    // Check for keywords and identifiers
    if (isalpha(source[pos])) {
        int start = pos;
        while (isalnum(source[pos]) || source[pos] == '_') pos++;
        int len = pos - start;
        strncpy(currentToken.value, &source[start], len);
        currentToken.value[len] = '\0';
        
        if (strcmp(currentToken.value, "int") == 0)
            currentToken.type = TOKEN_INT;
        else if (strcmp(currentToken.value, "print") == 0)
            currentToken.type = TOKEN_PRINT;
        else
            currentToken.type = TOKEN_IDENT;
        return;
    }
    
    // Check for numbers
    if (isdigit(source[pos])) {
        int start = pos;
        while (isdigit(source[pos])) pos++;
        int len = pos - start;
        strncpy(currentToken.value, &source[start], len);
        currentToken.value[len] = '\0';
        currentToken.type = TOKEN_NUMBER;
        return;
    }
    
    // Single character tokens
    currentToken.value[0] = source[pos];
    currentToken.value[1] = '\0';
    
    switch (source[pos]) {
        case '+': currentToken.type = TOKEN_PLUS; break;
        case '-': currentToken.type = TOKEN_MINUS; break;
        case '*': currentToken.type = TOKEN_MULTIPLY; break;
        case '/': currentToken.type = TOKEN_DIVIDE; break;
        case '=': currentToken.type = TOKEN_ASSIGN; break;
        case ';': currentToken.type = TOKEN_SEMICOLON; break;
        case '(': currentToken.type = TOKEN_LPAREN; break;
        case ')': currentToken.type = TOKEN_RPAREN; break;
        default: 
            currentToken.type = TOKEN_ERROR;
            char errMsg[100];
            sprintf(errMsg, "Unexpected character '%c'", source[pos]);
            error(errMsg);
    }
    pos++;
}

// Parser: Factor → NUMBER | IDENTIFIER | "(" Expression ")"
int factor() {
    if (currentToken.type == TOKEN_NUMBER) {
        int value = atoi(currentToken.value);
        nextToken();
        return value;
    }
    
    if (currentToken.type == TOKEN_IDENT) {
        char name[256];
        strcpy(name, currentToken.value);
        nextToken();
        return getVariable(name);
    }
    
    if (currentToken.type == TOKEN_LPAREN) {
        nextToken();
        int value = expression();
        expect(TOKEN_RPAREN, "Expected ')'");
        return value;
    }
    
    error("Expected number, identifier, or '('");
    return 0;
}

// Parser: Term → Factor (("*" | "/") Factor)*
int term() {
    int result = factor();
    
    while (currentToken.type == TOKEN_MULTIPLY || currentToken.type == TOKEN_DIVIDE) {
        TokenType op = currentToken.type;
        nextToken();
        int right = factor();
        
        if (op == TOKEN_MULTIPLY) {
            result = result * right;
        } else {
            if (right == 0) {
                error("Division by zero");
            }
            result = result / right;
        }
    }
    
    return result;
}

// Parser: Expression → Term (("+" | "-") Term)*
int expression() {
    int result = term();
    
    while (currentToken.type == TOKEN_PLUS || currentToken.type == TOKEN_MINUS) {
        TokenType op = currentToken.type;
        nextToken();
        int right = term();
        
        if (op == TOKEN_PLUS) {
            result = result + right;
        } else {
            result = result - right;
        }
    }
    
    return result;
}

// Parser: Statement → Declaration | PrintStmt
void statement() {
    int statementLine = currentToken.lineNumber;  // Track statement start line
    
    // Declaration: "int" IDENTIFIER "=" Expression ";"
    if (currentToken.type == TOKEN_INT) {
        nextToken();
        
        if (currentToken.type != TOKEN_IDENT) {
            error("Expected identifier after 'int'");
        }
        
        char name[256];
        strcpy(name, currentToken.value);
        nextToken();
        
        expect(TOKEN_ASSIGN, "Expected '=' in declaration");
        
        int value = expression();
        
        if (currentToken.type != TOKEN_SEMICOLON) {
            // Report error at the statement line
            int savedLine = currentToken.lineNumber;
            currentToken.lineNumber = statementLine;
            error("Expected ';' at end of statement");
            currentToken.lineNumber = savedLine;
        }
        nextToken();
        
        setVariable(name, value);
        return;
    }
    
    // PrintStmt: "print" "(" Expression ")" ";"
    if (currentToken.type == TOKEN_PRINT) {
        nextToken();
        
        expect(TOKEN_LPAREN, "Expected '(' after 'print'");
        
        int value = expression();
        
        expect(TOKEN_RPAREN, "Expected ')' after expression");
        expect(TOKEN_SEMICOLON, "Expected ';' at end of statement");
        
        printf("%d\n", value);
        return;
    }
    
    error("Expected 'int' or 'print' statement");
}

// Parser: Program → Statement*
void parse() {
    while (currentToken.type != TOKEN_EOF) {
        statement();
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: ./parser inputfile\n");
        return 1;
    }
    
    // Read source file
    FILE *file = fopen(argv[1], "r");
    if (!file) {
        printf("Error: Cannot open file '%s'\n", argv[1]);
        return 1;
    }
    
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    source = malloc(size + 1);
    if (!source) {
        printf("Error: Memory allocation failed\n");
        fclose(file);
        return 1;
    }
    
    fread(source, 1, size, file);
    source[size] = '\0';
    fclose(file);
    
    // Start parsing
    nextToken();
    parse();
    
    free(source);
    return 0;
}