#include <fstream>
#include <iostream>

#include "dt.h"

int main(int argc, char* argv[]) {
	if (argc < 2) {
		std::cerr << "Usage: " << argv[0]
			  << " <device_tree_file.dts>\n";
		return 1;
	}

	dt_format(argv[1], false);
}
