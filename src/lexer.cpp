#include "lexer.hpp"
#include "map.hpp"

namespace dflat
{
    using namespace std;

class Lexer : private LexerCore
{
    public:
        Lexer(String const&);
        Vector<TokenPtr> tokenize();
        TokenPtr singleToken();
        TokenPtr tryTokenizeVariable();
        TokenPtr tryTokenizeNumber();
        TokenPtr tryTokenizePunct();
        //TokenPtr singleToken();
        void skipWhitespace();
        TokenPtr lookupKeyword(String const&) const;
        TokenPtr lookupPunct1(char c) const;
        TokenPtr lookupPunct2(char c1, char c2) const;
};

LexerException::LexerException(String msg) noexcept
{
    message = msg;
}

const char* LexerException::what() const noexcept
{
    return message.c_str();
}

Lexer::Lexer(String const& input)
    : LexerCore(input)
{
}

TokenPtr Lexer::tryTokenizeVariable()
{
    char c = peek();
    String var = "";

    if (!isalpha(c) && (c!='_'))
    {
        return nullptr;
    }

    while (isalpha(c) || isdigit(c) || (c=='_'))
    {
        var += get();
        c = peek();
    }

    if(var.length() == 0)
        return nullptr;
    
    TokenPtr ret = lookupKeyword(var);

    if(ret == nullptr)
        return make_unique<VariableToken>(var);

    return ret;
}

TokenPtr Lexer::tryTokenizeNumber()
{
    const size_t initialInputPos = _pos;
    String digits = "";

    char index = peek();
    while ( index && isdigit(index) )
    {
        digits += index;
        next();
        index = peek();
    }

    if ( digits.length() > 0)
    {
        return make_unique<NumberToken>(stoi(digits));
    }
    else
    {
        _pos = initialInputPos;
        return nullptr;
    }
}

TokenPtr Lexer::lookupKeyword(String const& name) const
{
    static Map<String, TokType> const KWS
    {
        { "if", tokIf },
        { "else", tokElse },
        { "for", tokFor },
        { "while", tokWhile },
        { "new", tokNew },
    };

    auto it = KWS.find(name);

    if (it == KWS.end())
    {
        return nullptr;
    }

    switch (it->second)
    {
        case tokIf: return make_unique<IfToken>();
        case tokElse: return make_unique<ElseToken>();
        case tokFor: return make_unique<ForToken>();
        case tokWhile: return make_unique<WhileToken>();
        case tokNew: return make_unique<NewToken>();
        default: return nullptr;
    }
}

TokenPtr Lexer::lookupPunct1(char c) const
{
    switch (c)
    {
        case '(': return make_unique<LeftParenToken>();
        case ')': return make_unique<RightParenToken>();
        case ',': return make_unique<CommaToken>();
        case '{': return make_unique<LeftBraceToken>();
        case '}': return make_unique<RightBraceToken>();
        case '=': return make_unique<AssignToken>();
        case '+': return make_unique<PlusToken>();
        case '-': return make_unique<MinusToken>();
        case '*': return make_unique<MultiplyToken>();
        case '/': return make_unique<DivisionToken>();
        case '!': return make_unique<NotToken>();
        case '.': return make_unique<MemberToken>();
        case '\n': return make_unique<NewlineToken>();
        default: return nullptr;
    }
}

TokenPtr Lexer::lookupPunct2(char c1, char c2) const
{
    switch (c2)
    {
        case '=':
            switch (c1)
            {
                case '=': return make_unique<EqToken>();
                case '!': return make_unique<NotEqToken>();
                default: return nullptr;
            }

        case '&':
            if (c1 == '&') return make_unique<AndToken>();
            else return nullptr;

        case '|':
            if (c1 == '|') return make_unique<OrToken>();
            else return nullptr;

        default:
            return nullptr;
    }
}

TokenPtr Lexer::tryTokenizePunct()
{
    TokenPtr tok = nullptr;

    if (tok = lookupPunct2(peek(), peek_ahead(1)))
    {
        next();
        next();
        return tok;
    }
    else if (tok = lookupPunct1(peek()))
    {
        next();
        return tok;
    }
    else
    {
        return nullptr;
    }
}

void Lexer::skipWhitespace()
{
    char c = peek();

    while (c == ' ' || c == '\t') // no \n
    {
        next();
        c = peek();
    }
}

TokenPtr Lexer::singleToken(){
    TokenPtr tok = nullptr;

    if ((tok = tryTokenizeVariable()))
    {
        return tok;
    } 
    else if ((tok = tryTokenizeNumber()))
    {
        return tok;
    }
    else if ((tok = tryTokenizePunct()))
    {
        return tok;
    }
    
    return nullptr;
}

Vector<TokenPtr> Lexer::tokenize()
{
    Vector<TokenPtr> tokens;
    TokenPtr current = nullptr;
    skipWhitespace();

    while((current = singleToken()))
    {
        tokens.push_back(move(current));
        skipWhitespace();
    }

    if(!at_end()) //If not end, but no valid token was returned above, error.
    {
        String msg = "Lexer Error: Illegal character: ";
        msg.push_back(peek());
        throw LexerException(msg);
    }

    return tokens;
}

Vector<TokenPtr> tokenize(String const& input)
{
    Lexer lexer(input);
    return lexer.tokenize();
}

} //namespace dflat
