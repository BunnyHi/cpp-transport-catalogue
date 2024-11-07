#pragma once

#include "domain.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <deque>
#include <iostream>
#include <numeric>
#include <iterator>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <optional>
#include <map>

namespace transport {

    class TransportCatalogue {
    public:

        void AddStop(std::string_view stop_name, geo::Coordinates coords);
        void AddBus(std::string_view route_name, const std::vector<std::string_view>& stop_names, bool is_circular);
        bool StopExists(std::string_view name) const;
        bool BusExists(std::string_view name) const;
        const Stop* GetStopByName(std::string_view name) const;
        const BusRoute* GetBusByName(std::string_view name) const;
        std::vector<std::string> GetBusesByStop(std::string_view stop_name) const;
        void SetRoadDistance(const Stop* stopA, const Stop* stopB, double distance);
        std::map<std::string_view, const BusRoute*> GetSortedBuses() const;
        std::optional <InfoStop> GetStopInfo(std::string_view stop_name) const;
        std::optional <InfoRoute> GetBusInfo(std::string_view bus_name) const;
        double GetDistance(const Stop* from, const Stop* to) const;

    private:
        std::deque<Stop> stops_;
        std::deque<BusRoute> buses_;
        
        using StopMap = std::unordered_map<std::string_view, Stop*>;
        using BusRouteMap = std::unordered_map<std::string_view, BusRoute*>;
        using BusRouteMapOneTrip = std::unordered_map<std::string_view, BusRoute*>;
        using StopsMap = std::unordered_map<std::string_view, std::unordered_set<std::string_view>>;
        using DistanceMap = std::unordered_map<std::pair<const Stop*, const Stop*>, double, StopsHasher>;

        StopMap stop_names_;
        BusRouteMap bus_routes_;
        StopsMap stop_to_buses_;
        DistanceMap distances_;
        
    };

}// namespace transport