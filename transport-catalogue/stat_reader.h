#pragma once

#include "transport_catalogue.h"

namespace get_stat {

    void ParseAndPrintStat(const transport::TransportCatalogue& tansport_catalogue, std::string_view request,
        std::ostream& output);
} //namespace get_stat