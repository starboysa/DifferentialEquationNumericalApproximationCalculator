///////////////////////////////////////////////////////////////////////////////
//                     Differential Equation Numerical Approximation Calculator
///////////////////////////////////////////////////////////////////////////////
// A somewhat featured calculator language that is used to run Numerical
// Approximation on a first order ODE using:
//     Euler Method
//     Improved Euler Method
//     Runge Kutta (4)
//
///////////////////////////////////////////////////////////////////////////////

#include <cmath>
#include <iostream>
#include <string>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
//                                                       Normal People Solution
///////////////////////////////////////////////////////////////////////////////
// Initial solution with hard coded ODEs.
//
///////////////////////////////////////////////////////////////////////////////

// Decided to make this a class because at the time I didn't know where I was
// going with this assignment so being able to develop the assignment functions
// first.  Getting them working with hard coded differential eqs was
// important because I used the hard coded answers to check my calculator's
// correctness.
struct Input
{
  virtual float yPrime(float t, float y) = 0;

  float mT0;
  float mY0;
  float mTEnd;
};

///////////////////////////////////////////////////////////////////////////////
//                                                  Actual Assignment Functions
///////////////////////////////////////////////////////////////////////////////

float EulerMethod(Input* in, float h)
{
  int tCount = round((in->mTEnd - in->mT0) / h);
  float Yn = in->mY0;
  float Tn = in->mT0;

  for(int i = 0; i < tCount; ++i)
  {
    Yn = Yn + h * in->yPrime(Tn, Yn);
    Tn = Tn + h;
  }

  return Yn;
}

float ImprovedEulerMethod(Input* in, float h)
{
  int tCount = round((in->mTEnd - in->mT0) / h);
  float Yn = in->mY0;
  float Tn = in->mT0;

  for(int i = 0; i < tCount; ++i)
  {
    float left = in->yPrime(Tn, Yn);
    float right = in->yPrime(Tn + h, Yn + h * left);
    Yn = Yn + ((left + right) / 2) * h;
    Tn = Tn + h;
  }

  return Yn;
}

float RungeKutta(Input* in, float h)
{
  int tCount = round((in->mTEnd - in->mT0) / h);
  float Yn = in->mY0;
  float Tn = in->mT0;

  for (int i = 0; i < tCount; ++i)
  {
    float Kn1 = in->yPrime(Tn, Yn);
    float Kn2 = in->yPrime(Tn + h/2, Yn + h/2 * Kn1);
    float Kn3 = in->yPrime(Tn + h/2, Yn + h/2 * Kn2);
    float Kn4 = in->yPrime(Tn + h, Yn + h * Kn3);

    Yn = Yn + (h / 6) * (Kn1 + 2 * Kn2 + 2 * Kn3 + Kn4);
    Tn = Tn + h;
  }

  return Yn;
}

///////////////////////////////////////////////////////////////////////////////
//                                                          Hard Coded Diff Eqs
///////////////////////////////////////////////////////////////////////////////

struct HW6P5 : public Input
{
  HW6P5()
  {
    mT0 = 1;
    mY0 = -5;
    mTEnd = 2;
  }

  float yPrime(float t, float y) override
  {
    return 1 - 5 * t - 2 * y;
  }
};

struct HW6P6 : public Input
{
  HW6P6()
  {
    mT0 = 0.1;
    mY0 = 3;
    mTEnd = 1.1;
  }

  float yPrime(float t, float y) override
  {
    return t - 1.5 * y;
  }
};

struct TestExample1 : public Input
{
  TestExample1()
  {
    mT0 = 0;
    mY0 = 1;
    mTEnd = 1;
  }

  float yPrime(float t, float y) override
  {
    return t * t + y * y;
  }
};

struct TestExample2 : public Input
{
  TestExample2()
  {
    mT0 = 1;
    mY0 = 3;
    mTEnd = 2;
  }

  float yPrime(float t, float y) override
  {
    return sqrt(t + y);
  }
};

///////////////////////////////////////////////////////////////////////////////
//                                                       Insane People Solution
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//                                                                    Tokenizer
///////////////////////////////////////////////////////////////////////////////

// I'm making my y and t variables and my trig functions specific tokens to 
// make the AST interpreter trivial to write. If this was a more generic 
// calculator I'd need to  actually do function calls and variable lookup
// with semantic analysis, but trying to keep this as easy to implement as
// possible.
enum class TokenType
{
  Y,
  T,
  LiteralE,
  Number,
  Add,
  Minus,
  Asterisk,
  Divide,
  Power,
  Sqrt,
  OpenParens,
  CloseParens,
  TrigSin,
  TrigCos,
  TrigTan,

  TOTAL,

  BAD_TYPE
};

struct Token
{
  std::string mStr;
  TokenType mType = TokenType::BAD_TYPE;
};

///////////////////////////////////////////////////////////////////////////////
//                                                                 Parser / AST
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Grammer:
// 
// Expression0 = Expression1 ((+ -) Expression1)*
// Expression1 = Expression2 ((* /) Expression2)* (Expression2NoNegation)*
// Expression2 = Expression3 ((^) Expression3)*
// Expression3 = Expression4 | $Expression4 | -Expression4 | tan(Expression4) | sin(Expression4) | cos(Expression4)
// Expression2NoNegation = Expression3NoNegation ((^) Expression3NoNegation)
// Expression3NoNegation = Expression4 | $Expression4 | -Expression4 | tan(Expression4) | sin(Expression4) | cos(Expression4)
// Expression4 = Y | T | Number | ( Expression1 )
// 
// The odd NoNegation stuff allows us to do implicit multiplication (i.e. 3t = 3 * t)
// without the NoNegation this happens: y - 5 -> y * (-5)
// In a fuller language we'd have other operators (Unary+, dereferenceing) to
// do this for as well.
//
///////////////////////////////////////////////////////////////////////////////

// AST and Viistor Def.  The NoNegation nodes just user their regular node types, only the Parsing changes for those.
struct AbstractNode;
struct YNode;
struct TNode;
struct ENode;
struct NumberNode;
struct Expression0Node;
struct Expression1Node;
struct Expression2Node;
struct Expression3Node;

struct Visitor
{
  virtual bool Visit(AbstractNode* n) { return true; }
  virtual bool Visit(YNode* n) { return true; }
  virtual bool Visit(TNode* n) { return true; }
  virtual bool Visit(ENode* n) { return true; }
  virtual bool Visit(NumberNode* n) { return true; }
  virtual bool Visit(Expression0Node* n) { return true; }
  virtual bool Visit(Expression1Node* n) { return true; }
  virtual bool Visit(Expression2Node* n) { return true; }
  virtual bool Visit(Expression3Node* n) { return true; }
};

struct AbstractNode
{
  virtual void Walk(Visitor* v) = 0;
};

struct YNode : public AbstractNode
{
  virtual void Walk(Visitor* v) { v->Visit(this); };
};

struct TNode : public AbstractNode
{
  virtual void Walk(Visitor* v) { v->Visit(this); };
};

struct ENode : public AbstractNode
{
  virtual void Walk(Visitor* v) { v->Visit(this); };
};

struct NumberNode : public AbstractNode
{
  virtual void Walk(Visitor* v) { v->Visit(this); }

  Token mToken;
};

struct Expression2Node : public AbstractNode
{
  virtual void Walk(Visitor* v)
  {
    if (v->Visit(this))
    {
      mLeft->Walk(v);
      mRight->Walk(v);
    }
  }

  AbstractNode* mLeft;
  AbstractNode* mRight;
  Token mToken;
};

struct Expression1Node : public AbstractNode
{
  virtual void Walk(Visitor* v)
  {
    if (v->Visit(this))
    {
      mLeft->Walk(v);
      mRight->Walk(v);
    }
  }

  AbstractNode* mLeft;
  AbstractNode* mRight;
  Token mToken;
};

struct Expression0Node : public AbstractNode
{
  virtual void Walk(Visitor* v)
  {
    if (v->Visit(this))
    {
      mLeft->Walk(v);
      mRight->Walk(v);
    }
  }

  AbstractNode* mLeft;
  AbstractNode* mRight;
  Token mToken;
};

struct Expression3Node : public AbstractNode
{
  virtual void Walk(Visitor* v)
  {
    if (v->Visit(this))
    {
      mChild->Walk(v);
    }
  }

  AbstractNode* mChild;
  Token mToken;
};

///////////////////////////////////////////////////////////////////////////////
//                                                                       Parser
///////////////////////////////////////////////////////////////////////////////
// Just the implementation of the grammer.  Refer back to the previous header
// to see the grammer in regex format.
//
///////////////////////////////////////////////////////////////////////////////

struct Parser
{
  Parser(std::vector<Token> tokens) : mTokens(tokens)
  {

  }

  AbstractNode* GetAST()
  {
    return Expression0();
  }

  AbstractNode* Expression0()
  {
    auto ex3 = Expression1();
    if (ex3)
    {
      AbstractNode* prevNode = ex3;
      Token t;

      while (Accept(TokenType::Add, t) || Accept(TokenType::Minus, t))
      {
        auto ex1 = new Expression0Node();
        ex1->mLeft = prevNode;
        ex1->mToken = t;
        ex1->mRight = Expect(Expression1());
        prevNode = ex1;
      }

      return prevNode;
    }

    return nullptr;
  }

  AbstractNode* Expression1()
  {
    auto ex4 = Expression2();
    if (ex4)
    {
      AbstractNode* prevNode = ex4;
      Token t;

      while (Accept(TokenType::Divide, t) || Accept(TokenType::Asterisk, t))
      {
        auto ex2 = new Expression1Node();
        ex2->mLeft = prevNode;
        ex2->mToken = t;
        ex2->mRight = Expect(Expression2());
        prevNode = ex2;
      }

      AbstractNode* ex2;
      while (ex2 = Expression2NoNegation())
      {
        auto ex1 = new Expression1Node();
        ex1->mLeft = prevNode;
        ex1->mToken = { "*", TokenType::Asterisk };
        ex1->mRight = ex2;
        prevNode = ex1;
      }

      return prevNode;
    }

    return nullptr;
  }

  AbstractNode* Expression2NoNegation()
  {
    auto ex1 = Expression3NoNegation();
    if (ex1)
    {
      AbstractNode* prevNode = ex1;
      Token t;

      while (Accept(TokenType::Power, t))
      {
        auto ex1 = new Expression2Node();
        ex1->mLeft = prevNode;
        ex1->mToken = t;
        ex1->mRight = Expect(Expression3NoNegation());
        prevNode = ex1;
      }

      return prevNode;
    }

    return nullptr;
  }

  AbstractNode* Expression2()
  {
    auto ex1 = Expression3();
    if (ex1)
    {
      AbstractNode* prevNode = ex1;
      Token t;

      while (Accept(TokenType::Power, t))
      {
        auto ex1 = new Expression2Node();
        ex1->mLeft = prevNode;
        ex1->mToken = t;
        ex1->mRight = Expect(Expression3());
        prevNode = ex1;
      }

      return prevNode;
    }

    return nullptr;
  }

  AbstractNode* Expression3NoNegation()
  {
    Token t;
    if (Accept(TokenType::Sqrt, t) || Accept(TokenType::TrigTan, t) || Accept(TokenType::TrigSin, t) || Accept(TokenType::TrigCos, t))
    {
      auto ex1 = new Expression3Node();
      ex1->mToken = t;
      ex1->mChild = Expect(Expression4());

      return ex1;
    }
    else
    {
      return Expression4();
    }
  }

  AbstractNode* Expression3()
  {
    Token t;
    if (Accept(TokenType::Sqrt, t) || Accept(TokenType::Minus, t) || Accept(TokenType::TrigTan, t) || Accept(TokenType::TrigSin, t) || Accept(TokenType::TrigCos, t))
    {
      auto ex1 = new Expression3Node();
      ex1->mToken = t;
      ex1->mChild = Expect(Expression4());

      return ex1;
    }
    else
    {
      return Expression4();
    }
  }

  AbstractNode* Expression4()
  {
    auto y = Y();
    if (y) return y;

    auto t = T();
    if (t) return t;

    auto e = E();
    if (e) return e;

    auto n = Number();
    if (n) return n;

    if (Accept(TokenType::OpenParens))
    {
      auto ex1 = Expression0();
      Expect(TokenType::CloseParens);

      return ex1;
    }

    return nullptr;
  }

  YNode* Y()
  {
    if (Accept(TokenType::Y))
    {
      return new YNode();
    }

    return nullptr;
  }

  TNode* T()
  {
    if (Accept(TokenType::T))
    {
      return new TNode();
    }

    return nullptr;
  }

  ENode* E()
  {
    if (Accept(TokenType::LiteralE))
    {
      return new ENode();
    }

    return nullptr;
  }

  NumberNode* Number()
  {
    Token t;
    if (Accept(TokenType::Number, t))
    {
      auto n = new NumberNode();
      n->mToken = t;
      return n;
    }

    return nullptr;
  }

  bool Accept(TokenType type, Token& t)
  {
    if (mTokens.size() <= mPosition) return false;

    if (mTokens[mPosition].mType == type)
    {
      t = mTokens[mPosition];
      ++mPosition;
      return true;
    }

    return false;
  }

  bool Accept(TokenType type)
  {
    Token t;
    return Accept(type, t);;
  }

  bool Expect(TokenType type, Token& t)
  {
    return Expect(Accept(type, t));
  }

  bool Expect(TokenType type)
  {
    return Expect(Accept(type));
  }

  template<typename T>
  T Expect(T b)
  {
    if (!b) // for false and nullptr
    {
      // __debugbreak();
      // throw "BAD";
      mError = true;
      mErrorString = "Failed during parsing.  The equation isn't gramatically correct.";
    }

    return std::move(b);
  }

  bool InError() { return mError; }
  std::string GetErrorString() { return mErrorString; }

  bool mError = false;
  std::string mErrorString;
  std::vector<Token> mTokens;
  int mPosition = 0;
};

///////////////////////////////////////////////////////////////////////////////
//                                                              AST Interpreter
///////////////////////////////////////////////////////////////////////////////

struct ExecutionVisitor : public Visitor
{
  float mT;
  float mY;

  float mLastVal;

  virtual bool Visit(YNode* n)
  {
    mLastVal = mY;
    return false;
  }

  virtual bool Visit(TNode* n)
  {
    mLastVal = mT;
    return false;
  }

  virtual bool Visit(ENode* n)
  {
    mLastVal = std::exp(1);
    return false;
  }

  virtual bool Visit(NumberNode* n)
  {
    mLastVal = atof(n->mToken.mStr.c_str());
    return false;
  }

  virtual bool Visit(Expression2Node* n)
  {
    n->mLeft->Walk(this);
    float leftVal = mLastVal;

    if (n->mRight)
    {
      n->mRight->Walk(this);
      float rightVal = mLastVal;

      switch (n->mToken.mType)
      {
      case TokenType::Power:
        mLastVal = std::pow(leftVal, rightVal);
        break;
      }
    }

    return false;
  }

  virtual bool Visit(Expression0Node* n)
  {
    n->mLeft->Walk(this);
    float leftVal = mLastVal;

    if (n->mRight)
    {
      n->mRight->Walk(this);
      float rightVal = mLastVal;

      switch (n->mToken.mType)
      {
      case TokenType::Add:
        mLastVal = leftVal + rightVal;
        break;
      case TokenType::Minus:
        mLastVal = leftVal - rightVal;
        break;
      }
    }

    return false;
  }

  virtual bool Visit(Expression1Node* n)
  {
    n->mLeft->Walk(this);
    float leftVal = mLastVal;

    if (n->mRight)
    {
      n->mRight->Walk(this);
      float rightVal = mLastVal;

      switch (n->mToken.mType)
      {
      case TokenType::Asterisk:
        mLastVal = leftVal * rightVal;
        break;
      case TokenType::Divide:
        mLastVal = leftVal / rightVal;
        break;
      }
    }
    return false;
  }

  virtual bool Visit(Expression3Node* n)
  {
    n->mChild->Walk(this);
    float rightVal = mLastVal;

    switch (n->mToken.mType)
    {
    case TokenType::Minus:
      mLastVal = -rightVal;
      break;
    case TokenType::Sqrt:
      mLastVal = sqrt(rightVal);
      break;
    case TokenType::TrigTan:
      mLastVal = tan(rightVal);
      break;
    case TokenType::TrigSin:
      mLastVal = sin(rightVal);
      break;
    case TokenType::TrigCos:
      mLastVal = cos(rightVal);
      break;
    }

    return false;
  }
};

///////////////////////////////////////////////////////////////////////////////
//                                                                    Tokenizer
///////////////////////////////////////////////////////////////////////////////

#define DFA_END { mError = true;  mErrorString = "Unknown input '" + std::string{input[i]} + "' at position " + std::to_string(i); i = input.size() + 1; }

struct ExperimentalInputtedFunction : public Input
{
  void FromInput(std::string input)
  {
    std::vector<Token> tokens;
    std::string activeToken = "";

    // DFA impemented using switch statements.  Getto, I know.

    // 0 -> default
    // 1 -> reading number pre decimal
    // 2 -> reading number post decimal
    // 3, 4 -> trig tan
    // 5 -> s for sin or sqrt
    // 6 -> trig sin
    // 7, 8 -> trig cos
    // 9, 10 -> sqrt
    int nodeState = 0; 

    for(int i = 0; i < input.size() + 1; ++i)
    {
      if (nodeState == 0)
      {
        switch (input[i])
        {
        case 'y': tokens.push_back({ { input[i] }, TokenType::Y });
          break;
        case 't': 
          activeToken += input[i];
          nodeState = 3;
          break;
        case 'x': tokens.push_back({ { input[i] }, TokenType::T });
          break;

        case 's':
          activeToken += input[i];
          nodeState = 5;
          break;

        case 'c':
          activeToken += input[i];
          nodeState = 7;
          break;

        case 'e': tokens.push_back({{ input[i] }, TokenType::LiteralE });
          break;

        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
          activeToken += input[i];
          nodeState = 1;
          break;

        case '+': tokens.push_back({ { input[i] }, TokenType::Add });
          break;
        case '-': tokens.push_back({ { input[i] }, TokenType::Minus });
          break;
        case '*': tokens.push_back({ { input[i] }, TokenType::Asterisk });
          break;
        case '/': tokens.push_back({ { input[i] }, TokenType::Divide });
          break;
        case '^': tokens.push_back({ { input[i] }, TokenType::Power });
          break;

        case '$': // sqrt
          tokens.push_back({ { input[i] }, TokenType::Sqrt });
          break;

        case '(': tokens.push_back({ { input[i] }, TokenType::OpenParens });
          break;
        case ')': tokens.push_back({ { input[i] }, TokenType::CloseParens });
          break;

        case ' ':
        case '\t':
        case '\n':
        case '\0':
          break;

        default:
          DFA_END;
        }
      }
      else if(nodeState == 1)
      {
        switch (input[i])
        {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
          activeToken += input[i];
          break;

        case '.':
          activeToken += input[i];
          nodeState = 2;
          break;

        default:
          tokens.push_back({ activeToken, TokenType::Number });
          activeToken.clear();
          i -= 1;
          nodeState = 0;
        }
      }
      else if (nodeState == 2)
      {
        switch (input[i])
        {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
          activeToken += input[i];
          break;

        default:
          tokens.push_back({ activeToken, TokenType::Number });
          activeToken.clear();
          i -= 1;
          nodeState = 0;
        }
      }
      else if(nodeState == 3)
      {
        if(input[i] == 'a')
        {
          activeToken += input[i];
          nodeState = 4;
        }
        else
        {
          tokens.push_back({ activeToken, TokenType::T });
          activeToken.clear();
          i -= 1;
          nodeState = 0;
        }
      }
      else if (nodeState == 4)
      {
        if(input[i] == 'n')
        {
          tokens.push_back({ activeToken, TokenType::TrigTan });
          activeToken.clear();
          nodeState = 0;
        }
        else
        {
          DFA_END;
        }
      }
      else if (nodeState == 5)
      {
        if (input[i] == 'i')
        {
          activeToken += input[i];
          nodeState = 6;
        }
        else if(input[i] == 'q')
        {
          activeToken += input[i];
          nodeState = 9;
        }
        else
        {
          DFA_END;
        }
      }
      else if (nodeState == 6)
      {
        if (input[i] == 'n')
        {
          tokens.push_back({ activeToken, TokenType::TrigSin });
          activeToken.clear();
          nodeState = 0;
        }
        else
        {
          DFA_END;
        }

      }
      else if (nodeState == 7)
      {
        if (input[i] == 'o')
        {
          activeToken += input[i];
          nodeState = 8;
        }
        else
        {
          DFA_END;
        }
      }
      else if (nodeState == 8)
      {
        if (input[i] == 's')
        {
          tokens.push_back({ activeToken, TokenType::TrigCos });
          activeToken.clear();
          nodeState = 0;
        }
        else
        {
          DFA_END;
        }
      }
      else if (nodeState == 9)
      {
        if (input[i] == 'r')
        {
          activeToken += input[i];
          nodeState = 10;
        }
        else
        {
          DFA_END;
        }
      }
      else if (nodeState == 10)
      {
        if (input[i] == 't')
        {
          tokens.push_back({ activeToken, TokenType::Sqrt });
          activeToken.clear();
          nodeState = 0;
        }
        else
        {
          DFA_END;
        }
      }
    }

    Parser p(tokens);
    mRoot = p.GetAST();
    if(p.InError())
    {
      mError = true;
      mErrorString = p.GetErrorString();
    }
  }

  float yPrime(float t, float y) override
  {
    if (mError)
    {
      std::cout << mErrorString.c_str() << std::endl;
      return 0.0f;
    }

    ExecutionVisitor ev;
    ev.mT = t;
    ev.mY = y;
    mRoot->Walk(&ev);

    return ev.mLastVal;
  }

  AbstractNode* mRoot;
  bool mError = false;
  std::string mErrorString;
};

///////////////////////////////////////////////////////////////////////////////
//                                                 Boring Application Interface
///////////////////////////////////////////////////////////////////////////////

void main()
{
  std::cout << "y' = ";

  std::string fullLine;
  while (std::getline(std::cin, fullLine))
  {
    ExperimentalInputtedFunction input;
    input.FromInput(fullLine);
    if(input.mError)
    {
      std::cout << input.mErrorString << std::endl;
      std::cout << "Exit Application? (y/n): ";
      char c;
      std::cin >> c;

      if (c == 'y')
      {
        break;
      }
      else
      {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << std::endl << "y' = ";
        continue;
      }
    }

    std::cout << "t0 = ";
    float t0;
    std::cin >> t0;

    std::cout << "y0 = ";
    float y0;
    std::cin >> y0;

    std::cout << "tEnd = ";
    float tEnd;
    std::cin >> tEnd;

    input.mT0 = t0;
    input.mY0 = y0;
    input.mTEnd = tEnd;

    float h;
    std::cout << "Input step size (h): ";
    while (std::cin >> h)
    {
      std::cout << std::endl;
      std::cout << "Euler Method: ";
      std::cout << EulerMethod(&input, h) << std::endl;

      std::cout << "Improved Euler Method: ";
      std::cout << ImprovedEulerMethod(&input, h) << std::endl;

      std::cout << "Runge Kutta: ";
      std::cout << RungeKutta(&input, h) << std::endl << std::endl;

      std::cout << "Input step size h (anything but a number to exit): ";
    }

    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cout << std::endl << "y' = ";
  }
}
