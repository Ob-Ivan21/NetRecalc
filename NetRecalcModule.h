#pragma once

#include <iostream>
#include <fstream>;
#include <iomanip>;  
#include <string>;
#include <vector>;
#include <boost/config.hpp>
#include <vector>
#include <iostream>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/cuthill_mckee_ordering.hpp>
#include <boost/graph/properties.hpp>
#include <boost/graph/bandwidth.hpp>


class RecalcModule {
public:
	void recalculate(std::string name);
};