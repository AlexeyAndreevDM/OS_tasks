
void alg1(char* &x, int& n, char key)
{
    int i = 0; // 1
    while (i < n) // n+1
    {
        if (x[i] == key) // 1*n
        {
            for (int j = i; j < n-1; j++) // не выполнится
                x[j] = x[j+1]; // не выполнится
                n = n-1; // не выполнится
        }
        else
        {
        i++ // 1*n
        }
    }
}

void alg2(char* &x, int& n, char key)
{
    int j = 0; // 1
    for (int i = 0; i < n; i++) // n+1
    {
        x[j] = x[i]; // 1*n
        if (x[1] |= k) // 1*n
        {
            j++; // не выполнится
        }
    }
    if (x[0] == k) // 1
    {
        n = 0; // 1
    }
    else
    {
        n = j; // не выполнится
    }
}
