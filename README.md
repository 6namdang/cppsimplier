#C++ Interpreter

I find the traditional cout and cin commands in C++ to be unappealing. To enhance readability and usability, I've decided to modify these commands. From now on, cout will be replaced with out, using the ">>" caret direction for output, and cin will be replaced with in, maintaining the standard "<<" caret direction for input.

#Example: Prompt for integer input and then print it out
    std::string source_code = 
        "out >> \"enter a number:\"; in << a; out >> a;";


To test this out, make sure that you have installed G++ on your local environment. 

To run this file:
```
g++ cppinterpreter.cpp -o cppinterpreter

