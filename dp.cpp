/* Fibonacci series DP TopDown
*/
#include <cmath>
#include <cstdio>
#include <vector>
#include <iostream>
#include <algorithm>
using namespace std;
unsigned long long int coinCount(int coins[],int amount,int size)
{
	unsigned long long int arr[amount+1][size];
	for(int i=0;i<amount+1;i++)
		for(int j=0;j<size;j++)
			arr[i][j]=1;
    for(int i=1 ;i<=amount;i++)
	   for(int j=0;j<size;j++)
		{
			unsigned long long int x=((i-coins[j])>=0?arr[i-coins[j]][j]:0);
			unsigned long long int y=j>=1?arr[i][j-1]:0;
			arr[i][j]=x+y;
		}
	return arr[amount][size-1];
}
unsigned long long topDownFib(unsigned long long n,unsigned long long arr[])
{   if(arr[n]>0)
        return arr[n];
    else
    {
        if(n==0)
            return 0;
        if(n<=2)
            return 1;
        unsigned long long temp =topDownFib(n-1,arr)+topDownFib(n-2,arr);
        arr[n]=temp;
        return temp;
    }
}
unsigned long long bottomUpFib(unsigned long long n,unsigned long long arr[])
{
    arr[0]=0;arr[1]=1;
    for(unsigned long long i=2;i<=n;i++)
        arr[i]=arr[i-1]+arr[i-2];
    return arr[n];
}
int main() {
    unsigned long long n,tdResult,buResult;
    cin>>n;
    unsigned long long tdarr[n+1]={0};
    unsigned long long buarr[n+1]={0};

    tdResult=topDownFib(n,tdarr);
    buResult=bottomUpFib(n,buarr);
    cout<<tdResult<<" "<<buResult;

}
