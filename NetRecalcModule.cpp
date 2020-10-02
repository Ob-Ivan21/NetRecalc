#include "NetRecalcModule.h"

using namespace std;
using namespace boost;

void RecalcModule::recalculate(string name)
{
	//Boost def part
	typedef adjacency_list < vecS, vecS, undirectedS,
		property< vertex_color_t, default_color_type,
		property< vertex_degree_t, int > > >
		Graph;
	typedef graph_traits< Graph >::vertex_descriptor Vertex;
	typedef graph_traits< Graph >::vertices_size_type size_type;
	//

	std::vector <std::vector<float>> knots;
	std::string line;
	std::string::size_type sz;

	ifstream inFile;
	ofstream outFile;
	inFile.open("hex.k");
	outFile.open("hex_sorted.k");
	while (line != "*NODE")
	{
		getline(inFile, line);
	}
	getline(inFile, line);

	{	//Reading nodes and coordinates from file
		int ind;
		double x, y, z;
		getline(inFile, line);
		while (line != "$")
		{
			try {
				ind = stoi(line, &sz);
				x = stod(line = line.substr(sz), &sz);
				y = stod(line = line.substr(sz), &sz);
				z = stod(line = line.substr(sz), &sz);
			}
			catch (...) {
				break;
			}

			if (knots.size() < ind)
				knots.resize(ind);

			knots[ind - 1].push_back(x);
			knots[ind - 1].push_back(y);
			knots[ind - 1].push_back(z);

			getline(inFile, line);
		}
	}

	//Filling matrix with [i][j] meaning the distance between i-th and j-th node
	float** distance_matrix = new float* [knots.size()];
	for (size_t k = 0; k < knots.size(); k++)
		distance_matrix[k] = new float[knots.size()];

	for (size_t j = 0; j < knots.size(); j++)
	{
		distance_matrix[j][j] = 0;
		for (size_t k = j; k < knots.size(); k++)
		{
			distance_matrix[j][k] = sqrt(pow(knots[j][0] - knots[k][0], 2)
				+ pow(knots[j][1] - knots[k][1], 2)
				+ pow(knots[j][2] - knots[k][2], 2));
			distance_matrix[k][j] = distance_matrix[j][k];
		}
	}
	//

	//Building adjactency graph from distance matrix
	Graph G(knots.size());
	for (size_t j = 0; j < knots.size(); j++)
	{
		//cout << j << ": ";
		for (size_t k = j + 1; k < knots.size(); k++)
			/////////////////////////////////////////////
			if (distance_matrix[j][k] <= 1)
			{
				add_edge(j, k, G);
				//cout << k << " ";
			}
		//cout << endl;
	}
	//

	//algorythm defs
	graph_traits< Graph >::vertex_iterator ui, ui_end;
	property_map< Graph, vertex_degree_t >::type deg = get(vertex_degree, G);
	for (boost::tie(ui, ui_end) = vertices(G); ui != ui_end; ++ui)
		deg[*ui] = degree(*ui, G);
	property_map< Graph, vertex_index_t >::type index_map
		= get(vertex_index, G);

	std::cout << "original bandwidth: " << bandwidth(G) << std::endl;

	std::vector< Vertex > inv_perm(num_vertices(G));
	std::vector< size_type > perm(num_vertices(G));
	//

	//Cuthill_mckee_ordering
	cuthill_mckee_ordering(
		G, inv_perm.begin(), get(vertex_color, G), make_degree_map(G));

	for (size_type c = 0; c != inv_perm.size(); ++c)
		perm[index_map[inv_perm[c]]] = c;
	std::cout << "new bandwidth: "
		<< bandwidth(G,
			make_iterator_property_map(
				&perm[0], index_map, perm[0]))
		<< std::endl;

	{	//Avg distance check
		float avg_distance = 0;
		for (size_t k = 1; k < knots.size(); k++)
		{
			avg_distance += distance_matrix[k - 1][k];
			cout << "(" << k - 1 << "," << k << "):" << distance_matrix[k - 1][k] << " ";
		}
		avg_distance /= (knots.size() - 1);
		cout << endl << "avg_dist " << avg_distance << endl;
		avg_distance = 0;
		for (size_t k = 1; k < knots.size(); k++)
		{
			avg_distance += distance_matrix[index_map[inv_perm[k - 1]]][index_map[inv_perm[k]]];
			cout << "(" << k - 1 << "," << k << "):" << distance_matrix[index_map[inv_perm[k - 1]]][index_map[inv_perm[k]]] << " ";
		}
		avg_distance /= (knots.size() - 1);
		cout << endl << "new avg_dist " << avg_distance << endl;
	}

	/*cout << "Cuthill-McKee ordering:" << endl;
	cout << "  ";
	for (std::vector< Vertex >::const_iterator i = inv_perm.begin();
		i != inv_perm.end(); ++i)
			cout << index_map[*i] << " ";
	cout << endl;*/

	for (size_t k = 0; k < knots.size(); k++)
		delete[] distance_matrix[k];
	delete[] distance_matrix;

	{	//Rewrite to a new file
		inFile.seekg(0);
		while (line != "*NODE")
		{
			getline(inFile, line);
			outFile << line << endl;
		}
		getline(inFile, line);
		outFile << line << endl;

		{
			int ind;
			double x, y, z;
			getline(inFile, line);
			while (line != "$")
			{
				ind = stoi(line, &sz);
				outFile << std::setw(sz) << std::right << index_map[perm[ind - 1]] + 1;
				outFile << line.substr(sz) << endl;
				getline(inFile, line);
			}
		}
	}


	inFile.close();
	outFile.close();
}