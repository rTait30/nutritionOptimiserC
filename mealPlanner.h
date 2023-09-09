#pragma once

#include "main.h"

#include "recipes.h"

class MealPlanner {

public:

    MealPlanner();

    ~MealPlanner();

    void fetchRecipes(const char* databasePath, int numRecipes);

    void printRecipes();

    void populateGLPKData();

    std::vector<int> solveOptimization(const std::vector<double>& nutrientRequirements);

    std::vector<int> simpleSolveMealPlan(const std::vector<int> nutrient_positions, const std::vector<double>& lower_limits, const std::vector<double>& upper_limits);

    std::vector<Recipe> recipes;

private:

    int numRecipes;

    int numNutrients;

    vector<string> nutrientNames = {"Protein_g", "Fat_g", "Carbohydrates_g", "Calories_g", };

    sqlite3* db;

    //const char* databasePath;

    glp_prob* lp;

     // Member variable to hold fetched recipe data

    // ... (Other private helper functions) ...


};