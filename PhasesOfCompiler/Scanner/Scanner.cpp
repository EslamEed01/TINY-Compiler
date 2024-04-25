#include <bits/stdc++.h>  
#define TRUE 1
#define FALSE 0
#define MAXRESERVED 8
#define MAX_TOKEN_LEN  40
#define BuffLen 256

void Copy(char* a, const char* b, int n=0)
{
    if(n>0) {strncpy(a, b, n); a[n]=0;}
    else strcpy(a, b);
}
enum TokenType
{
    ENDFILE,ERROR,IF,THEN,ELSE,END,REPEAT,UNTIL,READ,WRITE,ID,
    NUM,ASSIGN,EQ,LT,PLUS,MINUS,TIMES,OVER,LPAREN,RPAREN,SEMI
};

enum StateType
{
  START,INASSIGN,NCOMMENT,INNUM,INID,DONE,INCOMMENT
} ;

FILE *source;
FILE* tiny_out;
FILE *code;

int LingthOfN = 0;
int EchoSource = TRUE;
static int EOF_flag = FALSE;
int TraceScan = TRUE;

char tokenString[MAX_TOKEN_LEN  + 1];
static char lineBuf[BuffLen];
static int Line_Pos = 0;
static int Buff_size = 0; 

struct Token
{
    TokenType tok;
    char str[MAX_TOKEN_LEN +1]{};

    Token(){str[0]=0; tok=ERROR;}
    Token(TokenType _type, const char* _str) {tok=_type; Copy(str, _str);}
};
const std::vector<Token> reserved_words = {
    {TokenType::IF, "if"},
    {TokenType::THEN, "then"},
    {TokenType::ELSE, "else"},
    {TokenType::END, "end"},
    {TokenType::REPEAT, "repeat"},
    {TokenType::UNTIL, "until"},
    {TokenType::READ, "read"},
    {TokenType::WRITE, "write"}
};
static int getNextChar()
{
    if (!(Line_Pos < Buff_size))
    {
        LingthOfN++;
        if (fgets(lineBuf, BuffLen - 1, source))
        {
            if (EchoSource)
                std::cerr<<(tiny_out, "%4d: %s", LingthOfN, lineBuf);
            Buff_size = strlen(lineBuf);
            Line_Pos = 0;
            return lineBuf[Line_Pos++];
        }
        else
        {
            EOF_flag = TRUE;
            return EOF;
        }
    }
    else
        return lineBuf[Line_Pos++];
}
static void ungetNextChar()
{
    Line_Pos--;
}
static TokenType reservedLookup(char *s)
{
    int i;
    for (i = 0; i < MAXRESERVED; i++)
        if (!strcmp(s, reserved_words[i].str))
            return reserved_words[i].tok;
    return ID;
}

void printToken(TokenType token, const char *tokenString)
{
    switch (token)
    {
    case IF:
    case THEN:
    case ELSE:
    case END:
    case REPEAT:
    case UNTIL:
    case READ:
    case WRITE:
        std::cerr<<(tiny_out,
                "reserved word: %s\n", tokenString);
        break;
    case ASSIGN:
        std::cerr<<(tiny_out, ":=\n");
        break;
    case LT:
        std::cerr<<(tiny_out, "<\n");
        break;
    case EQ:
        std::cerr<<(tiny_out, "=\n");
        break;
    case LPAREN:
        std::cerr<<(tiny_out, "(\n");
        break;
    case RPAREN:
        std::cerr<<(tiny_out, ")\n");
        break;
    case SEMI:
        std::cerr<<(tiny_out, ";\n");
        break;
    case PLUS:
        std::cerr<<(tiny_out, "+\n");
        break;
    case MINUS:
        std::cerr<<(tiny_out, "-\n");
        break;
    case TIMES:
        std::cerr<<(tiny_out, "*\n");
        break;
    case OVER:
        std::cerr<<(tiny_out, "/\n");
        break;
    case ENDFILE:
        std::cerr<<(tiny_out, "EOF\n");
        break;
    case NUM:
        std::cerr<<(tiny_out,
                "NUM, val= %s\n", tokenString);
        break;
    case ID:
        std::cerr<<(tiny_out,
                "ID, name= %s\n", tokenString);
        break;
    case ERROR:
        std::cerr<<(tiny_out,
                "ERROR: %s\n", tokenString);
        break;
    default: 
        std::cerr<<(tiny_out, "Unknown token: %d\n", token);
    }
}
int main()
{
    char P[40]="tiny.txt";
    source = fopen(P, "r");
    if (source == NULL)
    {
        std::cerr<<(stderr, "File %s not found\n", P);
        exit(1);
    }

    tiny_out =stdout;
    std::cerr<<(tiny_out, "\nTINY COMPILATION: %s\n", P);
    while (getToken() != ENDFILE);
}

TokenType getToken()
{ 
    int tokenStringIndex = 0;
    TokenType currentToken;
    StateType state = START;
    int save;
    while (state != DONE)
    {
        int c = getNextChar();
        save = TRUE;
        switch (state)
        {
        case START:
            if (isdigit(c))
                state = INNUM;
            else if (isalpha(c))
                state = INID;
            else if (c == ':')
                state = INASSIGN;
            else if ((c == ' ') || (c == '\t') || (c == '\n'))
                save = FALSE;
            else if (c == '{')
            {
                save = FALSE;
                state = INCOMMENT;
            }
            else
            {
                state = DONE;
                switch (c)
                {
                case EOF:
                    save = FALSE;
                    currentToken = ENDFILE;
                    break;
                case '=':
                    currentToken = EQ;
                    break;
                case '<':
                    currentToken = LT;
                    break;
                case '+':
                    currentToken = PLUS;
                    break;
                case '-':
                    currentToken = MINUS;
                    break;
                case '*':
                    currentToken = TIMES;
                    break;
                case '/':
                    currentToken = OVER;
                    break;
                case '(':
                    currentToken = LPAREN;
                    break;
                case ')':
                    currentToken = RPAREN;
                    break;
                case ';':
                    currentToken = SEMI;
                    break;
                default:
                    currentToken = ERROR;
                    break;
                }
            }
            break;
        case INCOMMENT:
            save = FALSE;
            if (c == EOF)
            {
                state = DONE;
                currentToken = ENDFILE;
            }
            else if (c == '}')
                state = START;
            break;
        case INASSIGN:
            state = DONE;
            if (c == '=')
                currentToken = ASSIGN;
            else
            {
                ungetNextChar();
                save = FALSE;
                currentToken = ERROR;
            }
            break;
        case INNUM:
            if (!isdigit(c))
            { 
                ungetNextChar();
                save = FALSE;
                state = DONE;
                currentToken = NUM;
            }
            break;
        case INID:
            if (!isalpha(c))
            { 
                ungetNextChar();
                save = FALSE;
                state = DONE;
                currentToken = ID;
            }
            break;
        case DONE:
        default: 
            std::cerr<<(tiny_out, "Scanner Bug: state= %d\n", state);
            state = DONE;
            currentToken = ERROR;
            break;
        }
        if ((save) && (tokenStringIndex <= MAX_TOKEN_LEN ))
            tokenString[tokenStringIndex++] = (char)c;
        if (state == DONE)
        {
            tokenString[tokenStringIndex] = '\0';
            if (currentToken == ID)
                currentToken = reservedLookup(tokenString);
        }
    }
    if (TraceScan)
    {
        std::cerr<<(tiny_out, "\t%d: ", LingthOfN);
        printToken(currentToken, tokenString);
    }
    return currentToken;
} 