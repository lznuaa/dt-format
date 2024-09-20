#include <fstream>
#include <iostream>
#include <map>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

// Function to get substring from the last newline character
std::string getSubstringFromLastNewline(const std::string& input) {
	size_t lastNewlinePos = input.rfind('\n');

	if (lastNewlinePos != std::string::npos) {
		if (lastNewlinePos + 1 < input.length()) {
			return input.substr(lastNewlinePos + 1);
		} else {
			return "";
		}
	} else {
		return input;
	}
}

// Utility to trim whitespace
std::string trim(const std::string& str) {
	const char* whitespace = " \t\n\r";
	size_t start = str.find_first_not_of(whitespace);
	if (start == std::string::npos) return "";
	size_t end = str.find_last_not_of(whitespace);
	return str.substr(start, end - start + 1);
}

std::string removeComments(const std::string& code) {
	std::string result;
	bool inMultiLineComment = false;
	for (size_t i = 0; i < code.size(); ++i) {
		if (!inMultiLineComment) {
			if (i + 1 < code.size() && code[i] == '/' &&
			    code[i + 1] == '/') {
				// Skip single-line comment
				while (i < code.size() && code[i] != '\n') {
					++i;
				}
				result += '\n';	 // Keep the new line character
			} else if (i + 1 < code.size() && code[i] == '/' &&
				   code[i + 1] == '*') {
				// Enter multi-line comment
				inMultiLineComment = true;
				++i;  // Skip the '*'
			} else {
				// Copy the character if it's not a comment
				result += code[i];
			}
		} else {
			if (i + 1 < code.size() && code[i] == '*' &&
			    code[i + 1] == '/') {
				// Exit multi-line comment
				inMultiLineComment = false;
				++i;  // Skip the '/'
			}
			// Ignore characters inside multi-line comments
		}
	}
	return result;
}

// Helper function to extract the hex value if it exists
bool extractHexValue(const std::string& str, unsigned long& hexValue) {
	size_t pos = str.find_last_of('@');
	if (pos != std::string::npos && pos + 1 < str.size()) {
		std::string hexPart = str.substr(pos + 1);
		std::istringstream iss(hexPart);
		iss >> std::hex >> hexValue;
		if (!iss.fail()) {
			return true;
		}
	}
	return false;
}

std::string getAfterColon(const std::string& str) {
	size_t pos = str.find(':');
	if (pos != std::string::npos && pos + 1 < str.size()) {
		return trim(str.substr(pos + 1));
	}
	return str;  // If no colon, return the original string
}

// Custom comparator for the map
struct CustomCompare {
	bool operator()(const std::string& ai, const std::string& bi) const {
		std::string a = removeComments(ai);
		std::string b = removeComments(bi);
		unsigned long hexA, hexB;
		bool aHasHex = extractHexValue(a, hexA);
		bool bHasHex = extractHexValue(b, hexB);

		if (aHasHex && bHasHex) {
			return hexA < hexB;  // Compare by hex value if both
					     // have @hexvalue
		} else if (aHasHex) {
			return false;  // a comes before b if only a has
				       // @hexvalue
		} else if (bHasHex) {
			return true;  // b comes before a if only b has
				      // @hexvalue
		} else {
			return getAfterColon(a) < getAfterColon(b);
			// Otherwise, compare lexicographically
		}
	}
};

std::map<std::string, int> g_map;

struct CustomCompare_property : public CustomCompare {
	bool operator()(const std::string& ai, const std::string& bi) const {
		int x = g_map.size();
		int y = x;

		if (bi.empty()) return false;

		if (ai.empty()) return true;

		if (ai.find(',') != std::string::npos) x = g_map.size() + 1;
		if (bi.find(',') != std::string::npos) y = g_map.size() + 1;

		if (g_map.find(ai) != g_map.end()) x = g_map[ai];
		if (g_map.find(bi) != g_map.end()) y = g_map[bi];

		if (ai == "status") x = INT32_MAX;
		if (bi == "status") y = INT32_MAX;

		if (x != y) return x < y;

		return CustomCompare::operator()(ai, bi);
	}
};

class DeviceTreeNode {
       public:
	std::string name;
	std::map<std::string, std::string, CustomCompare_property> properties;
	std::map<std::string, DeviceTreeNode*, CustomCompare> children;

	DeviceTreeNode(const std::string& name) : name(name) {}

	// Add property key-value pair
	void add_property(const std::string& key, const std::string& value) {
		properties[key] = value;
	}

	// Add child node
	void add_child(DeviceTreeNode* child) { children[child->name] = child; }

	// Destructor to free children
	~DeviceTreeNode() {
		for (auto& child : children) {
			delete child.second;
		}
	}

	// Print the node and its children recursively
	void print_tree(int indent = -1) const {
		std::string indentation;
		if (indent >= 0) {
			indentation = std::string(indent, '\t');
			std::cout << "\n" << indentation << name << " {\n";
		}

		for (const auto& prop : properties) {
			std::cout << indentation << "\t" << prop.first;
			if (prop.second.length() > 0)
				std::cout << " = " << prop.second;
			std::cout << ";\n";
		}
		for (const auto& child : children) {
			child.second->print_tree(indent + 1);
		}

		if (indent >= 0) std::cout << indentation << "};\n";
	}
};

std::string GetNodePath(std::vector<DeviceTreeNode*> node_stack) {
	std::string str;
	if (node_stack.size() <= 1) return str;

	for (int i = 1; i < node_stack.size(); i++) {
		str += getSubstringFromLastNewline(
		    getAfterColon(removeComments(trim(node_stack[i]->name))));
		str += '/';
	}

	return str;
}

class DeviceTreeParser {
       public:
	DeviceTreeNode* root;
	std::string last_key_value;
	std::map<DeviceTreeNode*, std::string> last_node_value;

	std::map<std::string, DeviceTreeNode*>
	    label_map;	// To store labeled nodes

	DeviceTreeParser() { root = new DeviceTreeNode("root"); }

	~DeviceTreeParser() { delete root; }

	// Parse the device tree file
	void parse(const std::string& file_path, bool warning = false) {
		std::ifstream file(file_path);
		if (!file.is_open()) {
			std::cerr << "Error: Could not open file " << file_path
				  << "\n";
			return;
		}

		std::string content((std::istreambuf_iterator<char>(file)),
				    std::istreambuf_iterator<char>());
		file.close();

		// Remove comments and trim the content
		content = remove_comments(content);
		content = trim(content);

		DeviceTreeNode* current_node = root;
		std::vector<DeviceTreeNode*> node_stack = {root};

		std::string buffer;
		for (size_t i = 0; i < content.size(); ++i) {
			char c = content[i];
			if (c == '{') {
				// Start of a new node
				buffer = trim(buffer);
				DeviceTreeNode* new_node =
				    new DeviceTreeNode(buffer);
				current_node->add_child(new_node);
				node_stack.push_back(new_node);

				CustomCompare compare;

				if (compare(new_node->name,
					    last_node_value[current_node]) &&
				    warning)
					std::cout << "wrong order: "
						  << GetNodePath(node_stack)
						  << "\n";

				last_node_value[current_node] = new_node->name;

				current_node = new_node;
				buffer.clear();
				last_key_value = "";

			} else if (c == '}') {
				// End of current node
				node_stack.pop_back();
				current_node = node_stack.back();
			} else if (c == ';') {
				// End of a property definition
				buffer = trim(buffer);
				if (!buffer.empty()) {
					auto key_value =
					    parse_key_value(buffer);
					if (!key_value.first.empty()) {
						current_node->add_property(
						    key_value.first,
						    key_value.second);

						CustomCompare_property compare;
						if (compare(key_value.first,
							    last_key_value) &&
						    warning)
							std::cout
							    << "wrong order: "
							    << GetNodePath(
								   node_stack)
							    << key_value.first
							    << "\n";

						last_key_value =
						    key_value.first;
					}
				}
				buffer.clear();
			} else {
				buffer += c;
			}
		}
	}

	void print_tree() const { root->print_tree(); }

       private:
	// Utility to remove comments
	std::string remove_comments(const std::string& content) const {
		std::regex comment_regex(R"(\/\*.*?\*\/|\/\/.*$)");
		return std::regex_replace(content, comment_regex, "");
	}

	// Parse a key-value pair from a line like "key = value;"
	std::pair<std::string, std::string> parse_key_value(
	    const std::string& line) const {
		size_t equal_pos = line.find('=');
		if (equal_pos == std::string::npos) return {trim(line), ""};

		std::string key = trim(line.substr(0, equal_pos));
		std::string value = trim(line.substr(equal_pos + 1));
		return {key, value};
	}
};

const char* g_order[] = {
	"compatible",
	"reg",
	"reg-names",
	"ranges",
	"#interrupt-cells",
	"interrupt-controller",
	"interrupts",
	"interrupt-names",
	"#gpio-cells",
	"gpio-controller",
	"gpio-ranges",
	"#address-cells",
	"#size-cells",
	"clocks",
	"clock-names",
	"assigned-clocks",
	"assigned-clock-parents",
	"assigned-clock-rates",
	"dmas",
	"dma-names",
};

int dt_format(std::string filename, bool check) {
	if (g_map.size() == 0) {
		for (int i = 0; i < sizeof(g_order) / sizeof(g_order[0]); i++)
			g_map[g_order[i]] = i;
	}

	DeviceTreeParser parser;
	parser.parse(filename, check);

	if (!check) parser.print_tree();

	return 0;
}
