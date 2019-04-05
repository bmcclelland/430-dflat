#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

#include "lexer.hpp"
#include "parser.hpp"
#include "typechecker.hpp"
#include "codegenerator.hpp"
#include "config.hpp"

using namespace std;
using namespace dflat;

int main(int argc, char* argv[])
{
    string fileName;

    if(argc == 2)
    {
        //read in a file name from command line:
        fileName = argv[1];
    }
    else
    {
        cout << "Usage: dflat SOURCEFILE" << endl;
        return 0;
    }

    ifstream file(fileName);
    stringstream buffer;
    string fileContents;

    if(file.is_open())
    {
        buffer << file.rdbuf();
        fileContents = buffer.str();
    }
    else
    {
        cout << "Error: File not found!\n";
        return 0;
    }

    try
    {
        //Take file contents and run compiler:

        //Run Lexer:
        Vector<TokenPtr> tokens = tokenize(fileContents);
        //for (TokenPtr const& token : tokens)
            //cout << token->toString() << endl;

        //Run Parser:
        //config::traceParse = true;
        Vector<ASNPtr> program = parse(tokens);
        //for (ASNPtr const& decl : program)
           //cout << decl->toString() << endl << endl;

        //Run TypeChecker:
        //config::traceTypeCheck = true;
        TypeEnv typeEnv = typeCheck(program);

        // Run CodeGenerator:
        String output = codeGenerator(program);
        std::cout << output << endl;
    }
    catch(exception& e)
    {
        cout << e.what() << endl;
    }

    return 0;
}
