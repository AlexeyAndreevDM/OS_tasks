
void alg1(char* &x, int& n, char key)
{
    int i = 0; // 1
    while (i < n) // n+1
    {
        if (x[i] == key) // 1*n
        {
            for (int j = i; j < n-1; j++) // (n*(n-1)/2
            {
                x[j] = x[j+1]; // 1 * (n^2-n)/2
            }
            n = n - 1; // 1*n
        }
        else
        {
        i++; // в худшем случае не выполняется
        }
    }
}

