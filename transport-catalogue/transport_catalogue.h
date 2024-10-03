#pragma once

#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <iterator>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "geo.h"

namespace transport {

    struct Stop {
        std::string name;
        Coordinates coordinates;
    };

    struct BusRoute {
        std::string name;
        std::vector<Stop*> stops;
        bool is_circular;
    };

    class TransportCatalogue {
    public:
        // Добавляет новую остановку в каталог
        void AddStop(const std::string& stop_name, Coordinates coords);

        // Добавляет новый маршрут в каталог
        void AddBusRoute(const std::string& route_name, const std::vector<std::string>& stop_names, bool is_circular);

        // Получает информацию о маршруте
        const BusRoute* GetBusRoute(const std::string& route_name) const;

        // Получает информацию об остановке
        const Stop* GetStop(const std::string& stop_name) const;

        // Метод для расчёта длины маршрута
        double GetRouteLength(const BusRoute& bus_route) const;

        // Метод для получения автобусов по остановке
        std::vector<std::string_view> GetBusesByStop(const std::string& stop_name) const;

    private:
        std::unordered_map<std::string, Stop> stops_;
        std::unordered_map<std::string, BusRoute> bus_routes_;
        std::unordered_map<std::string, std::unordered_set<std::string>> stop_to_buses_;
    };
} //namespace transport