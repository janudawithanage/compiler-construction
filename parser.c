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
} Token;

// Symbol table for variables
typedef struct {
    char name[256];
    int value;
} Variable;

Variable variables[100];
int varCount = 0;

// Global state
char *source;
int pos = 0;
Token currentToken;

// Function prototypes
void nextToken();
void parse();
void statement();
int expression();
int term();
int factor();
void error(const char *msg);

// Tokenizer implementation
void nextToken() {
    // Skip whitespace
    while (source[pos] && isspace(source[pos])) pos++;
    
    if (source[pos] == '\0') {
        currentToken.type = TOKEN_EOF;
        return;
    }
    
    // Check for keywords and identifiers
    if (isalpha(source[pos])) {
        int start = pos;
        while (isalnum(source[pos])) pos++;
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
    switch (source[pos]) {
        case '+': currentToken.type = TOKEN_PLUS; break;
        case '-': currentToken.type = TOKEN_MINUS; break;
        case '*': currentToken.type = TOKEN_MULTIPLY; break;
        case '/': currentToken.type = TOKEN_DIVIDE; break;
        case '=': currentToken.type = TOKEN_ASSIGN; break;
        case ';': currentToken.type = TOKEN_SEMICOLON; break;
        case '(': currentToken.type = TOKEN_LPAREN; break;
        case ')': currentToken.type = TOKEN_RPAREN; break;
        default: currentToken.type = TOKEN_ERROR;
    }
    pos++;
}

// TODO: Implement parser functions (statement, expression, term, factor)
// TODO: Implement symbol table lookup and storage
// TODO: Implement error handling

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: ./parser inputfile\n");
        return 1;
    }
    
    // Read source file
    FILE *file = fopen(argv[1], "r");
    if (!file) {
        printf("Error: Cannot open file %s\n", argv[1]);
        return 1;
    }
    
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    source = malloc(size + 1);
    fread(source, 1, size, file);
    source[size] = '\0';
    fclose(file);
    
    // Start parsing
    nextToken();
    parse();
    
    free(source);
    return 0;
}