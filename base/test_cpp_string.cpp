
#include<iostream>

using namespace std;

int string_test()
{
    string s = "abc defg";

    cout << "string length: " << s.length() << endl;
    cout << "string sizeof: " << s.size() << endl;

    return 0;
}


int main()
{
    return string_test();
}

