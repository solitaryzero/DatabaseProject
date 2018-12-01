%{
#include <string>

%}

%union {
    int token;
    std::string* data;    //acts as string or as some kind of char[] to store values
}