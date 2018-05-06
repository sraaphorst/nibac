// superduper.c
//
// Code graciously provided by Dr. Rudi Mathon.

int C[100][100];
int flag_first_call=0;

// initialize binomial matrix
void init_super_duper(int n) 
{
  int i,j;
  for (i=0; i<=n; i++) {
    C[i][1]=i;
    C[i][0]=1;
  }
  for (j=2; j<=n; j++) {
    C[j][j]=1;
    for (i=0; i<j; i++) C[i][j]=0;
    for (i=j+1; i<=n ; i++) 
      C[i][j]=C[i-1][j-1]+C[i-1][j];
  }
  for (i=0; i<=n; i++) {
    C[i][1]=i;
    C[i][0]=1;
  }
  flag_first_call=n;  
}


int super (int n, int k, int *vec)
{
  int i,p;
  if (flag_first_call < n ) init_super_duper(n);
   
  p = C[n][k];
  for (i=0; i<k; i++) p -=  C[n-vec[i]-1][k-i];
  return (p-1);
}

void duper (int n, int k, int num,int *vec)
{
  int i, j, ni, ki, s;
  if (flag_first_call < n ) init_super_duper(n);
  
  ni = C[n][k];  j = n;  ki = k;  s = num+1;
  for (i=0; i<k-1;i++) {
    while (s > ni-C[j][ki]) j -= 1;
    vec[i] = n-j-1; s += (C[j+1][ki]-ni);
    ki -= 1;  ni = C[j][ki];
  }
  vec[k-1] = n+s-ni-1;
}

