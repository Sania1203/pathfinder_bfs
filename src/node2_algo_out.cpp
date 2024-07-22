#include <memory>
#include <string>
#include <vector>
#include <queue>
#include <algorithm>
#include <iostream>
#include <sstream>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

struct Point {
    int x, y;
};

// Направления движения (вверх, вниз, влево, вправо)
const std::vector<Point> directions = {
    {0, 1},
    {1, 0},   
    {0, -1},
    {-1, 0}
};

class FieldMapSubscriber : public rclcpp::Node {
public:

    FieldMapSubscriber() : Node("field_map_subscriber") {
        subscription_ = this->create_subscription<std_msgs::msg::String>(
            "field_map", 10, std::bind(&FieldMapSubscriber::topic_callback, this, std::placeholders::_1));
    }

private:

    void topic_callback(const std_msgs::msg::String::SharedPtr message) const {

        // Парсинг входной карты
        std::vector<std::string> field = parse_field(message->data);
        
        //Ищем путь
        Point start = {-1, -1}, finish = {-1, -1};
        if (find_start_finish(field, start, finish)) {
            std::vector<Point> path = bfs(field, start, finish);

            if (!path.empty()) {
                // Считаем количество пробелов в пути
                size_t space_count = 1;
                for (const auto& point : path) {
                    if (field[point.y][point.x] == ' ') {
                        ++space_count;
                    }
                }
                // Выводим путь и его длину
                print_path(field, path);
                RCLCPP_INFO(this->get_logger(), "M: %zu", path.size() - space_count);
            } else {
                RCLCPP_INFO(this->get_logger(), "Нет пути из S в F!");
            }
        } else {
            RCLCPP_INFO(this->get_logger(), "Не найдена точка S/F!");
        }
    }

    std::vector<std::string> parse_field(const std::string &field_str) const {
        std::vector<std::string> field;
        std::stringstream ss(field_str);
        std::string line;

        while (std::getline(ss, line)) {
            field.push_back(line);
        }

        return field;
    }

    //
    bool find_start_finish(const std::vector<std::string> &field, Point &start, Point &finish) const {
        for (std::size_t y = 0; y < field.size(); ++y) {
            for (std::size_t x = 0; x < field[y].size(); ++x) {
                if (field[y][x] == 'S') {
                    start = {static_cast<int>(x), static_cast<int>(y)};
                } else if (field[y][x] == 'F') {
                    finish = {static_cast<int>(x), static_cast<int>(y)};
                }
            }
        }

        return (start.x != -1 && start.y != -1) && (finish.x != -1 && finish.y != -1);
    }

    std::vector<Point> bfs(const std::vector<std::string> &field, const Point &start, const Point &finish) const {
        int rows = field.size();                    //число рядов
        int cols = field[0].size();                 //число столбцов (колонок)
        std::vector<std::vector<bool>> visited(rows, std::vector<bool>(cols, false));
        std::vector<std::vector<Point>> parent(rows, std::vector<Point>(cols, {-1, -1}));

        std::queue<Point> queue;
        queue.push(start);
        visited[start.y][start.x] = true;

        while (!queue.empty()) {
            Point current = queue.front();
            queue.pop();

            if (current.x == finish.x && current.y == finish.y) {
                return reconstruct_path(parent, finish);
            }

            for (const auto &dir : directions) {
                Point neighbor = {current.x + dir.x, current.y + dir.y};

                // Проверяем возможность движения
                if (is_valid_move(field, visited, neighbor, dir)) {
                    queue.push(neighbor);
                    visited[neighbor.y][neighbor.x] = true;
                    parent[neighbor.y][neighbor.x] = current;
                }
            }
        }

        return {};  // Если путь не найден
    }

    // Метод проверки допустимого движения: пробелы разрешены при движении вбок, но не вверх
    bool is_valid_move(const std::vector<std::string> &field, const std::vector<std::vector<bool>> &visited,
                       const Point &p, const Point &dir) const {
        int rows = field.size();
        int cols = field[0].size();

        // Разрешаем движение вбок (влево/вправо) и учитываем пробелы
        if (dir.x != 0) {
            return (p.x >= 0) && (p.x < cols) && (p.y >= 0) && (p.y < rows) &&
                   (field[p.y][p.x] != 'X') && !visited[p.y][p.x];
        }

        // Запрещаем движение вверх и вниз на пробелы
        if (dir.y != 0) {
            return (p.x >= 0) && (p.x < cols) && (p.y >= 0) && (p.y < rows) &&
                   (field[p.y][p.x] != 'X') && (field[p.y][p.x] != ' ') && !visited[p.y][p.x];
        }

        return false;
    }

    //Восстанавливаем путь
    std::vector<Point> reconstruct_path(const std::vector<std::vector<Point>> &parent, const Point &finish) const {
        std::vector<Point> path;
        for (Point at = finish; at.x != -1 && at.y != -1; at = parent[at.y][at.x]) {
            path.push_back(at);
        }
        std::reverse(path.begin(), path.end());
        return path;
    }

    //Рисуем путь
    void print_path(const std::vector<std::string> &field, const std::vector<Point> &path) const {
        RCLCPP_INFO(this->get_logger(), "Путь из S в F:");
        std::vector<std::string> field_with_path = field;

        for (const auto &point : path) {
            if (field_with_path[point.y][point.x] != 'S' && field_with_path[point.y][point.x] != 'F' && field_with_path[point.y][point.x] != ' ') {
                field_with_path[point.y][point.x] = '*';  // Обозначим путь '*'
            }
        }

        for (const auto &line : field_with_path) {
            RCLCPP_INFO(this->get_logger(), "%s", line.c_str()); // Выводим строку, включая пробелы
        }
    }

    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr subscription_;
};

int main(int argc, char *argv[]) {
    rclcpp::init(argc, argv);                                   //Инициализируем ROS2
    rclcpp::spin(std::make_shared<FieldMapSubscriber>());       //Заускаем ноду
    rclcpp::shutdown();
    return 0;
}