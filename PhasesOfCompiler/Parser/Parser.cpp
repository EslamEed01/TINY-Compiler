#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
typedef long double ld;
typedef vector<int> v32;
typedef vector<ll> v64;
#define ln "\n"
#define forn(i,e) for(ll i = 0; i < e; i++)
#define forsn(i,s,e) for(ll i = s; i < e; i++)
#define dbg(x) cout<<#x<<" = "<<x<<endl;
#define TRUE 1
#define FALSE 0
#define MAXCHILDREN 3
#define MAXRESERVED 8
#define MAX_TOKEN_LEN 40
#define BuffLen 256

FILE *source;
FILE *tiny_out;
FILE *code;


void Copy(char* a, const char* b, int n=0)
{
    if(n>0) {strncpy(a, b, n); a[n]=0;}
    else strcpy(a, b);
}
int Error; 
int TraceParse =TRUE;
int TraceScan= FALSE;
int LingthOfN = 0;
int EchoSource = TRUE;
static int EOF_flag = FALSE;
char tokenString[MAX_TOKEN_LEN + 1];
static char lineBuf[BuffLen];
static int Line_Pos = 0;
static int Buff_size = 0;

enum TokenType
{
    ENDFILE,ERROR,IF,THEN,ELSE,END,REPEAT,UNTIL,READ,WRITE,ID,
    NUM,ASSIGN,EQ,LT,PLUS,MINUS,TIMES,OVER,LPAREN,RPAREN,SEMI
};
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
 enum NodeKind {StmtK,ExpK};
 enum StmtKind {IfK,RepeatK,AssignK,ReadK,WriteK};
 enum ExpKind {OpK,ConstK,IdK};
 enum ExpType {Void,Integer,Boolean};


enum StateType
{
  START,INASSIGN,NCOMMENT,INNUM,INID,DONE,INCOMMENT
} ;
typedef struct treeNode
   { struct treeNode * child[MAXCHILDREN];
     struct treeNode * sibling;
     int LingthOfN;
     NodeKind nodekind;
     union { StmtKind stmt; ExpKind exp;} kind;
     union { TokenType op;
             int val;
             char * name; } attr;
     ExpType type; 
   } TreeNode;

static TokenType token;
TreeNode *  parse ();

TreeNode * newStmtNode(StmtKind kind);

TreeNode * NewExpNode(ExpKind kind);


char * copyString(char * s)
{ int n;
  char * t;
  if (s==nullptr ) return nullptr ;
  n = strlen(s)+1;
  t =(char*) (n);
  if (t==nullptr )
    std::cerr<<tiny_out<<"Out of memory error at line\n"<<LingthOfN;
  else strcpy(t,s);
  return t;
}
static int  indentno = 0;
#define INDENT indentno+=2
#define UNINDENT indentno-=2

static void printSpaces()
{ int i;
  for (i=0;i<indentno;i++)
    std::cerr<<tiny_out<<" ";
}
void printToken(TokenType token, const char *tokenString);
void printTree( TreeNode * tree )
{ int i;
  INDENT;
  while (tree != nullptr ) {
    printSpaces();
    if (tree->nodekind==StmtK)
    { switch (tree->kind.stmt) {
        case IfK:
          std::cerr<<(tiny_out,"If\n");
          break;
        case RepeatK:
          std::cerr<<(tiny_out,"Repeat\n");
          break;
        case AssignK:
          std::cerr<<(tiny_out,"Assign to: %s\n",tree->attr.name);
          break;
        case ReadK:
          std::cerr<<(tiny_out,"Read: %s\n",tree->attr.name);
          break;
        case WriteK:
          std::cerr<<(tiny_out,"Write\n");
          break;
        default:
          std::cerr<<(tiny_out,"Unknown ExpNode kind\n");
          break;
      }
    }
    else if (tree->nodekind==ExpK)
    { switch (tree->kind.exp) {
        case OpK:
          std::cerr<<(tiny_out,"Op: ");
          printToken(tree->attr.op,"\0");
          break;
        case ConstK:
          std::cerr<<(tiny_out,"Const: %d\n",tree->attr.val);
          break;
        case IdK:
          std::cerr<<(tiny_out,"Id: %s\n",tree->attr.name);
          break;
        default:
          std::cerr<<(tiny_out,"Unknown ExpNode kind\n");
          break;
      }
    }
    else std::cerr<<(tiny_out,"Unknown node kind\n");
    for (i=0;i<MAXCHILDREN;i++)
         printTree(tree->child[i]);
    tree = tree->sibling;
  }
  UNINDENT;
}
static TreeNode * stmt_sequence();
static TreeNode * statement();
static TreeNode * if_stmt();
static TreeNode * repeat_stmt();
static TreeNode * assign_stmt();
static TreeNode * read_stmt();
static TreeNode * write_stmt();
static TreeNode * expfun();
static TreeNode * simple_exp();
static TreeNode * term();
static TreeNode * factor();


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

static void UnGetNextChar()
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

static void syntaxError(char * message)
{ std::cerr<<(tiny_out,"\n>>> ");
  std::cerr<<(tiny_out,"Syntax error at line %d: %s",Line_Pos,message);
  Error = TRUE;
}
TokenType getToken();

static void match(TokenType expected)
{ if (token == expected) token = getToken();
  else {
    syntaxError("unexpected token -> ");
    printToken(token,tokenString);
    std::cerr<<(tiny_out," ");
  }
}
int main()
{
    TreeNode * syntaxTree;

    char P[20]="tiny.txt";
    source = fopen(P, "r");
    if (source == nullptr)
    {
        std::cerr<<(stderr, "File %s not found\n", P);
        exit(1);
    }

    tiny_out = stdout;
    std::cerr<<"\nTINY COMPILATION\n";

    syntaxTree = parse();
  if (TraceParse) {
    std::cerr<<"\nSyntax tree:\n";
    printTree(syntaxTree);
  }
}

TreeNode * stmt_sequence()
{ TreeNode * t = statement();
  TreeNode * p = t;
  while ((token!=ENDFILE) && (token!=END) &&
         (token!=ELSE) && (token!=UNTIL))
  { TreeNode * q;
    match(SEMI);
    q = statement();
    if (q!=nullptr ) {
      if (t==nullptr) t = p = q;
      else 
      { p->sibling = q;
        p = q;
      }
    }
  }
  return t;
}
TreeNode * statement()
{ TreeNode * t = nullptr;
  switch (token) {
    case IF : t = if_stmt(); break;
    case REPEAT : t = repeat_stmt(); break;
    case ID : t = assign_stmt(); break;
    case READ : t = read_stmt(); break;
    case WRITE : t = write_stmt(); break;
    default : 
    syntaxError("unexpected token -> ");
    printToken(token,tokenString);
          token = getToken();
            break;
  } 
  return t;
}

TreeNode * if_stmt()
{ TreeNode * t = newStmtNode(IfK);
  match(IF);
  if (t!=nullptr) t->child[0] = expfun();
  match(THEN);
  if (t!=nullptr) t->child[1] = stmt_sequence();
  if (token==ELSE) {
    match(ELSE);
    if (t!=nullptr) t->child[2] = stmt_sequence();
  }
  match(END);
  return t;
}

TreeNode * repeat_stmt()
{ TreeNode * t = newStmtNode(RepeatK);
  match(REPEAT);
  if (t!=nullptr) t->child[0] = stmt_sequence();
  match(UNTIL);
  if (t!=nullptr) t->child[1] = expfun();
  return t;
}

TreeNode * assign_stmt()
{ TreeNode * t = newStmtNode(AssignK);
  if ((t!=nullptr) && (token==ID))
    t->attr.name = copyString(tokenString);
  match(ID);
  match(ASSIGN);
  if (t!=nullptr) t->child[0] = expfun();
  return t;
}

TreeNode * read_stmt()
{ TreeNode * t = newStmtNode(ReadK);
  match(READ);
  if ((t!=nullptr) && (token==ID))
    t->attr.name = copyString(tokenString);
  match(ID);
  return t;
}
TreeNode * expfun()
{ TreeNode * t = simple_exp();
  if ((token==LT)||(token==EQ)) {
    TreeNode * p = NewExpNode(OpK);
    if (p!=nullptr) {
      p->child[0] = t;
      p->attr.op = token;
      t = p;
    }
    match(token);
    if (t!=nullptr)
      t->child[1] = simple_exp();
  }
  return t;
}

TreeNode * simple_exp()
{ TreeNode * t = term();
  while ((token==PLUS)||(token==MINUS))
  { TreeNode * p = NewExpNode(OpK);
    if (p!=nullptr) {
      p->child[0] = t;
      p->attr.op = token;
      t = p;
      match(token);
      t->child[1] = term();
    }
  }
  return t;
}

TreeNode * term()
{ TreeNode * t = factor();
  while ((token==TIMES)||(token==OVER))
  { TreeNode * p = NewExpNode(OpK);
    if (p!=nullptr) {
      p->child[0] = t;
      p->attr.op = token;
      t = p;
      match(token);
      p->child[1] = factor();
    }
  }
  return t;
}

TreeNode * factor()
{ TreeNode * t = nullptr ;
  switch (token) {
    case NUM :
      t = NewExpNode(ConstK);
      if ((t!=nullptr) && (token==NUM))
        t->attr.val = atoi(tokenString);
      match(NUM);
      break;
    case ID :
      t = NewExpNode(IdK);
      if ((t!=nullptr) && (token==ID))
        t->attr.name = copyString(tokenString);
      match(ID);
      break;
    case LPAREN :
      match(LPAREN);
      t = expfun();
      match(RPAREN);
      break;
    default:
      syntaxError("unexpected token -> ");
      printToken(token,tokenString);
      token = getToken();
      break;
    }
  return t;
}
TreeNode * NewExpNode(ExpKind kind)
{  TreeNode* t = new TreeNode;
  int i;
  if (t==nullptr)
    std::cerr<<"Out of memory error at line \n"<<std::endl;
  else {
    for (i=0;i<MAXCHILDREN;i++) t->child[i] = nullptr;
    t->sibling = nullptr ;
    t->nodekind = ExpK;
    t->kind.exp = kind;
    t->LingthOfN = LingthOfN;
    t->type = Void;
  }
  return t;
}


TreeNode * parse()
{ TreeNode * t;
  token = getToken();
  t = stmt_sequence();
  if (token!=ENDFILE)
    syntaxError("Code ends before file\n");
  return t;
}
TokenType getToken()
{ 
    int tokenStringIndex = 0;
    TokenType CurrToken;
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
                    CurrToken = ENDFILE;
                    break;
                case '=':
                    CurrToken = EQ;
                    break;
                case '<':
                    CurrToken = LT;
                    break;
                case '+':
                    CurrToken = PLUS;
                    break;
                case '-':
                    CurrToken = MINUS;
                    break;
                case '*':
                    CurrToken = TIMES;
                    break;
                case '/':
                    CurrToken = OVER;
                    break;
                case '(':
                    CurrToken = LPAREN;
                    break;
                case ')':
                    CurrToken = RPAREN;
                    break;
                case ';':
                    CurrToken = SEMI;
                    break;
                default:
                    CurrToken = ERROR;
                    break;
                }
            }
            break;
        case INCOMMENT:
            save = FALSE;
            if (c == EOF)
            {
                state = DONE;
                CurrToken = ENDFILE;
            }
            else if (c == '}')
                state = START;
            break;
        case INASSIGN:
            state = DONE;
            if (c == '=')
                CurrToken = ASSIGN;
            else
            {
                UnGetNextChar();
                save = FALSE;
                CurrToken = ERROR;
            }
            break;
        case INNUM:
            if (!isdigit(c))
            { 
                UnGetNextChar();
                save = FALSE;
                state = DONE;
                CurrToken = NUM;
            }
            break;
        case INID:
            if (!isalpha(c))
            { 
                UnGetNextChar();
                save = FALSE;
                state = DONE;
                CurrToken = ID;
            }
            break;
        case DONE:
        default: 
            std::cerr<<(tiny_out, "Scanner Bug: state= %d\n", state);
            state = DONE;
            CurrToken = ERROR;
            break;
        }
        if ((save) && (tokenStringIndex <= MAX_TOKEN_LEN))
            tokenString[tokenStringIndex++] = (char)c;
        if (state == DONE)
        {
            tokenString[tokenStringIndex] = '\0';
            if (CurrToken == ID)
                CurrToken = reservedLookup(tokenString);
        }
    }
    if (TraceScan)
    {
        std::cerr<<(tiny_out, "\t%d: ", LingthOfN);
        printToken(CurrToken, tokenString);
    }
    return CurrToken;
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
TreeNode * newStmtNode(StmtKind kind)
{  TreeNode* t = new TreeNode;
  int i;
  if (t==nullptr)
    std::cerr<<(tiny_out,"Out of memory error at line %d\n",LingthOfN);
  else {
    for (i=0;i<MAXCHILDREN;i++) t->child[i] = nullptr;
    t->sibling = nullptr ;
    t->nodekind = StmtK;
    t->kind.stmt = kind;
    t->LingthOfN = LingthOfN;
  }
  return t;
}

TreeNode * write_stmt()
{ TreeNode * t = newStmtNode(WriteK);
  match(WRITE);
  if (t!=nullptr) t->child[0] = expfun();
  return t;
}
