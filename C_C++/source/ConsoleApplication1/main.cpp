#include <iostream>
#include <vector>
#include <algorithm>
#include <list>
#include <numeric>
#include <string>
using namespace std;

//std::vector<int> vecInt = { 0,1,2,3,4,1,2,5,6,7,45,4,12,0,44,41,32 };
//std::vector<std::string> vecStr = { "0","1" };

void Swap(int& a, int& b) {
	a = a ^ b;
	b = a ^ b;
	a = a ^ b;
}

void Swap1(int& a, int& b) {
	int temp = a;
	a = b;
	b = temp;
}

//≤Â»Î≈≈–Ú
void SelectSort(int* arr, int length)
{
	if (arr == nullptr || length <= 0)
		return;
	for (int i = 0; i < length; i++) {
		int index = i;
		for (int j = i + 1; j < length; j++) {
			if (arr[index] > arr[j]) {
				index = j;
			}
		}
		if (i != index)
			Swap1(arr[i], arr[index]);
	}
}

//√∞≈›≈≈–Ú
void BuddleSort(int* arr, int length) {
	for (int i = length - 1; i > 0; i--) {
		for (int j = 0; j < i; j++) {
			if (arr[j] > arr[j + 1]) {
				Swap(arr[j], arr[j + 1]);
			}
		}
	}
}

int main()
{
	int arr[] = { 2, 5, 1, 3, 6, 8,9,7, 2, 5, 4 };
	int length = 11;
	for (int i = 0; i < length; i++) {
		cout << arr[i] << " ";
	}
	cout << endl;
	//SelectSort(arr, length); 
	BuddleSort(arr, length);
	for (int i = 0; i < length; i++) {
		cout << arr[i] << " ";
	}
	cout << endl;
	return 0;
}