#include <iostream>
#include <cstdio>
#include <fstream>
#include <sstream>
#include "lexer.hpp"
using namespace std;
int main(int argc,char ** argv){
    // stringstream s;

    // s <<"int main(){"
    //     "printf(hello world);&"
    //     "return 1;}";

    // Tokenizer a(s.str());
    // a.display();
    // // string s="({[,=]})";
    // // cout<<endl<<s.find(",")<<endl;
    // cout<<string::npos;

    if (argc<2){
        cout<< "proper Usage:: generics_trial.cpp <filename>"<<endl;
        exit(EXIT_FAILURE);
    }
    else{
        Tokenizer x=Tokenizer("new.txt");
        x.tokenization();

    }

}