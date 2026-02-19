
void alg1(char* &x, int& n, char key)
{
    int i = 0; // 1
    while (i < n) // n+1
    {
        if (x[i] == key) // 1*n
        {
            for (int j = i; j < n-1; j++) // не выполнится
            {
                x[j] = x[j+1]; // не выполнится
            }
            n = n-1; // не выполнится
        }
        else
        {
        i++ // 1*n
        }
    }
}

