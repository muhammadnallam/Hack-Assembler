#include <iostream>
#include <unordered_map>
#include <vector>
#include <fstream>
#include <algorithm>
#include <bitset>

using std::cout;
using std::string;

struct Instruction {
    char type = 0;
    int value;
    string dest = "null";
    string comp = "null";
    string jump = "null";
};

class SymbolTable {
    std::unordered_map<string, int> symbols;
    int next_variable = 16;

    public:
        SymbolTable() {
            // Predefined symbols
            symbols["SP"] = 0;
            symbols["LCL"] = 1;
            symbols["ARG"] = 2;
            symbols["THIS"] = 3;
            symbols["THAT"] = 4;
            for (int i=0; i<16; i++) {
                // (R0-R15)
                symbols["R" + std::to_string(i)] = i;
            }
            symbols["SCREEN"] = 16384;
            symbols["KBD"] = 24576;
        }

        bool contains(string symbol) {
            return symbols.count(symbol);
        }

        void add(string symbol, int value) {
            symbols[symbol] = value;
        }

        int get_value(const string& symbol) {
            return symbols[symbol];
        }

        int allocate(string symbol) {
            symbols[symbol] = next_variable;
            return next_variable++;
        }
};

class Parser {
    SymbolTable& symbolTable;

    public:
        int counter = 0; // Order of instruction

        Parser(string filename, SymbolTable& table) 
            :symbolTable(table)
        {
            clean(filename);

            // Parse instructions
            Instruction current;
            for (string line : lines) {
                if (line[0] == '@') {
                    current = parse_a(line);
                } else {
                    current = parse_c(line);
                }
                instructions.push_back(current);
            }
        }

        std::vector<Instruction> get_instructions() {
            return instructions;
        }

    private:
        std::vector<string> lines;
        std::vector<Instruction> instructions; 

        void clean(string filename) {
            std::fstream file(filename, std::ios::in);
            string line;

            while (std::getline(file, line)) {
                // Remove empty lines
                if (line.length() == 0) {
                    continue;
                }
                // Remove whitespaces
                line.erase(
                    std::remove(line.begin(), line.end(), ' '),
                    line.end()
                );
                // Remove comments on a saperate line
                if (line[0] == '/') {
                    continue;
                }
                // Remove inline comments
                for (int i=0; i<line.length(); i++) {
                    if (line[i] == '/') {
                        line.erase(i); // Erase line starting from '/' symbol
                        break;
                    }
                }
                // If (LABEL), add Label with current counter
                if (line[0] == '(') {
                    string symbol = line.substr(1, line.length()-2);
                    symbolTable.add(symbol, counter);
                    continue;
                }

                lines.push_back(line);
                counter++;
            }
        }
        
        Instruction parse_a(string line) {
            Instruction instruction;
            instruction.type = 'A';
            string A = line.substr(1);

            try {
                instruction.value = std::stoi(A); // number
            } catch (const std::invalid_argument& e) {
                if (symbolTable.contains(A)) {
                    instruction.value = symbolTable.get_value(A); // symbol
                } else {
                    instruction.value = symbolTable.allocate(A); // variable
                }
            }
            return instruction;
        }

        Instruction parse_c(string line) {
            Instruction instruction;
            instruction.type = 'C';

            // Parse dest
            if (line.find('=') != string::npos) {
                instruction.dest = line.substr(0, line.find('='));
            } 
            // Parse jump
            if (line.find(';') != string::npos) {
                instruction.jump = line.substr(line.find(';')+1);
            }

            // Parse comp
            if (instruction.dest != "null" && instruction.jump != "null") {
                instruction.comp = line.substr(line.find('=')+1, line.find(';'));
            } 
            else if (instruction.dest != "null") {
                instruction.comp = line.substr(line.find('=')+1);
            } 
            else {
                instruction.comp = line.substr(0, line.find(';'));
            }

            return instruction;
        }
};

class Code {
    public:
        std::unordered_map<std::string, std::string> dest_codes = {
            {"null", "000"},
            {"M",    "001"},
            {"D",    "010"},
            {"MD",   "011"},
            {"A",    "100"},
            {"AM",   "101"},
            {"AD",   "110"},
            {"AMD",  "111"}
        };

        std::unordered_map<string, string> comp_codes = {
            {"0", "101010"},
            {"1", "111111"},
            {"-1", "111010"},
            {"D", "001100"},
            {"A", "110000"},
            {"!D", "001101"},
            {"!A", "110001"},
            {"-D", "001111"},
            {"-A", "110011"},
            {"D+1", "011111"},
            {"A+1", "110111"},
            {"D-1", "001110"},
            {"A-1", "110010"},
            {"D+A", "000010"},
            {"D-A", "010011"},
            {"A-D", "000111"},
            {"D&A", "000000"},
            {"D|A", "010101"}
        };

        std::unordered_map<std::string, std::string> jump_codes = {
            {"null", "000"},
            {"JGT",  "001"},
            {"JEQ",  "010"},
            {"JGE",  "011"},
            {"JLT",  "100"},
            {"JNE",  "101"},
            {"JLE",  "110"},
            {"JMP",  "111"}
        };

        string machine_code(Instruction instruction) {
            if (instruction.type == 'A') {
                return a_code(instruction);
            } else {
                return c_code(instruction);
            }
        }

    private:
        string a_code(Instruction instruction) {
            return std::bitset<16>(instruction.value).to_string();
        }
        
        string c_code(Instruction instruction) {
            string binary = "111";

            // Convert comp to binary
            string comp = instruction.comp;
            char a = '0';
            if (comp_codes[comp] == "") { // if M
                comp.replace(comp.find('M'), 1, 1, 'A');
                a = '1';
            }
            binary += a;
            binary += comp_codes[comp];

            // Convert dest to binary
            binary += dest_codes[instruction.dest];

            // Convert jump to binary
            binary += jump_codes[instruction.jump];

            return binary;
        }
};

int main(int argc, char* argv[]) {
    SymbolTable table;
    string filename = argv[1];
    Parser parser(filename, table);
    std::vector<Instruction> instructions = parser.get_instructions();

    Code code;
    
    filename.erase(filename.find(".asm"));
    filename += ".hack";

    std::ofstream hack(filename);
    
    for (Instruction ins : instructions) {
        string machine_code = code.machine_code(ins);
        hack << machine_code << std::endl;
    }
}