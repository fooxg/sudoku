#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <climits>

typedef struct cell cell;

class Sudoku {
	int board[81]{ 0 };
	inline static const std::string valid_characters = ".-0123456789";
	bool validate = false;
public:
	static bool verify_sudoku_string(std::string& str);
	static void print_sudoku(std::string& str);
	void print_sudoku();
	void string_to_board(std::string& str);
	std::string get_line();
	bool validate_sudoku();
	void process_dlx(int solutions, std::vector<cell*> s);
	void solve(std::string& str);
};

class DLX {
	Sudoku* callback;
	cell* h;
	int solutions = 0;
	bool find_all = false;
public:
	DLX(int board[81], Sudoku* callback);
	~DLX();
	void cover(cell* c);
	void uncover(cell* c);
	cell* choose_column();
	void search(std::vector<cell*> s);
	void run(bool find_all);
};

bool Sudoku::verify_sudoku_string(std::string& str) {
	for (char c : str) {
		bool wrong_char = true;
		for (char v : valid_characters) {
			if (c == v) {
				wrong_char = false;
				break;
			}
		}
		if (wrong_char) return false;
	}
	return true;
}

// TO DO: separate cell from column
struct cell {
	cell* C;
	int ID;
	int S;
	cell* U;
	cell* D;
	cell* L;
	cell* R;
	cell(int ID) {
		this->C = this;
		this->S = 0;
		this->ID = ID;
		this->U = this;
		this->D = this;
		this->L = this;
		this->R = this;
	}
	cell(cell* C, int ID) : cell(ID) {
		this->C = C;
	}
};

void Sudoku::print_sudoku(std::string& str) {
	std::cout << "+-------+-------+-------+" << std::endl;
	for (int y = 0; y < 9; y++) {
		std::cout << "| ";
		for (int x = 0; x < 9; x++) {
			std::cout << str[x + 9 * y] << " ";
			if ((x + 1) % 3 == 0) std::cout << "| ";
		}
		std::cout << std::endl;
		if ((y + 1) % 3 == 0) {
			std::cout << "+-------+-------+-------+" << std::endl;
		}
	}
}

void Sudoku::print_sudoku() {
	std::string line = get_line();
	print_sudoku(line);
}

void Sudoku::string_to_board(std::string& str) {
	for (unsigned long int i = 0; i < str.length(); i++)
		if (str[i] != '.' and str[i] != '-' and str[i] != '0')
			board[i] = str[i] - '0';
		else
			board[i] = 0;
}

std::string Sudoku::get_line() {
	std::string out;
	for (int c : board)
		out += c + '0';
	return out;
}

bool Sudoku::validate_sudoku() {
	// TO DO: add function to check starting position contradictions
	return true;
}

/// Callback function used to get results from finished DLX processing 
void Sudoku::process_dlx(int solutions, std::vector<cell*> s) {
	std::vector<int> x;
	for (cell* c : s) {
		x.push_back(c->ID);
	}
	std::sort(x.begin(), x.end());
	for (unsigned long int i = 0; i < x.size(); i++) {
		board[i] = x[i] % 9 + 1;
	}
	if (solutions > 1) std::cout << "More than one solution found" << std::endl;
}

void Sudoku::solve(std::string& str) {
	if (verify_sudoku_string(str)) {
		string_to_board(str);
		DLX dlx(board, this);
		dlx.run(validate);
	}
}

/// Inicialisation and link building
DLX::DLX(int board[81], Sudoku* callback) {
	this->callback = callback;
	cell* cols[324];
	h = new cell(400);
	for (int col = 0; col < 324; col++) {
		cell* c = new cell(col);
		c->R = h;
		c->L = h->L;
		h->L->R = c;
		h->L = c;
		cols[col] = c;
	}
	// constrains
	auto pos_constraint = [](int x, int y, int k) {return x * 9 + y; };
	auto row_constraint = [](int x, int y, int k) {return 81 + x * 9 + k; };
	auto col_constraint = [](int x, int y, int k) {return 162 + y * 9 + k; };
	auto box_constraint = [](int x, int y, int k) {return 243 + (3 * (x / 3) + y / 3) * 9 + k; };
	auto row_num = [](int x, int y, int k) {return x * 81 + y * 9 + k; };

	auto link_rows = [](cell* a, cell* b, cell* c, cell* d) {
		a->R = b;
		b->R = c;
		c->R = d;
		d->R = a;
		a->L = d;
		d->L = c;
		c->L = b;
		b->L = a;
	};

	auto link_row_to_column = [](cell* d) {
		cell* c = d->C;
		c->S += 1;
		d->D = c;
		d->U = c->U;
		c->U->D = d;
		c->U = d;
	};

	auto create_links = [&](int x, int y, int k) {
		cell* pos = new cell(cols[pos_constraint(x, y, k)], row_num(x, y, k));
		cell* row = new cell(cols[row_constraint(x, y, k)], row_num(x, y, k));
		cell* col = new cell(cols[col_constraint(x, y, k)], row_num(x, y, k));
		cell* box = new cell(cols[box_constraint(x, y, k)], row_num(x, y, k));
		link_rows(pos, row, col, box);

		link_row_to_column(pos);
		link_row_to_column(row);
		link_row_to_column(col);
		link_row_to_column(box);
	};

	for (int x = 0; x < 9; x++) {
		for (int y = 0; y < 9; y++) {
			if (board[x * 9 + y] == 0)
				for (int k = 0; k < 9; k++) create_links(x, y, k);
			else
				create_links(x, y, board[x * 9 + y] - 1);
		}
	}
}

DLX::~DLX() {
	cell* pt = h->R;
	while (pt != h) {
		cell* col = pt;
		pt = pt->D;
		while (pt != col) {
			pt = pt->D;
			delete pt->U;
		}
		pt = pt->R;
		delete pt->L;
	}
	delete h;
}

void DLX::cover(cell* c) {
	c->R->L = c->L;
	c->L->R = c->R;
	cell* i = c->D;
	while (i != c) {
		cell* j = i->R;
		while (j != i) {
			j->D->U = j->U;
			j->U->D = j->D;
			j->C->S--;
			j = j->R;
		}
		i = i->D;
	}
}

void DLX::uncover(cell* c) {
	cell* i = c->U;
	while (i != c) {
		cell* j = i->L;
		while (j != i) {
			j->C->S++;
			j->D->U = j;
			j->U->D = j;
			j = j->L;
		}
		i = i->U;
	}
	c->R->L = c;
	c->L->R = c;
}

/// Get column with fewest 1
cell* DLX::choose_column() {
	cell* c = h;
	int s = INT_MAX;
	cell* j = h->R;
	while (j != h) {
		if (j->S < s) {
			c = j;
			s = j->S;
		}
		j = j->R;
	}
	return c;
}

void DLX::search(std::vector<cell*> s) {
	// If found solution exit recursion
	if (!find_all and solutions > 0) {
		return;  
	}
	// Process found solution
	if (h == h->R) {
		solutions++; 
		callback->process_dlx(solutions, s);
		return;
	}
	// Recursion loop
	else {
		cell* c = choose_column();
		cover(c);

		cell* r = c->D;
		while (r != c) {
			s.push_back(r);
			cell* j = r->R;
			while (j != r) {
				cover(j->C);
				j = j->R;
			}

			search(s);

			r = s.back();
			s.pop_back();
			c = r->C;
			j = r->L;
			while (j != r) {
				uncover(j->C);
				j = j->L;
			}
			r = r->D;
		}
		uncover(c);
		return;
	}
}

void DLX::run(bool find_all) {
	if (h != h->R) {
		this->solutions = 0;
		this->find_all = find_all;
		search(std::vector<cell*> {});
	}
}