#include <memory>
#include <string>
#include <iostream>
#include <vector>
#include <sstream>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

using namespace std::chrono_literals;

// Структура для представления прямоугольного препятствия
struct Rectangle {
    int x1, y1;  // Верхний левый угол
    int x2, y2;  // Нижний правый угол
};

class FieldCreatorNode : public rclcpp::Node
{
public:
    FieldCreatorNode()
    : Node("field_creator_node")
    {
        publisher_ = this->create_publisher<std_msgs::msg::String>("field_map", 10);
        input_parameters();
    }

private:
    void input_parameters()
    {
        int X, Y, p;
        int S_x, S_y, F_x, F_y;

        std::cout << "Введите размеры поля (X Y): ";
        std::cin >> X >> Y;

        std::cout << "Введите координаты точки S (x y): ";
        std::cin >> S_x >> S_y;

        std::cout << "Введите координаты точки F (x y): ";
        std::cin >> F_x >> F_y;

        std::cout << "Введите количество препятствий: ";
        std::cin >> p;

        std::vector<Rectangle> obstacles(p);
        for (int i = 0; i < p; ++i) {
            std::cout << "Введите координаты диагональных точек препятствия " << i + 1 << " (x1 y1 x2 y2): ";
            std::cin >> obstacles[i].x1 >> obstacles[i].y1 >> obstacles[i].x2 >> obstacles[i].y2;
        }

        create_field(X, Y, {S_x, S_y}, {F_x, F_y}, obstacles);
    }

    void create_field(int X, int Y, const std::pair<int, int>& S, const std::pair<int, int>& F,
                      const std::vector<Rectangle>& obstacles)
    {
        // Создаем прямоугольное поле
        std::vector<std::vector<char>> field(Y, std::vector<char>(X, '.'));

        // Отметим точку S
        if (S.first >= 0 && S.first < X && S.second >= 0 && S.second < Y) {
            field[S.second][S.first] = 'S';
        }

        // Отметим точку F
        if (F.first >= 0 && F.first < X && F.second >= 0 && F.second < Y) {
            field[F.second][F.first] = 'F';
        }

        // Отметим препятствия
        for (const auto& rect : obstacles) {
            int x_start = std::min(rect.x1, rect.x2);
            int x_end = std::max(rect.x1, rect.x2);
            int y_start = std::min(rect.y1, rect.y2);
            int y_end = std::max(rect.y1, rect.y2);
            for (int x = x_start; x <= x_end; ++x) {
                for (int y = y_start; y <= y_end; ++y) {
                    if (x >= 0 && x < X && y >= 0 && y < Y) {
                        field[y][x] = 'X'; // "X" обозначает препятствие
                    }
                }
            }
        }

        // Формируем строку для публикации
        std::stringstream field_map;
        for (const auto& row : field) {
            for (char cell : row) {
                field_map << cell << " ";
            }
            field_map << "\n";
        }

        // Публикуем карту
        auto message = std_msgs::msg::String();
        message.data = field_map.str();
        publisher_->publish(message);
        RCLCPP_INFO(this->get_logger(), "Field map published.");
    }

    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher_;
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<FieldCreatorNode>());
    rclcpp::shutdown();
    return 0;
}