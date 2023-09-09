#include "recipes.h"

#include <iomanip>

void Recipe::printInfo() {

	cout << "Recipe: " << id << "\n" << name << "\nVegan: " << is_vegan << "\nVegetarian: " << is_vegetarian << "\nNutrients:" << endl;

	for (int i = 0; i < nutrients.size(); i++) {

		cout << fixed << setprecision(1) << nutrients[i] << ", ";
	}

	cout << endl;
}