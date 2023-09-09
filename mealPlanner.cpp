#include "mealPlanner.h"

MealPlanner::MealPlanner() {

    db = nullptr;
        
    lp = nullptr;
    
    numNutrients = 0;

    numRecipes = 0;

}

void MealPlanner::fetchRecipes(const char* databasePath, int numRecipesIn) {

    numRecipes = numRecipesIn;

    cout << "trying to fetch recipes from " << databasePath << endl;

    int rc = sqlite3_open(databasePath, &db);

    //int rc = sqlite3_open_v2("file::memory:", &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);

    if (rc) {

        std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << std::endl;

        cout << rc << endl;
    }

    std::string sqlString = "SELECT * FROM recipes LIMIT " + std::to_string(numRecipes);

    const char* sql = sqlString.c_str();// Get the first record

    sqlite3_stmt* stmt;

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {

        std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {

        // Retrieve data from the first row and work with it in C++
        int id = sqlite3_column_int(stmt, 0);

        const char* textName = (const char*)sqlite3_column_text(stmt, 1);

        string name = string(textName);

        bool is_vegan = sqlite3_column_int(stmt, 53);

        bool is_vegetarian = sqlite3_column_int(stmt, 54);

        //double calories = sqlite3_column_double(stmt, 10);

        std::vector<double> nutrients;

        for (int i = 7; i < 52; i++) {

            double columnValue = sqlite3_column_double(stmt, i);

            nutrients.push_back(columnValue);
        }

        /*
        std::cout << "ID: " << id << ", Name: " << name << ", Vegan: " << is_vegan << ", Vegetarian: " << is_vegetarian << ", Calories: " << calories << std::endl;

        for (double nutrient : nutrients) {

            std::cout << nutrient << " ";
        }

        std::cout << endl;
        */

        recipes.push_back(Recipe(id, is_vegan, is_vegetarian, nutrients, name));
    }

    cout << endl;

    // Finalize the statement and close the database connection
    sqlite3_finalize(stmt);

    sqlite3_close(db);
}

void MealPlanner::printRecipes() {

    for (Recipe& recipe : recipes) {

        recipe.printInfo();
    }
}

//const std::vector<Recipe>& recipes

void MealPlanner::populateGLPKData() {

    cout << "populating for " << numNutrients << " nutrients" << endl;

    lp = glp_create_prob();

    cout << "numRecipes " << numRecipes << endl;

    // Add rows (constraints) for each recipe
    for (int i = 1; i <= numRecipes; i++) {
        glp_add_rows(lp, 1);
        glp_set_row_name(lp, i, ("recipe_" + std::to_string(i)).c_str()); // Assign a name like "recipe_1", "recipe_2", ...
        glp_set_row_bnds(lp, i, GLP_LO, 0.0, 0.0); // Lower bound (0), no upper bound
    }

    // Add a row for requirements
    glp_add_rows(lp, 1);
    glp_set_row_name(lp, numRecipes + 1, "requirements"); // Name the requirements row
    glp_set_row_bnds(lp, numRecipes + 1, GLP_LO, 0.0, 0.0); // Lower bound (0), no upper bound


    // Define columns (one for each nutrient)
    std::vector<int> nutrientVariables;

    for (int j = 0; j < numNutrients; j++) {
        nutrientVariables.push_back(glp_add_cols(lp, 1));
        glp_set_col_name(lp, nutrientVariables[j], ("x" + std::to_string(j + 1)).c_str());
        glp_set_col_kind(lp, nutrientVariables[j], GLP_FX); // Binary variable (0 or 1)
        glp_set_obj_coef(lp, nutrientVariables[j], 0.0); // Objective coefficient for recipe variables (minimize)
    }

    // Define variables (one for each recipe, to indicate whether it's included or not)
    std::vector<int> recipeInclusionVariables;
    for (int i = 0; i < numRecipes; i++) {
        recipeInclusionVariables.push_back(glp_add_cols(lp, 1));
        glp_set_col_name(lp, recipeInclusionVariables[i], ("recipe_inclusion_" + std::to_string(i + 1)).c_str());
        glp_set_col_kind(lp, recipeInclusionVariables[i], GLP_BV); // Binary variable (0 or 1, indicating inclusion)
        glp_set_obj_coef(lp, recipeInclusionVariables[i], 0.0); // Objective coefficient for inclusion variables (minimize)
    }

    cout << "rows: " << glp_get_num_rows(lp) << endl;

    cout << "columns: " << glp_get_num_cols(lp) << endl;


    // Create and set the constraint matrix

    int numConstraintEntries = ((numRecipes * numNutrients) + numNutrients);

    cout << numConstraintEntries << endl;

    int* ia = new int[numConstraintEntries + 1];
    int* ja = new int[numConstraintEntries + 1];
    double* ar = new double[numConstraintEntries + 1];
    int constraintCount = 1;

    for (int i = 0; i < numRecipes; i++) {
        for (int j = 0; j < numNutrients; j++) {
            
            ia[constraintCount] = i + 1; // Constraints are numbered from 1
            ja[constraintCount] = j + 1;
            ar[constraintCount] = recipes[i].nutrients[j];

            constraintCount++;
        }
    }

    // Add constraints to ensure nutrient requirements are met (values will be set later)

    for (int j = 0; j < numNutrients; j++) {

        cout << j << endl;

        ia[constraintCount] = numRecipes + 1; // Constraints are numbered from 1
        ja[constraintCount] = j + 1; // Use a non-existent variable index (1 + j) for the constant
        ar[constraintCount] = 0.0; // Initial constraint value set to 0

        constraintCount++;

    }

    cout << "hello" << endl;

    cout << ia[1] << endl << ja[1] << endl << ar[1] << endl;

    cout << ia[41] << endl << ja[41] << endl << ar[41] << endl;

    glp_load_matrix(lp, constraintCount - 1, ia, ja, ar);

    // Clean up memory allocated for ia, ja, and ar
    delete[] ia;
    delete[] ja;
    delete[] ar;

    


}


std::vector<int> MealPlanner::solveOptimization(const std::vector<double>& nutrientRequirements) {

    if (!lp) {

        cout << "problem not initialised" << endl;
        
        return { 0,0,0 };
    }

    cout << "updating requirements" << endl;

    int rowIdx = -1; // Initialize with an invalid value

    for (int i = 1; i <= glp_get_num_rows(lp); i++) {
        const char* rowName = glp_get_row_name(lp, i);
        if (rowName != NULL && std::string(rowName) == "requirements") {
            rowIdx = i;
            break; // Stop searching once found
        }
    }

    if (rowIdx != -1) {
        // The "requirements" row was found at index rowIdx
        // You can access and modify this row as needed

        cout << "index: " << rowIdx << endl;

        int* indices = new int[numNutrients];

        const double* values = nutrientRequirements.data();

        for (int j = 0; j < numNutrients; j++) {

            indices[j] = j + 1;

            cout << indices[j] << " | " << values[j] << endl;
        
        }

        glp_set_mat_row(lp, rowIdx, 1, indices, values); // Update the coefficient for column 

        delete[] indices;

        delete[] values;

    }

    else {

        // The "requirements" row was not found

        cout << "didnt find" << endl;
    }

    cout << "creating objective function" << endl;

    // Set the objective to minimize the total number of recipes
    for (int i = 1; i <= numRecipes; i++) {
        glp_set_obj_coef(lp, i, 1.0);
    }

    cout << "solving" << endl;

    // Solve the LP problem
    glp_iocp params;
    glp_init_iocp(&params);
    params.presolve = GLP_ON;
    glp_intopt(lp, &params);

    // Check the status of the solver
    int status = glp_mip_status(lp);

    if (status == GLP_OPT) {
        // Solution found, print the selected recipes
        std::vector<int> selectedRecipes;
        for (int i = 1; i <= numRecipes; i++) {
            double recipeAmount = glp_mip_col_val(lp, i);
            if (recipeAmount > 0.5) {
                selectedRecipes.push_back(i);
            }
        }

        // Print the selected recipes
        std::cout << "Selected Recipes:" << std::endl;
        for (int recipeIdx : selectedRecipes) {
            std::cout << "Recipe " << recipeIdx << std::endl;
        }
    }
    else {
        // No solution found or an error occurred
        std::cout << "No feasible solution found." << std::endl;
    }

    // Clean up the LP problem
    glp_delete_prob(lp);

    return { 0 };
}



// Solver function
std::vector<int> MealPlanner::simpleSolveMealPlan(const std::vector<int> nutrient_positions, const std::vector<double>& lower_limits, const std::vector<double>& upper_limits) {

    // Create a GLPK linear programming problem
    //glp_prob* lp;

    numNutrients = nutrient_positions.size();

    cout << endl << "recipes: " << numRecipes << ", nutrients: " << numNutrients << endl << "requirements: ";

    for (double nutrient : lower_limits) {

        cout << nutrient << ", ";
    }

    cout << endl;

    lp = glp_create_prob();

    glp_set_prob_name(lp, "SimpleMealPlan");

    // Add decision variables (binary)
    glp_add_cols(lp, numRecipes);

    for (int i = 1; i <= numRecipes; i++) {

        // Set variable bounds to binary (0 or 1)
        glp_set_col_kind(lp, i, GLP_BV);

        //glp_set_col_bnds(lp, i, GLP_DB, 0.0, 1.0); // Bounds for binary variable (0 or 1)
    }

    // Define constraints for nutrient requirements

    // Add constraints
    glp_add_rows(lp, numNutrients);

    for (int i = 1; i <= numNutrients; i++) {

        glp_set_row_bnds(lp, i, GLP_DB, lower_limits[i - 1], upper_limits[i - 1]);

        // Define coefficients for the constraint matrix for each recipe
        std::vector<int> ind(numRecipes + 1); // Array to store row indices
        std::vector<double> val(numRecipes + 1); // Array to store coefficients

        for (int j = 1; j <= numRecipes; j++) {

            double nutrientContent = recipes[j - 1].nutrients[nutrient_positions[i - 1]]; // Nutrient content of recipe j

            // Set the coefficient for recipe j in constraint i
            ind[j] = j;
            val[j] = nutrientContent;
        }

        // Set the nonzero coefficients for the current row (constraint)
        glp_set_mat_row(lp, i, numRecipes, ind.data(), val.data());
    }

    //minimize recipes

    // Set the objective direction to minimize
    glp_set_obj_dir(lp, GLP_MIN);

    for (int i = 1; i <= numRecipes; i++) {
        glp_set_obj_coef(lp, i, 1.0);
    }

    // Create a parameter structure and initialize it with default values
    glp_iocp params;
    glp_init_iocp(&params);

    // Set specific solver parameters as needed
    params.presolve = GLP_ON; // Enable presolve (can improve performance)
    params.tm_lim = 5000; // Set a time limit in milliseconds (adjust as needed)
    //params.mip_gap = 0.01; // Set MIP gap tolerance (adjust as needed)

    // Set the MIP method (optional)
    // params.meth = GLP_MIP_METHOD; // Use a specific MIP method


    // Solve the MIP problem
    glp_intopt(lp, &params);

    // Check the MIP solver status and retrieve the solution

    std::vector<int> returnMeals;

    if (glp_mip_status(lp) == GLP_OPT or glp_mip_status(lp) == GLP_FEAS) {

        if (glp_mip_status(lp) == GLP_OPT) {

            std::cout << "Optimal Solution Found:" << std::endl;

        }

        else {

            std::cout << "Best solution found within time limit:" << std::endl;
        }
        // Retrieve decision variable values (which recipes are selected), objective value, etc.

        // Retrieve decision variable values (which recipes are selected)

        std::vector<double> totals(numNutrients);

        for (int i = 0; i < numNutrients; i++) {

            totals[i] = 0;
        }

        for (int i = 1; i <= numRecipes; i++) {

            int selected = static_cast<int>(glp_mip_col_val(lp, i));

            if (selected == 1) {

                int recipe = i - 1;

                returnMeals.push_back(recipe);

                std::cout << endl << "Recipe " << recipe << " is selected: " << endl;

                cout << recipes[recipe].name << endl;

                for (int j = 0; j < numNutrients; j++) {

                    double nutrient = recipes[recipe].nutrients[nutrient_positions[j]];

                    cout << nutrient << ", ";

                    totals[j] += nutrient;
                }

                cout << endl;
            }
        }

        cout << endl;

        for (int j = 0; j < numNutrients; j++) {

            cout << lower_limits[j] << " | " << totals[j] << " | " << upper_limits[j] << endl;
        }

        cout << endl;

        // You can also retrieve the objective value if needed
        double objectiveValue = glp_mip_obj_val(lp);
        std::cout << "Objective Value: " << objectiveValue << std::endl;

    }
    else {

        std::cout << "No optimal solution found." << std::endl;

    }

    glp_delete_prob(lp);

    return returnMeals;
}


MealPlanner::~MealPlanner() {
    // Add any necessary cleanup code here
}
