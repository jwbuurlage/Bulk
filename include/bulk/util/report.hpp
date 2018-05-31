#pragma once

#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace bulk::util {

class table {
  public:
    table(std::string title, std::string entry_title = "") : title_(title) {
        column(entry_title);
    }

    template <typename... Ts>
    void columns(std::string col_name, Ts... rest) {
        column(col_name);
        columns(rest...);
    }

    void columns() {}

    void column(std::string col_name) {
        columns_.push_back(col_name);
        column_width_.push_back(col_name.size());
    }

    template <typename... Ts>
    void row(std::string row_name, Ts... results) {
        std::vector<std::string> row_entries;
        push_all(row_entries, row_name, results...);
        row_entries.resize(columns_.size());
        entries_.push_back(row_entries);
    }

    void push_all(std::vector<std::string>) {}

    template <typename T, typename... Ts>
    void push_all(std::vector<std::string>& ys, T x, Ts... xs) {
        std::stringstream ss;
        ss << x;
        auto text = ss.str();
        ys.push_back(text);

        auto index = ys.size() - 1;
        if (text.length() > column_width_[index]) {
            column_width_[index] = text.length();
        }

        push_all(ys, xs...);
    }

    std::string print() {
        std::stringstream result;
        result << title_ << "\n";

        std::stringstream divider;
        divider << "|";
        for (auto w : column_width_) {
            divider << std::string(w + 2, '-') << "|";
        }
        divider << "\n";

        auto output = [&](auto& list) {
            result << "|";
            for (auto i = 0u; i < list.size(); ++i) {
                result << " " << std::left << std::setw(column_width_[i])
                       << list[i] << " |";
            }
            result << "\n";
        };

        output(columns_);
        result << divider.str();
        for (auto& entry : entries_) {
            output(entry);
        }

        return result.str();
    }

  private:
    std::string title_;
    std::vector<std::vector<std::string>> entries_;
    std::vector<std::string> columns_;
    std::vector<unsigned int> column_width_;
};

} // namespace bulk::util
