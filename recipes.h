#pragma once

#include <iostream>
#include <stdio.h>
#include <vector>

using namespace std;

class Recipe {

private:

    

public:

    int id;
    bool is_vegan;
    bool is_vegetarian;

    string name;

    vector<double> nutrients;

    Recipe(const int& id, const bool& is_vegan, const bool& is_vegetarian, const vector<double>& nutrients, string name)

        : id(id), is_vegan(is_vegan), is_vegetarian(is_vegetarian), nutrients(nutrients), name(name) {}

    void printInfo();

};