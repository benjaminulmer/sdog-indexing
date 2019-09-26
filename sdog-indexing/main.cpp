#include "Program.h"

int main(int argc, char* argv[]) {
	Program p;
	//p.testOperations(10000000, 15);
	p.benchmarkAll(10000000, 21);
	system("pause");
	return 0;
}