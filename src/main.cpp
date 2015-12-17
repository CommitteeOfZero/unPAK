// unPAK++
// 5pb PAK extractor
// Based on Ruby script by carstene1ns
// Currently only supports S;G Phenogram PAKs
//
// Copyright Benjamin Moir 2015
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <fstream>
#include <vector>
#include <string>

const char *MAGIC = "filemark";
const int OFFSET_ENTRIES = 56;
const int OFFSET_FILES = OFFSET_ENTRIES + 8;
const int ENTRY_SIZE_N = 32;
const int ENTRY_SIZE_O = 16;
const int ENTRY_SIZE_S = 16;
const int ENTRY_SIZE = ENTRY_SIZE_N + ENTRY_SIZE_O + ENTRY_SIZE_S;

struct entry_t
{
	const char *name;
	int offset, size;
};

int main(int argc, char **argv)
{
	std::string input, output("");

	for (int i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help"))
		{
			printf("usage: unPAK -i input.pak [parameters]\n\n");
			printf(" -h, --help\t\tPrints usage information and exits.\n");
			printf(" -v, --version\t\tPrints version and exits.\n");
			printf(" -i, --input file.pak\tSpecifies input file.\n");
			printf(" -o, --output path\tSpecifies output path.\n");
			return 0;
		}
		else if (!strcmp(argv[i], "-v") || !strcmp(argv[i], "--version"))
		{
			printf("unPAK v1.0\n");
			return 0;
		}
		else if (!strcmp(argv[i], "-i") || !strcmp(argv[i], "--input"))
		{
			input = argv[i + 1];
		}
		else if (!strcmp(argv[i], "-o") || !strcmp(argv[i], "--output"))
		{
			output = argv[i + 1]; // todo: actually use this
		}
	}

	// load file
	char *buf;
	std::vector<entry_t> files;
	std::ifstream ifs(input, std::ios::binary);

	// wrong type?
	int length = strlen(MAGIC);
	buf = new char[length + 1];
	ifs.read(buf, length);
	if (strcmp(buf, MAGIC))
	{
		printf("%s is not a PAK file!\n", input.c_str());
		return 0;
	}
	delete[] buf;

	// get entries
	ifs.seekg(OFFSET_ENTRIES);
	buf = new char[8];
	ifs.read(buf, 8);
	int entries = atoi(buf);
	delete[] buf;
	printf("Found %d files:\n", entries);

	ifs.seekg(OFFSET_FILES);
	printf("%-32s | %-16s | %-16s\n", "NAME", "OFFSET", "SIZE");
	printf("----------------------------------------------------------------------\n");
	for (int e = 0; e < entries; e++)
	{
		char *name = new char[ENTRY_SIZE_N];
		char *offset = new char[ENTRY_SIZE_O];
		char *size = new char[ENTRY_SIZE_S];
		ifs.read(name, ENTRY_SIZE_N);
		ifs.read(offset, ENTRY_SIZE_O);
		ifs.read(size, ENTRY_SIZE_S);
		printf("%-32s | %-16s | %-16s\n", name, offset, size);
		files.push_back({ name, atoi(offset), atoi(size) });
		delete[] offset;
		delete[] size;
	}
	printf("----------------------------------------------------------------------\n");

	// extract
	const int OFFSET_DATA = OFFSET_FILES + entries * ENTRY_SIZE;
	printf("\nNow extracting...\n");
	for (int i = 0; i < (int)files.size(); i++)
	{
		// progress
		printf("\r%3d/%d %3d%%", i, entries, i * 100 / entries);

		// write
		char *file = new char[files[i].size];
		ifs.seekg(OFFSET_DATA + files[i].offset);
		ifs.read(file, files[i].size);
		std::ofstream ofs(output + '/' + files[i].name, std::ios::binary);
		ofs << std::string(file, files[i].size);
		ofs.close();
		delete[] file;
	}
	printf("\r=== All done! ===");

	ifs.close();
	return 0;
}

