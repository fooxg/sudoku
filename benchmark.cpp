#include <string>
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include "sudoku-solver.hpp"

int main() {
	Sudoku su;
	std::string line;
	std::string result_t;

	// File handles
	std::ifstream sudoku_file;
	std::ofstream results_file;
	sudoku_file.open("list_of_sudokus.txt");
	results_file.open("output.txt");

	if (sudoku_file.is_open() and results_file.is_open())
	{
		while ( std::getline(sudoku_file, line))
		{
			auto start = std::chrono::system_clock::now();
			su.solve(line); // Solving sudoku from given line
			auto end = std::chrono::system_clock::now();
			result_t = su.get_line(); // Getting board as string of characters

			// Save result and print to console
			std::cout << "Solved in: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << " microseconds" << std::endl;
			results_file << result_t << '\t' << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << '\n';
		}

		// File handles
		sudoku_file.close();
		results_file.close();
	}
	return 0;
}