// int f(int a) {
//     return a;
// }

// int g(int a) {
//     int b;
//     int c;
//     if (a == 0) {
//         b = 1;
//         c = 1;
//     }
//     else {
//         b = 2;
//         c = 2;
//     }
//     return b + c;
// }

// int invfib(int n) {
//     if (n >= 10) {
//         return 1;
//     }
//     return invfib(n + 1) + invfib(n + 2);

// }

// int longfunc() {
//     int a = 0;
//     a = a + 1;
//     int b = 0;
//     b = b + 1;
//     int c = 0;
//     c = c + 1;
//     int d = 0;
//     d = d + 1;
//     int e = 0;
//     e = e + 1;
//     int f = 0;
//     f = f + 1;
//     int g = 0;
//     g = g + 1;
//     int h = 0;
//     h = h + 1;
//     int i = 0;
//     i = i + 1;
//     int j = 0;
//     j = j + 1;
//     int k = 0;
//     k = k + 1;
//     int l = 0;
//     l = l + 1;
//     int m = 0;
//     m = m + 1;
//     int n = 0;
//     n = n + 1;
//     int o = 0;
//     o = o + 1;
//     int p = 0;
//     p = p + 1;
//     int q = 0;
//     q = q + 1;
//     int r = 0;
//     r = r + 1;
//     int s = 0;
//     s = s + 1;
//     int t = 0;
//     t = t + 1;
//     int u = 0;
//     u = u + 1;
//     int v = 0;
//     v = v + 1;
//     int w = 0;
//     w = w + 1;
//     int x = 0;
//     x = x + 1;
//     int y = 0;
//     y = y + 1;
//     int z = 0;
//     z = z + 1;

//     return a + b + c + d + e + f + g + h + i + j + k + l + m + n + o + p + q + r + s + t + u + v + w + x + y + z;
// }

// int simple_phi_test(int a) {
//     int b;
//     if (a == 0) {
//         b = 1;
//     }
//     else {
//         b = 0;
//     }
//     return b;
// }

int fib(n) {
    if (n <= 1) {
        return 1;
    }
    return fib(n - 1) + fib(n - 2);
}

int main(void) {
    return fib(10);
}