#include "main.h"

using json = nlohmann::json;

using namespace std;

struct solverInput {

	vector<int> nutrient_positions;
	vector<double> weights;
	vector<double> lower_limits;
	vector<double> upper_limits;
};


solverInput argumentise(const map<string, vector<double>> nutrient_data) {

	solverInput input;

	vector<string>all_nutrients = { "Protein_g","Fat_g","Carbohydrates_g","Calories","Sucrose_g","Glucose_g","Fructose_g","Lactose_g","Maltose_g","Sugar_g","Galactose_g","Fiber_g","Calcium_mg","Iron_mg","Magnesium_mg","Phosphorus_mg","Potassium_mg","Sodium_mg","Zinc_mg","Copper_mg","Manganese_mg","Selenium_mcg","Vitamin_A_IU","Retinol_mcg","Retinol_Equivalents_mcg","Beta_Carotene_mcg","Alpha_Carotene_mcg","Vitamin_E_mcg","Vitamin_D_mcg","Vitamin_D2_Ergocalciferol_mcg","Vitamin_D3_Cholecalciferol_mcg","Beta_Cryptoxanthin_mcg","Lycopene_mcg","Lutein_and_Zeanxanthin_mcg","Vitamin_C_mg","Thiamin_B1_mg","Riboflavin_B2_mg","Niacin_B3_mg","Vitamin_B5_mg","Vitamin_B6_mg","Folate_B9_mcg","Vitamin_B12_mcg","Choline_mg","Cholestorol_mg","Saturated_Fat_g" };

	for (int i = 0; i < all_nutrients.size(); i++) {

		for (const auto&nutrient : nutrient_data) {

			if (all_nutrients[i] == nutrient.first) {

				input.nutrient_positions.push_back(i);
				input.weights.push_back(nutrient.second[0]);
				input.lower_limits.push_back(nutrient.second[1]);
				input.upper_limits.push_back(nutrient.second[2]);
			}
		}
	}

	return input;
};

string jsonStr = R"(
        
		{
			"meals_day": 4,
			"nutrients": {

				"Protein_g": [1, 239, 400],
				"Fat_g": [1, 106, 200],
				"Carbohydrates_g": [1, 318, 600],
				"Calories": [1, 3000, 6000],

				"Sugar_g": [1, 0, 30],

				"Fiber_g": [1, 10, 40]

			},

			"excluded_recipes": [],
			"excluded_ingredients": ["kale"]
		}
    )";

/*
	,

			"Calcium_mg": [1, 1000, 2000],
			"Iron_mg": [1, 8, 12],
			"Magnesium_mg": [1, 400, 800],
			"Phosphorus_mg": [1, 700, 1400],
			"Potassium_mg": [1, 2500, 6000],
			"Sodium_mg": [1, 1000, 3000],
			"Zinc_mg": [1, 10, 20],
			"Copper_mg": [1, 0.9, 2],
			"Manganese_mg": [1, 2, 4],
			"Selenium_mcg": [1, 55, 100],

			"Cholestorol_mg": [1, 0, 400],
			"Saturated_Fat_g": [1, 0, 20]

*/

extern "C" std::vector <int> calculate(string jsonStr) {

	json jsonData = json::parse(jsonStr);

	// Extract the "nutrients" object
	json nutrients = jsonData["nutrients"];

	// Create a std::map to store the nutrient data
	std::map<std::string, std::vector<double>> nutrient_data;

	// Iterate through the nutrients object and extract the arrays
	for (auto it = nutrients.begin(); it != nutrients.end(); ++it) {

		std::string nutrientName = it.key(); // Get the nutrient name
		std::vector<double> nutrientValues = it.value(); // Get the nutrient values

		nutrient_data[nutrientName] = nutrientValues; // Store in the map
	}

	solverInput input = argumentise(nutrient_data);

	for (int i = 0; i < input.nutrient_positions.size(); i++) {

		cout << input.nutrient_positions[i] << ", " << input.weights[i] << ", " << input.lower_limits[i] << ", " << input.upper_limits[i] << endl;
	}

	cout << "hello" << endl;

	MealPlanner planner;

	planner.fetchRecipes("C:/Users/ryant/OneDrive/Documents/Prog/NutritionOptimiser2/microdiet/server/recieve/production.db", 2500);

	//std::vector<double> lower_limits = { 60, 70, 300, 2200, 1, 1, 1, 1, 1, 1, 1, 30 ,1000, 8, 400, 700, 3400, 3000, 11, 1, 2.3, 55, 3000, 300, 500, 3000, 300, 5, 5, 5, 5, 400, 4000, 2000, 90, 1.2, 1.3, 16, 5, 1.3, 400, 2.4, 550, 200, 10};

	/*

	std::vector<double> all_nutrients = 
	
	{ 239, 106, 318, 3182, //(4) macros
		
	1, 1, 1, 1, 1, 1, 1, //(7) sugars
		
	30 , //(1) fiber
		
	1000, 8, 400, 700, 3400, 3000, 11, 1, 2.3, 55, // (10) minerals
		
	3000, 300, 500, 3000, 300, // (5) A, Retinol, Ret equ, Beta caro, Alpha caro
		
	5, 5, 5, 5, 400, 4000, 2000, // (7) E, D, D2, D3, Beta Crypto, Lyco, Lutein
		
	90, 1.2, 1.3, 16, 5, 1.3, 400, 2.4,  // (8) C, B1, B2, B3, B5, B6, B9, B12
	
	550, 200, 10}; //(3) choline, chelestorl, sat fat

	cout << "here" << endl;

	std::vector<int> nutrient_positions;

	std::vector<double> lower_limits;

	std::vector<double> upper_limits;

	//nutrient_positions = { 0,1,2,3 };

	//lower_limits = { 239, 106, 318, 3182 };

	//upper_limits = { 300, 150, 350, 3500 };

	nutrient_positions.resize(all_nutrients.size());

	lower_limits = all_nutrients;

	upper_limits.resize(all_nutrients.size());

	for (int i = 0; i < lower_limits.size(); i++) {

		nutrient_positions[i] = i;

		upper_limits[i] = lower_limits[i] * 15;

		cout << lower_limits[i] << ", " << upper_limits[i] << endl;
	}

	*/

	std::vector <int> meals;
	
	//meals = planner.simpleSolveMealPlan(nutrient_positions, lower_limits, upper_limits);

	meals = planner.simpleSolveMealPlan(input.nutrient_positions, input.lower_limits, input.upper_limits);

	for (int meal : meals) {

		cout << meal << endl;
	}

	return meals;

}
