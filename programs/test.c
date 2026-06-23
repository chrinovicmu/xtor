int add(int a, int b);
int subtract(int a, int b);
int multiply(int a, int b);
int compute_sum(int n);

int add(int a, int b) {
    return a + b;
}

int subtract(int a, int b) {
    return a - b;
}

int multiply(int a, int b) {
    return a * b;
}

int compute_sum(int n) {
    int total = 0;
    for (int i = 1; i <= n; i++) {
        total = add(total, i);
    }
    return total;
}

int main(void) {
    int x = 10;
    int y = 5;

    int res1 = add(x, y);       
    int res2 = subtract(x, y);  
    int res3 = multiply(res1, res2); 

    int loop_res = compute_sum(5); 

    int final_score = add(res3, loop_res); 

    return final_score; 
}

