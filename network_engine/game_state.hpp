#include <SFML/Network.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

namespace game {

class control_struct final {
public:
    int move_x;
    int move_y;
    int rotate;
    bool changed;
    int sprite_status;
};

class object: public sf::RectangleShape{
    sf::Vector2f _velocity_coeff;
    float _rotation_coeff;

    sf::Vector2i _velocity;
    int _rotation;

public:
    int sprite_status;

    void update() {
        move({_velocity_coeff.x * _velocity.x, _velocity_coeff.y * _velocity.y});
        rotate(sf::radians(_rotation_coeff * _rotation));
    }

    void set_coeff_velocity_and_rot(sf::Vector2f velocity_coeff, float rotation_coeff){
        _velocity_coeff = velocity_coeff;
    }

    void set_velocity_and_rot(sf::Vector2i new_velocity, int new_rotation){
        _velocity = new_velocity;
        _rotation = new_rotation;
    }
};

class game_state_client final {
public:
    std::unordered_map<uint64_t, object> player_objects;
};

class game_state_server final{
    public:
        uint64_t next_player_unique_id = 0;
        std::vector<std::pair<uint64_t, object>> player_objects;

        void add_player(){
            player_objects.emplace_back();
            player_objects.back().first = next_player_unique_id++;
            player_objects.back().second.set_coeff_velocity_and_rot({1.0, 1.0}, 0.01);
        }

        void update_state(){
            for(int i = 0; i < player_objects.size(); ++i){
                player_objects[i].second.update();
            }
        }
};

class Mob {
public:
    size_t index = 0;
    typename sf::Vector2f coords = {0, 0};
    Mob(sf::Vector2f coords) : coords(coords) {};
    Mob() = default;

    // add sprite
};

}