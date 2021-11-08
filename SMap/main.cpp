#include "Map.h"
#include "Map.cpp"
#include <vector>

int main() {
	Map<int> a;
	a.Insert(1);
	a.Insert(3);
	a.Insert(2);
	a.Insert(4);
	auto c = a.Search(2);
	auto b = a.begin();
	int i = 0;
	for (auto b : a) {
		i++;
	}
	a.Delete(3);
	return 0;
}