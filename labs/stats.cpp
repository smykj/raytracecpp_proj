#include <cctype>
#include <fstream>
#include <iostream>
#include <print>
#include <sstream>
#include <vector>
using std::string;

struct result_t {
  size_t sum;
  size_t r_count;
  size_t sentences;
  std::vector<std::string> words;
};

class file_processor {

  static std::istream &getword(std::istream &in, std::string &str) {

    str.clear();
    char chr;
    std::string temp;
    if (in.get(chr) && std::isalnum(chr) != 0) {
      temp.push_back(chr);
      while (in.get(chr) && std::isalnum(chr) != 0) {
        temp.push_back(chr);
      }
      if (!in.bad()) {
        str = temp;
      }
      if (!in.bad() && in.eof()) {
        in.clear(std::ios_base::eofbit);
      }
    }
    return in;
  }

  static bool is_num(const std::string &word) {
    for (auto &&chr : word) {
      if (std::isdigit(chr) == 0) {
        return false;
      }
    }
    return true;
  }

  static int sum(const std::string &word) {
    if (!is_num(word)) {
      return 0;
    }
    int sum = 0;
    for (auto &&chr : word) {
      sum += chr - '0';
    }
    return sum;
  }

  static int count_r(const std::string &word) {
    int count = 0;
    for (auto &&chr : word) {
      if (chr == 'r') {
        ++count;
      }
    }
    return count;
  }

  static void to_lower(std::string &word) {
    for (auto &&chr : word) {
      chr = std::tolower(chr);
    }
  }

public:
  static void process_file(std::ifstream &file, result_t &output) {
    output = result_t();

    for (string line; std::getline(file, line, '.');) {
      std::istringstream linestream(line);
      bool has_word = false;
      for (string word; getword(linestream, word);) {
        if (!word.empty()) {
          has_word = true;
        }
        if (is_num(word)) {
          output.sum += sum(word);
        } else {
          output.r_count += count_r(word);
          to_lower(word);
          output.words.emplace_back(word);
        }
      }
      if (has_word) {
        ++output.sentences;
      }
    }
  }
};

// this one was done first but i decided to reimplement it in a more OOP fashion
void process_file_simple(std::ifstream &file, result_t &output) {
  output = result_t();
  char streamchar;
  bool has_word = false;
  size_t potential_sum = 0;
  bool is_number = true;
  string word_aggregate;
  while (file.get(streamchar)) {

    if (std::isalnum(streamchar) != 0) {
      has_word = true;
      word_aggregate.append(1, std::tolower(streamchar));

      if (std::isdigit(streamchar) != 0) {
        potential_sum += (streamchar - '0');
      } else {
        if (streamchar == 'r') {
          ++output.r_count;
        }
        potential_sum = 0;
        is_number = false;
      }
    } else {
      if (streamchar == '.' && has_word) {
        ++output.sentences;
        has_word = false;
      }

      if (is_number) {
        output.sum += potential_sum;
        potential_sum = 0;
      } else {
        output.words.emplace_back(word_aggregate);
      }

      word_aggregate = "";
      is_number = true;
    }
  }
}

int main(int argc, char *argv[]) {
  std::ifstream fstr;
  result_t result;

  if (argc < 2) {
    std::cout << "no input file\n";
    return 1;
  }

  for (int i = 1; i < argc; ++i) {
    fstr = std::ifstream(argv[i]);

    if (!fstr.is_open()) {
      continue;
    }
    file_processor::process_file(fstr, result);
    std::println("File {}: sum={}, r_count={}, sentences={}", argv[i],
                 result.sum, result.r_count, result.sentences);
    for (auto &&word : result.words) {
      std::println(std::cerr, "File {} contains \"{}\"", argv[i], word);
    }
    std::cout << '\n';
  }

  return 0;
}
