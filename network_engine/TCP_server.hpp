#include <SFML/Network.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <iostream>
#include <string>
#include <list>

#pragma once

#include "game_state.hpp"

namespace game {

class TCP_server final {
    
private:
    sf::TcpListener _listener;
    sf::SocketSelector _selector;

    std::list<sf::TcpSocket> _clients;
    std::vector<sf::Packet> _incoming_messages;
    std::vector<sf::Packet> _outcoming_messages;

    ushort _port;
    sf::Time _timeout;

public:
    //port number and timeout in milliseconds (0 for infinity)
    TCP_server(ushort port, sf::Time timeout): _port(port), _timeout(timeout) {}; 
    
    void init() {
        if (_listener.listen(_port) != sf::Socket::Status::Done) {
            std::cerr << "Error while starting listening to connections" << std::endl;
            return;
        }

        // bind the listener to a port
        _selector.add(_listener);
    }

    void wait_and_handle(game_state& global_state) {
        if (_selector.wait(_timeout)) { 
            //handling all connections
            if (_selector.isReady(_listener)){
                // accept a new connection
                sf::TcpSocket client;
                //_clients.push_back(sf::TcpSocket()); 
                if (_listener.accept(client) != sf::Socket::Status::Done) { //must not block thread.
                    std::cerr << "Error while accepting client's socket" << std::endl;
                    return;
                }
                else {
                    std::cerr << "Accepted client on " << (client).getRemoteAddress().value() <<
                        " and port " << (client).getRemotePort() << std::endl;
                    _selector.add(client);
                    _clients.push_back(std::move(client)); 
                    _incoming_messages.emplace_back(); //adding new message packet to same index as created client
                    _outcoming_messages.emplace_back();
                    global_state.player_objects.emplace_back();

                    global_state.player_objects.back().set_velocity({1.0, 1.0});
                }
            }
            int i = 0;
            for (auto& client: _clients) {
                if (_selector.isReady(client)){
                    if (client.receive(_incoming_messages[i]) != sf::Socket::Status::Done) {
                        std::cout << "recieving error (maybe socket not connected)" << std::endl;
                        _incoming_messages[i++].clear();
                    }
                }
            }
        }
        else {
            std::cerr << "await timeout happend. Do something with this information, or don't" << std::endl; 
        }
    }

    void read_packets(game_state& global_state) {
        int move;
        float time, speed;
        for (int i = 0; i < _clients.size(); ++i){
            if (_incoming_messages[i].getDataSize() != 0) {
                _incoming_messages[i] >> move >> time;
                speed = time / 6;
                std::cout << time << ' ' << move << '\n';
                switch (move) {
                    case 1:
                        //std::cout << "w\n";
                        global_state.player_objects[i].update(0, -speed, 0);
                        break;
                    case 2:
                        //std::cout << "s\n";
                        global_state.player_objects[i].update(0, speed, 0);
                        break;
                    case 3:
                        //std::cout << "a\n"; 
                        global_state.player_objects[i].update(-speed, 0, 0);
                        break;
                    case 4:
                        //std::cout << "d\n";
                        global_state.player_objects[i].update(speed, 0, 0);
                        break;
                    case 5:
                        global_state.player_objects[i].update(0, 0, -speed);
                        break;
                    case 6:
                        global_state.player_objects[i].update(0, 0, speed);
                        break;
                    default:
                        /* stop buttom */
                        break;
                }
                // >> vert >> horz >> rot;
                // std::cout << i << ": got message: vert: " << vert << " horz: "
                //     << horz << " rot: " << rot << std::endl;
                //global_state.player_objects[i].update({vert, horz, rot});
                _incoming_messages[i].clear();
            }
        }
    }

    void create_messages(game_state& global_state) {
        ushort client_count = _clients.size();
        std::cout << "client count " << client_count << std::endl; 

        for (int i = 0; i < client_count; ++i) {
            _outcoming_messages[i] << client_count;
            for (int j = 0; j < client_count; ++j) {
                _outcoming_messages[i] << global_state.player_objects[j].getPosition().x
                                       << global_state.player_objects[j].getPosition().y
                                       << global_state.player_objects[j].getRotation().asRadians();
            }
        }
    }

    void send_packets() {
        int i = 0;
        for (auto&& it_client = _clients.begin(); it_client != _clients.end(); ++it_client) {
            try {
                if (it_client->send(_outcoming_messages[i++]) != sf::Socket::Status::Done) {
                    std::cerr << "Error while sending to " << it_client->getRemoteAddress().value() 
                        << " " << it_client->getLocalPort() << std::endl;
                }
            }
            catch (std::bad_optional_access& e) {
                // _clients.erase(it_client);
                // break;
            }
        }
    }

    void clear_outcome() {
        for (int i = 0; i < _outcoming_messages.size(); ++i) {
            _outcoming_messages[i].clear();
        }
    }
};
}
