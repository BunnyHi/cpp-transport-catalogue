#pragma once

#include <algorithm>
#include <cassert>
#include <cmath>
#include <deque>
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

    // Структура информации о маршруте
    struct InfoRoute {
        std::string name;
        int count_unique_stops;
        double length;
    };

    struct StopsHasher {

        size_t operator()(const std::pair<const Stop*, const Stop*> stops) const;

    private:
        std::hash<std::string_view> hasher;
    };

    class TransportCatalogue {
    public:
        // Добавляет новую остановку в каталог
        void AddStop(std::string_view stop_name, Coordinates coords);

        // Добавляет новый маршрут в каталог
        void AddBusRoute(std::string_view route_name, const std::vector<std::string_view>& stop_names, bool is_circular);

        // Получает информацию о маршруте
        const BusRoute* GetBusRoute(std::string_view route_name) const;

        // Получает информацию об остановке
        const Stop* GetStop(std::string_view stop_name) const;

        // Метод для получения автобусов по остановке
        std::vector<std::string_view> GetBusesByStop(std::string_view stop_name) const;

        // Метод получения информации о маршруте
        const InfoRoute GetInfoRoute(const BusRoute*) const;

    private:
        std::deque<Stop> stops_;
        std::deque<BusRoute> buses_;

        std::unordered_map<std::string_view, Stop*> stop_names_;
        std::unordered_map<std::string_view, BusRoute*> bus_routes_;
        std::unordered_map<std::string_view, std::unordered_set<std::string_view>> stop_to_buses_;

        // Метод для расчёта длины маршрута
        double GetRouteLength(const BusRoute& bus_route) const;
    };
} //namespace transport