#include <iostream>
#include "HW1.h"
using namespace std;

int main() {
	//HW1();
	int* x = new int[10];
	for (int i = 0; i < 10; i++) {
		x[i] = i;
	}
	cout << (int)(*x);
	cout << (int)(*(x + 3));
	return 0;
}

