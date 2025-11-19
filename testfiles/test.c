#include <stdio.h>
#include <stdlib.h>


int add(int x, int y)
{
    int res = x + y; 

    return res; 
}

int main(int argc, char *argv[])
{
    int x = 6; 
    int y = 1; 

    int res = add(x, y); 
    printf("res = %d\n", res); 

    return EXIT_SUCCESS;
}
